# Trails in the Sky 2nd Chapter Demo Loose File Loader

Experimental adaptation of [Hinkiii/sora1looseload](https://github.com/Hinkiii/sora1looseload)
for the Steam demo of *Trails in the Sky 2nd Chapter*.

Upstream commit: `04e898e369e3019d5aa2cb13a7209de39c643a4a`

License: MIT (see `LICENSE`)

## Why the original DLL is unsafe on the 2nd Demo

The original 20-byte `InitialFileCheck` signature occurs twice in the current
2nd Demo executable. The upstream scanner returns the first occurrence, but it
belongs to a different 499-byte function with a different register layout. The
compatible 913-byte file-check function is the second occurrence.

This adaptation:

- extends that signature through the ABI-defining register moves;
- wildcards only the RIP-relative security-cookie displacement;
- requires every hook signature to have exactly one match;
- aborts if a Detours operation fails;
- writes `sora2looseload.log` instead of overwriting `console.log`.

## Verified target (static analysis only)

Current Steam Demo executable checked on 2026-08-22:

```text
File:    sora_2nd.exe
SHA-256: DB7016EBDAB7F6AFBF45DCB9CC78FA2C5E9F4B62CB48AAC3487F2707498C0617
Build:   24831051
```

The extended `InitialFileCheck`, `DebugLogger`, and `LocaleHandler` signatures
each match exactly once. The executable imports `XINPUT1_4.dll` ordinals 2 and
3, which are exported by the proxy.

This is not runtime proof. Do not put the DLL in a game directory until the
build succeeds and an isolated loose-file test has a backup/removal plan.

## Static verification

Requires Python and `pefile`:

```powershell
py tools\verify_target.py "C:\Program Files (x86)\Steam\steamapps\common\Trails in the Sky 2nd Chapter Demo\sora_2nd.exe"
```

## Build

Open `sora1looseload.sln` with Visual Studio 2022 and build `Release|x64`.
The project output remains `xinput1_4.dll` because it is an XInput proxy.

## Intended loose-file layout

The hook checks the game directory using the internal asset path, for example:

```text
Trails in the Sky 2nd Chapter Demo\asset_en\dx11\image\example.dds
Trails in the Sky 2nd Chapter Demo\script\scena\example.dat
```

Locale-prefixed paths must be confirmed from `sora2looseload.log` during the
first runtime test. The loader expects decompressed loose files; LZ4 Frame is a
PAC storage detail and should not be retained in the loose DDS itself.

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
