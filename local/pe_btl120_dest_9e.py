#!/usr/bin/env python3
"""Census dest/clip stores that alias Aya+0x252 via +0x1B4+0x9E."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

REGS = [
    "$0", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra",
]


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def dis(word: int, pc: int) -> str:
    op = (word >> 26) & 0x3F
    rs = (word >> 21) & 0x1F
    rt = (word >> 16) & 0x1F
    rd = (word >> 11) & 0x1F
    sa = (word >> 6) & 0x1F
    fn = word & 0x3F
    imm = word & 0xFFFF
    simm = imm - 0x10000 if imm & 0x8000 else imm
    tgt = jal_target(word)
    if word == 0:
        return "nop"
    if op == 0:
        names = {
            0x00: f"sll {REGS[rd]}, {REGS[rt]}, {sa}",
            0x08: f"jr {REGS[rs]}",
            0x09: f"jalr {REGS[rd]}, {REGS[rs]}",
            0x21: f"addu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
        }
        return names.get(fn, f"spec {fn:02x}")
    if op == 2:
        return f"j {tgt:08X}"
    if op == 3:
        return f"jal {tgt:08X}"
    if op == 4:
        return f"beq {REGS[rs]}, {REGS[rt]}, {pc + 4 + simm * 4:08X}"
    if op == 5:
        return f"bne {REGS[rs]}, {REGS[rt]}, {pc + 4 + simm * 4:08X}"
    if op == 9:
        return f"addiu {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0C:
        return f"andi {REGS[rt]}, {REGS[rs]}, 0x{imm:X}"
    if op == 0x0F:
        return f"lui {REGS[rt]}, 0x{imm:X}"
    loads = {
        0x20: "lb", 0x21: "lh", 0x23: "lw", 0x24: "lbu", 0x25: "lhu",
        0x28: "sb", 0x29: "sh", 0x2B: "sw",
    }
    if op in loads:
        return f"{loads[op]} {REGS[rt]}, {simm}({REGS[rs]})"
    return f"op={op:02x}"


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1

    print("=== sb/lbu/sh/lh/sw/lw imm 0x9E / 0x9C / 0xA0 / 0x9F ===")
    for pc in range(0x80010000, 0x800E8000, 4):
        w = u32(exe, pc)
        op = (w >> 26) & 0x3F
        if op not in (0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B):
            continue
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if simm in (0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2):
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== addiu 0x9E ===")
    for pc in range(0x80010000, 0x800E8000, 4):
        w = u32(exe, pc)
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        if op in (0x09, 0x0D) and imm == 0x9E:
            print(f"  {pc:08X}  {dis(w, pc)}")

    print("\n=== 3C5D8 extent / stores ===")
    # dump until jr ra after a reasonable window; find end by next-function guess
    for pc in range(0x8003C5D8, 0x8003C800, 4):
        w = u32(exe, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")
        if w == 0x03E00008:
            print(f"  {pc + 4:08X}  {u32(exe, pc + 4):08X}  {dis(u32(exe, pc + 4), pc + 4)}")
            break

    print("\n=== jal 3C5D8 ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(exe, pc)
        if (w >> 26) == 3 and jal_target(w) == 0x8003C5D8:
            print(f"  {pc:08X}")

    print("\n=== sw D2FC absolute ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(exe, pc)
        # lui hi 8009 / 800A then sw lo D2FC
        if (w >> 26) == 0x0F and (w & 0xFFFF) in (0x8009, 0x800A):
            pass
    # scan sw with imm matching D2FC low
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(exe, pc)
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        if op == 0x2B and imm in (0xD2FC, 0x52FC):
            print(f"  {pc:08X}  {dis(w, pc)}")

    print("\n=== C6D5C first 8w (is it code?) ===")
    for pc in range(0x800C6D5C, 0x800C6D5C + 32, 4):
        w = u32(exe, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
