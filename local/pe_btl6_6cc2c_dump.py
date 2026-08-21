#!/usr/bin/env python3
"""Dump 6C5BC from jal 3D050 through exclusive end."""
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


def simm16(imm: int) -> int:
    return imm - 0x10000 if imm >= 0x8000 else imm


def decode(w: int) -> str:
    op = w >> 26
    rs = (w >> 21) & 31
    rt = (w >> 16) & 31
    rd = (w >> 11) & 31
    fn = w & 63
    imm = w & 0xFFFF
    simm = simm16(imm)
    if op == 0:
        if fn == 8:
            return f"jr ${REGS[rs]}"
        if fn == 0x21:
            return f"addu ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if w == 0:
            return "nop"
        return f"special fn={fn:#x}"
    if op == 3:
        return f"jal {((w & 0x03FFFFFF) << 2) | 0x80000000:#010x}"
    if op == 4:
        return f"beq ${REGS[rs]}, ${REGS[rt]}, {simm}"
    if op == 9:
        return f"addiu ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 0x0C:
        return f"andi ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0D:
        return f"ori ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0F:
        return f"lui ${REGS[rt]}, {imm:#x}"
    if op == 0x20:
        return f"lb ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x23:
        return f"lw ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x24:
        return f"lbu ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x28:
        return f"sb ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x2B:
        return f"sw ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    return f"op={op:#x}"


def main() -> int:
    exe = (Path(__file__).resolve().parents[1] / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    for va in range(0x8006CB60, 0x8006CC68, 4):
        w = word_at(exe, va)
        print(f"{va:08X}  {w:08X}  {decode(w)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
