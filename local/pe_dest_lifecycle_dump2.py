#!/usr/bin/env python3
"""Follow-up dump: 3F3C4 gate, 6BECC JT, mode 9/10, +0xEC, CE2+8."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
HDR = 0x800
PE_IMG_LBA = 1013
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")

REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def exe_off(addr: int) -> int:
    return addr - TADDR + HDR


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


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
            9: f"jalr {REGS[rd]}, {REGS[rs]}",
            0x21: f"addu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x23: f"subu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x24: f"and {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x25: f"or {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x26: f"xor {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2A: f"slt {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2B: f"sltu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
        }
        return names.get(fn, f"spec fn={fn:#x}")
    if op == 1:
        names = {0: "bltz", 1: "bgez", 16: "bltzal", 17: "bgezal"}
        return f"{names.get(rt, f'regimm{rt}')} {REGS[rs]}, {addr + 4 + simm * 4:#x}"
    if op == 2:
        return f"j {tgt:#x}"
    if op == 3:
        return f"jal {tgt:#x}"
    if op == 4:
        return f"beq {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}"
    if op == 5:
        return f"bne {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}"
    if op == 6:
        return f"blez {REGS[rs]}, {addr + 4 + simm * 4:#x}"
    if op == 7:
        return f"bgtz {REGS[rs]}, {addr + 4 + simm * 4:#x}"
    if op == 8:
        return f"addi {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 9:
        return f"addiu {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0A:
        return f"slti {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0B:
        return f"sltiu {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0C:
        return f"andi {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0D:
        return f"ori {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0E:
        return f"xori {REGS[rt]}, {REGS[rs]}, {imm:#x}"
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
    if op == 0x29:
        return f"sh {REGS[rt]}, {simm}({REGS[rs]})"
    if op == 0x2B:
        return f"sw {REGS[rt]}, {simm}({REGS[rs]})"
    return f"op{op:02x} {word:08X}"


def dump_window(data: bytes, start: int, end: int, title: str) -> None:
    print(f"\n===== {title} {start:#x}..{end:#x} {(end - start) // 4}w")
    for addr in range(start, end, 4):
        w = load_u32(data, addr)
        print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")


def scan_xrefs_to(data: bytes, target: int) -> list[int]:
    hits = []
    want = (target & 0x0FFFFFFF) >> 2
    for addr in range(0x80010000, 0x801EE000, 4):
        w = load_u32(data, addr)
        if (w >> 26) == 3 and (w & 0x03FFFFFF) == want:
            hits.append(addr)
    return hits


def find_disc() -> pathlib.Path:
    pointer = ROOT / "local" / "pe_disc1.path"
    return pathlib.Path(pointer.read_text().strip().splitlines()[0].strip())


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            out += fh.read(FORM1_USER)
    return bytes(out)


def walk_script(blob: bytes, base: int, limit: int = 40) -> list[dict]:
    rows = []
    pc = base
    for _ in range(limit):
        if pc + 8 > len(blob):
            break
        word = struct.unpack_from("<I", blob, pc)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        span = 8 + argc * 4
        if pc + span > len(blob):
            break
        imms = [struct.unpack_from("<I", blob, pc + 8 + i * 4)[0] for i in range(argc)]
        rows.append({"rel": pc - base, "word": word, "op": op, "argc": argc, "imms": imms, "span": span})
        pc += span
    return rows


def main() -> int:
    exe = (ROOT / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1

    print("===== xrefs 3F074 / 6BD68 / 6BE4C")
    for name, va in (
        ("3F074", 0x8003F074),
        ("6BD68", 0x8006BD68),
        ("6BE4C", 0x8006BE4C),
        ("6EBB0", 0x8006EBB0),
        ("2B278", 0x8002B278),
        ("2BC74", 0x8002BC74),
        ("2CEE0", 0x8002CEE0),
        ("2CF24", 0x8002CF24),
    ):
        hits = scan_xrefs_to(exe, va)
        print(f"  {name}: {len(hits)} {[hex(h) for h in hits]}")

    dump_window(exe, 0x8003F3C4, 0x8003F420, "3F3C4 head through 3F074")
    dump_window(exe, 0x8003F650, 0x8003F750, "3F3C4 D280 load tail")

    print("\n===== 6BECC JT 0x800113B0")
    for i in range(7):
        tgt = load_u32(exe, 0x800113B0 + i * 4)
        print(f"  state {i} -> {tgt:#010x}")

    print("\n===== 6C1CC JT 0x800113D0")
    for i in range(8):
        tgt = load_u32(exe, 0x800113D0 + i * 4)
        print(f"  ed-32={i} (ed={i+32}) -> {tgt:#010x}")

    dump_window(exe, 0x8006BD68, 0x8006BECC, "func_8006BD68")

    dump_window(exe, 0x8002B200, 0x8002B2C0, "mode9 around 2B278")
    dump_window(exe, 0x8002BC00, 0x8002BCC0, "mode10 around 2BC74")
    dump_window(exe, 0x8002CE80, 0x8002CEE0, "before 2CEE0")

    # +0xEC (236) stores: sb to overlay+0xEC
    print("\n===== sb to overlay+0xEC (scan sw/sb 236 near 800B0CD8 materializations)")
    # scan sb rt, 236(rs) where rs was loaded as overlay
    for addr in range(0x80010000, 0x801EE000, 4):
        w = load_u32(exe, addr)
        if (w >> 26) != 0x28:
            continue
        rs = (w >> 21) & 31
        rt = (w >> 16) & 31
        soff = w & 0xFFFF
        if soff >= 0x8000:
            soff -= 0x10000
        if soff == 236:
            print(f"  {addr:08X}  {dis(w, addr)}")

    # 6B4F8 return / completion
    dump_window(exe, 0x8006B930, 0x8006BD68, "6B4F8 after Writer A")

    # CE2+8
    disc = find_disc()
    ce = read_form1(disc, PE_IMG_LBA + 396, 32)
    body = ce[8 : 8 + 23864]
    print(f"\n===== CE2+8 body {len(body)} sha={hashlib.sha256(body).hexdigest()}")
    print(f"  first 64 words:")
    for i in range(0, 64, 4):
        w = struct.unpack_from("<I", body, i)[0]
        print(f"    +{i:02X} {w:08X}")
    # look for pointer-like words (0x00xxxxxx in range)
    ptrs = []
    for i in range(0, min(len(body), 512), 4):
        w = struct.unpack_from("<I", body, i)[0]
        if 8 <= (w & 0xFFFFFF) < 23864 and (w >> 24) in (0, 0x80):
            ptrs.append((i, w))
    print(f"  pointer-like in first 512B: {len(ptrs)}")
    for i, w in ptrs[:40]:
        print(f"    +{i:03X} {w:08X}")

    # M0367I scripts first-visit
    s0, s1, s2 = 33, 170, 78
    m367 = read_form1(disc, PE_IMG_LBA + 0x15050 + s0 + s1, s2)
    LIST = 0x1B314
    print("\n===== M0367I type scripts first 20")
    for typ in range(5):
        rel = struct.unpack_from("<I", m367, LIST + 8 + typ * 4)[0]
        base = LIST + rel
        rows = walk_script(m367, base, 24)
        print(f"\n  type {typ} base={base:#x}")
        for r in rows:
            print(
                f"    +{r['rel']:04X} op={r['op']:#06x} argc={r['argc']} "
                f"imms={[hex(x) for x in r['imms']]}"
            )

    # 6B4F8: does it compare dest?
    print("\n===== 6B4F8 beq/bne/j")
    for addr in range(0x8006B4F8, 0x8006BD68, 4):
        w = load_u32(exe, addr)
        op = w >> 26
        if op in (2, 4, 5, 6, 7) or (op == 0 and (w & 63) == 8):
            print(f"  {addr:08X}  {dis(w, addr)}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
