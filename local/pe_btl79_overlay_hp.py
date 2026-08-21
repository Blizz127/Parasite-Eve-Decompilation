#!/usr/bin/env python3
"""Full-EXE 4D4 / D244 / 17FDC table / overlay HP stores."""
from __future__ import annotations

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, va - TADDR + HDR)[0]


def op(w: int) -> int:
    return w >> 26


def rs(w: int) -> int:
    return (w >> 21) & 31


def rt(w: int) -> int:
    return (w >> 16) & 31


def imm16(w: int) -> int:
    return w & 0xFFFF


def simm16(w: int) -> int:
    v = w & 0xFFFF
    return v - 0x10000 if v >= 0x8000 else v


def main() -> None:
    blob = EXE.read_bytes()
    assert hashlib.sha1(blob).hexdigest() == SHA1
    print(f"exe size {len(blob):#x}")

    print("=== table slots for 17FDC / 17FF0 / 17FE4 ===")
    for i in range(0x200):
        t = load_u32(blob, 0x800910A0 + i * 4)
        if t in (0x80017FDC, 0x80017FF0, 0x80017FE4, 0x800192B8, 0x800192C8):
            print(f"  D_800910A0[{i:#x}] = {t:#x}")

    print("=== full-file words 0x8009D244 / 0x8009D278 ===")
    for i in range(0, len(blob) - 3, 4):
        w = struct.unpack_from("<I", blob, i)[0]
        if w in (0x8009D244, 0x8009D278, 0x8009D28C):
            print(f"  {w:#x} at file+{i:#x}")

    print("=== lui/store forming 0x8009D244 anywhere in file ===")
    n = 0
    for i in range(0, len(blob) - 8, 4):
        w = struct.unpack_from("<I", blob, i)[0]
        if op(w) != 0x0F:
            continue
        w2 = struct.unpack_from("<I", blob, i + 4)[0]
        if rs(w2) != rt(w):
            continue
        if op(w2) not in (0x28, 0x29, 0x2B, 9):
            continue
        base = (imm16(w) << 16) + simm16(w2)
        if base == 0x8009D244:
            va = TADDR + (i - HDR)
            print(f"  file+{i:#x} va~{va:#x} {w:08x} {w2:08x}")
            n += 1
    print(f"  count={n}")

    print("=== sb/sh/sw gp+0x4D4 in whole file as raw words ===")
    # gp-rel encodings we already have; also search imm 0x4D4 stores
    for i in range(0, len(blob) - 3, 4):
        w = struct.unpack_from("<I", blob, i)[0]
        if op(w) in (0x28, 0x29, 0x2B) and imm16(w) == 0x4D4:
            va = TADDR + (i - HDR)
            print(f"  {va:#x} op={op(w):#x} rs={rs(w)} rt={rt(w)}")

    print("=== type-6 0x64 / 0x89 sites ===")
    # done via existing bases in other script

    print("=== 0x64 table ===")
    print(f"  0x64 {load_u32(blob, 0x800910A0 + 0x64 * 4):#x}")
    print(f"  mode5 setter nearby table hunt done")


if __name__ == "__main__":
    main()
