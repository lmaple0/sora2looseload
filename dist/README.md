# Trails in the Sky 2nd Chapter Loose File Loader

[中文说明](README.zh-CN.md)

Loads loose mod files before matching files in the game's PAC archives. It does
not modify PAC files or patch `sora_2nd.exe`.

## Installation

### Windows

1. Keep the official Steam `sora_2nd.exe`. Back up any existing
   `xinput1_4.dll` from another mod.
2. Put `xinput1_4.dll` into the game root directory, next to `sora_2nd.exe`.
3. Optionally put `sora2looseload.ini` in the same directory.
4. Add the mod's loose files using their exact relative paths, then fully
   restart the game.

### Linux / Steam Deck

Install the same files in the game root directory, then add this Steam launch
option:

```text
WINEDLLOVERRIDES="xinput1_4=n,b" %command%
```

Keep file and directory names exactly as supplied by the mod.

## Loose-file layout and languages

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
Simplified Chinese. Install only the files for the language you use. Missing
files continue loading from the original PAC archives.

## Logging

Edit `sora2looseload.ini` in the game root:

```ini
[Logging]
Enabled=1
```

`Enabled=0` is the default. `SORA2LOOSELOAD_LOG` overrides the INI when the
environment variable exists: `1` enables logging; any other value disables it.
The log is replaced on each launch.

## Removal and compatibility

Delete this project's DLL, INI, and log, then restore any DLL you backed up.
Keep the official executable and do not combine this DLL with an EXE that
embeds another loose-file loader.

Current static target: Steam Build `25386012`, `sora_2nd.exe` 1.3.2.0. The
current target still needs a fresh in-game smoke test.

Forked from [Hinkiii/sora1looseload](https://github.com/Hinkiii/sora1looseload).
Licensed under MIT.
