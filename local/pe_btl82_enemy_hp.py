#!/usr/bin/env python3
"""Find subtractive sh +0x0C that are not the known D278 DAMAGE sites."""
from __future__ import annotations

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800
TEXT_END = TADDR + 0x1EE000

KNOWN = {0x8001E940, 0x8001F704, 0x80020210, 0x8001F4B0, 0x80029418, 0x8002AE7C}


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, va - TADDR + HDR)[0]


def op(w: int) -> int:
    return w >> 26


def rs(w: int) -> int:
    return (w >> 21) & 31


def rt(w: int) -> int:
    return (w >> 16) & 31


def rd(w: int) -> int:
    return (w >> 11) & 31


def simm16(w: int) -> int:
    v = w & 0xFFFF
    return v - 0x10000 if v >= 0x8000 else v


def funct(w: int) -> int:
    return w & 0x3F


def main() -> None:
    blob = EXE.read_bytes()
    assert hashlib.sha1(blob).hexdigest() == SHA1
    text = blob[HDR : HDR + 0x1EE000]
    print("=== sh +0x0C preceded by subu of same rt ===")
    for i in range(4, len(text), 4):
        w = struct.unpack_from("<I", text, i)[0]
        if op(w) != 0x29 or simm16(w) != 0x0C:
            continue
        va = TADDR + i
        prev = struct.unpack_from("<I", text, i - 4)[0]
        prev2 = struct.unpack_from("<I", text, i - 8)[0]
        is_sub = (op(prev) == 0 and funct(prev) == 0x23 and rd(prev) == rt(w))
        is_sub2 = (op(prev2) == 0 and funct(prev2) == 0x23 and rd(prev2) == rt(w))
        is_addiu = (op(prev) == 9 and rt(prev) == rt(w) and simm16(prev) != 0)
        if is_sub or is_sub2 or is_addiu:
            mark = "KNOWN" if va in KNOWN else "NEW"
            print(f"  {va:08X}  sh ${rt(w)},0xc(${rs(w)})  prev={prev:08X} prev2={prev2:08X}  {mark}")

    print("\n=== lh/lhu +0x0C then blez (death readers) ===")
    for i in range(0, len(text) - 8, 4):
        w = struct.unpack_from("<I", text, i)[0]
        if op(w) not in (0x21, 0x25) or simm16(w) != 0x0C:
            continue
        w2 = struct.unpack_from("<I", text, i + 4)[0]
        w3 = struct.unpack_from("<I", text, i + 8)[0]
        if op(w2) == 6 or op(w3) == 6:
            va = TADDR + i
            print(f"  {va:08X}  load then blez")


if __name__ == "__main__":
    main()
