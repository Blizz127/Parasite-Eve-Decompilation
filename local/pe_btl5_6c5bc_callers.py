#!/usr/bin/env python3
"""PE-BTL5 research: 6C5BC caller identity + body control flow.

Prove whether field tick 3F3C4 → 35558 → 35B24 reaches 6C5BC on the
NYPD 0x55 path, and map 6C5BC returns / +0xEE / +0xE stores.
"""
from __future__ import annotations

import hashlib
import struct
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800
TEXT_START = 0x8001220C
TEXT_END = 0x80090000

REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def va2off(va: int) -> int:
    return va - LOAD + EXE_HDR


def word_at(exe: bytes, va: int) -> int:
    return struct.unpack_from("<I", exe, va2off(va))[0]


def simm16(imm: int) -> int:
    return imm - 0x10000 if imm >= 0x8000 else imm


def jal_target(w: int) -> int:
    return ((w & 0x03FFFFFF) << 2) | 0x80000000


def decode(w: int) -> dict:
    return {
        "op": w >> 26,
        "rs": (w >> 21) & 0x1F,
        "rt": (w >> 16) & 0x1F,
        "rd": (w >> 11) & 0x1F,
        "sa": (w >> 6) & 0x1F,
        "fn": w & 0x3F,
        "imm": w & 0xFFFF,
        "simm": simm16(w & 0xFFFF),
        "raw": w,
    }


def fmt(va: int, w: int) -> str:
    d = decode(w)
    op, rs, rt, rd, fn = d["op"], d["rs"], d["rt"], d["rd"], d["fn"]
    imm, simm = d["imm"], d["simm"]
    if op == 0x0F:
        return f"{va:08X}  lui     ${REGS[rt]}, {imm:#x}"
    if op == 0x09:
        return f"{va:08X}  addiu   ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 0x0D:
        return f"{va:08X}  ori     ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0C:
        return f"{va:08X}  andi    ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x03:
        return f"{va:08X}  jal     {jal_target(w):#010x}"
    if op == 0x02:
        return f"{va:08X}  j       {jal_target(w):#010x}"
    if op == 0x04:
        return f"{va:08X}  beq     ${REGS[rs]}, ${REGS[rt]}, {va + 4 + simm * 4:#010x}"
    if op == 0x05:
        return f"{va:08X}  bne     ${REGS[rs]}, ${REGS[rt]}, {va + 4 + simm * 4:#010x}"
    if op == 0x06:
        return f"{va:08X}  blez    ${REGS[rs]}, {va + 4 + simm * 4:#010x}"
    if op == 0x07:
        return f"{va:08X}  bgtz    ${REGS[rs]}, {va + 4 + simm * 4:#010x}"
    if op == 0x01 and rt == 0:
        return f"{va:08X}  bltz    ${REGS[rs]}, {va + 4 + simm * 4:#010x}"
    if op == 0x01 and rt == 1:
        return f"{va:08X}  bgez    ${REGS[rs]}, {va + 4 + simm * 4:#010x}"
    if op == 0x28:
        return f"{va:08X}  sb      ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x29:
        return f"{va:08X}  sh      ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x2B:
        return f"{va:08X}  sw      ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x24:
        return f"{va:08X}  lbu     ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x20:
        return f"{va:08X}  lb      ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x23:
        return f"{va:08X}  lw      ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x21:
        return f"{va:08X}  lh      ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x25:
        return f"{va:08X}  lhu     ${REGS[rt]}, {simm:#x}(${REGS[rs]})"
    if op == 0x00:
        if fn == 0x08:
            return f"{va:08X}  jr      ${REGS[rs]}"
        if fn == 0x09:
            return f"{va:08X}  jalr    ${REGS[rd]}, ${REGS[rs]}"
        if fn == 0x21:
            if rs == 0:
                return f"{va:08X}  move    ${REGS[rd]}, ${REGS[rt]}"
            if rt == 0:
                return f"{va:08X}  move    ${REGS[rd]}, ${REGS[rs]}"
            return f"{va:08X}  addu    ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x00:
            return f"{va:08X}  nop" if w == 0 else f"{va:08X}  sll     ${REGS[rd]}, ${REGS[rt]}, {d['sa']}"
        if fn == 0x23:
            return f"{va:08X}  subu    ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x24:
            return f"{va:08X}  and     ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x25:
            return f"{va:08X}  or      ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x2A:
            return f"{va:08X}  slt     ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x2B:
            return f"{va:08X}  sltu    ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x02:
            return f"{va:08X}  srl     ${REGS[rd]}, ${REGS[rt]}, {d['sa']}"
        if fn == 0x03:
            return f"{va:08X}  sra     ${REGS[rd]}, ${REGS[rt]}, {d['sa']}"
        if fn == 0x0C:
            return f"{va:08X}  syscall"
    if op == 0x0A:
        return f"{va:08X}  slti    ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 0x0B:
        return f"{va:08X}  sltiu   ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0E:
        return f"{va:08X}  xori    ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    return f"{va:08X}  .word   {w:#010x}"


def jals_in(exe: bytes, lo: int, hi: int) -> list[tuple[int, int]]:
    out = []
    for va in range(lo, hi, 4):
        w = word_at(exe, va)
        if (w >> 26) == 0x03:
            out.append((va, jal_target(w)))
    return out


def sites_to(exe: bytes, target: int) -> list[int]:
    hits = []
    for va in range(TEXT_START, TEXT_END, 4):
        w = word_at(exe, va)
        if (w >> 26) == 0x03 and jal_target(w) == target:
            hits.append(va)
    return hits


def main() -> int:
    exe = (Path(__file__).resolve().parents[1] / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1

    print("=== 35558 prologue / nearby ===")
    for va in range(0x80035540, 0x800355A0, 4):
        mark = ">>" if va in (0x80035558, 0x80035560) else "  "
        print(f"{mark} {fmt(va, word_at(exe, va))}")

    print("\n=== 35558 jals through 35B50 ===")
    for va, tgt in jals_in(exe, 0x80035558, 0x80035C80):
        delay = fmt(va + 4, word_at(exe, va + 4))
        print(f"  {fmt(va, word_at(exe, va))}  delay {delay}")

    print("\n=== jal sites to 35558 ===")
    print([hex(x) for x in sites_to(exe, 0x80035558)])
    print("jal sites to 35560", [hex(x) for x in sites_to(exe, 0x80035560)])

    print("\n=== 3F3C4 prologue + first 80 words of jals ===")
    for va in range(0x8003F3C4, 0x8003F3C4 + 32, 4):
        print("  ", fmt(va, word_at(exe, va)))
    print("--- all jals in 3F3C4 window 0x8003F3C4..0x8003F800 ---")
    for va, tgt in jals_in(exe, 0x8003F3C4, 0x8003F800):
        print(f"  {fmt(va, word_at(exe, va))}  delay {fmt(va + 4, word_at(exe, va + 4))}")

    print("\n=== 3F4E0..3F510 (known 65400 / 35558 sites) ===")
    for va in range(0x8003F4D0, 0x8003F520, 4):
        print("  ", fmt(va, word_at(exe, va)))

    print("\n=== 3F074 prologue and jr-ra hunt ===")
    for va in range(0x8003F060, 0x8003F0A0, 4):
        print("  ", fmt(va, word_at(exe, va)))
    # find jr ra after 3F074
    for va in range(0x8003F074, 0x8003F400, 4):
        w = word_at(exe, va)
        d = decode(w)
        if d["op"] == 0x00 and d["fn"] == 0x08 and d["rs"] == 31:
            print(f"  jr $ra at {va:#x}")
            for p in range(va - 8, va + 16, 4):
                print("   ", fmt(p, word_at(exe, p)))
            break

    print("\n=== jal sites to 3F074 / 3F3C4 / 6C1CC / 6D60C / 24A3C ===")
    for name, addr in [
        ("3F074", 0x8003F074),
        ("3F3C4", 0x8003F3C4),
        ("6C1CC", 0x8006C1CC),
        ("6C1D4", 0x8006C1D4),
        ("6D60C", 0x8006D60C),
        ("24A3C", 0x80024A3C),
        ("35558", 0x80035558),
        ("6C5BC", 0x8006C5BC),
        ("6C4C4", 0x8006C4C4),
        ("6BECC", 0x8006BECC),
    ]:
        hits = sites_to(exe, addr)
        print(f"  {name} {addr:#010x}  n={len(hits)}  { [hex(x) for x in hits[:12]] }")

    print("\n=== 6D60C prologue + jals (first 80) + +0xE stores ===")
    for va in range(0x8006D60C, 0x8006D60C + 48, 4):
        print("  ", fmt(va, word_at(exe, va)))
    # estimate end
    end = None
    for va in range(0x8006D60C, 0x8006D60C + 0x800, 4):
        w = word_at(exe, va)
        d = decode(w)
        if d["op"] == 0x00 and d["fn"] == 0x08 and d["rs"] == 31:
            nxt = word_at(exe, va + 8)
            nd = decode(nxt)
            if nd["op"] == 0x09 and nd["rs"] == 29 and nd["rt"] == 29 and nd["simm"] < 0:
                end = va + 8
                break
            end = va + 8
    print(f"  estimated end {end and hex(end)} words={end and (end - 0x8006D60C)//4}")
    if end:
        print("  jals:")
        for va, tgt in jals_in(exe, 0x8006D60C, end):
            print(f"    {fmt(va, word_at(exe, va))}")
        print("  sb/sh/sw offset 0xE:")
        for va in range(0x8006D60C, end, 4):
            w = word_at(exe, va)
            op = w >> 26
            if op in (0x28, 0x29, 0x2B) and (w & 0xFFFF) == 0xE:
                print("   ", fmt(va, w))

    print("\n=== 6C1CC start + 6C358 state context ===")
    for va in range(0x8006C1C0, 0x8006C250, 4):
        print("  ", fmt(va, word_at(exe, va)))

    print("\n=== 6C5BC control: branches, +0xEE stores, v0 materialization ===")
    FN, END = 0x8006C5BC, 0x8006CC68
    print("--- +0xEE sb ---")
    for va in range(FN, END, 4):
        w = word_at(exe, va)
        if (w >> 26) == 0x28 and (w & 0xFFFF) == 0xEE:
            print("  ", fmt(va, w))
            for p in range(va - 12, va + 8, 4):
                print("     ", fmt(p, word_at(exe, p)))
    print("--- lbu +0xEE ---")
    for va in range(FN, END, 4):
        w = word_at(exe, va)
        if (w >> 26) == 0x24 and (w & 0xFFFF) == 0xEE:
            print("  ", fmt(va, w))
    print("--- addiu v0, zero, imm (return-ish) ---")
    for va in range(FN, END, 4):
        w = word_at(exe, va)
        d = decode(w)
        if d["op"] == 0x09 and d["rs"] == 0 and d["rt"] == 2:
            print("  ", fmt(va, w))
    print("--- j / jr ---")
    for va in range(FN, END, 4):
        w = word_at(exe, va)
        d = decode(w)
        if d["op"] == 0x02:
            print("  ", fmt(va, w), "delay", fmt(va + 4, word_at(exe, va + 4)))
        if d["op"] == 0x00 and d["fn"] == 0x08:
            print("  ", fmt(va, w))

    print("\n=== 6C5BC first 80 words (body after prologue) ===")
    for va in range(0x8006C5BC, 0x8006C5BC + 0x140, 4):
        print("  ", fmt(va, word_at(exe, va)))

    print("\n=== 144FC state 0x38 window (jal 6D60C) ===")
    for va in range(0x80014590, 0x800145E0, 4):
        print("  ", fmt(va, word_at(exe, va)))

    print("\n=== 35B24 reachability: is it after actor-list bne, unconditional? ===")
    for va in range(0x80035AE0, 0x80035B80, 4):
        print("  ", fmt(va, word_at(exe, va)))

    print("\n=== 35558 first 40 words ===")
    for va in range(0x80035558, 0x80035558 + 160, 4):
        print("  ", fmt(va, word_at(exe, va)))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
