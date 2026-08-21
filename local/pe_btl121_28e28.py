#!/usr/bin/env python3
"""Dump 28E28 slot+4 store and 15DAC tid copy."""
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
    if op == 0x0F:
        return f"lui {REGS[rt]}, 0x{imm:X}"
    loads = {
        0x20: "lb", 0x21: "lh", 0x23: "lw", 0x24: "lbu", 0x25: "lhu",
        0x28: "sb", 0x29: "sh", 0x2B: "sw",
    }
    if op in loads:
        return f"{loads[op]} {REGS[rt]}, {simm}({REGS[rs]})"
    return f"op={op:02x}"


def dump(data, start, end):
    for pc in range(start, end, 4):
        w = u32(data, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    data = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1
    print("=== 28DE0-28E94 ===")
    dump(data, 0x80028DE0, 0x80028E94)
    print("\n=== jal 28E00-ish / 15DAC table slot for sh+4 ===")
    # 15DAC jtbl often at 80010xxx
    print(f"15DAC first {u32(data, 0x80015DAC):08X}")
    print("\n=== 26xxx writers that might be menu ===")
    dump(data, 0x800269B0, 0x800269E0)
    return 0


if __name__ == "__main__":
    sys.exit(main())
