#!/usr/bin/env python3
"""Identify dest+0x9E zero writers and callers."""
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
            0x2A: f"slt {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2B: f"sltu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
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
    if op == 0x0D:
        return f"ori {REGS[rt]}, {REGS[rs]}, 0x{imm:X}"
    if op == 0x0F:
        return f"lui {REGS[rt]}, 0x{imm:X}"
    loads = {
        0x20: "lb", 0x21: "lh", 0x23: "lw", 0x24: "lbu", 0x25: "lhu",
        0x28: "sb", 0x29: "sh", 0x2B: "sw",
    }
    if op in loads:
        return f"{loads[op]} {REGS[rt]}, {simm}({REGS[rs]})"
    return f"op={op:02x}"


def find_fn_start(data: bytes, site: int) -> int:
    pc = site
    while pc > 0x80010000:
        w = u32(data, pc)
        if (w & 0xFFFF0000) == 0x27BD0000:  # addiu $sp
            return pc
        pc -= 4
    return site


def dump(data: bytes, start: int, end: int) -> None:
    for pc in range(start, end, 4):
        w = u32(data, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")


def find_jals(data: bytes, target: int) -> list[int]:
    hits = []
    for pc in range(0x80010000, 0x800E8000, 4):
        w = u32(data, pc)
        if (w >> 26) == 3 and jal_target(w) == target:
            hits.append(pc)
    return hits


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1

    for site, label in (
        (0x8003C5BC, "sb0 dest+0x9E before 3C5D8"),
        (0x8003C87C, "sb0 dest+0x9E"),
        (0x8003AF14, "3AF14"),
        (0x8006CBF8, "6CBF8 sb dest+0x9E"),
        (0x80030390, "30220-ish sb +0x9E"),
    ):
        start = find_fn_start(exe, site)
        print(f"\n===== {label} site {site:08X} fn={start:08X} =====")
        # dump from start until next addiu sp or 80 words
        end = start + 4
        seen = 0
        while end < start + 0x400:
            w = u32(exe, end)
            if (w & 0xFFFF0000) == 0x27BD0000 and end > start:
                break
            end += 4
            seen += 1
            if seen > 160:
                break
        dump(exe, start, min(end, start + 0x180))
        print("  jals:", [f"{p:08X}" for p in find_jals(exe, start)])

    print("\n===== 29120-29160 (lbu dest+0x9E vs actor+0x252) =====")
    dump(exe, 0x80029120, 0x80029160)

    return 0


if __name__ == "__main__":
    sys.exit(main())
