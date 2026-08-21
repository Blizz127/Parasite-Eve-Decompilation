#!/usr/bin/env python3
"""BTL120: 6F39C 0x6C prelude + Aya+0x252 store census."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80010000
HDR = 0x800


def exe_off(addr: int) -> int:
    return addr - BASE + HDR


def u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def u16(data: bytes, addr: int) -> int:
    return struct.unpack_from("<H", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


REGS = [
    "$0", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra",
]


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
            0x02: f"srl {REGS[rd]}, {REGS[rt]}, {sa}",
            0x03: f"sra {REGS[rd]}, {REGS[rt]}, {sa}",
            0x04: f"sllv {REGS[rd]}, {REGS[rt]}, {REGS[rs]}",
            0x08: f"jr {REGS[rs]}",
            0x09: f"jalr {REGS[rd]}, {REGS[rs]}",
            0x0C: "syscall",
            0x10: f"mfhi {REGS[rd]}",
            0x12: f"mflo {REGS[rd]}",
            0x18: f"mult {REGS[rs]}, {REGS[rt]}",
            0x19: f"multu {REGS[rs]}, {REGS[rt]}",
            0x1A: f"div {REGS[rs]}, {REGS[rt]}",
            0x1B: f"divu {REGS[rs]}, {REGS[rt]}",
            0x20: f"add {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x21: f"addu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x23: f"subu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x24: f"and {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x25: f"or {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x26: f"xor {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x27: f"nor {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2A: f"slt {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2B: f"sltu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
        }
        return names.get(fn, f"spec fn={fn:02x}")
    if op == 1:
        kind = {0: "bltz", 1: "bgez", 16: "bltzal", 17: "bgezal"}.get(rt, f"regimm {rt}")
        return f"{kind} {REGS[rs]}, {pc + 4 + simm * 4:08X}"
    if op == 2:
        return f"j {tgt:08X}"
    if op == 3:
        return f"jal {tgt:08X}"
    if op == 4:
        return f"beq {REGS[rs]}, {REGS[rt]}, {pc + 4 + simm * 4:08X}"
    if op == 5:
        return f"bne {REGS[rs]}, {REGS[rt]}, {pc + 4 + simm * 4:08X}"
    if op == 6:
        return f"blez {REGS[rs]}, {pc + 4 + simm * 4:08X}"
    if op == 7:
        return f"bgtz {REGS[rs]}, {pc + 4 + simm * 4:08X}"
    if op == 8:
        return f"addi {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 9:
        return f"addiu {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0A:
        return f"slti {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0B:
        return f"sltiu {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0C:
        return f"andi {REGS[rt]}, {REGS[rs]}, 0x{imm:X}"
    if op == 0x0D:
        return f"ori {REGS[rt]}, {REGS[rs]}, 0x{imm:X}"
    if op == 0x0E:
        return f"xori {REGS[rt]}, {REGS[rs]}, 0x{imm:X}"
    if op == 0x0F:
        return f"lui {REGS[rt]}, 0x{imm:X}"
    loads = {
        0x20: "lb", 0x21: "lh", 0x23: "lw", 0x24: "lbu", 0x25: "lhu",
        0x28: "sb", 0x29: "sh", 0x2B: "sw",
    }
    if op in loads:
        return f"{loads[op]} {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x0F:
        return f"lui {REGS[rt]}, 0x{imm:X}"
    return f"op={op:02x} {word:08X}"


def dump_range(data: bytes, start: int, end: int) -> None:
    for pc in range(start, end, 4):
        w = u32(data, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")


def find_jals(data: bytes, target: int) -> list[int]:
    hits = []
    # TEXT roughly 0x80010000..0x8008xxxx
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(data, pc)
        if (w >> 26) == 3 and jal_target(w) == target:
            hits.append(pc)
    return hits


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    exe = root / "build" / "disc1.candidate.exe"
    data = exe.read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1

    print("=== 6F39C full ===")
    dump_range(data, 0x8006F39C, 0x8006F6D4)

    print("\n=== D4698 ===")
    dump_range(data, 0x800D4698, 0x800D4704)

    print("\n=== D4704 first 40w ===")
    dump_range(data, 0x800D4704, 0x800D4704 + 40 * 4)

    print("\n=== E1044[0x10..0x22] ===")
    for i in range(0x10, 0x23):
        print(f"  E1044[{i:02X}] = {u32(data, 0x800E1044 + i * 4):08X}")

    print("\n=== 0x80011618 / 0x80093162 ===")
    print(f"  11618 = {u32(data, 0x80011618):08X}")
    print(f"  93162 = {u16(data, 0x80093162):04X}")

    print("\n=== jal 6F39C ===")
    for pc in find_jals(data, 0x8006F39C):
        print(f"  {pc:08X}")
    print("=== jal 6F6D4 ===")
    for pc in find_jals(data, 0x8006F6D4):
        print(f"  {pc:08X}")
    print("=== jal 6F8EC ===")
    for pc in find_jals(data, 0x8006F8EC):
        print(f"  {pc:08X}")
    print("=== jal 6FC18 ===")
    for pc in find_jals(data, 0x8006FC18):
        print(f"  {pc:08X}")

    print("\n=== sb/lbu imm 0x252 (594) ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(data, pc)
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if op in (0x20, 0x24, 0x28) and simm == 0x252:
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== addiu/ori 0x252 ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(data, pc)
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        if op in (0x09, 0x0D) and imm == 0x252:
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== sw/sh covering +0x250/+0x252 ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(data, pc)
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if op in (0x29, 0x2B) and simm in (0x250, 0x252, 0x24E, 0x24C):
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
