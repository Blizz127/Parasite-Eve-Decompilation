#!/usr/bin/env python3
"""Dump 6C5BC +0xEE jump table, state 11, 3F074 poll, pre-29810 ori 2."""
from __future__ import annotations

import hashlib
import struct
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
HDR = 0x800


def w(exe: bytes, va: int) -> int:
    return struct.unpack_from("<I", exe, va - LOAD + HDR)[0]


def jal_tgt(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> None:
    exe = (Path(__file__).resolve().parents[1] / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1

    print("=== JT 0x800113F0 +0xEE 0..13 ===")
    for i in range(14):
        addr = 0x800113F0 + i * 4
        tgt = w(exe, addr)
        print(f"  EE={i:2d}  {addr:08X}  -> {tgt:08X}")

    print("\n=== 6C1CC JT 0x800113D0 (v1=byte-32, 8 states) ===")
    for i in range(8):
        addr = 0x800113D0 + i * 4
        print(f"  st={i}  {addr:08X}  -> {w(exe, addr):08X}")

    print("\n=== 297C0..29820 (pre-29810 ori 2?) ===")
    for va in range(0x800297C0, 0x80029830, 4):
        word = w(exe, va)
        extra = ""
        if word >> 26 == 3:
            extra = f"  jal {jal_tgt(word):#010x}"
        print(f"  {va:08X}  {word:08X}{extra}")

    print("\n=== 3F074 jals ===")
    for va in range(0x8003F074, 0x8003F2F8, 4):
        word = w(exe, va)
        if word >> 26 == 3:
            print(f"  {va:08X}  jal {jal_tgt(word):#010x}  delay {w(exe, va+4):08X}")

    print("\n=== 3F1F0..3F240 (6BECC / 6C4C4 / 6C5BC poll) ===")
    for va in range(0x8003F1F0, 0x8003F248, 4):
        word = w(exe, va)
        extra = f"  jal {jal_tgt(word):#010x}" if word >> 26 == 3 else ""
        print(f"  {va:08X}  {word:08X}{extra}")

    print("\n=== state-11 region 0x8006C9A0..0x8006CA20 ===")
    for va in range(0x8006C9A0, 0x8006CA40, 4):
        word = w(exe, va)
        extra = f"  jal {jal_tgt(word):#010x}" if word >> 26 == 3 else ""
        print(f"  {va:08X}  {word:08X}{extra}")

    print("\n=== 144FC 0x3A D1A0 ori 2 ===")
    for va in range(0x800145F8, 0x80014630, 4):
        print(f"  {va:08X}  {w(exe, va):08X}")

    print("\n=== 6C5BC first 8 + last 8 words ===")
    for va in list(range(0x8006C5BC, 0x8006C5BC + 32, 4)) + list(range(0x8006CC48, 0x8006CC68, 4)):
        print(f"  {va:08X}  {w(exe, va):08X}")

    print("\n=== 6CC68 prologue (next fn) ===")
    for va in range(0x8006CC68, 0x8006CC68 + 16, 4):
        print(f"  {va:08X}  {w(exe, va):08X}")

    # 35558 overlay bit 0x200 skip vs 35B24
    print("\n=== 35558 0x200 skip target 35C04 vs 35B24 ===")
    print(f"  355C4 andi  {w(exe, 0x800355C4):08X}")
    print(f"  355C8 bne   {w(exe, 0x800355C8):08X}")
    print(f"  35C04 word  {w(exe, 0x80035C04):08X}")


if __name__ == "__main__":
    main()
