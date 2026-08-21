#!/usr/bin/env python3
"""3C818 clear path, dest+0x8C writers, 35558 3AF14 call."""
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
            0x02: f"srl {REGS[rd]}, {REGS[rt]}, {sa}",
            0x03: f"sra {REGS[rd]}, {REGS[rt]}, {sa}",
            0x08: f"jr {REGS[rs]}",
            0x09: f"jalr {REGS[rd]}, {REGS[rs]}",
            0x21: f"addu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x23: f"subu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x24: f"and {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x25: f"or {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
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
    if op == 6:
        return f"blez {REGS[rs]}, {pc + 4 + simm * 4:08X}"
    if op == 7:
        return f"bgtz {REGS[rs]}, {pc + 4 + simm * 4:08X}"
    if op == 9:
        return f"addiu {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0A:
        return f"slti {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0B:
        return f"sltiu {REGS[rt]}, {REGS[rs]}, {simm}"
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


def dump(data: bytes, start: int, end: int) -> None:
    for pc in range(start, end, 4):
        w = u32(data, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1

    print("=== 35558 35AC0-35B40 ===")
    dump(exe, 0x80035AC0, 0x80035B40)

    print("\n=== 3C2E0 tail 3C580-3C5D4 ===")
    dump(exe, 0x8003C580, 0x8003C5D4)

    print("\n=== 3C818 3CA50-3CAD8 ===")
    dump(exe, 0x8003CA50, 0x8003CAD8)

    print("\n=== sb dest+0x8C (imm 0x8C) ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(exe, pc)
        op = (w >> 26) & 0x3F
        if op != 0x28:
            continue
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if simm == 0x8C:
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== 3B144 first 40w ===")
    dump(exe, 0x8003B144, 0x8003B144 + 160)

    print("\n=== 6CC68 / 6CD40 dest args ===")
    dump(exe, 0x8006CD20, 0x8006CD80)

    print("\n=== 3C638 first 20w ===")
    dump(exe, 0x8003C638, 0x8003C638 + 80)

    return 0


if __name__ == "__main__":
    sys.exit(main())
