#!/usr/bin/env python3
"""Extract 6F39C 0x6C prelude blob and census overlay +0x252 stores."""
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


def u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def u16(data: bytes, addr: int) -> int:
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
            0x0C: "syscall",
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
        # Mode 2 Form 1 user data at +24, 2048 bytes
        out.extend(raw[off + 24 : off + 24 + 2048])
    return bytes(out)


def blob_u32(blob: bytes, addr: int) -> int:
    return struct.unpack_from("<I", blob, addr - LOAD)[0]


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1

    start = u16(exe, 0x80093162)
    end = u16(exe, 0x80093164)
    dest = u32(exe, 0x80011618)
    print(f"93162 start={start} 93164 end={end} count={end - start}")
    print(f"11618 dest={dest:08X}")
    print(f"B0DD8 exe={u32(exe, 0x800B0DD8):08X}")

    disc = pathlib.Path((root / "local" / "pe_disc1.path").read_text().strip())
    blob = read_bin_sectors(disc, PE_IMG_LBA + start, end - start)
    print(f"blob len={len(blob)} sha1={hashlib.sha1(blob).hexdigest()}")

    recs = [
        0x801F1BD8, 0x801F1C58, 0x801F1D00, 0x801F1D8C,
        0x801F1E18, 0x801F1EA4, 0x801F1EF0,
    ]
    for rec in recs:
        print(f"\n=== rec {rec:08X} words[0..0x40) ===")
        for off in range(0, 0x40, 4):
            w = blob_u32(blob, rec + off)
            print(f"  +{off:02X} = {w:08X}")

    rec = 0x801F1BD8
    print("\n=== rec+0x30 handler dump (64w) ===")
    fn = blob_u32(blob, rec + 0x30)
    print(f"  fn={fn:08X}")
    if LOAD <= fn < LOAD + len(blob):
        for pc in range(fn, min(fn + 256, LOAD + len(blob)), 4):
            w = blob_u32(blob, pc)
            print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== overlay sb/lbu imm 0x252 ===")
    for off in range(0, len(blob) - 3, 4):
        w = struct.unpack_from("<I", blob, off)[0]
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if op in (0x20, 0x24, 0x28) and simm == 0x252:
            print(f"  {LOAD + off:08X}  {w:08X}  {dis(w, LOAD + off)}")

    print("\n=== overlay sb $0 any imm (first 40) ===")
    n = 0
    for off in range(0, len(blob) - 3, 4):
        w = struct.unpack_from("<I", blob, off)[0]
        op = (w >> 26) & 0x3F
        rt = (w >> 16) & 0x1F
        if op == 0x28 and rt == 0:
            print(f"  {LOAD + off:08X}  {w:08X}  {dis(w, LOAD + off)}")
            n += 1
            if n >= 40:
                break

    print("\n=== overlay addiu/ori 0x252 ===")
    for off in range(0, len(blob) - 3, 4):
        w = struct.unpack_from("<I", blob, off)[0]
        op = (w >> 26) & 0x3F
        imm = w & 0xFFFF
        if op in (0x09, 0x0D) and imm == 0x252:
            print(f"  {LOAD + off:08X}  {w:08X}  {dis(w, LOAD + off)}")

    print("\n=== EXE rec 800E216C (0x6B / extra 0x16) +0x30 ===")
    rec16 = 0x800E216C
    for off in range(0, 0x40, 4):
        print(f"  +{off:02X} = {u32(exe, rec16 + off):08X}")

    print("\n=== 2A578 ===")
    for pc in range(0x8002A578, 0x8002A5C0, 4):
        w = u32(exe, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    print("\n=== 2AC00-2AC90 ===")
    for pc in range(0x8002AC00, 0x8002AC90, 4):
        w = u32(exe, pc)
        print(f"  {pc:08X}  {w:08X}  {dis(w, pc)}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
