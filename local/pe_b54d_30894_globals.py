#!/usr/bin/env python3
"""Recover lui+addiu globals and 30894 control-flow skeleton."""
from __future__ import annotations

import hashlib
import struct
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
FUNC, END = 0x80030894, 0x800314E4
REG = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, addr - LOAD + 0x800)[0]


def sext16(x: int) -> int:
    return x - 0x10000 if x & 0x8000 else x


def main() -> None:
    data = (Path(__file__).resolve().parents[1] / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1

    last_lui = {}
    addrs = []
    pc = FUNC
    while pc < END:
        w = u32(data, pc)
        op = (w >> 26) & 63
        rs, rt = (w >> 21) & 31, (w >> 16) & 31
        imm = w & 0xFFFF
        if op == 15:
            last_lui[rt] = (pc, imm << 16)
        elif op in (8, 9, 32, 33, 35, 36, 37, 40, 41, 43) and rs in last_lui:
            base_pc, base = last_lui[rs]
            addr = (base + sext16(imm)) & 0xFFFFFFFF
            if 0x80000000 <= addr <= 0x801FFFFF:
                addrs.append((pc, REG[rt] if op in (8, 9) else REG[rt], addr, op, REG[rs]))
        # branches
        if op in (4, 5, 6, 7) or (op == 1 and rt in (0, 1)):
            tgt = pc + 4 + (sext16(imm) << 2)
            names = {4: "beq", 5: "bne", 6: "blez", 7: "bgtz"}
            mnem = names.get(op, "regimm")
            print(f"BR {pc:08X}  {mnem} -> {tgt:08X}  {'INSIDE' if FUNC<=tgt<END else 'OUT'}")
        pc += 4

    print("\n=== GLOBALS ===")
    seen = {}
    for pc, rt, addr, op, rs in addrs:
        seen.setdefault(addr, []).append((pc, op, rt, rs))
    for addr in sorted(seen):
        refs = seen[addr]
        ops = ",".join({
            8: "addi", 9: "addiu", 32: "lb", 33: "lh", 35: "lw",
            36: "lbu", 37: "lhu", 40: "sb", 41: "sh", 43: "sw",
        }[o] for _, o, _, _ in refs)
        print(f"{addr:#010x}  n={len(refs):2d}  {ops}  first={refs[0][0]:#010x}")

    # 5DADC exact words
    print("\n=== 5DADC 7 words ===")
    for i in range(8):
        pc = 0x8005DADC + i * 4
        print(f"{pc:08X}  {u32(data, pc):08X}")

    print("\n=== 77A64 14 words / 77AA4 6 / 77B04 10 / 77B34 10 ===")
    for start, n in ((0x80077A64, 16), (0x80077AA4, 8), (0x80077B04, 12), (0x80077B34, 12)):
        print(f"-- {start:#x} --")
        for i in range(n):
            pc = start + i * 4
            print(f"{pc:08X}  {u32(data, pc):08X}")


if __name__ == "__main__":
    main()
