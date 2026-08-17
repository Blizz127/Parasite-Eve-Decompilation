#!/usr/bin/env python3
"""PE-BTL6 research: EE=13 body and func_8006CC68.

Authority: Disc 1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
Does not invent CD completion, andi 0xFC, or 0x55 advance.
"""
from __future__ import annotations

import hashlib
import struct
from collections import Counter
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800
OVERLAY = 0x800B0CD8
BYTE_E = OVERLAY + 0xE
EE13 = 0x8006C9F8
FN_6C5BC_END = 0x8006CC68
FN_6CC68 = 0x8006CC68
TEXT_START = 0x8001220C
TEXT_END = 0x80090000

REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]
STORE_OPS = {0x28: "sb", 0x29: "sh", 0x2B: "sw"}
LOAD_OPS = {0x20: "lb", 0x24: "lbu", 0x21: "lh", 0x25: "lhu", 0x23: "lw"}


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


def dest_reg(d: dict) -> int | None:
    op, fn = d["op"], d["fn"]
    if op == 0x00 and fn in (
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x00, 0x02, 0x03, 0x04, 0x06, 0x07, 0x08, 0x09,
        0x0A, 0x0B, 0x2A, 0x2B, 0x10, 0x12,
    ):
        return d["rd"]
    if op in (0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
              0x20, 0x21, 0x23, 0x24, 0x25, 0x22, 0x26):
        return d["rt"]
    return None


def fmt(va: int, w: int, abs_reg: dict[int, int] | None = None) -> str:
    d = decode(w)
    op, rs, rt, rd, fn = d["op"], d["rs"], d["rt"], d["rd"], d["fn"]
    imm, simm = d["imm"], d["simm"]
    extra = ""
    if abs_reg is None:
        abs_reg = {}
    if op == 0x0F:
        return f"{va:08X}  {w:08X}  lui     ${REGS[rt]}, {imm:#x}  # -> {imm << 16:#010x}"
    if op == 0x09:
        if rs in abs_reg:
            extra = f"  # -> {(abs_reg[rs] + simm) & 0xFFFFFFFF:#010x}"
        return f"{va:08X}  {w:08X}  addiu   ${REGS[rt]}, ${REGS[rs]}, {simm}{extra}"
    if op == 0x0D:
        return f"{va:08X}  {w:08X}  ori     ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0C:
        return f"{va:08X}  {w:08X}  andi    ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x03:
        return f"{va:08X}  {w:08X}  jal     {jal_target(w):#010x}"
    if op == 0x02:
        return f"{va:08X}  {w:08X}  j       {jal_target(w):#010x}"
    if op == 0x04:
        return f"{va:08X}  {w:08X}  beq     ${REGS[rs]}, ${REGS[rt]}, {va + 4 + simm * 4:#010x}"
    if op == 0x05:
        return f"{va:08X}  {w:08X}  bne     ${REGS[rs]}, ${REGS[rt]}, {va + 4 + simm * 4:#010x}"
    if op == 0x06:
        return f"{va:08X}  {w:08X}  blez    ${REGS[rs]}, {va + 4 + simm * 4:#010x}"
    if op == 0x07:
        return f"{va:08X}  {w:08X}  bgtz    ${REGS[rs]}, {va + 4 + simm * 4:#010x}"
    if op == 0x01 and rt == 0:
        return f"{va:08X}  {w:08X}  bltz    ${REGS[rs]}, {va + 4 + simm * 4:#010x}"
    if op == 0x01 and rt == 1:
        return f"{va:08X}  {w:08X}  bgez    ${REGS[rs]}, {va + 4 + simm * 4:#010x}"
    if op in STORE_OPS:
        if rs in abs_reg:
            extra = f"  # -> {(abs_reg[rs] + simm) & 0xFFFFFFFF:#010x}"
        return f"{va:08X}  {w:08X}  {STORE_OPS[op]:7s} ${REGS[rt]}, {simm:#x}(${REGS[rs]}){extra}"
    if op in LOAD_OPS:
        if rs in abs_reg:
            extra = f"  # -> {(abs_reg[rs] + simm) & 0xFFFFFFFF:#010x}"
        return f"{va:08X}  {w:08X}  {LOAD_OPS[op]:7s} ${REGS[rt]}, {simm:#x}(${REGS[rs]}){extra}"
    if op == 0x00:
        if fn == 0x08:
            return f"{va:08X}  {w:08X}  jr      ${REGS[rs]}"
        if fn == 0x09:
            return f"{va:08X}  {w:08X}  jalr    ${REGS[rd]}, ${REGS[rs]}"
        if fn == 0x21:
            if rs == 0:
                return f"{va:08X}  {w:08X}  move    ${REGS[rd]}, ${REGS[rt]}"
            if rt == 0:
                return f"{va:08X}  {w:08X}  move    ${REGS[rd]}, ${REGS[rs]}"
            return f"{va:08X}  {w:08X}  addu    ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x00:
            if w == 0:
                return f"{va:08X}  {w:08X}  nop"
            return f"{va:08X}  {w:08X}  sll     ${REGS[rd]}, ${REGS[rt]}, {d['sa']}"
        if fn == 0x23:
            return f"{va:08X}  {w:08X}  subu    ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x24:
            return f"{va:08X}  {w:08X}  and     ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x25:
            return f"{va:08X}  {w:08X}  or      ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x2A:
            return f"{va:08X}  {w:08X}  slt     ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x2B:
            return f"{va:08X}  {w:08X}  sltu    ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        names = {0x10: "mfhi", 0x12: "mflo", 0x18: "mult", 0x19: "multu", 0x1A: "div", 0x1B: "divu"}
        if fn in (0x10, 0x12):
            return f"{va:08X}  {w:08X}  {names[fn]:7s} ${REGS[rd]}"
        if fn in names:
            return f"{va:08X}  {w:08X}  {names[fn]:7s} ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x02:
            return f"{va:08X}  {w:08X}  srl     ${REGS[rd]}, ${REGS[rt]}, {d['sa']}"
        if fn == 0x03:
            return f"{va:08X}  {w:08X}  sra     ${REGS[rd]}, ${REGS[rt]}, {d['sa']}"
    if op == 0x0A:
        return f"{va:08X}  {w:08X}  slti    ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 0x0B:
        return f"{va:08X}  {w:08X}  sltiu   ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    return f"{va:08X}  {w:08X}  op={op:#x} fn={fn:#x} rs=${REGS[rs]} rt=${REGS[rt]} rd=${REGS[rd]}"


def track(exe: bytes, start: int, end: int):
    abs_reg: dict[int, int] = {}
    for va in range(start, end, 4):
        w = word_at(exe, va)
        d = decode(w)
        snap = dict(abs_reg)
        yield va, w, d, snap
        op, rs, rt, simm = d["op"], d["rs"], d["rt"], d["simm"]
        if op == 0x0F:
            abs_reg[rt] = (d["imm"] << 16) & 0xFFFFFFFF
        elif op == 0x09 and rs in abs_reg:
            abs_reg[rt] = (abs_reg[rs] + simm) & 0xFFFFFFFF
        else:
            killed = dest_reg(d)
            if killed is not None and killed != 0:
                abs_reg.pop(killed, None)


def find_fn_end(exe: bytes, start: int, hard: int) -> int:
    last = None
    for va in range(start, hard, 4):
        w = word_at(exe, va)
        d = decode(w)
        if d["op"] == 0x00 and d["fn"] == 0x08 and d["rs"] == 31:
            last = va + 8
            nxt = word_at(exe, va + 8) if va + 8 < hard else 0
            nd = decode(nxt)
            if nd["op"] == 0x09 and nd["rs"] == 29 and nd["rt"] == 29 and nd["simm"] < 0:
                return va + 8
    return last or hard


def covers_byte_e(addr: int, kind: str) -> bool:
    width = {"sb": 1, "sh": 2, "sw": 4}[kind]
    return addr <= BYTE_E < addr + width


def dump_fn(exe: bytes, start: int, name: str, hard: int = 0) -> int:
    end = find_fn_end(exe, start, hard or start + 0x2000)
    words = (end - start) // 4
    print(f"\n=== {name} {start:#010x}..{end:#010x} exclusive  {words} words ===")
    jals = []
    stores_e = []
    for va, w, d, snap in track(exe, start, end):
        if d["op"] == 0x03:
            jals.append((va, jal_target(w)))
        if d["op"] in STORE_OPS:
            kind = STORE_OPS[d["op"]]
            base = snap.get(d["rs"])
            if base is not None:
                addr = (base + d["simm"]) & 0xFFFFFFFF
                if covers_byte_e(addr, kind):
                    stores_e.append((va, kind, addr))
    print(f"  jr-ra end words={words}")
    print(f"  +0xE stores={stores_e or 'NONE'}")
    counts = Counter(t for _, t in jals)
    print(f"  jal callees ({len(jals)}):")
    for tgt, n in counts.most_common():
        sites = [hex(va) for va, t in jals if t == tgt]
        print(f"    {tgt:#010x}  x{n}  {sites}")
    print("  prologue 12:")
    for va in range(start, min(start + 48, end), 4):
        print(f"    {fmt(va, word_at(exe, va))}")
    print("  epilogue 12:")
    for va in range(max(start, end - 48), end, 4):
        print(f"    {fmt(va, word_at(exe, va))}")
    return end


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1, "EXE SHA-1"
    print("EXE SHA-1 OK")

    print("\n=== EE=13 body 0x8006C9F8..0x8006CC68 ===")
    jals = []
    stores_e = []
    for va, w, d, snap in track(exe, EE13, FN_6C5BC_END):
        print(fmt(va, w, snap))
        if d["op"] == 0x03:
            jals.append((va, jal_target(w), word_at(exe, va + 4)))
        if d["op"] in STORE_OPS:
            kind = STORE_OPS[d["op"]]
            base = snap.get(d["rs"])
            if base is not None:
                addr = (base + d["simm"]) & 0xFFFFFFFF
                if covers_byte_e(addr, kind):
                    stores_e.append((va, kind, addr))
    print(f"\nEE13 jals ({len(jals)}):")
    for va, tgt, delay in jals:
        print(f"  {va:#010x} -> {tgt:#010x}  delay {fmt(va + 4, delay)}")
    print(f"EE13 +0xE stores: {stores_e or 'NONE'}")

    print("\n=== 6C5BC jal 6CC68 sites with a0/a1 setup ===")
    for va in range(0x8006C5BC, FN_6C5BC_END, 4):
        w = word_at(exe, va)
        if (w >> 26) == 3 and jal_target(w) == FN_6CC68:
            print(f"\n  site {va:#010x}")
            for pva in range(va - 24, va + 12, 4):
                mark = ">>" if pva == va else "  "
                print(f"  {mark} {fmt(pva, word_at(exe, pva))}")

    end_6cc68 = dump_fn(exe, FN_6CC68, "func_8006CC68")
    dump_fn(exe, 0x8006CDA4, "func_8006CDA4")
    dump_fn(exe, 0x8003D050, "func_8003D050")
    dump_fn(exe, 0x8006698C, "func_8006698C")
    dump_fn(exe, 0x8003D834, "func_8003D834")

    print("\n=== TEXT jal sites targeting 6CC68 ===")
    sites = []
    for va in range(TEXT_START, TEXT_END, 4):
        w = word_at(exe, va)
        if (w >> 26) == 3 and jal_target(w) == FN_6CC68:
            sites.append(va)
    print(f"count={len(sites)}")
    for va in sites:
        print(f"  {va:#010x}")

    print(f"\n6CC68 exclusive end {end_6cc68:#010x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
