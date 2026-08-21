#!/usr/bin/env python3
"""Disassemble 0x6C overlay streams and hunt offset-0x252 encodings."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PE_IMG_LBA = 1013
LOAD = 0x801ED7F8

REGS = [
    "$0", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra",
]


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def u32e(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def u16e(data: bytes, addr: int) -> int:
    return struct.unpack_from("<H", data, exe_off(addr))[0]


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
        return names.get(fn, f"spec fn={fn:02x} {word:08X}")
    if op == 2:
        return f"j {tgt:08X}"
    if op == 3:
        return f"jal {tgt:08X}"
    if op == 4:
        return f"beq {REGS[rs]}, {REGS[rt]}, {pc + 4 + simm * 4:08X}"
    if op == 5:
        return f"bne {REGS[rs]}, {REGS[rt]}, {pc + 4 + simm * 4:08X}"
    if op == 8:
        return f"addi {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 9:
        return f"addiu {REGS[rt]}, {REGS[rs]}, {simm}"
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
    return f"op={op:02x} {word:08X}"


def read_bin_sectors(bin_path: pathlib.Path, lba: int, count: int) -> bytes:
    raw = bin_path.read_bytes()
    out = bytearray()
    for i in range(count):
        off = (lba + i) * 2352
        out.extend(raw[off + 24 : off + 24 + 2048])
    return bytes(out)


def blob_u32(blob: bytes, addr: int) -> int:
    return struct.unpack_from("<I", blob, addr - LOAD)[0]


def looks_code(word: int) -> bool:
    op = (word >> 26) & 0x3F
    return op in (0, 2, 3, 4, 5, 8, 9, 0x0B, 0x0C, 0x0D, 0x0F, 0x20, 0x23, 0x24, 0x28, 0x2B)


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    start = u16e(exe, 0x80093162)
    end = u16e(exe, 0x80093164)
    disc = pathlib.Path((root / "local" / "pe_disc1.path").read_text().strip())
    blob = read_bin_sectors(disc, PE_IMG_LBA + start, end - start)

    def dump_fn(addr: int, nwords: int) -> None:
        print(f"\n=== {addr:08X} ({nwords}w) ===")
        for i in range(nwords):
            pc = addr + i * 4
            if pc < LOAD or pc + 4 > LOAD + len(blob):
                print(f"  {pc:08X}  OUT")
                break
            w = blob_u32(blob, pc)
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    dump_fn(0x801EDC44, 80)
    dump_fn(0x801F1F28, 16)

    print("\n=== 801F1B98 32 bytes ===")
    off = 0x801F1B98 - LOAD
    print(blob[off:off + 32].hex())

    print("\n=== u16 0x0252 in overlay ===")
    for i in range(0, len(blob) - 1, 2):
        if struct.unpack_from("<H", blob, i)[0] == 0x0252:
            print(f"  {LOAD + i:08X}")

    print("\n=== u32 0x00000252 in overlay ===")
    for i in range(0, len(blob) - 3, 4):
        if struct.unpack_from("<I", blob, i)[0] == 0x252:
            print(f"  {LOAD + i:08X}")

    print("\n=== EXE DF6AC (0x6B rec+0) first 20w ===")
    for pc in range(0x800DF6AC, 0x800DF6AC + 80, 4):
        w = u32e(exe, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== 68014 ===")
    for pc in range(0x80068014, 0x80068100, 4):
        w = u32e(exe, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== 6CD50-6CD90 ===")
    for pc in range(0x8006CD50, 0x8006CDA4, 4):
        w = u32e(exe, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== 2A540-2A578 (6F6D4 args) ===")
    for pc in range(0x8002A540, 0x8002A580, 4):
        w = u32e(exe, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== split-offset: addiu 0x200..0x250 then nearby sb ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32e(exe, pc)
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if op == 9 and 0x200 <= simm <= 0x250:
            rt = (w >> 16) & 0x1F
            # look ahead 8 insns for sb using that rt as base
            for k in range(1, 10):
                w2 = u32e(exe, pc + k * 4)
                op2 = (w2 >> 26) & 0x3F
                rs2 = (w2 >> 21) & 0x1F
                imm2 = w2 & 0xFFFF
                simm2 = imm2 - 0x10000 if imm2 & 0x8000 else imm2
                if op2 == 0x28 and rs2 == rt:
                    total = simm + simm2
                    if total == 0x252 or abs(total - 0x252) < 4:
                        print(f"  {pc:08X} addiu +{simm:X} -> {pc + k * 4:08X} sb +{simm2} total={total:X}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
