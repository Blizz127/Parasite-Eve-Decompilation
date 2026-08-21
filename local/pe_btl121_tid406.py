#!/usr/bin/env python3
"""Census literal 406 / BE834 / slot+4 tid publishers."""
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


def dump(data: bytes, start: int, end: int) -> None:
    for pc in range(start, end, 4):
        print(f"  {pc:08X}  {u32(data, pc):08X}  {dis(u32(data, pc), pc)}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    data = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1

    print("=== addiu/ori/slti imm 406 (0x196) ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(data, pc)
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        if op in (0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D) and imm == 0x196:
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== addiu/ori 387 (0x183) ===")
    n = 0
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32(data, pc)
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        if op in (0x09, 0x0D) and imm == 0x183:
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")
            n += 1
            if n >= 20:
                break

    print("\n=== sh rt, 4(rs) in TEXT (tid slot) first 40 ===")
    n = 0
    for pc in range(0x80010000, 0x80040000, 4):
        w = u32(data, pc)
        if (w >> 26) == 0x29 and (w & 0xFFFF) == 4:
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")
            n += 1
            if n >= 40:
                break

    print("\n=== 16818 context ===")
    dump(data, 0x800167F0, 0x80016850)

    print("\n=== 21920-21950 ===")
    dump(data, 0x80021920, 0x80021960)

    print("\n=== jal 16818 / 21938 ===")
    for tgt, name in ((0x80016818, "16818?"),):
        pass
    # find function starts near those sh sites
    print("\n=== jal targets that write slot+4: find callers of funcs ===")
    for site in (0x80016818, 0x80021938, 0x80025E40, 0x80025E9C):
        # walk back to addiu sp
        pc = site
        while pc > 0x80010000:
            w = u32(data, pc)
            if (w & 0xFFFF0000) == 0x27BD0000:
                print(f"  site {site:08X} fn {pc:08X}")
                hits = []
                for p in range(0x80010000, 0x80090000, 4):
                    ww = u32(data, p)
                    if (ww >> 26) == 3 and jal_target(ww) == pc:
                        hits.append(p)
                print(f"    jals: {[f'{h:08X}' for h in hits[:12]]}")
                break
            pc -= 4

    return 0


if __name__ == "__main__":
    sys.exit(main())
