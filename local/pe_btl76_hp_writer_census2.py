#!/usr/bin/env python3
"""PE-BTL76 — complete HP-field store census + 2AE60/1D340/2F300 dumps."""
from __future__ import annotations

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800
TEXT_END = TADDR + 0x1EE000
D278 = 0x8009D278

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
    text = blob[HDR : HDR + 0x1EE000]
    return [TADDR + i for i in range(0, len(text), 4)
            if struct.unpack_from("<I", text, i)[0] == want]


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
        if f == 0x20:
            return f"add ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x21:
            return f"addu ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x22:
            return f"sub ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x23:
            return f"subu ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x2A:
            return f"slt ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        return f"special {f:#x}"
    if o == 2:
        return f"j {(va & 0xF0000000) | ((w & 0x03FFFFFF) << 2):#x}"
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
    if o == 0x0C:
        return f"andi ${REGS[rt(w)]},${REGS[rs(w)]},{imm16(w):#x}"
    if o == 0x0F:
        return f"lui ${REGS[rt(w)]},{imm16(w):#x}"
    names = {
        0x20: "lb", 0x21: "lh", 0x23: "lw", 0x24: "lbu", 0x25: "lhu",
        0x28: "sb", 0x29: "sh", 0x2B: "sw",
    }
    if o in names:
        return f"{names[o]} ${REGS[rt(w)]},{simm16(w):#x}(${REGS[rs(w)]})"
    return f"op{o:#x} {w:08x}"


def dump_window(blob: bytes, start: int, n: int) -> None:
    for i in range(n):
        va = start + i * 4
        if va >= TEXT_END:
            break
        w = load_u32(blob, va)
        print(f"  {va:08X}  {w:08X}  {dis(w, va)}")


def collect_d278_loads(blob: bytes) -> list[tuple[int, int]]:
    """Return (va, dest_reg) for every D278 pointer load."""
    text = blob[HDR : HDR + 0x1EE000]
    loads = []
    for i in range(0, len(text), 4):
        w = struct.unpack_from("<I", text, i)[0]
        va = TADDR + i
        # gp-rel lw 0x508($gp)
        if op(w) == 0x23 and rs(w) == 28 and imm16(w) == 0x508:
            loads.append((va, rt(w)))
            continue
        # lui / lw -0x2d88
        if op(w) == 0x0F and imm16(w) in (0x8009, 0x800A) and i + 4 < len(text):
            w2 = struct.unpack_from("<I", text, i + 4)[0]
            if op(w2) == 0x23 and rs(w2) == rt(w):
                base = (imm16(w) << 16) + simm16(w2)
                if base == D278:
                    loads.append((va + 4, rt(w2)))
    return loads


def track_stores(blob: bytes, loads: list[tuple[int, int]]) -> list[dict]:
    stores = []
    seen = set()
    for va, dest in loads:
        live = {dest}
        for j in range(1, 120):
            a = va + j * 4
            if a >= TEXT_END:
                break
            ww = load_u32(blob, a)
            o = op(ww)
            if o == 0 and funct(ww) == 0x21:
                if rt(ww) == 0 and rs(ww) in live:
                    live.add(rd(ww))
                elif rs(ww) == 0 and rt(ww) in live:
                    live.add(rd(ww))
            if o == 9 and simm16(ww) == 0 and rs(ww) in live:
                live.add(rt(ww))
            if o in (0x28, 0x29, 0x2B) and rs(ww) in live:
                off = simm16(ww)
                if off in (0x0C, 0x0E, 0x1C):
                    key = (a, off, o)
                    if key not in seen:
                        seen.add(key)
                        stores.append({
                            "store_va": a,
                            "load_va": va,
                            "off": off,
                            "width": {0x28: "sb", 0x29: "sh", 0x2B: "sw"}[o],
                            "dis": dis(ww, a),
                            "src_reg": rt(ww),
                        })
            if ww == 0x03E00008:
                break
    return stores


def arith_before(blob: bytes, store_va: int) -> str:
    """Look back a few insns for add/sub/andi of the stored register."""
    bits = []
    for j in range(1, 12):
        a = store_va - j * 4
        if a < TADDR:
            break
        w = load_u32(blob, a)
        bits.append(f"{dis(w, a)}")
    return " ; ".join(reversed(bits))


def main() -> None:
    blob = EXE.read_bytes()
    assert hashlib.sha1(blob).hexdigest() == SHA1
    loads = collect_d278_loads(blob)
    print(f"D278 loads: {len(loads)}")
    stores = track_stores(blob, loads)
    print(f"\n=== D278-relative stores to +0x0C/+0x0E/+0x1C ({len(stores)}) ===")
    for s in stores:
        print(f"  {s['store_va']:08X}  {s['width']} +{s['off']:#x}  "
              f"via {s['load_va']:08X}  {s['dis']}")
        print(f"           prev: {arith_before(blob, s['store_va'])}")

    print("\n=== 2AE50 restore neighborhood ===")
    dump_window(blob, 0x8002AE40, 40)
    print("2AE?? func start guess / jal 293F4 at 2AE80")
    print("293F4 sites:", [f"{x:#x}" for x in jal_sites(blob, 0x800293F4)])
    print("2F300 sites:", [f"{x:#x}" for x in jal_sites(blob, 0x8002F300)])

    print("\n=== 2F300 (death-gate callee when HP>0) ===")
    dump_window(blob, 0x8002F300, 24)

    print("\n=== 1D340 prefix (rec=4 writer, 4D4-gated) ===")
    dump_window(blob, 0x8001D340, 48)

    print("\n=== 109B0 slot tmpl +0x0C/+0x0E/+0x1C ===")
    for off in (0x0C, 0x0E, 0x1C):
        v = struct.unpack_from("<H", blob, exe_off(0x800109B0 + off))[0]
        print(f"  109B0+{off:#x} = {v:#x} ({v})")
    print("=== 10928 default +0x0C/+0x0E/+0x1C ===")
    for off in (0x0C, 0x0E, 0x1C):
        v = struct.unpack_from("<H", blob, exe_off(0x80010928 + off))[0]
        print(f"  10928+{off:#x} = {v:#x} ({v})")

    print("\n=== 2FF78 dest is *D254, not D278 ===")
    print("2FF78 sites:", [f"{x:#x}" for x in jal_sites(blob, 0x8002FF78)])

    print("\n=== function containing 29350 ===")
    dump_window(blob, 0x800292C0, 28)


if __name__ == "__main__":
    main()
