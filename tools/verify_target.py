#!/usr/bin/env python3
"""Read-only signature and XInput import check for sora_2nd.exe."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys

import pefile


PATTERNS = {
    "initial_file_check": (
        bytes.fromhex(
            "48 89 5C 24 20 55 56 57 41 56 41 57 48 8D AC 24 "
            "90 FC FF FF 48 81 EC 70 04 00 00 48 8B 05 00 00 00 00 "
            "48 33 C4 48 89 85 60 03 00 00 41 8B F1 45 8B F8 48 8B "
            "DA 48 8B F9 44 89 49 50"
        ),
        "x" * 30 + "?" * 4 + "x" * 26,
    ),
    "debug_logger": (
        bytes.fromhex("83 F9 02 0F 8C 82 00 00 00 4C 89 4C 24 20 53 57"),
        "x" * 16,
    ),
    "locale_handler": (
        bytes.fromhex(
            "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 A8 FE FF FF"
        ),
        "x" * 21,
    ),
}


def matches(data: bytes, pattern: bytes, mask: str) -> list[int]:
    result = []
    for offset in range(len(data) - len(pattern) + 1):
        if all(mark == "?" or data[offset + index] == pattern[index]
               for index, mark in enumerate(mask)):
            result.append(offset)
    return result


def file_offset_to_rva(pe: pefile.PE, offset: int) -> int:
    for section in pe.sections:
        start = section.PointerToRawData
        if start <= offset < start + section.SizeOfRawData:
            return section.VirtualAddress + offset - start
    raise ValueError(f"file offset 0x{offset:X} is not inside a section")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("exe", type=Path)
    args = parser.parse_args()

    data = args.exe.read_bytes()
    pe = pefile.PE(data=data, fast_load=False)
    found = {}
    for name, (pattern, mask) in PATTERNS.items():
        offsets = matches(data, pattern, mask)
        found[name] = [
            {"file_offset": f"0x{offset:X}", "rva": f"0x{file_offset_to_rva(pe, offset):X}"}
            for offset in offsets
        ]

    xinput = []
    for descriptor in pe.DIRECTORY_ENTRY_IMPORT:
        if descriptor.dll.lower() == b"xinput1_4.dll":
            xinput.extend(item.ordinal for item in descriptor.imports)

    report = {
        "path": str(args.exe.resolve()),
        "sha256": hashlib.sha256(data).hexdigest().upper(),
        "machine": f"0x{pe.FILE_HEADER.Machine:X}",
        "patterns": found,
        "xinput1_4_ordinals": sorted(xinput),
    }
    print(json.dumps(report, indent=2))

    valid = all(len(items) == 1 for items in found.values()) and sorted(xinput) == [2, 3]
    if not valid:
        print("Target rejected: signatures are not unique or XInput imports changed.",
              file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
