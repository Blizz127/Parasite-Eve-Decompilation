#!/usr/bin/env python3
"""PE-BTL79 — 293F4 a0, mode=3, gp+0x104==7, jalr, 201DC, B6A80."""
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
GP = 0x8009CD70

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
        if f == 0x21:
            return f"addu ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        if f == 0x23:
            return f"subu ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        return f"special {f:#x}"
    if o == 2:
        return f"j {(va & 0xF0000000) | ((w & 0x03FFFFFF) << 2):#x}"
    if o == 3:
        return f"jal {jal_target(w):#x}"
    if o == 4:
        return f"beq ${REGS[rs(w)]},${REGS[rt(w)]}, {va + 4 + simm16(w) * 4:#x}"
    if o == 5:
        return f"bne ${REGS[rs(w)]},${REGS[rt(w)]}, {va + 4 + simm16(w) * 4:#x}"
    if o == 9:
        return f"addiu ${REGS[rt(w)]},{REGS[rs(w)]},{simm16(w)}"
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


def dump_window(blob: bytes, start: int, n: int) -> None:
    for i in range(n):
        va = start + i * 4
        if va >= TEXT_END:
            break
        w = load_u32(blob, va)
        print(f"  {va:08X}  {w:08X}  {dis(w, va)}")


def a0_at_jal(blob: bytes, jal_va: int) -> str:
    """Walk back ~12 insns for a0 materialization. Delay slot is jal+4."""
    bits = []
    delay = load_u32(blob, jal_va + 4)
    bits.append(f"delay {dis(delay, jal_va + 4)}")
    for j in range(1, 16):
        a = jal_va - j * 4
        w = load_u32(blob, a)
        bits.append(f"{a:08X} {dis(w, a)}")
        o = op(w)
        if o == 9 and rt(w) == 4:  # addiu a0
            return f"addiu $a0,$zero,{simm16(w)} | " + " ; ".join(reversed(bits[:4]))
        if o == 0 and funct(w) == 0x21 and rd(w) == 4:
            return f"addu $a0,${REGS[rs(w)]},${REGS[rt(w)]} | " + " ; ".join(reversed(bits[:4]))
        if o == 0x0D and rt(w) == 4:
            return f"ori $a0 | " + " ; ".join(reversed(bits[:4]))
    return "unknown | " + " ; ".join(reversed(bits[:6]))


def find_func_start(blob: bytes, va: int) -> int:
    start = max(TADDR, va - 0x400)
    best = va
    for a in range(va, start, -4):
        w = load_u32(blob, a)
        if op(w) == 9 and rt(w) == 29 and rs(w) == 29 and simm16(w) < 0:
            return a
        if w == 0x03E00008 and a + 8 <= va:
            return a + 8
    return best


def main() -> None:
    blob = EXE.read_bytes()
    assert hashlib.sha1(blob).hexdigest() == SHA1

    print("=== 293F4 jal a0 ===")
    for va in jal_sites(blob, 0x800293F4):
        print(f"  {va:#x}  {a0_at_jal(blob, va)}")

    print("\n=== D28C stores with nearby li ===")
    text = blob[HDR : HDR + 0x1EE000]
    for i in range(0, len(text), 4):
        w = struct.unpack_from("<I", text, i)[0]
        va = TADDR + i
        is_gp = op(w) == 0x2B and rs(w) == 28 and imm16(w) == 0x51C
        is_lui = False
        if op(w) == 0x2B and simm16(w) == -0x2D74:
            is_lui = True
        if not (is_gp or is_lui):
            continue
        src = "unk"
        for j in range(1, 8):
            ww = load_u32(blob, va - j * 4)
            if op(ww) == 9 and rt(ww) == rt(w) and rs(ww) == 0:
                src = f"li {simm16(ww)}"
                break
            if ww == 0 and rt(w) == 0:
                src = "zero"
                break
        print(f"  {va:08X}  {dis(w, va)}  src={src}")

    print("\n=== gp+0x104 stores that write 7 ===")
    for i in range(0, len(text), 4):
        w = struct.unpack_from("<I", text, i)[0]
        if op(w) != 0x28 or rs(w) != 28 or imm16(w) != 0x104:
            continue
        va = TADDR + i
        src = "unk"
        for j in range(1, 10):
            ww = load_u32(blob, va - j * 4)
            if op(ww) == 9 and rt(ww) == rt(w) and rs(ww) == 0:
                src = f"li {simm16(ww)}"
                break
            if op(ww) == 9 and rt(ww) == rt(w) and simm16(ww) == 1:
                src = f"incr {dis(ww, va - j * 4)}"
                break
        print(f"  {va:08X}  {dis(w, va)}  src={src}")

    print("\n=== table / lui materialize 19D20 33A2C 2AE60 1D340 ===")
    wants = {0x80019D20, 0x80033A2C, 0x8002AE60, 0x8001D340, 0x8002AA98}
    for i in range(0, len(blob) - 4, 4):
        w = struct.unpack_from("<I", blob, i)[0]
        if w in wants:
            va = TADDR + (i - HDR) if i >= HDR else i
            print(f"  word {w:#x} at file+{i:#x} va~{va:#x}")

    print("\n=== 201DC callers / 207B0 callers ===")
    for tgt in (0x800201DC, 0x800207B0, 0x80020780, 0x8001FC74, 0x80023ED8):
        sites = jal_sites(blob, tgt)
        print(f"  {tgt:#x} jals={ [hex(x) for x in sites] }")

    print("\n=== 2A7F8 / 2A860 (2AA98 callers) ===")
    dump_window(blob, 0x8002A7F0, 56)

    print("\n=== 2A4E0 1D340 call site ===")
    dump_window(blob, 0x8002A4C0, 24)

    print("\n=== B6A80 materialize stores ===")
    for i in range(0, len(text) - 8, 4):
        w = struct.unpack_from("<I", text, i)[0]
        if op(w) != 0x0F:
            continue
        w2 = struct.unpack_from("<I", text, i + 4)[0]
        if rs(w2) != rt(w):
            continue
        base = (imm16(w) << 16) + simm16(w2)
        if base == 0x800B6A80:
            va = TADDR + i
            print(f"  {va:08X}  {dis(w, va)}")
            print(f"  {va+4:08X}  {dis(w2, va+4)}")
            dump_window(blob, va, 8)

    print("\n=== 910A0 slots pointing at 19D20/33A2C ===")
    for i in range(0x200):
        t = load_u32(blob, 0x800910A0 + i * 4)
        if t in (0x80019D20, 0x80033A2C, 0x8002AE60, 0x8001D340):
            print(f"  table[{i:#x}] = {t:#x}")

    print("\n=== 108F0 JT ===")
    for i in range(8):
        print(f"  [{i}] {load_u32(blob, 0x800108F0 + i * 4):#x}")

    print("\n=== 207DC neighborhood func start ===")
    print(f"start {find_func_start(blob, 0x800207DC):#x}")
    dump_window(blob, find_func_start(blob, 0x800207DC), 20)
    print("207DC jals of containing func:")
    start = find_func_start(blob, 0x800207DC)
    print("jals to start", [hex(x) for x in jal_sites(blob, start)])


if __name__ == "__main__":
    main()
