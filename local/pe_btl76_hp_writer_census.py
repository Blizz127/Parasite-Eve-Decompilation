#!/usr/bin/env python3
"""PE-BTL76 — census of writers to the proven Aya HP triple.

Proven fields (BTL2): D_8009D278 record +0x0C current, +0x0E snapshot,
+0x1C cap. 293F4 is COPY. 2FF78 dest is *D254 (actor), not D278.

Does not import production C.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800
GP = 0x8009CD70
D278 = GP + 0x508  # 0x8009D278
TEXT_END = 0x80010000 + 0x1EE000

REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def jal_sites(blob: bytes, target: int) -> list[int]:
    want = 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)
    out = []
    text = blob[HDR : HDR + 0x1EE000]
    for i in range(0, len(text), 4):
        if struct.unpack_from("<I", text, i)[0] == want:
            out.append(TADDR + i)
    return out


def op(w: int) -> int:
    return w >> 26


def rs(w: int) -> int:
    return (w >> 21) & 31


def rt(w: int) -> int:
    return (w >> 16) & 31


def rd(w: int) -> int:
    return (w >> 11) & 31


def imm16(w: int) -> int:
    return w & 0xFFFF


def simm16(w: int) -> int:
    v = w & 0xFFFF
    return v - 0x10000 if v >= 0x8000 else v


def funct(w: int) -> int:
    return w & 0x3F


def dis(w: int, va: int) -> str:
    o = op(w)
    if o == 0:
        f = funct(w)
        if w == 0:
            return "nop"
        if f == 0x08:
            return f"jr ${REGS[rs(w)]}"
        if f == 0x09:
            return f"jalr ${REGS[rd(w)]},${REGS[rs(w)]}"
        if f == 0x21:
            return f"addu ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x23:
            return f"subu ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x24:
            return f"and ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x25:
            return f"or ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x2A:
            return f"slt ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x2B:
            return f"sltu ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        return f"special {f:#x}"
    if o == 2:
        return f"j {((va & 0xF0000000) | ((w & 0x03FFFFFF) << 2)):#x}"
    if o == 3:
        return f"jal {jal_target(w):#x}"
    if o == 4:
        return f"beq ${REGS[rs(w)]},${REGS[rt(w)]}, {va + 4 + simm16(w) * 4:#x}"
    if o == 5:
        return f"bne ${REGS[rs(w)]},${REGS[rt(w)]}, {va + 4 + simm16(w) * 4:#x}"
    if o == 6:
        return f"blez ${REGS[rs(w)]}, {va + 4 + simm16(w) * 4:#x}"
    if o == 7:
        return f"bgtz ${REGS[rs(w)]}, {va + 4 + simm16(w) * 4:#x}"
    if o == 8:
        return f"addi ${REGS[rt(w)]},${REGS[rs(w)]},{simm16(w)}"
    if o == 9:
        return f"addiu ${REGS[rt(w)]},${REGS[rs(w)]},{simm16(w)}"
    if o == 0x0A:
        return f"slti ${REGS[rt(w)]},${REGS[rs(w)]},{simm16(w)}"
    if o == 0x0B:
        return f"sltiu ${REGS[rt(w)]},${REGS[rs(w)]},{simm16(w)}"
    if o == 0x0C:
        return f"andi ${REGS[rt(w)]},${REGS[rs(w)]},{imm16(w):#x}"
    if o == 0x0D:
        return f"ori ${REGS[rt(w)]},${REGS[rs(w)]},{imm16(w):#x}"
    if o == 0x0F:
        return f"lui ${REGS[rt(w)]},{imm16(w):#x}"
    names = {
        0x20: "lb", 0x21: "lh", 0x23: "lw", 0x24: "lbu", 0x25: "lhu",
        0x28: "sb", 0x29: "sh", 0x2B: "sw",
    }
    if o in names:
        return f"{names[o]} ${REGS[rt(w)]},{simm16(w):#x}(${REGS[rs(w)]})"
    return f"op{o:#x} {w:08x}"


def find_func_start(blob: bytes, va: int) -> int:
    """Walk backward to a likely prologue (addiu sp) or prior jr."""
    start = max(TADDR, va - 0x800)
    best = va
    for a in range(va, start, -4):
        w = load_u32(blob, a)
        if op(w) == 9 and rt(w) == 29 and rs(w) == 29 and simm16(w) < 0:
            best = a
            break
        if w == 0x03E00008 and a + 8 <= va:
            best = a + 8
            break
    return best


def dump_window(blob: bytes, start: int, n: int) -> None:
    for i in range(n):
        va = start + i * 4
        if va >= TEXT_END:
            break
        w = load_u32(blob, va)
        print(f"  {va:08X}  {w:08X}  {dis(w, va)}")


def main() -> int:
    blob = EXE.read_bytes()
    assert hashlib.sha1(blob).hexdigest() == SHA1
    text = blob[HDR : HDR + 0x1EE000]

    print("=== D278 identity ===")
    print(f"GP={GP:#x} D278={D278:#x} gp+0x508={GP + 0x508:#x}")

    print("\n=== lw/sw gp+0x508 (D278 pointer) ===")
    d278_uses = []
    for i in range(0, len(text), 4):
        w = struct.unpack_from("<I", text, i)[0]
        if imm16(w) != 0x508:
            continue
        if op(w) not in (0x23, 0x2B):  # lw sw
            continue
        if rs(w) != 28:  # gp
            continue
        va = TADDR + i
        d278_uses.append((va, w, dis(w, va)))
        print(f"  {va:08X}  {dis(w, va)}")
    print(f"count={len(d278_uses)}")

    print("\n=== lui/addiu materialize 0x8009D278 ===")
    for i in range(0, len(text) - 4, 4):
        w = struct.unpack_from("<I", text, i)[0]
        if op(w) != 0x0F:
            continue
        if imm16(w) not in (0x8009, 0x800A):
            continue
        w2 = struct.unpack_from("<I", text, i + 4)[0]
        # addiu/lw/sw with lo that forms D278
        if op(w2) in (9, 0x23, 0x2B) and rs(w2) == rt(w):
            base = (imm16(w) << 16) + simm16(w2)
            if base == D278 or (imm16(w) == 0x800A and simm16(w2) == -0x2D88):
                va = TADDR + i
                print(f"  {va:08X}  {dis(w, va)}")
                print(f"  {va + 4:08X}  {dis(w2, va + 4)}  => {base:#x}")

    print("\n=== sh/sb/sw +0x0C/+0x0E/+0x1C after nearby lw D278 ===")
    # For each D278 load, scan forward ~80 words for stores to those offs
    # on the loaded register or copies of it.
    seen_stores = []
    for va, w, _ in d278_uses:
        if op(w) != 0x23:
            continue
        dest = rt(w)
        live = {dest}
        for j in range(1, 96):
            a = va + j * 4
            if a >= TEXT_END:
                break
            ww = load_u32(blob, a)
            o = op(ww)
            # copies: addu rd, rs, zero / addu rd, zero, rs / addiu rt, rs, 0
            if o == 0 and funct(ww) == 0x21:
                if rt(ww) == 0 and rs(ww) in live:
                    live.add(rd(ww))
                elif rs(ww) == 0 and rt(ww) in live:
                    live.add(rd(ww))
            if o == 9 and simm16(ww) == 0 and rs(ww) in live:
                live.add(rt(ww))
            if o in (0x28, 0x29, 0x2B) and rs(ww) in live:
                off = simm16(ww)
                if off in (0x0C, 0x0E, 0x1C, 0x0D, 0x0F, 0x1D):
                    rec = (a, dis(ww, a), va)
                    seen_stores.append(rec)
                    print(f"  STORE {a:08X}  {dis(ww, a)}  via D278-load {va:08X}")
            if o in (0x20, 0x21, 0x23, 0x24, 0x25) and rs(ww) in live:
                off = simm16(ww)
                if off in (0x0C, 0x0E, 0x1C):
                    print(f"  READ  {a:08X}  {dis(ww, a)}  via D278-load {va:08X}")
            if ww == 0x03E00008:
                break

    print("\n=== 29350 blez reader neighborhood ===")
    dump_window(blob, 0x80029320, 28)

    print("\n=== 293F4 remainder after COPY cut (a0==1 arm) ===")
    dump_window(blob, 0x80029448, 80)

    print("\n=== 2FF78 tag4/5/10 stores ===")
    dump_window(blob, 0x80030090, 12)
    print("2FF78 jal sites:", [f"{x:#x}" for x in jal_sites(blob, 0x8002FF78)])
    print("293F4 jal sites:", [f"{x:#x}" for x in jal_sites(blob, 0x800293F4)])

    print("\n=== 2F658 INIT copy 10928 -> B8A20 (includes HP 45) ===")
    print("2F658 jal sites:", [f"{x:#x}" for x in jal_sites(blob, 0x8002F658)])
    print(f"10928+0x0C={struct.unpack_from('<H', blob, exe_off(0x80010928 + 0x0C))[0]:#x}")

    print("\n=== 209F0 D278 stores (command bytes, not HP) ===")
    dump_window(blob, 0x80020B30, 24)

    print("\n=== type-3 stream hunt for 0x85 and following ===")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
