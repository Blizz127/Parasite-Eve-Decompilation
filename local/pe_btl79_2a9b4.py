#!/usr/bin/env python3
"""Live 299CC tail at 2A9B4 when mode==0 and 4D4==0."""
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

REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, va - TADDR + HDR)[0]


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
    if o == 9:
        return f"addiu ${REGS[rt(w)]},${REGS[rs(w)]},{simm16(w)}"
    if o == 0x0B:
        return f"sltiu ${REGS[rt(w)]},${REGS[rs(w)]},{imm16(w):#x}"
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
        w = load_u32(blob, va)
        print(f"  {va:08X}  {w:08X}  {dis(w, va)}")


def ptr_sites(blob: bytes, addr: int) -> list[int]:
    out = []
    for i in range(0, len(blob) - 3, 4):
        if struct.unpack_from("<I", blob, i)[0] == addr:
            out.append(i)
    return out


def main() -> None:
    blob = EXE.read_bytes()
    assert hashlib.sha1(blob).hexdigest() == SHA1

    print("=== 2A9B0 live mode-0 dispatcher ===")
    dump_window(blob, 0x8002A9B0, 80)

    print("\n=== 2AA20 / 2AA24 epilogue ===")
    dump_window(blob, 0x8002AA00, 40)

    print("\n=== pointer words for dead funcs ===")
    for addr in (0x80021DE8, 0x80019D20, 0x80033A2C, 0x8002CEE0, 0x8002CF24,
                 0x8002AE60, 0x8001D340, 0x80025EE8, 0x8002B0E8):
        sites = ptr_sites(blob, addr)
        print(f"  {addr:#x} count={len(sites)} file={[hex(x) for x in sites[:12]]}")

    print("\n=== 17FE0 mode 5/6 func + jals ===")
    dump_window(blob, 0x80017FD0, 20)
    print("17FDC jals", [hex(x) for x in jal_sites(blob, 0x80017FDC)])
    print("17FF0 jals", [hex(x) for x in jal_sites(blob, 0x80017FF0)])

    print("\n=== 29350 death-reader func start / jals ===")
    dump_window(blob, 0x80029340, 20)
    # walk back to addiu sp
    start = 0x80029340
    for a in range(0x80029340, 0x80029000, -4):
        w = load_u32(blob, a)
        if op(w) == 9 and rt(w) == 29 and rs(w) == 29 and simm16(w) < 0:
            start = a
            break
    print("start", hex(start), "jals", [hex(x) for x in jal_sites(blob, start)])


if __name__ == "__main__":
    main()
