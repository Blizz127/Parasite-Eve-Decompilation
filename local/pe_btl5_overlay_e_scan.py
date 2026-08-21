#!/usr/bin/env python3
"""PE-BTL5 research: EXE census of D_800B0CD8+0xE stores and 6914C.

Authority: Disc 1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
Not production. Does not invent overlay returns.
"""
from __future__ import annotations

import hashlib
import struct
import sys
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800
TEXT_START = 0x8001220C
TEXT_END = 0x80090000
OVERLAY = 0x800B0CD8
BYTE_E = OVERLAY + 0xE  # D_800B0CE6
WORD_COVER = OVERLAY + 0xC  # sw here would cover +0xE
FN_6914C = 0x8006914C
FN_144FC = 0x800144FC
GATE_3B = 0x80014630

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


def dest_reg(d: dict) -> int | None:
    op, fn = d["op"], d["fn"]
    if op == 0x00 and fn in (
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x00, 0x02, 0x03, 0x04, 0x06, 0x07, 0x08, 0x09,
        0x0A, 0x0B, 0x2A, 0x2B,
    ):
        return d["rd"]
    if op in (0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
              0x20, 0x21, 0x23, 0x24, 0x25, 0x22, 0x26,
              0x31, 0x32):
        return d["rt"]
    if op == 0x00 and fn == 0x00 and d["raw"] == 0:
        return None
    return None


STORE_OPS = {0x28: "sb", 0x29: "sh", 0x2B: "sw"}
LOAD_OPS = {0x20: "lb", 0x24: "lbu", 0x21: "lh", 0x25: "lhu", 0x23: "lw"}


def fmt(va: int, w: int, lui: dict[int, int], abs_reg: dict[int, int]) -> str:
    d = decode(w)
    op, rs, rt, rd, fn = d["op"], d["rs"], d["rt"], d["rd"], d["fn"]
    imm, simm = d["imm"], d["simm"]
    extra = ""
    if op == 0x0F:
        extra = f"  # -> {imm << 16:#010x}"
        return f"{va:08X}  {w:08X}  lui     ${REGS[rt]}, {imm:#x}{extra}"
    if op == 0x09:
        if rs in abs_reg:
            extra = f"  # -> {(abs_reg[rs] + simm) & 0xFFFFFFFF:#010x}"
        return f"{va:08X}  {w:08X}  addiu   ${REGS[rt]}, ${REGS[rs]}, {simm}{extra}"
    if op == 0x0D:
        if rs in abs_reg:
            extra = f"  # -> {(abs_reg[rs] | imm) & 0xFFFFFFFF:#010x}"
        return f"{va:08X}  {w:08X}  ori     ${REGS[rt]}, ${REGS[rs]}, {imm:#x}{extra}"
    if op == 0x0C:
        return f"{va:08X}  {w:08X}  andi    ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0A:
        return f"{va:08X}  {w:08X}  slti    ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 0x0B:
        extra = f"  # {imm}" if imm < 0x8000 else ""
        return f"{va:08X}  {w:08X}  sltiu   ${REGS[rt]}, ${REGS[rs]}, {imm:#x}{extra}"
    if op == 0x03:
        return f"{va:08X}  {w:08X}  jal     {jal_target(w):#010x}"
    if op == 0x02:
        return f"{va:08X}  {w:08X}  j       {jal_target(w):#010x}"
    if op == 0x04:
        tgt = va + 4 + simm * 4
        return f"{va:08X}  {w:08X}  beq     ${REGS[rs]}, ${REGS[rt]}, {tgt:#010x}"
    if op == 0x05:
        tgt = va + 4 + simm * 4
        return f"{va:08X}  {w:08X}  bne     ${REGS[rs]}, ${REGS[rt]}, {tgt:#010x}"
    if op == 0x06:
        tgt = va + 4 + simm * 4
        return f"{va:08X}  {w:08X}  blez    ${REGS[rs]}, {tgt:#010x}"
    if op == 0x07:
        tgt = va + 4 + simm * 4
        return f"{va:08X}  {w:08X}  bgtz    ${REGS[rs]}, {tgt:#010x}"
    if op == 0x01 and rt == 0:
        tgt = va + 4 + simm * 4
        return f"{va:08X}  {w:08X}  bltz    ${REGS[rs]}, {tgt:#010x}"
    if op == 0x01 and rt == 1:
        tgt = va + 4 + simm * 4
        return f"{va:08X}  {w:08X}  bgez    ${REGS[rs]}, {tgt:#010x}"
    if op in STORE_OPS:
        extra = ""
        if rs in abs_reg:
            extra = f"  # -> {(abs_reg[rs] + simm) & 0xFFFFFFFF:#010x}"
        return (
            f"{va:08X}  {w:08X}  {STORE_OPS[op]:7s} ${REGS[rt]}, "
            f"{simm:#x}(${REGS[rs]}){extra}"
        )
    if op in LOAD_OPS:
        extra = ""
        if rs in abs_reg:
            extra = f"  # -> {(abs_reg[rs] + simm) & 0xFFFFFFFF:#010x}"
        return (
            f"{va:08X}  {w:08X}  {LOAD_OPS[op]:7s} ${REGS[rt]}, "
            f"{simm:#x}(${REGS[rs]}){extra}"
        )
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
            return (
                f"{va:08X}  {w:08X}  addu    ${REGS[rd]}, "
                f"${REGS[rs]}, ${REGS[rt]}"
            )
        if fn == 0x20:
            return (
                f"{va:08X}  {w:08X}  add     ${REGS[rd]}, "
                f"${REGS[rs]}, ${REGS[rt]}"
            )
        if fn == 0x23:
            return (
                f"{va:08X}  {w:08X}  subu    ${REGS[rd]}, "
                f"${REGS[rs]}, ${REGS[rt]}"
            )
        if fn == 0x24:
            return (
                f"{va:08X}  {w:08X}  and     ${REGS[rd]}, "
                f"${REGS[rs]}, ${REGS[rt]}"
            )
        if fn == 0x25:
            return (
                f"{va:08X}  {w:08X}  or      ${REGS[rd]}, "
                f"${REGS[rs]}, ${REGS[rt]}"
            )
        if fn == 0x27:
            return f"{va:08X}  {w:08X}  nor     ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}"
        if fn == 0x00:
            if w == 0:
                return f"{va:08X}  {w:08X}  nop"
            return (
                f"{va:08X}  {w:08X}  sll     ${REGS[rd]}, "
                f"${REGS[rt]}, {d['sa']}"
            )
        if fn == 0x02:
            return (
                f"{va:08X}  {w:08X}  srl     ${REGS[rd]}, "
                f"${REGS[rt]}, {d['sa']}"
            )
        if fn == 0x03:
            return (
                f"{va:08X}  {w:08X}  sra     ${REGS[rd]}, "
                f"${REGS[rt]}, {d['sa']}"
            )
        if fn == 0x04:
            return (
                f"{va:08X}  {w:08X}  sllv    ${REGS[rd]}, "
                f"${REGS[rt]}, ${REGS[rs]}"
            )
        if fn == 0x2A:
            return (
                f"{va:08X}  {w:08X}  slt     ${REGS[rd]}, "
                f"${REGS[rs]}, ${REGS[rt]}"
            )
        if fn == 0x2B:
            return (
                f"{va:08X}  {w:08X}  sltu    ${REGS[rd]}, "
                f"${REGS[rs]}, ${REGS[rt]}"
            )
        if fn == 0x08:
            return f"{va:08X}  {w:08X}  jr      ${REGS[rs]}"
        if fn == 0x0C:
            return f"{va:08X}  {w:08X}  syscall"
        if fn in (0x10, 0x12, 0x18, 0x19, 0x1A, 0x1B):
            names = {
                0x10: "mfhi", 0x12: "mflo", 0x18: "mult",
                0x19: "multu", 0x1A: "div", 0x1B: "divu",
            }
            if fn in (0x10, 0x12):
                return f"{va:08X}  {w:08X}  {names[fn]:7s} ${REGS[rd]}"
            return (
                f"{va:08X}  {w:08X}  {names[fn]:7s} ${REGS[rs]}, "
                f"${REGS[rt]}"
            )
    if op == 0x0E:
        return f"{va:08X}  {w:08X}  xori    ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    return f"{va:08X}  {w:08X}  .word   {w:#010x}  op={op:#x} fn={fn:#x}"


def track_abs(exe: bytes, start: int, end: int):
    """Walk TEXT, yield (va, word, abs_reg snapshot after instruction)."""
    lui: dict[int, int] = {}
    abs_reg: dict[int, int] = {}
    for va in range(start, end, 4):
        w = word_at(exe, va)
        d = decode(w)
        op, rs, rt, imm, simm = d["op"], d["rs"], d["rt"], d["imm"], d["simm"]
        killed = dest_reg(d)
        # snapshot before kill for this instruction's memory op
        snap = dict(abs_reg)
        if op == 0x0F:
            lui[rt] = (imm << 16) & 0xFFFFFFFF
            abs_reg[rt] = lui[rt]
        elif op == 0x09:
            if rt == 0:
                pass
            elif rs in abs_reg:
                abs_reg[rt] = (abs_reg[rs] + simm) & 0xFFFFFFFF
            elif rs in lui:
                abs_reg[rt] = (lui[rs] + simm) & 0xFFFFFFFF
            else:
                abs_reg.pop(rt, None)
        elif op == 0x0D:
            if rt == 0:
                pass
            elif rs in abs_reg:
                abs_reg[rt] = (abs_reg[rs] | imm) & 0xFFFFFFFF
            else:
                abs_reg.pop(rt, None)
        elif op == 0x00 and d["fn"] == 0x21:  # addu / move
            rd, rs2, rt2 = d["rd"], d["rs"], d["rt"]
            if rs2 == 0 and rt2 in abs_reg:
                abs_reg[rd] = abs_reg[rt2]
            elif rt2 == 0 and rs2 in abs_reg:
                abs_reg[rd] = abs_reg[rs2]
            else:
                abs_reg.pop(rd, None)
        elif killed is not None and killed != 0:
            # keep lui half if this is lui itself (already handled)
            if op != 0x0F:
                abs_reg.pop(killed, None)
                lui.pop(killed, None)
        yield va, w, d, snap, dict(abs_reg), dict(lui)


def covers_byte_e(addr: int, kind: str) -> bool:
    if kind == "sb":
        return addr == BYTE_E
    if kind == "sh":
        return addr <= BYTE_E < addr + 2
    if kind == "sw":
        return addr <= BYTE_E < addr + 4
    return False


def estimate_fn_end(exe: bytes, start: int, limit: int = 0x2000) -> int:
    """First jr $ra whose delay slot completes, after a matching addiu $sp."""
    last_jr = None
    for va in range(start, start + limit, 4):
        w = word_at(exe, va)
        d = decode(w)
        if d["op"] == 0x00 and d["fn"] == 0x08 and d["rs"] == 31:
            last_jr = va + 8
            # Heuristic: if next word looks like a new function (addiu sp,-N
            # or sw ra) treat this as the end. Keep going until a later
            # function's typical prologue after a gap is risky; stop at
            # first jr-ra that is not inside a likely inner path if the
            # stack is restored.
            nxt = word_at(exe, va + 4)
            nxt2 = word_at(exe, va + 8) if va + 8 < start + limit else 0
            # typical next-fn: addiu $sp, $sp, negative OR sw $ra
            nd = decode(nxt2)
            if nd["op"] == 0x09 and nd["rs"] == 29 and nd["rt"] == 29 and nd["simm"] < 0:
                return va + 8
            if nd["op"] == 0x2B and nd["rt"] == 31:
                return va + 8
            if (nxt2 >> 16) == 0x27BD and simm16(nxt2 & 0xFFFF) < 0:
                return va + 8
    return last_jr or (start + limit)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    exe_path = root / "build" / "disc1.candidate.exe"
    if not exe_path.is_file():
        print(f"FAIL: missing {exe_path}", file=sys.stderr)
        return 1
    exe = exe_path.read_bytes()
    if hashlib.sha1(exe).hexdigest() != SHA1:
        print("FAIL: EXE SHA-1", file=sys.stderr)
        return 1

    stores = []
    loads_e = []
    andi3_consumers = []
    jal_6914c = []
    sb_off_e = []  # every sb/sh/sw with simm==0xE regardless of base

    pending_lbu_e: list[tuple[int, int]] = []  # (va, dest_rt)

    for va, w, d, snap, abs_now, lui in track_abs(exe, TEXT_START, TEXT_END):
        op, rs, rt, simm = d["op"], d["rs"], d["rt"], d["simm"]
        if op == 0x03:
            tgt = jal_target(w)
            if tgt == FN_6914C:
                delay = word_at(exe, va + 4)
                jal_6914c.append((va, delay))
        if op in STORE_OPS:
            kind = STORE_OPS[op]
            base = snap.get(rs)
            addr = None
            if base is not None:
                addr = (base + simm) & 0xFFFFFFFF
            if simm == 0xE or simm == 14:
                sb_off_e.append((va, kind, REGS[rt], REGS[rs], base, addr))
            if addr is not None and covers_byte_e(addr, kind):
                stores.append((va, kind, REGS[rt], REGS[rs], simm, addr, w))
        if op in LOAD_OPS:
            kind = LOAD_OPS[op]
            base = snap.get(rs)
            addr = (base + simm) & 0xFFFFFFFF if base is not None else None
            if addr == BYTE_E or (simm == 0xE and kind in ("lb", "lbu") and base == OVERLAY):
                loads_e.append((va, kind, REGS[rt], REGS[rs], simm, addr, w))
                pending_lbu_e.append((va, rt))
            elif simm == 0xE and kind in ("lb", "lbu"):
                loads_e.append((va, kind, REGS[rt], REGS[rs], simm, addr, w))
                pending_lbu_e.append((va, rt))
        if op == 0x0C and d["imm"] == 3:
            # andi dest, src, 3 immediately after a +0xE load of src
            for load_va, load_rt in pending_lbu_e[-8:]:
                if d["rs"] == load_rt and va - load_va <= 16:
                    andi3_consumers.append((load_va, va, REGS[load_rt], REGS[d["rt"]]))
                    break

    print("=== EXE SHA-1 OK ===")
    print(f"D_800B0CD8+0xE = {BYTE_E:#010x} (D_800B0CE6)")
    print()
    print("=== STORES covering D_800B0CE6 ===")
    for rec in stores:
        va, kind, rt, rs, simm, addr, w = rec
        print(
            f"  {va:08X}  {w:08X}  {kind} ${rt}, {simm:#x}(${rs}) "
            f"-> {addr:#010x}"
        )
        for off in range(-12, 16, 4):
            pva = va + off
            if TEXT_START <= pva < TEXT_END:
                ww = word_at(exe, pva)
                mark = ">>" if pva == va else "  "
                print(f"    {mark} {fmt(pva, ww, {}, {})}")
        print()

    print("=== offset-+0xE stores (any base, including unresolved) ===")
    for rec in sb_off_e:
        va, kind, rt, rs, base, addr = rec
        print(
            f"  {va:08X}  {kind} ${rt}, 0xe(${rs})  "
            f"base={base and hex(base)} addr={addr and hex(addr)}"
        )
    print()

    print("=== loads of +0xE / D_800B0CE6 (resolved or offset 0xE lb/lbu) ===")
    for rec in loads_e:
        va, kind, rt, rs, simm, addr, w = rec
        resolved = "RESOLVED" if addr == BYTE_E else ("overlay?" if addr == OVERLAY + 0xE else "unresolved-or-other")
        print(
            f"  {va:08X}  {w:08X}  {kind} ${rt}, {simm:#x}(${rs}) "
            f"addr={addr and hex(addr)} {resolved}"
        )
    print()

    print("=== lbu/+0xE then andi 3 consumers ===")
    for load_va, and_va, src, dst in andi3_consumers:
        print(f"  load {load_va:08X} andi {and_va:08X}  {src} -> {dst}")
        for pva in range(load_va, and_va + 8, 4):
            print(f"    {fmt(pva, word_at(exe, pva), {}, {})}")
        print()

    print(f"=== jal func_8006914C sites ({len(jal_6914c)}) ===")
    for va, delay in jal_6914c:
        dd = decode(delay)
        print(f"  {va:08X}  jal 0x8006914C  delay {delay:08X}  {fmt(va + 4, delay, {}, {})}")
        # a0 in delay slot or previous few
        for pva in range(va - 12, va + 8, 4):
            if TEXT_START <= pva < TEXT_END:
                print(f"    {fmt(pva, word_at(exe, pva), {}, {})}")
        print()

    fn_end = estimate_fn_end(exe, FN_6914C)
    print(f"=== func_8006914C estimate end {fn_end:#010x} size {(fn_end - FN_6914C) // 4} words ===")
    print("--- disasm ---")
    lui: dict[int, int] = {}
    abs_reg: dict[int, int] = {}
    for va, w, d, snap, abs_now, lui_now in track_abs(exe, FN_6914C, fn_end):
        print(fmt(va, w, lui_now, snap))
        lui, abs_reg = lui_now, abs_now

    # Also dump the 0x55 3B gate window
    print()
    print("=== 0x55 state 0x3B gate 0x80014630 window ===")
    for va in range(0x80014620, 0x80014670, 4):
        print(fmt(va, word_at(exe, va), {}, {}))

    # Check 6A674 known store of D_800B0CE6
    print()
    print("=== sanity: 6A674 region stores of D_800B0CE6 already found above ===")
    known = [s for s in stores if 0x8006A674 <= s[0] < 0x8006A674 + 0x260]
    print(f"  found {len(known)} store(s) in func_8006A674")

    # Check 144FC for jal 6914C
    jal_in_144fc = [s for s in jal_6914c if FN_144FC <= s[0] < FN_144FC + 0x200]
    print(f"  jal 6914C inside 144FC: {len(jal_in_144fc)}")
    print(f"  jal 6914C inside 0x55 gate window: "
          f"{[hex(s[0]) for s in jal_6914c if 0x80014630 <= s[0] < 0x80014658]}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
