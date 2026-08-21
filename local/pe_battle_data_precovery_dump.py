#!/usr/bin/env python3
"""One-shot dump of live NYPD battle data contracts from EXE + PE.IMG."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
PE_IMG_LBA = 1013
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048
M0005I_REL = 0x266A
M0005I_PACKED = 0x0600A921
M0005I_CHUNK2 = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
M0367I_REL = 0x15050
M0367I_PACKED = 0x04E0AA21
M0367I_CHUNK2 = "ab9af4f446a6f9a1f8f79f1f80b862c4d516beddbede1229afab2dfc30b44b1e"
LIST_OFF = 0x202A4
DESC_OFF = 0x25014
CE214_SHA = "db785a5eea1f78f945284e57955605326f5856adda1ebc83d0a95d7a0142b1b2"

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
        return f"beq {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}"
    if op == 5:
        return f"bne {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}"
    if op == 8:
        return f"addi {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 9:
        return f"addiu {REGS[rt]}, {REGS[rs]}, {simm}"
    if op == 0x0C:
        return f"andi {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0D:
        return f"ori {REGS[rt]}, {REGS[rs]}, {imm:#x}"
    if op == 0x0F:
        return f"lui {REGS[rt]}, {REGS[rs] and 0 or ''}{imm:#x}".replace(" 0x", " ")
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


def find_disc(root: pathlib.Path) -> pathlib.Path:
    path = pathlib.Path(root.joinpath("local/pe_disc1.path").read_text().strip().splitlines()[0].strip())
    assert path.is_file(), path
    return path


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            out += fh.read(FORM1_USER)
    return bytes(out)


def decode_word(w: int) -> tuple[int, int, int]:
    return w & 0x1FFF, (w >> 13) & 0xF, w >> 17


def walk_script(blob: bytes, off: int, limit: int = 64) -> list[dict]:
    rows = []
    pc = off
    end = min(len(blob), off + 0x4000)
    for _ in range(limit):
        if pc + 4 > end:
            break
        word = struct.unpack_from("<I", blob, pc)[0]
        op, argc, kinds = decode_word(word)
        imm_off = pc + 4
        extra = 0
        if argc > 5:
            extra = 4
        imms = []
        for i in range(argc):
            io = imm_off + extra + i * 4
            if io + 4 > end:
                break
            imms.append(struct.unpack_from("<I", blob, io)[0])
        rows.append(
            {
                "pc": pc,
                "rel": pc - off,
                "word": word,
                "op": op,
                "argc": argc,
                "kinds": kinds,
                "imms": imms,
            }
        )
        pc = imm_off + extra + argc * 4
    return rows


def dump_window(data: bytes, start: int, end: int, title: str) -> None:
    print(f"\n===== {title} {start:#x}..{end:#x} {(end-start)//4}w")
    for addr in range(start, end, 4):
        w = load_u32(data, addr)
        print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")


def walk_dir12(blob: bytes, packed: int) -> list[dict]:
    count = packed >> 22
    rec = packed & 0x3FFFFF
    rows = []
    for i in range(count):
        off = rec + i * 12
        size, ptrw, w8 = struct.unpack_from("<III", blob, off)
        ptr = ptrw & 0xFFFFFF
        rows.append(
            {
                "i": i,
                "off": off,
                "size": size,
                "ptr": ptrw & 0xFFFFFF,
                "idb": blob[off + 7],
                "ida": blob[off + 0xB],
                "ptrw": ptrw,
                "w8": w8,
                "byte2": blob[ptr + 2] if ptr + 3 <= len(blob) else None,
                "byte0": blob[ptr] if ptr < len(blob) else None,
                "byte1": blob[ptr + 1] if ptr + 2 <= len(blob) else None,
            }
        )
    return rows


def main() -> int:
    root = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
    data = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1
    disc = find_disc(root)

    dump_window(data, 0x8006B35C, 0x8006B4F8, "func_8006B35C")
    dump_window(data, 0x8006B7C0, 0x8006B828, "6B4F8 hdr+0x0C B0E70")
    dump_window(data, 0x8006C0E0, 0x8006C180, "6C0E0/6C118 B0E70 bind")
    dump_window(data, 0x800362B8, 0x800363F4, "func_800362B8")

    print("\n===== 3F074 first jals")
    for addr in range(0x8003F074, 0x8003F120, 4):
        w = load_u32(data, addr)
        if w >> 26 == 3:
            print(f"  {addr:08X} jal {jal_target(w):#x}")

    print("\n===== D_800910A0 selected ops")
    for op in (0, 1, 2, 4, 5, 8, 0xB, 0xC, 0xE, 0x11, 0x12, 0x14, 0x1A, 0x1C, 0x1E, 0x1F, 0x20, 0x2E, 0x2F, 0x30, 0x40, 0x41, 0x4B, 0x4E, 0x54, 0x59, 0x5A, 0x5E, 0x64, 0x65, 0x6A, 0x6B, 0x6F, 0x70, 0x77, 0x79, 0x82, 0x84, 0x85, 0x86, 0x88, 0x89, 0x9B, 0x9C, 0xAA, 0xAB, 0xB7, 0xCE, 0xD9, 0xDC, 0xE1, 0xED):
        h = load_u32(data, 0x800910A0 + op * 4)
        print(f"  op {op:#06x} -> {h:#010x}")

    sec0, sec1, sec2 = M0005I_PACKED & 0xFF, (M0005I_PACKED >> 8) & 0xFFF, M0005I_PACKED >> 20
    chunk0 = read_form1(disc, PE_IMG_LBA + M0005I_REL, sec0)
    chunk1 = read_form1(disc, PE_IMG_LBA + M0005I_REL + sec0, sec1)
    chunk2 = read_form1(disc, PE_IMG_LBA + M0005I_REL + sec0 + sec1, sec2)
    full = chunk0 + chunk1 + chunk2
    print("\n===== m0005i packages")
    print(f"  chunk0 {len(chunk0)} {hashlib.sha256(chunk0).hexdigest()}")
    print(f"  chunk1 {len(chunk1)} {hashlib.sha256(chunk1).hexdigest()}")
    print(f"  chunk2 {len(chunk2)} {hashlib.sha256(chunk2).hexdigest()}")
    print(f"  full   {len(full)} {hashlib.sha256(full).hexdigest()}")
    assert hashlib.sha256(chunk2).hexdigest() == M0005I_CHUNK2

    hdr = struct.unpack_from("<I", chunk2, 4)[0] & 0x3FFFFF
    print(f"\n===== m0005i chunk2 hdr @{hdr:#x}")
    for off in range(0, 0x40, 4):
        w = struct.unpack_from("<I", chunk2, hdr + off)[0]
        print(f"  hdr+{off:02X} = {w:08X} count={w>>22} off={w & 0x3FFFFF:#x}")

    word0c = struct.unpack_from("<I", chunk2, hdr + 0x0C)[0]
    print("\n===== hdr+0x0C B0E70 records")
    for row in walk_dir12(chunk2, word0c):
        ptr = row["ptr"]
        payload = chunk2[ptr:ptr + min(row["size"], 32)] if row["size"] else chunk2[ptr:ptr + 16]
        print(
            f"  [{row['i']}] @{row['off']:#x} idA={row['ida']} idB={row['idb']} "
            f"ptr={ptr:#x} size={row['size']} b0={row['byte0']} b1={row['byte1']} "
            f"b2={row['byte2']} head={payload[:16].hex()}"
        )

    word10 = struct.unpack_from("<I", chunk2, hdr + 0x10)[0]
    print("\n===== hdr+0x10 Writer A records")
    for row in walk_dir12(chunk2, word10):
        ptr = row["ptr"]
        head = chunk2[ptr:ptr + 8].hex() if ptr + 8 <= len(chunk2) else ""
        print(
            f"  [{row['i']}] type={row['ida']} cmd={row['idb']:#x} ptr={ptr:#x} "
            f"size={row['size']} b0={row['byte0']} b1={row['byte1']} b2={row['byte2']} head={head}"
        )

    print("\n===== 12574 list")
    n = struct.unpack_from("<I", chunk2, LIST_OFF + 4)[0]
    print(f"  count={n} word0={struct.unpack_from('<I', chunk2, LIST_OFF)[0]:#x}")
    for i in range(n):
        w = struct.unpack_from("<I", chunk2, LIST_OFF + 8 + i * 4)[0]
        script = LIST_OFF + w
        print(f"  type[{i}] rel={w:#x} script={script:#x}")

    print("\n===== 125E0 desc")
    print(f"  bytes={chunk2[DESC_OFF:DESC_OFF+8].hex()}")

    for typ, rel_i in [(0, 0), (1, 1), (3, 3), (5, 5), (6, 6)]:
        rel = struct.unpack_from("<I", chunk2, LIST_OFF + 8 + typ * 4)[0]
        off = LIST_OFF + rel
        rows = walk_script(chunk2, off, 24)
        print(f"\n===== type {typ} script @{off:#x} first {len(rows)}")
        for r in rows:
            print(
                f"  +{r['rel']:04X} {r['word']:08X} op={r['op']:#06x} "
                f"argc={r['argc']} kinds={r['kinds']:05x} imms={[hex(x) for x in r['imms']]}"
            )

    bank = read_form1(disc, PE_IMG_LBA + 396, 32)
    print(f"\n===== CE2=14 bank {len(bank)} {hashlib.sha256(bank).hexdigest()}")
    assert hashlib.sha256(bank).hexdigest() == CE214_SHA
    section = struct.unpack_from("<I", bank, 4)[0]
    packed = struct.unpack_from("<I", bank, section + 0x10)[0]
    print(f"  section={section:#x} packed={packed:08X} count={packed>>22}")
    for row in walk_dir12(bank, packed):
        ptr = row["ptr"]
        sha = hashlib.sha256(bank[ptr:ptr + row["size"]]).hexdigest() if row["size"] else ""
        print(
            f"  [{row['i']}] idA={row['ida']} idB={row['idb']} ptr={ptr:#x} "
            f"size={row['size']} enc={row['byte0']} bonesm1={row['byte1']} "
            f"frames={row['byte2']} sha={sha[:16]}"
        )

    s0, s1, s2 = M0367I_PACKED & 0xFF, (M0367I_PACKED >> 8) & 0xFFF, M0367I_PACKED >> 20
    m367_2 = read_form1(disc, PE_IMG_LBA + M0367I_REL + s0 + s1, s2)
    print(f"\n===== M0367I chunk2 {len(m367_2)} {hashlib.sha256(m367_2).hexdigest()}")
    assert hashlib.sha256(m367_2).hexdigest() == M0367I_CHUNK2
    hdr367 = struct.unpack_from("<I", m367_2, 4)[0] & 0x3FFFFF
    print(f"  hdr @{hdr367:#x}")
    for off in range(0, 0x40, 4):
        w = struct.unpack_from("<I", m367_2, hdr367 + off)[0]
        print(f"  hdr+{off:02X} = {w:08X} count={w>>22} off={w & 0x3FFFFF:#x}")
    word10_367 = struct.unpack_from("<I", m367_2, hdr367 + 0x10)[0]
    print("  Writer A rows:")
    for row in walk_dir12(m367_2, word10_367):
        print(f"    type={row['ida']} cmd={row['idb']:#x} ptr={row['ptr']:#x} size={row['size']}")
    word0c_367 = struct.unpack_from("<I", m367_2, hdr367 + 0x0C)[0]
    print("  hdr+0x0C rows:")
    for row in walk_dir12(m367_2, word0c_367):
        print(f"    idA={row['ida']} idB={row['idb']} ptr={row['ptr']:#x} size={row['size']}")

    print("\n===== D_800930D8[0..31]")
    for i in range(32):
        h = struct.unpack_from("<H", data, exe_off(0x800930D8 + i * 2))[0]
        print(f"  [{i:2d}] {h}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
