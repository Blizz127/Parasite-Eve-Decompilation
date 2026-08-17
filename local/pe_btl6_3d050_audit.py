#!/usr/bin/env python3
"""PE-BTL6 research: func_8003D050 stores and callees.

Authority: Disc 1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
Does not invent EE=13 completion.
"""
from __future__ import annotations

import hashlib
import struct
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800
FN = 0x8003D050
END = 0x8003D834
REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]
STORE_OPS = {0x28: "sb", 0x29: "sh", 0x2B: "sw"}


def va2off(va: int) -> int:
    return va - LOAD + EXE_HDR


def word_at(exe: bytes, va: int) -> int:
    return struct.unpack_from("<I", exe, va2off(va))[0]


def simm16(imm: int) -> int:
    return imm - 0x10000 if imm >= 0x8000 else imm


def jal_target(w: int) -> int:
    return ((w & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    exe = (Path(__file__).resolve().parents[1] / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    print(f"func_8003D050 {(END-FN)//4} words")
    print("=== first 40 words ===")
    for va in range(FN, FN + 160, 4):
        w = word_at(exe, va)
        op = w >> 26
        rs = (w >> 21) & 31
        rt = (w >> 16) & 31
        rd = (w >> 11) & 31
        fn = w & 63
        imm = w & 0xFFFF
        simm = simm16(imm)
        if op == 3:
            print(f"{va:08X}  {w:08X}  jal {jal_target(w):#010x}")
        elif op in STORE_OPS:
            print(f"{va:08X}  {w:08X}  {STORE_OPS[op]} ${REGS[rt]}, {simm:#x}(${REGS[rs]})")
        elif op == 0x23:
            print(f"{va:08X}  {w:08X}  lw ${REGS[rt]}, {simm:#x}(${REGS[rs]})")
        elif op == 0x0F:
            print(f"{va:08X}  {w:08X}  lui ${REGS[rt]}, {imm:#x}")
        elif op == 0x09:
            print(f"{va:08X}  {w:08X}  addiu ${REGS[rt]}, ${REGS[rs]}, {simm}")
        elif op == 0 and fn == 0x21:
            print(f"{va:08X}  {w:08X}  addu ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}")
        else:
            print(f"{va:08X}  {w:08X}  op={op:#x} fn={fn:#x}")

    print("\n=== all jals ===")
    for va in range(FN, END, 4):
        w = word_at(exe, va)
        if w >> 26 == 3:
            print(f"  {va:#010x} -> {jal_target(w):#010x}  delay {word_at(exe, va+4):08X}")

    print("\n=== stores through $s0 (a0 overlay+0x14) ===")
    for va in range(FN, END, 4):
        w = word_at(exe, va)
        op = w >> 26
        rs = (w >> 21) & 31
        rt = (w >> 16) & 31
        simm = simm16(w & 0xFFFF)
        if op in STORE_OPS and rs == 16:
            print(f"  {va:08X}  {STORE_OPS[op]} ${REGS[rt]}, {simm:#x}($s0)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
