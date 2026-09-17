# Trails in the Sky 2nd Chapter Loose File Loader

Open-source adaptation of [Hinkiii/sora1looseload](https://github.com/Hinkiii/sora1looseload)
for the Steam demo and release build of *Trails in the Sky 2nd Chapter*.

This repository is a fork of Hinkiii's project. The 2nd Chapter adaptation
started from upstream commit `04e898e369e3019d5aa2cb13a7209de39c643a4a`.

License: MIT (see `LICENSE`)

## Why the original DLL is unsafe on the 2nd builds

The original 20-byte `InitialFileCheck` signature occurs twice in the current
2nd Demo executable. The upstream scanner returns the first occurrence, but it
belongs to a different 499-byte function with a different register layout. The
compatible 913-byte file-check function is the second occurrence.

This adaptation:

- extends that signature through the ABI-defining register moves;
- wildcards only the RIP-relative security-cookie displacement;
- requires every hook signature to have exactly one match;
- aborts if a Detours operation fails;
- disables file logging by default instead of overwriting `console.log`;
- writes `sora2looseload.log` only when `SORA2LOOSELOAD_LOG=1` is set;
- scans and hooks the game's `DebugLogger` only while diagnostic logging is enabled;
- keeps loading functional when the optional game logger signature changes;
- initializes the real system XInput DLL once, even under concurrent calls;
- checks localized loose paths only when an actual disk override exists, leaving
  the game's original fallback behavior untouched otherwise.

## EXE compatibility boundary

Keep the official Steam `sora_2nd.exe`. This project does not patch, replace, or
redistribute the game executable.

Do not combine this DLL with a pre-patched executable that embeds another loose
file loader. Both loaders hook `InitialFileCheck`; a pre-patched entry point is
intentionally rejected by `tools/verify_target.py`. Voice mods only need their
matching `voice`, `table`/`table_sc`, and `script`/`script_sc` resources. The DLL
provides the loose-file override layer.

## Verified targets

Steam release executable checked on 2026-09-17:

```text
File:            sora_2nd.exe
File version:    1.3.1.0
SHA-256:         485EFF96B37B11860F39C2D1A7390D6C91F4046E79519A85076E1CA4CDB6B616
Steam Build ID:  25340742
Validation:      static checks; prior build passed a user-confirmed runtime test
```

Historical Steam Demo executable checked on 2026-08-22:

```text
File:    sora_2nd.exe
SHA-256: DB7016EBDAB7F6AFBF45DCB9CC78FA2C5E9F4B62CB48AAC3487F2707498C0617
Build:   24831051
```

On the release build, the signature targeting the 913-byte `InitialFileCheck` and
`LocaleHandler` signatures each match exactly once. The optional `DebugLogger`
signature also matches exactly once. The executable imports `XINPUT1_4.dll`
ordinals 2 and 3, which are exported by the proxy.

The current source and artifact pass clean build, target-signature, proxy-export,
and isolated logging checks. A prior build passed a user-confirmed release-game
runtime test; the current artifact still needs a fresh in-game smoke test.

This is not runtime proof. Do not put the DLL in a game directory until the
build succeeds and an isolated loose-file test has a backup/removal plan.

## Static verification

Requires Python and `pefile`:

```powershell
py tools\verify_target.py "C:\Program Files (x86)\Steam\steamapps\common\Trails in the Sky 2nd Chapter\sora_2nd.exe"
py tools\verify_target.py --with-log-hook "C:\Program Files (x86)\Steam\steamapps\common\Trails in the Sky 2nd Chapter\sora_2nd.exe"
py tools\verify_proxy.py dist\xinput1_4.dll
py -m unittest discover -s tools -p "test_*.py"
```

## Build

Open `sora1looseload.sln` with Visual Studio 2022 and build `Release|x64`.
The project output remains `xinput1_4.dll` because it is an XInput proxy.

## Install and remove

1. Verify that Steam has restored the official `sora_2nd.exe`.
2. Back up any existing game-root `xinput1_4.dll` from another mod.
3. Copy `dist\xinput1_4.dll` next to `sora_2nd.exe`.
4. Place loose resources under their exact game-relative paths.
5. Fully restart the game before testing.

To remove the loader, delete this `xinput1_4.dll` and restore the previous DLL,
if one existed. Loose resources are not modified or deleted by the loader.

## Intended loose-file layout

The hook checks the game directory using the internal asset path, for example:

```text
Trails in the Sky 2nd Chapter\table_sc\t_item.tbl
Trails in the Sky 2nd Chapter\script_sc\scena\example.dat
```

To diagnose locale-prefixed paths, launch the game with
`SORA2LOOSELOAD_LOG=1` and inspect `sora2looseload.log`. Logging is otherwise
disabled and the file is not created. The loader passes loose-file bytes through
unchanged; use the exact representation expected by the game for that resource.

## Texture compatibility findings

Do not replace the current LZ4 loose textures with directly decompressed DDS files.
That experiment caused the `c45` body materials to disappear while the head and hair
remained visible. All eight files were restored byte-for-byte from backup.

`44 44 53 20` confirms only that a file is structurally a DDS; it does not prove that
the 2nd Demo resource path, texture format, alpha semantics, and model material flags
are compatible. Keep the original `04 22 4D 18` LZ4 Frames until those layers are
validated together.

```text
chr5000_c00.mdl -> estell_1st_a/q/n.dds + estell_1st_body_a.dds
chr5000_c45.mdl -> estell_dtb_a/q/n.dds + estell_body_a.dds
chr5000_c10.mdl -> neither custom texture set
```

Offline inspection shows that `estell_1st_*` decode to BC7 with official 2nd Demo
dimensions and mip counts, while `estell_dtb_*` decode to DXT5. This is structural
evidence only, not runtime compatibility proof.
