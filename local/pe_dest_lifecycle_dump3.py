#!/usr/bin/env python3
"""Targeted facts: dest hdr CE2, 6E6A8, 0x9D, D1C4, mode JT, CE2+8."""
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
            0x2A: f"slt {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2B: f"sltu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
        }
        return names.get(fn, f"spec fn={fn:#x}")
    if op == 1:
        names = {0: "bltz", 1: "bgez"}
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


def packed_secs(packed: int) -> tuple[int, int, int]:
    return packed & 0xFF, (packed >> 8) & 0xFFF, packed >> 20


def walk12(blob: bytes, packed: int) -> list[dict]:
    count = packed >> 22
    rec = packed & 0x3FFFFF
    rows = []
    for i in range(count):
        off = rec + i * 12
        if off + 12 > len(blob):
            break
        size, ptrw, w8 = struct.unpack_from("<III", blob, off)
        rows.append(
            {
                "i": i,
                "off": off,
                "size": size,
                "ptr": ptrw & 0xFFFFFF,
                "idb": blob[off + 7],
                "ida": blob[off + 0xB],
                "b0": blob[ptrw & 0xFFFFFF] if (ptrw & 0xFFFFFF) < len(blob) else None,
            }
        )
    return rows


def dest_chunk2(exe: bytes, disc: pathlib.Path, idx: int) -> tuple[bytes, int]:
    rel = load_u32(exe, 0x80093378 + idx * 8)
    packed = load_u32(exe, 0x80093378 + idx * 8 + 4)
    s0, s1, s2 = packed_secs(packed)
    c2 = read_form1(disc, PE_IMG_LBA + rel + s0 + s1, s2)
    hdr = struct.unpack_from("<I", c2, 4)[0] & 0x3FFFFF
    return c2, hdr


def main() -> int:
    exe = (ROOT / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    disc = find_disc()

    print("===== dest hdr bytes 0-7 / CE2 candidate")
    for name, idx in (
        ("M0005I", 4),
        ("M0367I", 366),
        ("M0319I", 318),
        ("M0239I", 238),
        ("M0058I", 57),
        ("M0035I", 34),
        ("M0136I", 135),
    ):
        c2, hdr = dest_chunk2(exe, disc, idx)
        print(
            f"  {name} hdr@{hdr:#x} bytes={c2[hdr:hdr+8].hex()} "
            f"b0={c2[hdr]} b1={c2[hdr+1]} b2={c2[hdr+2]} b3={c2[hdr+3]}"
        )
        w30 = struct.unpack_from("<I", c2, hdr + 0x30)[0]
        print(f"    hdr+0x30={w30:08X} count={w30>>22} off={w30 & 0x3FFFFF:#x}")
        w20 = struct.unpack_from("<I", c2, hdr + 0x20)[0]
        print(f"    hdr+0x20={w20:08X} count={w20>>22} off={w20 & 0x3FFFFF:#x}")
        recs0c = walk12(c2, struct.unpack_from("<I", c2, hdr + 0x0C)[0])
        recs10 = walk12(c2, struct.unpack_from("<I", c2, hdr + 0x10)[0])
        print(
            f"    0C idBs={[r['idb'] for r in recs0c]} "
            f"WA n={len(recs10)} types={sorted({r['ida'] for r in recs10})}"
        )

    dump_window(exe, 0x8006E6A8, 0x8006E7E8, "func_8006E6A8")
    print("\n===== 6E6A8 xrefs", [hex(h) for h in scan_xrefs_to(exe, 0x8006E6A8)[:12]], "count", len(scan_xrefs_to(exe, 0x8006E6A8)))

    h9d = load_u32(exe, 0x800910A0 + 0x9D * 4)
    print(f"\n===== 0x9D handler {h9d:#x}")
    dump_window(exe, h9d, h9d + 0x80, "op 0x9D")

    dump_window(exe, 0x8001220C, 0x800123F0, "1220C D1C4=D280")

    print("\n===== D1C4 stores")
    for addr in range(0x80010000, 0x801EE000, 4):
        w = load_u32(exe, addr)
        if (w >> 26) != 0x2B:
            continue
        rs = (w >> 21) & 31
        soff = w & 0xFFFF
        if soff >= 0x8000:
            soff -= 0x10000
        if rs == 28 and 0x8009CD70 + soff == 0x8009D1C4:
            print(f"  {addr:08X} gp {dis(w, addr)}")
            continue
        for j in range(1, 6):
            ba = addr - j * 4
            if ba < 0x80010000:
                break
            lw = load_u32(exe, ba)
            if (lw >> 26) == 0x0F and ((lw >> 16) & 31) == rs:
                hi = lw & 0xFFFF
                if (hi << 16) + soff == 0x8009D1C4:
                    print(f"  {addr:08X} lui {dis(w, addr)}")

    # mode 9/10 function starts: walk back to addiu sp
    dump_window(exe, 0x8002B14C, 0x8002B200, "before mode9 body")
    dump_window(exe, 0x8002BB40, 0x8002BC00, "before mode10 body")

    # battle mode JT at 2B2BC
    print("\n===== gp+0x104 JT near 2B2BC")
    # find table
    w = load_u32(exe, 0x8002B2BC)
    print(f"  2B2BC {dis(w, 0x8002B2BC)}")
    dump_window(exe, 0x8002B2BC, 0x8002B2E0, "mode9 JT load")

    # 35038 +0x1AC empty path
    dump_window(exe, 0x80035190, 0x80035220, "35038 B0E70 copy")

    # CE2+8 more
    ce = read_form1(disc, PE_IMG_LBA + 396, 32)
    body = ce[8:8 + 23864]
    print("\n===== CE2+8 extended")
    print(f"  +00 {body[0]:02X} {body[1]:02X} {body[2]:02X} {body[3]:02X}")
    # TMD-like? 0x41 = TMD ID often
    for off in (0, 0x40, 0x80, 0xC0, 0x100, 0x140, 0x180, 0x1C0, 0x200):
        words = [f"{struct.unpack_from('<I', body, off+i)[0]:08X}" for i in range(0, 16, 4)]
        print(f"  +{off:03X} {' '.join(words)}")
    # look at 362B8 first stores from +0x1AC
    dump_window(exe, 0x800362B8, 0x80036340, "362B8 head")

    # 6B4F8 return
    dump_window(exe, 0x8006BD20, 0x8006BD68, "6B4F8 epilogue")

    # overlay +0x154 writers
    print("\n===== sw to overlay+0x154 (340)")
    for addr in range(0x80010000, 0x801EE000, 4):
        w = load_u32(exe, addr)
        if (w >> 26) != 0x2B:
            continue
        soff = w & 0xFFFF
        if soff >= 0x8000:
            soff -= 0x10000
        if soff == 340:
            print(f"  {addr:08X}  {dis(w, addr)}")

    # 6C1CC callers context
    dump_window(exe, 0x80024A3C, 0x80024A90, "24A3C state0 jal 6C1CC")

    # window shas
    def wsha(a, b):
        return hashlib.sha256(exe[exe_off(a):exe_off(b)]).hexdigest()

    print("\n===== window SHAs")
    for a, b, n in (
        (0x8006B35C, 0x8006B4F8, "6B35C"),
        (0x8006B4F8, 0x8006BD68, "6B4F8"),
        (0x8006BECC, 0x8006C1CC, "6BECC"),
        (0x8006C0E4, 0x8006C174, "WriterB"),
        (0x8006B84C, 0x8006B880, "WriterA"),
        (0x8002B24C, 0x8002B298, "mode9"),
        (0x8002BC44, 0x8002BC7C, "mode10"),
        (0x8002CEE0, 0x8002CF2C, "mode7"),
        (0x80017BB4, 0x80017C00, "op31"),
        (0x80019450, h9d + 0x40 if h9d else 0x80019490, "op9d"),
    ):
        print(f"  {n} {a:#x}..{b:#x} {(b-a)//4}w {wsha(a,b)}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
