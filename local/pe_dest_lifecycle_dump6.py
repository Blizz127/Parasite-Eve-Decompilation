#!/usr/bin/env python3
"""1220C loop around 3F3C4; mode9 start; 6BECC returns."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
HDR = 0x800
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")

REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def exe_off(a: int) -> int:
    return a - TADDR + HDR


def load_u32(data: bytes, a: int) -> int:
    return struct.unpack_from("<I", data, exe_off(a))[0]


def jal_target(w: int) -> int:
    return ((w & 0x03FFFFFF) << 2) | 0x80000000


def dis(word: int, addr: int) -> str:
    op = word >> 26
    rs = (word >> 21) & 31
    rt = (word >> 16) & 31
    rd = (word >> 11) & 31
    fn = word & 63
    imm = word & 0xFFFF
    simm = imm - 0x10000 if imm >= 0x8000 else imm
    tgt = ((word & 0x03FFFFFF) << 2) | (addr & 0xF0000000)
    if op == 0:
        if word == 0:
            return "nop"
        names = {
            8: f"jr {REGS[rs]}",
            9: f"jalr {REGS[rd]}, {REGS[rs]}",
            0x21: f"addu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x24: f"and {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x25: f"or {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2B: f"sltu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
        }
        return names.get(fn, f"spec fn={fn:#x}")
    if op == 2:
        return f"j {tgt:#x}"
    if op == 3:
        return f"jal {tgt:#x}"
    if op == 4:
        return f"beq {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}"
    if op == 5:
        return f"bne {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}"
    if op == 9:
        return f"addiu {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0D:
        return f"ori {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0F:
        return f"lui {REGS[rt]}, {imm:#x}"
    if op == 0x23:
        return f"lw {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x2B:
        return f"sw {REGS[rt]}, {simm}({REGS[rs]})"
    return f"op{op:02x} {word:08X}"


def main() -> int:
    exe = (ROOT / "build/disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    print("===== 1220C 123D8..124F4")
    for addr in range(0x800123D8, 0x800124F8, 4):
        w = load_u32(exe, addr)
        print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")
    print("\n===== mode9 2B0EC..2B298")
    for addr in range(0x8002B0EC, 0x8002B298, 4):
        w = load_u32(exe, addr)
        print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")
    print("\n===== 6BECC v0=1 return sites")
    for addr in range(0x8006BECC, 0x8006C1CC, 4):
        w = load_u32(exe, addr)
        if w == 0x24020001:  # addiu v0, zero, 1
            print(f"  {addr:08X} addiu v0, 1")
    print("  6C184", hex(load_u32(exe, 0x8006C184)), dis(load_u32(exe, 0x8006C184), 0x8006C184))
    print("  6C19C", hex(load_u32(exe, 0x8006C19C)), dis(load_u32(exe, 0x8006C19C), 0x8006C19C))
    return 0


if __name__ == "__main__":
    sys.exit(main())
