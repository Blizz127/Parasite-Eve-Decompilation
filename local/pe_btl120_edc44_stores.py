#!/usr/bin/env python3
"""Hunt 801EDC44 stores and who jalrs rec+0 / CE870."""
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
    if op == 9:
        return f"addiu {REGS[rt]}, {REGS[rs]}, {simm}"
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


def read_bin_sectors(bin_path: pathlib.Path, lba: int, count: int) -> bytes:
    raw = bin_path.read_bytes()
    out = bytearray()
    for i in range(count):
        off = (lba + i) * 2352
        out.extend(raw[off + 24 : off + 24 + 2048])
    return bytes(out)


def blob_u32(blob: bytes, addr: int) -> int:
    return struct.unpack_from("<I", blob, addr - LOAD)[0]


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    start = u16e(exe, 0x80093162)
    end = u16e(exe, 0x80093164)
    disc = pathlib.Path((root / "local" / "pe_disc1.path").read_text().strip())
    blob = read_bin_sectors(disc, PE_IMG_LBA + start, end - start)

    print("=== overlay sb / sh / sw with large imm or Aya ===")
    for off in range(0, len(blob) - 3, 4):
        w = struct.unpack_from("<I", blob, off)[0]
        op = (w >> 26) & 0x3F
        if op not in (0x28, 0x29, 0x2B):
            continue
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if abs(simm) >= 0x200 or simm in (0x0E, 0x14, 0x16, 0x18, 0x1A, 0x98):
            print(f"  {LOAD + off:08X}  {w:08X}  {dis(w, LOAD + off)}")

    print("\n=== overlay lui 800A / D254 refs ===")
    for off in range(0, len(blob) - 3, 4):
        w = struct.unpack_from("<I", blob, off)[0]
        if w in (0x3C02800A, 0x3C04800A, 0x3C03800A, 0x3C05800A, 0x3C06800A):
            print(f"  {LOAD + off:08X}  {dis(w, LOAD + off)}")
            for k in range(1, 6):
                w2 = struct.unpack_from("<I", blob, off + k * 4)[0]
                print(f"    {LOAD + off + k * 4:08X}  {w2:08X}  {dis(w2, LOAD + off + k * 4)}")

    print("\n=== overlay jal targets (unique) ===")
    jals = set()
    for off in range(0, len(blob) - 3, 4):
        w = struct.unpack_from("<I", blob, off)[0]
        if (w >> 26) == 3:
            jals.add(jal_target(w))
    for t in sorted(jals):
        print(f"  jal {t:08X}")

    print("\n=== EXE jal CE870 ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32e(exe, pc)
        if (w >> 26) == 3 and jal_target(w) == 0x800CE870:
            print(f"  {pc:08X}")

    print("\n=== EXE jal 6DDCC ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32e(exe, pc)
        if (w >> 26) == 3 and jal_target(w) == 0x8006DDCC:
            print(f"  {pc:08X}")

    print("\n=== EXE uses of E1044 (lui 800E + 10xx) ===")
    # D_800E1044
    for pc in range(0x80010000, 0x800E2000, 4):
        w = u32e(exe, pc)
        # lw/sw ..., 4164($at) where at is 800E and 4164=0x1044
        if (w & 0xFFFF) == 0x1044 and ((w >> 26) & 0x3F) in (0x23, 0x2B):
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== CE870 first 60w ===")
    for pc in range(0x800CE870, 0x800CE870 + 240, 4):
        w = u32e(exe, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== who lw 0x8C then lw 0 / jalr (slot+0x8C) ===")
    for pc in range(0x80010000, 0x800E2000, 4):
        w = u32e(exe, pc)
        # lw rt, 140(rs)
        if (w >> 26) == 0x23 and (w & 0xFFFF) == 0x8C:
            print(f"  {pc:08X}  {dis(w, pc)}")

    print("\n=== D2FC writers (gp+0x58C = 1420) ===")
    for pc in range(0x80010000, 0x80090000, 4):
        w = u32e(exe, pc)
        op = (w >> 26) & 0x3F
        rs = (w >> 21) & 0x1F
        imm = w & 0xFFFF
        if op == 0x2B and rs == 28 and imm == 1420:
            print(f"  {pc:08X}  {dis(w, pc)}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
