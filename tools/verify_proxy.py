#!/usr/bin/env python3
"""Validate the architecture and public exports of a built XInput proxy."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys

import pefile


EXPECTED_EXPORTS = {
    "XInputEnable": 5,
    "XInputGetAudioDeviceIds": 10,
    "XInputGetBatteryInformation": 7,
    "XInputGetCapabilities": 4,
    "XInputGetKeystroke": 8,
    "XInputGetState": 2,
    "XInputSetState": 3,
}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("dll", type=Path)
    args = parser.parse_args()

    data = args.dll.read_bytes()
    pe = pefile.PE(data=data, fast_load=False)
    exports = {
        symbol.name.decode("ascii"): symbol.ordinal
        for symbol in pe.DIRECTORY_ENTRY_EXPORT.symbols
        if symbol.name
    }
    report = {
        "path": str(args.dll.resolve()),
        "sha256": hashlib.sha256(data).hexdigest().upper(),
        "machine": f"0x{pe.FILE_HEADER.Machine:X}",
        "exports": exports,
    }
    print(json.dumps(report, indent=2))

    if pe.FILE_HEADER.Machine != pefile.MACHINE_TYPE["IMAGE_FILE_MACHINE_AMD64"]:
        print("Proxy rejected: expected an AMD64 DLL.", file=sys.stderr)
        return 2
    if exports != EXPECTED_EXPORTS:
        print("Proxy rejected: XInput exports or ordinals changed.", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
