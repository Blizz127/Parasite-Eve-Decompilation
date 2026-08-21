#!/usr/bin/env python3
"""B0E70 writers, 1A918 entry, and type 0/3/5 script heads."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
LIST_OFF = 0x202A4
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048

REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def exe_off(addr: int) -> int:
    return addr - TADDR + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def dis(word: int, addr: int) -> str:
    op = word >> 26
    rs = (word >> 21) & 31
    rt = (word >> 16) & 31
    rd = (word >> 11) & 31
    sa = (word >> 6) & 31
    fn = word & 63
    imm = word & 0xFFFF
    simm = imm - 0x10000 if imm >= 0x8000 else imm
    tgt = ((word & 0x03FFFFFF) << 2) | (addr & 0xF0000000)
    if op == 0:
        if word == 0:
            return "nop"
        names = {
            0: f"sll {REGS[rd]}, {REGS[rt]}, {sa}",
            2: f"srl {REGS[rd]}, {REGS[rt]}, {sa}",
            3: f"sra {REGS[rd]}, {REGS[rt]}, {sa}",
            8: f"jr {REGS[rs]}",
            9: f"jalr {REGS[rs]}",
            0x21: f"addu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x23: f"subu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x24: f"and {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x25: f"or {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2B: f"sltu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
        }
        return names.get(fn, f"spec {fn:02x}")
    if op == 2:
        return f"j {tgt:#x}"
    if op == 3:
        return f"jal {tgt:#x}"
    if op == 4:
        return f"beq {REGS[rs]}, {REGS[rt]}, {addr+4+simm*4:#x}"
    if op == 5:
        return f"bne {REGS[rs]}, {REGS[rt]}, {addr+4+simm*4:#x}"
    if op == 9:
        return f"addiu {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0C:
        return f"andi {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0D:
        return f"ori {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0F:
        return f"lui {REGS[rt]}, {imm:#x}"
    if op == 0x20:
        return f"lb {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x21:
        return f"lh {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x23:
        return f"lw {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x24:
        return f"lbu {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x25:
        return f"lhu {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x28:
        return f"sb {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x2B:
        return f"sw {REGS[rt]}, {simm}({REGS[rs]})"
    return f"op{op:02x} {word:08x}"


def find_disc(root: pathlib.Path):
    p = root / "local" / "pe_disc1.path"
    if p.is_file():
        path = pathlib.Path(p.read_text().strip().splitlines()[0].strip())
        if path.is_file():
            return path
    return None


def read_form1(disc, lba, nsec):
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            out += fh.read(FORM1_USER)
    return bytes(out)


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    data = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1
    disc = find_disc(root)
    sec0, sec1, sec2 = PACKED & 0xFF, (PACKED >> 8) & 0xFFF, PACKED >> 20
    chunk2 = read_form1(disc, PE_IMG_LBA + REL + sec0 + sec1, sec2)
    assert hashlib.sha256(chunk2).hexdigest() == CHUNK2_SHA

    print("===== 6B4F8 around +0x198 store 6B804")
    for addr in range(0x8006B7C0, 0x8006B860, 4):
        w = load_u32(data, addr)
        print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")

    print("\n===== 35038 +0x1AC beq through empty arm")
    for addr in range(0x80035280, 0x80035380, 4):
        w = load_u32(data, addr)
        print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")

    hdr_off = struct.unpack_from("<I", chunk2, 4)[0] & 0x3FFFFF
    word18 = struct.unpack_from("<I", chunk2, hdr_off + 0x18)[0]
    obj_off = struct.unpack_from("<I", chunk2, (word18 & 0x3FFFFF) + 4)[0] & 0x00FFFFFF
    print(f"\n===== 1A918 obj @{obj_off:#x} +0x2C..+0x80")
    for off in range(0x2C, 0x80, 2):
        h = struct.unpack_from("<H", chunk2, obj_off + off)[0]
        extra = ""
        if off % 4 == 0:
            extra = f" w={struct.unpack_from('<I', chunk2, obj_off + off)[0]:08X}"
        print(f"  +{off:02X} h={h:04X}{extra}")

    print("\n===== record base obj+0x1CC first 3 * 28")
    rec = obj_off + 0x1CC
    for i in range(3):
        row = rec + i * 28
        hs = [struct.unpack_from("<H", chunk2, row + j)[0] for j in range(0, 28, 2)]
        ws = [struct.unpack_from("<I", chunk2, row + j)[0] for j in range(0, 28, 4)]
        print(f"  rec[{i}] @{row-obj_off:#x} h={['%04X'%x for x in hs]} w={['%08X'%x for x in ws]}")

    print("\n===== type script heads")
    raws = {
        0: struct.unpack_from("<I", chunk2, LIST_OFF + 8)[0],
        1: struct.unpack_from("<I", chunk2, LIST_OFF + 12)[0],
        3: struct.unpack_from("<I", chunk2, LIST_OFF + 20)[0],
        5: struct.unpack_from("<I", chunk2, LIST_OFF + 28)[0],
        6: struct.unpack_from("<I", chunk2, LIST_OFF + 32)[0],
    }
    for typ, raw in raws.items():
        off = LIST_OFF + raw
        print(f"  type {typ} @{off:#x}")
        for i in range(0, 0x40, 4):
            w = struct.unpack_from("<I", chunk2, off + i)[0]
            print(
                f"    +{i:03X} {w:08X} op={w & 0x1FFF:#06x} "
                f"argc={(w>>13)&0xF} kinds={w>>17:05x}"
            )

    # 6C1CC PE.IMG table for Writer B package
    print("\n===== D_800930D8[20..24] PE.IMG halves")
    for i in range(20, 26):
        h = struct.unpack_from("<H", data, exe_off(0x800930D8 + i * 2))[0]
        print(f"  [ {i} ] = {h:#x} ({h})")

    return 0


if __name__ == "__main__":
    sys.exit(main())
