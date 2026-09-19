# Trails in the Sky 2nd Chapter Loose File Loader

[中文说明](README.zh-CN.md)

Loads loose mod files before matching files in the game's PAC archives. It does
not modify PAC files or patch `sora_2nd.exe`.

## Installation

### Windows

1. Download the latest release.
2. Keep the official Steam `sora_2nd.exe`. Back up any existing
   `xinput1_4.dll` from another mod.
3. Put `xinput1_4.dll` into the game root directory, next to `sora_2nd.exe`.
4. Optionally put `sora2looseload.ini` in the same directory.
5. Add the mod's loose files using their exact relative paths, then fully
   restart the game.

### Linux / Steam Deck

Install the same files in the game root directory, then add this Steam launch
option:

```text
WINEDLLOVERRIDES="xinput1_4=n,b" %command%
```

Keep file and directory names exactly as supplied by the mod.

## Loose-file layout and languages

Common loose-file directories are:

```text
Trails in the Sky 2nd Chapter/
├── voice/
├── table/       # Japanese
├── table_en/    # English
├── table_sc/    # Simplified Chinese
├── script/      # Japanese
├── script_en/   # English
├── script_sc/   # Simplified Chinese
├── sora_2nd.exe
└── xinput1_4.dll
```

Directories without a suffix are Japanese, `_en` is English, and `_sc` is
Simplified Chinese. Install the files for the language you use; a mod does not
need to provide every language directory. Only matching loose files are
substituted. A missing `_en` or `_sc` loose file does not fall back to the
unsuffixed Japanese directory; the game continues loading its matching language
resource from the original PAC archives.

## Logging

Edit `sora2looseload.ini` in the game root:

```ini
[Logging]
Enabled=1
```

`Enabled=0` is the default and creates no log. When present, the environment
variable `SORA2LOOSELOAD_LOG` overrides the INI: `1` enables logging; any other
value disables it. Logs are written to `sora2looseload.log` and replaced on
each launch.

## Removal

Delete this project's `xinput1_4.dll`, `sora2looseload.ini`, and log file, then
restore the DLL you backed up, if any. Mod resources are never deleted by the
loader.

## Compatibility

- Current static target: Steam Build `25386012`, `sora_2nd.exe` 1.3.2.0.
- Keep the official executable. Do not combine this DLL with an EXE that embeds
  another loose-file loader.
- Build and static checks pass; the current 1.3.2 target still needs a fresh
  in-game smoke test.

## Source and build

Forked from [Hinkiii/sora1looseload](https://github.com/Hinkiii/sora1looseload)
at `04e898e369e3019d5aa2cb13a7209de39c643a4a`. Microsoft Detours is pinned to
`adb07604aa56508448b95bf037c2a6d0d3b6831a`. Licensed under MIT.

Open `sora1looseload.sln` in Visual Studio 2022 and build `Release|x64`.

```powershell
py tools\verify_target.py "C:\Program Files (x86)\Steam\steamapps\common\Trails in the Sky 2nd Chapter\sora_2nd.exe"
py tools\verify_proxy.py dist\xinput1_4.dll
```
