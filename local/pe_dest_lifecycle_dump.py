#!/usr/bin/env python3
"""Destination / resource-lifecycle dump from EXE + PE.IMG.

Independent of the runtime lane. No UE5 / gameplay edits.
"""
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


def load_u16(data: bytes, addr: int) -> int:
    return struct.unpack_from("<H", data, exe_off(addr))[0]


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
            4: f"sllv {REGS[rd]}, {REGS[rt]}, {REGS[rs]}",
            6: f"srlv {REGS[rd]}, {REGS[rt]}, {REGS[rs]}",
            7: f"srav {REGS[rd]}, {REGS[rt]}, {REGS[rs]}",
            8: f"jr {REGS[rs]}",
            9: f"jalr {REGS[rd]}, {REGS[rs]}",
            0x0C: "syscall",
            0x10: f"mfhi {REGS[rd]}",
            0x12: f"mflo {REGS[rd]}",
            0x18: f"mult {REGS[rs]}, {REGS[rt]}",
            0x19: f"multu {REGS[rs]}, {REGS[rt]}",
            0x1A: f"div {REGS[rs]}, {REGS[rt]}",
            0x1B: f"divu {REGS[rs]}, {REGS[rt]}",
            0x20: f"add {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x21: f"addu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x22: f"sub {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x23: f"subu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x24: f"and {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x25: f"or {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x26: f"xor {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x27: f"nor {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
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


def find_disc() -> pathlib.Path:
    pointer = ROOT / "local" / "pe_disc1.path"
    path = pathlib.Path(pointer.read_text().strip().splitlines()[0].strip())
    assert path.is_file(), path
    return path


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            chunk = fh.read(FORM1_USER)
            assert len(chunk) == FORM1_USER
            out += chunk
    return bytes(out)


def dump_window(data: bytes, start: int, end: int, title: str) -> None:
    print(f"\n===== {title} {start:#x}..{end:#x} {(end - start) // 4}w")
    for addr in range(start, end, 4):
        w = load_u32(data, addr)
        print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")


def scan_jals(data: bytes, start: int, end: int) -> list[tuple[int, int]]:
    rows = []
    for addr in range(start, end, 4):
        w = load_u32(data, addr)
        if w >> 26 == 3:
            rows.append((addr, jal_target(w)))
    return rows


def scan_xrefs_to(data: bytes, target: int, text_start: int = 0x80010000, text_end: int = 0x801EE000) -> list[int]:
    hits = []
    want = (target & 0x0FFFFFFF) >> 2
    for addr in range(text_start, text_end, 4):
        w = load_u32(data, addr)
        if (w >> 26) == 3 and (w & 0x03FFFFFF) == want:
            hits.append(addr)
    return hits


def scan_stores_imm(data: bytes, dest_va: int, imm: int) -> list[dict]:
    """Find addiu $rt,$zero,imm then later sw $rt to dest_va (lui/lo or gp)."""
    hits = []
    # addiu rt, zero, imm
    for addr in range(0x80010000, 0x801EE000, 4):
        w = load_u32(data, addr)
        if (w >> 26) != 9:
            continue
        rs = (w >> 21) & 31
        rt = (w >> 16) & 31
        simm = w & 0xFFFF
        if simm >= 0x8000:
            simm -= 0x10000
        if rs != 0 or simm != imm:
            continue
        # look ahead 12 words for sw rt, dest
        for i in range(1, 16):
            wa = addr + i * 4
            sw = load_u32(data, wa)
            if (sw >> 26) != 0x2B:
                continue
            srs = (sw >> 21) & 31
            srt = (sw >> 16) & 31
            soff = sw & 0xFFFF
            if soff >= 0x8000:
                soff -= 0x10000
            if srt != rt:
                continue
            # gp-relative: gp=0x8009CD70, dest = gp+off
            if srs == 28:
                if 0x8009CD70 + soff == dest_va:
                    hits.append({"li": addr, "sw": wa, "via": "gp", "rt": rt})
            # lui+addiu/lo pattern: look back for lui srs
            for j in range(1, 8):
                ba = wa - j * 4
                if ba < 0x80010000:
                    break
                lw = load_u32(data, ba)
                if (lw >> 26) == 0x0F and ((lw >> 16) & 31) == srs:
                    hi = lw & 0xFFFF
                    computed = (hi << 16) + soff
                    if computed == dest_va:
                        hits.append({"li": addr, "sw": wa, "via": f"lui@{ba:#x}", "rt": rt})
    return hits


def decode_token(exe: bytes, token: int) -> str:
    out = []
    for i in range(6):
        idx = (token >> ((5 - i) * 5 + 2)) & 0x1F
        ch = exe[exe_off(0x800930B4) + idx]
        if 97 <= ch < 123:
            ch -= 32
        out.append(chr(ch))
    return "".join(out)


def walk12(blob: bytes, packed: int) -> list[dict]:
    count = packed >> 22
    rec = packed & 0x3FFFFF
    rows = []
    for i in range(count):
        off = rec + i * 12
        if off + 12 > len(blob):
            break
        size, ptrw, w8 = struct.unpack_from("<III", blob, off)
        ptr = ptrw & 0xFFFFFF
        payload = blob[ptr : ptr + size] if ptr + size <= len(blob) else b""
        rows.append(
            {
                "i": i,
                "off": off,
                "size": size,
                "ptr": ptr,
                "idb": blob[off + 7],
                "ida": blob[off + 0xB],
                "sha256": hashlib.sha256(payload).hexdigest() if payload else "",
                "b0": blob[ptr] if ptr < len(blob) else None,
                "b1": blob[ptr + 1] if ptr + 1 < len(blob) else None,
                "b2": blob[ptr + 2] if ptr + 2 < len(blob) else None,
            }
        )
    return rows


def main() -> int:
    exe = (ROOT / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    disc = find_disc()

    print("===== 3F074 jals")
    for addr, tgt in scan_jals(exe, 0x8003F074, 0x8003F3C4):
        print(f"  {addr:08X} jal {tgt:#010x}")

    print("\n===== 3F3C4 jals")
    for addr, tgt in scan_jals(exe, 0x8003F3C4, 0x8003F750):
        print(f"  {addr:08X} jal {tgt:#010x}")

    print("\n===== xrefs to 6B35C / 6B4F8 / 6BECC / 6C1CC / 6C118")
    for name, va in (
        ("6B35C", 0x8006B35C),
        ("6B4F8", 0x8006B4F8),
        ("6BECC", 0x8006BECC),
        ("6C1CC", 0x8006C1CC),
        ("6C118", 0x8006C118),
        ("6C140", 0x8006C140),
        ("34FC4", 0x80034FC4),
        ("125E0", 0x800125E0),
        ("2CF24", 0x8002CF24),
        ("2CEE0", 0x8002CEE0),
    ):
        hits = scan_xrefs_to(exe, va)
        print(f"  {name} {va:#x}: {len(hits)} sites {[hex(h) for h in hits[:20]]}")

    dump_window(exe, 0x8006B35C, 0x8006B4F8, "func_8006B35C")
    dump_window(exe, 0x8006B4F8, 0x8006B79C, "func_8006B4F8 load prefix")
    dump_window(exe, 0x8006B79C, 0x8006BD68, "func_8006B4F8 publish+tail")

    print("\n===== 6BECC xrefs + window")
    dump_window(exe, 0x8006BECC, 0x8006C1CC, "func_8006BECC")
    dump_window(exe, 0x8006C1CC, 0x8006C4C4, "func_8006C1CC")

    print("\n===== D_8009D28C stores of 7/9/10")
    for imm in (7, 9, 10):
        hits = scan_stores_imm(exe, 0x8009D28C, imm)
        print(f"  value={imm} hits={len(hits)}")
        for h in hits:
            print(f"    li@{h['li']:#x} sw@{h['sw']:#x} via={h['via']} rt={h['rt']}")

    print("\n===== D_8009D280 writers (sw to D280)")
    # scan sw to 0x8009D280 via lui or gp+0x510
    hits280 = []
    for addr in range(0x80010000, 0x801EE000, 4):
        w = load_u32(exe, addr)
        if (w >> 26) != 0x2B:
            continue
        rs = (w >> 21) & 31
        rt = (w >> 16) & 31
        soff = w & 0xFFFF
        if soff >= 0x8000:
            soff -= 0x10000
        if rs == 28 and 0x8009CD70 + soff == 0x8009D280:
            hits280.append((addr, rt, "gp+0x510"))
            continue
        for j in range(1, 8):
            ba = addr - j * 4
            if ba < 0x80010000:
                break
            lw = load_u32(exe, ba)
            if (lw >> 26) == 0x0F and ((lw >> 16) & 31) == rs:
                hi = lw & 0xFFFF
                if (hi << 16) + soff == 0x8009D280:
                    hits280.append((addr, rt, f"lui@{ba:#x}"))
    print(f"  {len(hits280)} stores")
    for addr, rt, via in hits280:
        print(f"    sw@{addr:#x} rt={REGS[rt]} via={via}  {dis(load_u32(exe, addr), addr)}")

    print("\n===== D_8009D280 comparisons (lw then beq/bne)")
    # find lw from D280
    loads = []
    for addr in range(0x80010000, 0x801EE000, 4):
        w = load_u32(exe, addr)
        if (w >> 26) != 0x23:
            continue
        rs = (w >> 21) & 31
        rt = (w >> 16) & 31
        soff = w & 0xFFFF
        if soff >= 0x8000:
            soff -= 0x10000
        if rs == 28 and 0x8009CD70 + soff == 0x8009D280:
            loads.append((addr, rt, "gp"))
            continue
        for j in range(1, 8):
            ba = addr - j * 4
            if ba < 0x80010000:
                break
            lw = load_u32(exe, ba)
            if (lw >> 26) == 0x0F and ((lw >> 16) & 31) == rs:
                hi = lw & 0xFFFF
                if (hi << 16) + soff == 0x8009D280:
                    loads.append((addr, rt, f"lui@{ba:#x}"))
    print(f"  {len(loads)} loads")
    for addr, rt, via in loads:
        print(f"    lw@{addr:#x} rt={REGS[rt]} via={via}")

    print("\n===== 3F074 prefix around 6B35C/6B4F8/6BECC")
    dump_window(exe, 0x8003F074, 0x8003F250, "3F074 head")

    print("\n===== opcode 0x31 handler")
    h31 = load_u32(exe, 0x800910A0 + 0x31 * 4)
    print(f"  D_800910A0[0x31]={h31:#x}")
    dump_window(exe, h31, h31 + 0x80, "op 0x31")

    print("\n===== opcode 0x94 handler (mode read)")
    h94 = load_u32(exe, 0x800910A0 + 0x94 * 4)
    print(f"  D_800910A0[0x94]={h94:#x}")

    print("\n===== 2CEE0 mode-7 gate")
    dump_window(exe, 0x8002CEE0, 0x8002CF40, "2CEE0")

    # CE2=14 +0x8 body
    ce214 = read_form1(disc, PE_IMG_LBA + 396, 32)
    print(f"\n===== CE2=14 bank {len(ce214)} {hashlib.sha256(ce214).hexdigest()}")
    ce_hdr = struct.unpack_from("<I", ce214, 4)[0] & 0x3FFFFF
    print(f"  hdr @{ce_hdr:#x}")
    for off in range(0, 0x40, 4):
        w = struct.unpack_from("<I", ce214, ce_hdr + off)[0]
        print(f"  hdr+{off:02X} = {w:08X} count={w>>22} off={w & 0x3FFFFF:#x}")
    recs0c = walk12(ce214, struct.unpack_from("<I", ce214, ce_hdr + 0x0C)[0])
    print("  hdr+0x0C:")
    for r in recs0c:
        print(
            f"    i={r['i']} idA={r['ida']} idB={r['idb']} ptr={r['ptr']:#x} "
            f"size={r['size']} sha={r['sha256']}"
        )
        body = ce214[r["ptr"] : r["ptr"] + min(r["size"], 64)]
        print(f"      head64={body.hex()}")
        # dump first 16 words
        for i in range(0, min(64, r["size"]), 4):
            w = struct.unpack_from("<I", ce214, r["ptr"] + i)[0]
            print(f"      +{i:02X} {w:08X}")

    recs10 = walk12(ce214, struct.unpack_from("<I", ce214, ce_hdr + 0x10)[0])
    print(f"  hdr+0x10 Writer B dir count={len(recs10)}")
    for r in recs10:
        print(
            f"    i={r['i']} idA={r['ida']} idB={r['idb']:#x} ptr={r['ptr']:#x} "
            f"size={r['size']} enc={r['b0']} bones-1={r['b1']} frames={r['b2']}"
        )

    return 0


if __name__ == "__main__":
    sys.exit(main())
