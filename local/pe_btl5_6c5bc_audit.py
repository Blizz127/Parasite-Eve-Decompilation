#!/usr/bin/env python3
"""PE-BTL5 research: bound func_8006C5BC and prove its callers.

Authority: Disc 1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
Not production. Does not invent overlay returns or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import struct
from collections import Counter
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800
TEXT_START = 0x8001220C
TEXT_END = 0x80090000
OVERLAY = 0x800B0CD8
BYTE_E = OVERLAY + 0xE
FN = 0x8006C5BC
NEXT_KNOWN = 0x8006CC68  # func_8006CC68 from B45 evidence
PATH_FNS = {
    "144FC": (0x800144FC, 0x80014694),
    "29810": (0x80029810, 0x800299CC),
    "6C4C4": (0x8006C4C4, 0x8006C5BC),
    "6BECC": (0x8006BECC, 0x8006C1CC),
    "6C1CC": (0x8006C1CC, 0x8006C4C4),
    "6914C": (0x8006914C, 0x80069594),
}

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
        if fn == 0x0C:
            return f"{va:08X}  {w:08X}  syscall"
        names = {0x10: "mfhi", 0x12: "mflo", 0x18: "mult", 0x19: "multu", 0x1A: "div", 0x1B: "divu"}
        if fn in (0x10, 0x12):
            return f"{va:08X}  {w:08X}  {names[fn]:7s} ${REGS[rd]}"
        if fn in names:
            return f"{va:08X}  {w:08X}  {names[fn]:7s} ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x02:
            return f"{va:08X}  {w:08X}  srl     ${REGS[rd]}, ${REGS[rt]}, {d['sa']}"
        if fn == 0x03:
            return f"{va:08X}  {w:08X}  sra     ${REGS[rd]}, ${REGS[rt]}, {d['sa']}"
    if op == 0x0E:
        return f"{va:08X}  {w:08X}  xori    ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0A:
        return f"{va:08X}  {w:08X}  slti    ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 0x0B:
        return f"{va:08X}  {w:08X}  sltiu   ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    return f"{va:08X}  {w:08X}  .word   {w:#010x}  op={op:#x} fn={fn:#x}"


def track(exe: bytes, start: int, end: int):
    lui: dict[int, int] = {}
    abs_reg: dict[int, int] = {}
    for va in range(start, end, 4):
        w = word_at(exe, va)
        d = decode(w)
        op, rs, rt, imm, simm = d["op"], d["rs"], d["rt"], d["imm"], d["simm"]
        killed = dest_reg(d)
        snap = dict(abs_reg)
        if op == 0x0F:
            lui[rt] = (imm << 16) & 0xFFFFFFFF
            abs_reg[rt] = lui[rt]
        elif op == 0x09:
            if rt != 0 and rs in abs_reg:
                abs_reg[rt] = (abs_reg[rs] + simm) & 0xFFFFFFFF
            elif rt != 0:
                abs_reg.pop(rt, None)
        elif op == 0x0D:
            if rt != 0 and rs in abs_reg:
                abs_reg[rt] = (abs_reg[rs] | imm) & 0xFFFFFFFF
            elif rt != 0:
                abs_reg.pop(rt, None)
        elif op == 0x00 and d["fn"] == 0x21:
            rd, rs2, rt2 = d["rd"], d["rs"], d["rt"]
            if rs2 == 0 and rt2 in abs_reg:
                abs_reg[rd] = abs_reg[rt2]
            elif rt2 == 0 and rs2 in abs_reg:
                abs_reg[rd] = abs_reg[rs2]
            else:
                abs_reg.pop(rd, None)
        elif killed is not None and killed != 0 and op != 0x0F:
            abs_reg.pop(killed, None)
            lui.pop(killed, None)
        yield va, w, d, snap


def covers_byte_e(addr: int, kind: str) -> bool:
    width = {"sb": 1, "sh": 2, "sw": 4}[kind]
    return addr <= BYTE_E < addr + width


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
            if nd["op"] == 0x2B and nd["rt"] == 31:
                return va + 8
    return last or hard


def walk_back_to_prologue(exe: bytes, site: int) -> int:
    va = site
    while va > TEXT_START:
        w = word_at(exe, va)
        d = decode(w)
        if d["op"] == 0x09 and d["rs"] == 29 and d["rt"] == 29 and d["simm"] < 0:
            prev = word_at(exe, va - 8) if va - 8 >= TEXT_START else 0
            pd = decode(prev)
            if pd["op"] == 0x00 and pd["fn"] == 0x08 and pd["rs"] == 31:
                return va
            if va == site or va < site - 4:
                # typical function start
                nxt = word_at(exe, va + 4)
                nd = decode(nxt)
                if nd["op"] == 0x2B and nd["rt"] == 31:
                    return va
                if d["simm"] <= -16:
                    return va
        va -= 4
        if site - va > 0x2000:
            break
    return site


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    exe_path = root / "build" / "disc1.candidate.exe"
    exe = exe_path.read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1, "EXE SHA-1"

    print("=== EXE SHA-1 OK ===")
    print(f"func_8006C5BC start {FN:#010x}")
    print(f"known next func_8006CC68 {NEXT_KNOWN:#010x}")
    print(f"delta words {(NEXT_KNOWN - FN) // 4}")

    # prologue
    print("\n=== prologue 16 words ===")
    abs_reg: dict[int, int] = {}
    for va, w, d, snap in track(exe, FN, FN + 64):
        print(fmt(va, w, snap))
        abs_reg = snap

    fn_end = find_fn_end(exe, FN, FN + 0x2000)
    print(f"\n=== estimated jr-ra end {fn_end:#010x} words={(fn_end - FN) // 4} ===")
    print("=== last 24 words of estimated body ===")
    for va in range(max(FN, fn_end - 96), fn_end, 4):
        print(fmt(va, word_at(exe, va)))

    print("\n=== 16 words at known next 6CC68 ===")
    for va in range(NEXT_KNOWN - 16, NEXT_KNOWN + 48, 4):
        mark = ">>" if va == NEXT_KNOWN else "  "
        print(f"{mark} {fmt(va, word_at(exe, va))}")

    # census inside [FN, NEXT_KNOWN)
    jals = []
    stores_e = []
    stores_off_e = []
    ori_andi = []
    jr_ra = []
    jalr = []
    for va, w, d, snap in track(exe, FN, NEXT_KNOWN):
        op, rs, rt, simm = d["op"], d["rs"], d["rt"], d["simm"]
        if op == 0x03:
            jals.append((va, jal_target(w), word_at(exe, va + 4)))
        if op == 0x00 and d["fn"] == 0x09:
            jalr.append(va)
        if op == 0x00 and d["fn"] == 0x08 and rs == 31:
            jr_ra.append(va)
        if op in STORE_OPS:
            kind = STORE_OPS[op]
            base = snap.get(rs)
            addr = (base + simm) & 0xFFFFFFFF if base is not None else None
            if simm == 0xE:
                stores_off_e.append((va, kind, REGS[rt], REGS[rs], base, addr, w))
            if addr is not None and covers_byte_e(addr, kind):
                stores_e.append((va, kind, REGS[rt], REGS[rs], simm, addr, w))
        if op == 0x0C and d["imm"] in (0xFC, 0x3, 0x1, 0x2):
            ori_andi.append((va, "andi", d["imm"], w))
        if op == 0x0D and d["imm"] in (0x1, 0x2, 0x3, 0x4):
            ori_andi.append((va, "ori", d["imm"], w))

    print(f"\n=== jr $ra sites in window: {[hex(x) for x in jr_ra]} ===")
    print(f"=== jalr sites: {[hex(x) for x in jalr]} ===")
    print(f"=== jal callees ({len(jals)}) ===")
    counts = Counter(t for _, t, _ in jals)
    for tgt, n in counts.most_common():
        sites = [hex(va) for va, t, _ in jals if t == tgt]
        print(f"  {tgt:#010x}  x{n}  sites={sites}")
    print("\n=== jal with delay slot ===")
    for va, tgt, delay in jals:
        print(f"  {fmt(va, word_at(exe, va))}")
        print(f"      delay {fmt(va + 4, delay)}")

    print("\n=== +0xE / D_800B0CE6 stores (resolved) ===")
    for rec in stores_e:
        va, kind, rt, rs, simm, addr, w = rec
        print(f"  {va:08X}  {kind} ${rt}, {simm:#x}(${rs}) -> {addr:#010x}")
        for pva in range(va - 16, va + 12, 4):
            mark = ">>" if pva == va else "  "
            print(f"    {mark} {fmt(pva, word_at(exe, pva))}")

    print("\n=== offset-+0xE stores (any base) ===")
    for rec in stores_off_e:
        va, kind, rt, rs, base, addr, w = rec
        print(f"  {va:08X}  {kind} ${rt}, 0xe(${rs}) base={base and hex(base)} addr={addr and hex(addr)}")

    print("\n=== ori/andi 1/2/3/4/FC in body ===")
    for va, kind, imm, w in ori_andi:
        print(f"  {fmt(va, w)}")

    # all TEXT jal to 6C5BC
    print("\n=== TEXT jal sites targeting 6C5BC ===")
    sites = []
    for va in range(TEXT_START, TEXT_END, 4):
        w = word_at(exe, va)
        if (w >> 26) == 0x03 and jal_target(w) == FN:
            delay = word_at(exe, va + 4)
            sites.append((va, delay))
    print(f"count={len(sites)}")
    for va, delay in sites:
        pro = walk_back_to_prologue(exe, va)
        print(f"\n  site {va:#010x}  estimated fn start {pro:#010x}  offset {va - pro:#x}")
        print(f"    {fmt(va, word_at(exe, va))}")
        print(f"    delay {fmt(va + 4, delay)}")
        for pva in range(va - 24, va + 28, 4):
            mark = ">>" if pva == va else "  "
            print(f"    {mark} {fmt(pva, word_at(exe, pva))}")

    print("\n=== PATH_FNS jal 6C5BC? ===")
    for name, (lo, hi) in PATH_FNS.items():
        hits = [hex(va) for va, _ in sites if lo <= va < hi]
        jals_in = []
        for va in range(lo, hi, 4):
            w = word_at(exe, va)
            if (w >> 26) == 0x03:
                jals_in.append((va, jal_target(w)))
        print(f"  {name} [{lo:#x},{hi:#x}) jal6C5BC={hits or 'NONE'}  jal_count={len(jals_in)}")
        if name in ("6C1CC", "6BECC") or hits:
            for va, tgt in jals_in:
                if tgt in (FN, 0x8006C4C4, 0x8006BECC, 0x8006C1CC, 0x8006914C):
                    print(f"      {va:#010x} -> {tgt:#010x}")

    # 6C1CC window may be larger; dump jals of the 6C358 region
    print("\n=== region 6C1CC..6C4C4 jals ===")
    for va in range(0x8006C1CC, 0x8006C4C4, 4):
        w = word_at(exe, va)
        if (w >> 26) == 0x03:
            print(f"  {fmt(va, w)}")
            print(f"      delay {fmt(va + 4, word_at(exe, va + 4))}")

    print("\n=== 6C358 ± 40 words ===")
    for va in range(0x8006C358 - 0x40, 0x8006C358 + 0x60, 4):
        mark = ">>" if va == 0x8006C358 else "  "
        print(f"{mark} {fmt(va, word_at(exe, va))}")

    print("\n=== 35B24 ± 24 words ===")
    for va in range(0x80035B24 - 0x40, 0x80035B24 + 0x50, 4):
        mark = ">>" if va == 0x80035B24 else "  "
        print(f"{mark} {fmt(va, word_at(exe, va))}")

    print("\n=== 3F22C ± 32 words ===")
    for va in range(0x8003F22C - 0x50, 0x8003F22C + 0x60, 4):
        mark = ">>" if va == 0x8003F22C else "  "
        print(f"{mark} {fmt(va, word_at(exe, va))}")

    # recursive: who jals the caller functions
    print("\n=== who jals the 6C5BC caller functions (first-level) ===")
    caller_fns = []
    for va, _ in sites:
        caller_fns.append(walk_back_to_prologue(exe, va))
    for cfn in sorted(set(caller_fns)):
        hits = []
        for va in range(TEXT_START, TEXT_END, 4):
            w = word_at(exe, va)
            if (w >> 26) == 0x03 and jal_target(w) == cfn:
                hits.append(va)
        print(f"  fn {cfn:#010x}  jal_sites={len(hits)}  {[hex(x) for x in hits[:20]]}")

    # 144FC / 29810 jal census (confirm no 6C5BC)
    print("\n=== 144FC all jals ===")
    for va in range(0x800144FC, 0x80014694, 4):
        w = word_at(exe, va)
        if (w >> 26) == 0x03:
            print(f"  {fmt(va, w)} delay {fmt(va + 4, word_at(exe, va + 4))}")
    print("=== 29810 all jals ===")
    for va in range(0x80029810, 0x800299CC, 4):
        w = word_at(exe, va)
        if (w >> 26) == 0x03:
            print(f"  {fmt(va, w)} delay {fmt(va + 4, word_at(exe, va + 4))}")
    print("=== 6914C all jals ===")
    for va in range(0x8006914C, 0x80069594, 4):
        w = word_at(exe, va)
        if (w >> 26) == 0x03:
            print(f"  {fmt(va, w)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
