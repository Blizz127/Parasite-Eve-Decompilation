#!/usr/bin/env python3
"""Dump 1735C / 1AA78 / 1C614 / 6C118 and live m0005i tables."""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048
TYPE1_OFF = 0x21678


def exe_off(addr: int) -> int:
    return addr - TADDR + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def load_u16(data: bytes, addr: int) -> int:
    return struct.unpack_from("<H", data, exe_off(addr))[0]


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def jal_sites(data: bytes, target: int) -> list[int]:
    jal_word = 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)
    sites = []
    for offset in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, offset)[0] == jal_word:
            sites.append(TADDR + offset - 0x800)
    return sites


REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


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
        if fn == 0 and word == 0:
            return "nop"
        if fn == 0:
            return f"sll     {REGS[rd]}, {REGS[rt]}, {sa}"
        if fn == 2:
            return f"srl     {REGS[rd]}, {REGS[rt]}, {sa}"
        if fn == 3:
            return f"sra     {REGS[rd]}, {REGS[rt]}, {sa}"
        if fn == 4:
            return f"sllv    {REGS[rd]}, {REGS[rt]}, {REGS[rs]}"
        if fn == 8:
            return f"jr      {REGS[rs]}"
        if fn == 9:
            return f"jalr    {REGS[rd]}, {REGS[rs]}"
        if fn == 0x0C:
            return "syscall"
        if fn == 0x0D:
            return "break"
        if fn == 0x10:
            return f"mfhi    {REGS[rd]}"
        if fn == 0x12:
            return f"mflo    {REGS[rd]}"
        if fn == 0x18:
            return f"mult    {REGS[rs]}, {REGS[rt]}"
        if fn == 0x19:
            return f"multu   {REGS[rs]}, {REGS[rt]}"
        if fn == 0x1A:
            return f"div     {REGS[rs]}, {REGS[rt]}"
        if fn == 0x20:
            return f"add     {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        if fn == 0x21:
            return f"addu    {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        if fn == 0x22:
            return f"sub     {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        if fn == 0x23:
            return f"subu    {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        if fn == 0x24:
            return f"and     {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        if fn == 0x25:
            return f"or      {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        if fn == 0x26:
            return f"xor     {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        if fn == 0x27:
            return f"nor     {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        if fn == 0x2A:
            return f"slt     {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        if fn == 0x2B:
            return f"sltu    {REGS[rd]}, {REGS[rs]}, {REGS[rt]}"
        return f"special fn={fn:02x}"
    if op == 1:
        if rt == 0:
            return f"bltz    {REGS[rs]}, {addr + 4 + simm * 4:#x}"
        if rt == 1:
            return f"bgez    {REGS[rs]}, {addr + 4 + simm * 4:#x}"
        return f"regimm rt={rt}"
    if op == 2:
        return f"j       {tgt:#x}"
    if op == 3:
        return f"jal     {tgt:#x}"
    if op == 4:
        return f"beq     {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}"
    if op == 5:
        return f"bne     {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}"
    if op == 6:
        return f"blez    {REGS[rs]}, {addr + 4 + simm * 4:#x}"
    if op == 7:
        return f"bgtz    {REGS[rs]}, {addr + 4 + simm * 4:#x}"
    if op == 8:
        return f"addi    {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 9:
        return f"addiu   {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0A:
        return f"slti    {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0B:
        return f"sltiu   {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0C:
        return f"andi    {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0D:
        return f"ori     {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0E:
        return f"xori    {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0F:
        return f"lui     {REGS[rt]}, {imm:#x}"
    if op == 0x20:
        return f"lb      {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x21:
        return f"lh      {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x23:
        return f"lw      {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x24:
        return f"lbu     {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x25:
        return f"lhu     {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x28:
        return f"sb      {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x29:
        return f"sh      {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x2B:
        return f"sw      {REGS[rt]}, {simm}({REGS[rs]})"
    return f"op={op:02x} {word:08x}"


def dump_fn(data: bytes, start: int, end: int, name: str) -> None:
    words = (end - start) // 4
    print(f"\n===== {name} {start:#x}..{end:#x}  {words}w  sha={window_sha(data, start, end)}")
    print(f"jal sites -> this: {[hex(s) for s in jal_sites(data, start)]}")
    jals = []
    for addr in range(start, end, 4):
        w = load_u32(data, addr)
        line = dis(w, addr)
        print(f"  {addr:08X}  {w:08X}  {line}")
        if w >> 26 == 3:
            jals.append(jal_target(w))
    print(f"direct jals: {[hex(x) for x in jals]}")


def find_disc(root: pathlib.Path) -> pathlib.Path | None:
    pointer = root / "local" / "pe_disc1.path"
    if pointer.is_file():
        line = pointer.read_text().strip().splitlines()[0].strip()
        path = pathlib.Path(line)
        if path.is_file():
            return path
    env = os.environ.get("PE_DISC1_BIN", "").strip()
    if env:
        path = pathlib.Path(env)
        if path.is_file():
            return path
    return None


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            out += fh.read(FORM1_USER)
    return bytes(out)


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    exe = root / "build" / "disc1.candidate.exe"
    data = exe.read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1
    print("EXE SHA-1 OK")

    dump_fn(data, 0x8001735C, 0x800173F4, "func_8001735C")
    dump_fn(data, 0x8001AA78, 0x8001ACE0, "func_8001AA78")
    dump_fn(data, 0x8001C614, 0x8001C7DC, "func_8001C614")
    dump_fn(data, 0x8006C0E0, 0x8006C180, "func_8006C0E0_around_B0E70")

    print("\n===== table[0x08]")
    print(hex(load_u32(data, 0x800910A0 + 0x08 * 4)))

    print("\n===== 1AA78 word counts")
    for end in (0x8001ACD8, 0x8001ACDC, 0x8001ACE0):
        print(f"  ..{end:#x} {(end-0x8001AA78)//4}w sha={window_sha(data, 0x8001AA78, end)}")

    print("\n===== 1C614 word counts")
    for end in (0x8001C7D4, 0x8001C7D8, 0x8001C7DC):
        print(f"  ..{end:#x} {(end-0x8001C614)//4}w sha={window_sha(data, 0x8001C614, end)}")

    disc = find_disc(root)
    if disc is None:
        print("NO DISC")
        return 0
    n0 = PACKED & 0x3FF
    n1 = (PACKED >> 10) & 0x3FF
    n2 = (PACKED >> 20) & 0x3FF
    lba2 = PE_IMG_LBA + REL + n0 + n1
    chunk2 = read_form1(disc, lba2, n2)
    print(f"\n===== chunk2 {len(chunk2)} sha={hashlib.sha256(chunk2).hexdigest()}")
    assert hashlib.sha256(chunk2).hexdigest() == CHUNK2_SHA

    # type-1 stream at +0x21678
    print("\n===== type-1 stream from +0x0A0")
    for i in range(0x0A0, 0x120, 4):
        w = struct.unpack_from("<I", chunk2, TYPE1_OFF + i)[0]
        print(f"  +{i:03X}  {w:08X}  op={w & 0x1FFF:#x} argc={(w>>13)&0xF} kinds={w>>17:04x}")

    # hdr at s5 = chunk2 + (lw(chunk2+4) & 0x3FFFFF)
    hdr_off = struct.unpack_from("<I", chunk2, 4)[0] & 0x3FFFFF
    print(f"\n===== chunk2 hdr off={hdr_off:#x}")
    for off in range(0, 0x40, 4):
        w = struct.unpack_from("<I", chunk2, hdr_off + off)[0]
        print(f"  hdr+{off:02X} = {w:08X}")

    # overlay+0x948 comes from hdr+0x18 relative
    word18 = struct.unpack_from("<I", chunk2, hdr_off + 0x18)[0]
    obj_off = (struct.unpack_from("<I", chunk2, (word18 & 0x3FFFFF) + 4)[0] & 0x00FFFFFF)
    print(f"\n===== 1A918 obj off={obj_off:#x}")
    for off in range(0, 0x40, 2):
        if off % 4 == 0:
            w = struct.unpack_from("<I", chunk2, obj_off + off)[0]
            print(f"  obj+{off:02X} = {w:08X}")
        else:
            h = struct.unpack_from("<H", chunk2, obj_off + off)[0]
            print(f"  obj+{off:02X} h = {h:04X}")

    # B0E70 is overlay+0x198 — not in chunk2 image itself.
    # Look at 6B4F8 stores around 0x198 and 6C118.
    print("\n===== 6B4F8 stores mentioning 0x198 / 0x1C0 / 0x948")
    for addr in range(0x8006B4F8, 0x8006BD68, 4):
        w = load_u32(data, addr)
        line = dis(w, addr)
        if "0x198" in line or "408" in line or "0x1c0" in line.lower() or "448" in line or "0x948" in line:
            print(f"  {addr:08X}  {w:08X}  {line}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
