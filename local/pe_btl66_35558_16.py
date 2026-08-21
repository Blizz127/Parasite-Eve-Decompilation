#!/usr/bin/env python3
"""Find +0x16 writers and 6BECC jals in 35558 / 35E04 / 6B4F8."""
from __future__ import annotations

import pathlib
import struct

EXE = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe")
TADDR = 0x80010000
HDR = 0x800


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, va - TADDR + HDR)[0]


def jal_target(word: int) -> int | None:
    if (word >> 26) != 3:
        return None
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> None:
    blob = EXE.read_bytes()
    ranges = [
        ("35558", 0x80035558, 0x80035C84),
        ("35E04", 0x80035E04, 0x80035F50),
        ("35C84", 0x80035C84, 0x80035E04),
        ("6B4F8", 0x8006B4F8, 0x8006BD68),
        ("6BECC", 0x8006BECC, 0x8006C1CC),
    ]
    for name, start, end in ranges:
        print(f"\n=== {name} {start:#x}..{end:#x} ===")
        jals = []
        sh16 = []
        for va in range(start, end, 4):
            w = load_u32(blob, va)
            jt = jal_target(w)
            if jt:
                jals.append((va, jt))
            # sh rt, 0x16(rs) = 0xA4 ?? 0016
            if (w & 0xFC00FFFF) == 0xA4000016:
                sh16.append((va, w))
            if (w & 0xFC00FFFF) == 0xA4000012:
                print(f"  {va:#010x} sh +0x12 {w:08x}")
        print("  sh +0x16", [f"{va:#x}" for va, _ in sh16])
        print("  jals:")
        for va, jt in jals:
            print(f"    {va:#010x} -> {jt:#010x}")


if __name__ == "__main__":
    main()
