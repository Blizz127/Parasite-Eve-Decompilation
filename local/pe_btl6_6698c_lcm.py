#!/usr/bin/env python3
"""Dump 6698C tail ctc2 LCM sources."""
from __future__ import annotations

import hashlib
import struct
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800
REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def word_at(exe: bytes, va: int) -> int:
    return struct.unpack_from("<I", exe, va - LOAD + EXE_HDR)[0]


def main() -> int:
    exe = (Path(__file__).resolve().parents[1] / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    for va in range(0x80066B00, 0x80066B60, 4):
        w = word_at(exe, va)
        op = w >> 26
        rs = (w >> 21) & 31
        rt = (w >> 16) & 31
        rd = (w >> 11) & 31
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm
        if op == 0x23:
            print(f"{va:08X}  {w:08X}  lw ${REGS[rt]}, {simm:#x}(${REGS[rs]})")
        elif op == 0x12 and rs == 6:
            print(f"{va:08X}  {w:08X}  ctc2 ${REGS[rt]}, rd={rd}")
        elif op == 0x12 and rs == 4:
            print(f"{va:08X}  {w:08X}  mtc2 ${REGS[rt]}, rd={rd}")
        elif op == 0x0F:
            print(f"{va:08X}  {w:08X}  lui ${REGS[rt]}, {imm:#x}")
        elif op == 9:
            print(f"{va:08X}  {w:08X}  addiu ${REGS[rt]}, ${REGS[rs]}, {simm}")
        else:
            print(f"{va:08X}  {w:08X}  op={op:#x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
