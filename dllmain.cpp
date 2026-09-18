#include "pch.h"
#include "detours.h"
#include <windows.h>
#include <cstdio>
#include <psapi.h>
#include <atomic>
#include <share.h>
#include <cstdarg>

char g_gameDir[MAX_PATH] = {};
FILE* gLogFile = nullptr;
std::atomic<__int64> g_localeMgr{ 0 };

bool IsLoggingEnabled() {
    char enabled[8] = {};
    SetLastError(ERROR_SUCCESS);
    const DWORD enabledLength = GetEnvironmentVariableA(
        "SORA2LOOSELOAD_LOG", enabled, sizeof(enabled));
    if (enabledLength != 0) {
        return enabledLength < sizeof(enabled) && strcmp(enabled, "1") == 0;
    }

    // An explicitly empty environment variable disables logging and overrides
    // the INI, just like any other value except the exact string "1".
    if (GetLastError() != ERROR_ENVVAR_NOT_FOUND)
        return false;

    char configPath[MAX_PATH] = {};
    if (!g_gameDir[0] ||
        sprintf_s(configPath, "%ssora2looseload.ini", g_gameDir) < 0) {
        return false;
    }

    return GetPrivateProfileIntA("Logging", "Enabled", 0, configPath) == 1;
}

bool InitLogFile() {
    if (!IsLoggingEnabled())
        return false;

    char logPath[MAX_PATH] = {};
    if (sprintf_s(logPath, "%ssora2looseload.log", g_gameDir) < 0)
        return false;

    gLogFile = _fsopen(logPath, "w", _SH_DENYNO);
    if (gLogFile) {
        fprintf(gLogFile, "---- Log Started ----\n");
        fflush(gLogFile);
    }
    return gLogFile != nullptr;
}

void Log(const char* fmt, ...) {
    if (!gLogFile)
        return;

    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    buf[sizeof(buf) - 1] = '\0';
    va_end(args);

    fprintf(gLogFile, "%s\n", buf);
    fflush(gLogFile);
}

bool FileExistsOnDisk(const char* path) {
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

uintptr_t FindUniquePattern(uintptr_t base, DWORD size, const char* pattern, const char* mask, size_t* matchCount) {
    const size_t patternLength = strlen(mask);
    uintptr_t result = 0;
    size_t count = 0;

    if (patternLength != 0 && patternLength <= size) {
        size_t anchor = 0;
        while (anchor < patternLength && mask[anchor] == '?')
            ++anchor;

        for (uintptr_t i = 0; i <= size - patternLength; i++) {
            if (anchor < patternLength &&
                *(const unsigned char*)(base + i + anchor) != (unsigned char)pattern[anchor]) {
                continue;
            }

            bool found = true;
            for (uintptr_t j = 0; j < patternLength; j++) {
                if (mask[j] != '?' && pattern[j] != *(char*)(base + i + j)) {
                    found = false;
                    break;
                }
            }
            if (found) {
                result = base + i;
                ++count;
            }
        }
    }

    if (matchCount)
        *matchCount = count;
    return count == 1 ? result : 0;
}

bool BuildLoosePath(const char* relativePath, char* fullPath, size_t fullPathSize) {
    if (!relativePath || !relativePath[0] || !g_gameDir[0] ||
        relativePath[0] == '\\' || relativePath[0] == '/' ||
        (relativePath[1] == ':' &&
            ((relativePath[0] >= 'A' && relativePath[0] <= 'Z') ||
             (relativePath[0] >= 'a' && relativePath[0] <= 'z')))) {
        return false;
    }

    if (sprintf_s(fullPath, fullPathSize, "%s%s", g_gameDir, relativePath) < 0)
        return false;

    for (char* p = fullPath; *p; ++p) {
        if (*p == '/')
            *p = '\\';
    }
    return true;
}

bool TryLooseFile(const char* relativePath, char* fullPath, size_t fullPathSize, const char* kind) {
    if (!BuildLoosePath(relativePath, fullPath, fullPathSize))
        return false;

    Log("[MOD] Checking %s loose file: '%s'", kind, fullPath);
    if (!FileExistsOnDisk(fullPath))
        return false;

    Log("[MOD] Found %s loose file: '%s'", kind, fullPath);
    return true;
}

typedef __int64(__fastcall* InitialFileCheck_t)(__int64 a1, const char* a2, unsigned int a3, unsigned int a4, unsigned __int16 a5);
typedef void(__fastcall* DebugLogger_t)(int a1, __int64 a2, __int64 a3, const char* a4, ...);
typedef void(__fastcall* LocaleHandler_t)(__int64 mgr, char* dest, unsigned __int16* locale, char* src, int zero);

InitialFileCheck_t oInitialFileCheck = nullptr;
DebugLogger_t oDebugLogger = nullptr;
LocaleHandler_t oLocaleHandler = nullptr;

__int64 __fastcall InitialFileCheck(__int64 a1, const char* a2, unsigned int a3, unsigned int a4, unsigned __int16 a5)
{
    if (!a2)
        return oInitialFileCheck(a1, a2, a3, a4, a5);

    size_t len = strlen(a2);
    if (len >= 4 && _stricmp(a2 + len - 4, ".pac") == 0)
        return oInitialFileCheck(a1, a2, a3, a4, a5);

    static thread_local char safeFullPath[MAX_PATH];
    static thread_local char localizedPath[MAX_PATH];
    const char* finalPath = a2;

    const __int64 localeMgr = g_localeMgr.load(std::memory_order_acquire);
    if (oLocaleHandler && localeMgr)
    {
        localizedPath[0] = '\0';
        oLocaleHandler(localeMgr, localizedPath, (unsigned __int16*)&a5, (char*)a2, 0);

        if (localizedPath[0] && _stricmp(localizedPath, a2) != 0 &&
            TryLooseFile(localizedPath, safeFullPath, sizeof(safeFullPath), "localized")) {
            finalPath = safeFullPath;
        } else if (!localizedPath[0]) {
            Log("[DEBUG] LocaleHandler returned empty path for '%s'", a2);
        }
    }

    if (finalPath == a2 && TryLooseFile(a2, safeFullPath, sizeof(safeFullPath), "standard")) {
        finalPath = safeFullPath;
    }

    Log("[MOD] Passing '%s' to original InitialFileCheck", finalPath);
    return oInitialFileCheck(a1, finalPath, a3, a4, a5);
}



void __fastcall DebugLogger(int a1, __int64 a2, __int64 a3, const char* a4, ...) {
    if (!gLogFile || !a4) return;
    char buffer[4096] = { 0 };
    va_list va;
    va_start(va, a4);
    vsnprintf(buffer, sizeof(buffer), a4, va);
    va_end(va);
    if (gLogFile) {
        fprintf(gLogFile, "[LOG] %s\n", buffer);
        fflush(gLogFile);
    }
}

void __fastcall hkLocaleHandler(__int64 mgr, char* dest, unsigned __int16* locale, char* src, int zero) {
    g_localeMgr.store(mgr, std::memory_order_release);
    oLocaleHandler(mgr, dest, locale, src, zero);
}

DWORD WINAPI InitializeHooks(LPVOID) {
    const bool loggingEnabled = InitLogFile();

    uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
    if (!base) {
        Log("Failed to get module handle.");
        return 0;
    }

    MODULEINFO moduleInfo = {};
    if (!GetModuleInformation(GetCurrentProcess(), (HMODULE)base, &moduleInfo, sizeof(moduleInfo))) {
        Log("Failed to query the main module.");
        return 0;
    }
    DWORD moduleSize = moduleInfo.SizeOfImage;

            // The old 20-byte prologue occurs twice in the 2nd Chapter Demo.
            // Extend through the ABI-defining register moves, while wildcarding
            // only the RIP-relative security-cookie displacement.
            const char* initialFileCheckSig_GLB =
                "\x48\x89\x5C\x24\x20\x55\x56\x57\x41\x56\x41\x57\x48\x8D\xAC\x24"
                "\x90\xFC\xFF\xFF\x48\x81\xEC\x70\x04\x00\x00\x48\x8B\x05\x00\x00"
                "\x00\x00\x48\x33\xC4\x48\x89\x85\x60\x03\x00\x00\x41\x8B\xF1\x45"
                "\x8B\xF8\x48\x8B\xDA\x48\x8B\xF9\x44\x89\x49\x50";
            const char* initialFileCheckMask_GLB =
                "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxxxxxxxxxx";
            const char* initialFileCheckSig_CLE = "\x48\x89\x5C\x24\x20\x55\x56\x57\x41\x56\x41\x57\x48\x81\xEC\x60\x02\x00\x00";
            const char* initialFileCheckMask_CLE = "xxxxxxxxxxxxxxxxxxx";
            const char* debugLoggerSig = "\x83\xF9\x02\x0F\x8C\x82\x00\x00\x00\x4C\x89\x4C\x24\x20\x53\x57";
            const char* debugLoggerMask = "xxxxxxxxxxxxxxxx";
            const char* localeHandlerSig = "\x40\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\xAC\x24\xA8\xFE\xFF\xFF";
            const char* localeHandlerMask = "xxxxxxxxxxxxxxxxxxxxx";

    Log("Scanning for signatures...");
    size_t initialMatches = 0;
    size_t debugMatches = 0;
    size_t localeMatches = 0;
    uintptr_t debugLoggerAddr = 0;
    if (loggingEnabled)
        debugLoggerAddr = FindUniquePattern(base, moduleSize, debugLoggerSig, debugLoggerMask, &debugMatches);
    uintptr_t initialFileCheckAddr = FindUniquePattern(base, moduleSize, initialFileCheckSig_GLB, initialFileCheckMask_GLB, &initialMatches);
    if (!initialFileCheckAddr && initialMatches == 0)
        initialFileCheckAddr = FindUniquePattern(base, moduleSize, initialFileCheckSig_CLE, initialFileCheckMask_CLE, &initialMatches);
    uintptr_t localeHandlerAddr = FindUniquePattern(base, moduleSize, localeHandlerSig, localeHandlerMask, &localeMatches);

    if (!initialFileCheckAddr || !localeHandlerAddr) {
        Log("Aborting due to missing core signatures.");
        Log("InitialFileCheck: %p (%zu matches), DebugLogger: %p (%zu matches), LocaleHandler: %p (%zu matches)",
            (void*)initialFileCheckAddr, initialMatches, (void*)debugLoggerAddr, debugMatches,
            (void*)localeHandlerAddr, localeMatches);
        return 0;
    }
    if (loggingEnabled && !debugLoggerAddr)
        Log("DebugLogger signature unavailable; continuing with loader diagnostics only.");

    oInitialFileCheck = (InitialFileCheck_t)initialFileCheckAddr;
    oLocaleHandler = (LocaleHandler_t)localeHandlerAddr;
    if (debugLoggerAddr)
        oDebugLogger = (DebugLogger_t)debugLoggerAddr;

    LONG result = DetourTransactionBegin();
    if (result == NO_ERROR)
        result = DetourUpdateThread(GetCurrentThread());
    if (result == NO_ERROR)
        result = DetourAttach((void**)&oInitialFileCheck, InitialFileCheck);
    if (result == NO_ERROR && debugLoggerAddr)
        result = DetourAttach((void**)&oDebugLogger, DebugLogger);
    if (result == NO_ERROR)
        result = DetourAttach((void**)&oLocaleHandler, hkLocaleHandler);
    if (result != NO_ERROR) {
        DetourTransactionAbort();
        Log("Failed to attach detours: %ld", result);
        return 0;
    }
    result = DetourTransactionCommit();
    if (result != NO_ERROR) {
        Log("Failed to commit detours: %ld", result);
        return 0;
    }

    Log("All detours attached successfully.");
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        const DWORD pathLength = GetModuleFileNameA(NULL, g_gameDir, MAX_PATH);
        if (pathLength == 0 || pathLength >= MAX_PATH)
            g_gameDir[0] = '\0';

        char* lastSlash = strrchr(g_gameDir, '\\');
        if (lastSlash)
            *(lastSlash + 1) = '\0';
        else
            g_gameDir[0] = '\0';

        HANDLE thread = CreateThread(nullptr, 0, InitializeHooks, nullptr, 0, nullptr);
        if (thread)
            CloseHandle(thread);
    }
    return TRUE;
}
