#!/usr/bin/env python3
"""Can 2AA98 run when 4D4==0? Where is type-6 0xAE? JT[7] proof."""
from __future__ import annotations

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
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


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


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


def kinds_of(word: int, word2: int, argc: int) -> list[int]:
    k = word >> 17
    out = []
    for i in range(argc):
        if i == 5:
            k = word2
        out.append(k & 7)
        k >>= 3
    return out


def load_chunk2() -> bytes:
    pointer = (ROOT / "local" / "pe_disc1.path").read_text().strip().splitlines()[0].strip()
    sec0, sec1, sec2 = PACKED & 0xFF, (PACKED >> 8) & 0xFFF, PACKED >> 20
    out = bytearray()
    with open(pointer, "rb") as fh:
        for i in range(sec2):
            fh.seek((PE_IMG_LBA + REL + sec0 + sec1 + i) * 2352 + 24)
            out += fh.read(2048)
    data = bytes(out)
    assert hashlib.sha256(data).hexdigest() == CHUNK2_SHA
    return data


def find_ops(blob: bytes, off: int, want: set[int], limit: int = 400) -> None:
    pc = 0
    n = 0
    while n < limit and pc + 8 <= 0x3000:
        word = struct.unpack_from("<I", blob, off + pc)[0]
        word2 = struct.unpack_from("<I", blob, off + pc + 4)[0]
        opc = word & 0x1FFF
        argc = (word >> 13) & 0xF
        if opc in want or opc in (0x12, 0x55, 0xAE, 0x2A, 0x89):
            kinds = kinds_of(word, word2, argc)
            imms = [struct.unpack_from("<I", blob, off + pc + 8 + i * 4)[0]
                    for i in range(min(argc, 8))]
            print(f"  +{pc:04X}  op={opc:#06x} argc={argc} kinds={kinds} imms={[hex(x) for x in imms]}")
        if opc == 0x00 and n > 0:
            pass
        pc += 8 + argc * 4
        n += 1


def main() -> None:
    blob = EXE.read_bytes()
    assert hashlib.sha1(blob).hexdigest() == SHA1

    print("=== 299CC 4D4 gate and 2A7F8 / 2A880 / 2A8C0 ===")
    dump_window(blob, 0x80029A64, 20)
    print("--- 2A7F0 ---")
    dump_window(blob, 0x8002A7F0, 48)
    print("--- 2A860 ---")
    dump_window(blob, 0x8002A860, 36)

    print("\n=== 2AA98 JT at 0x800108F0 ===")
    for i in range(8):
        t = load_u32(blob, 0x800108F0 + i * 4)
        print(f"  [{i}] {t:#x}")

    print("\n=== 0xAF / 0xAD table ===")
    print(f"0xAD {load_u32(blob, 0x800910A0 + 0xAD * 4):#x}")
    print(f"0xAF {load_u32(blob, 0x800910A0 + 0xAF * 4):#x}")
    print(f"0x00 {load_u32(blob, 0x800910A0 + 0x00 * 4):#x}")

    print("\n=== 21F00 mode store neighborhood ===")
    dump_window(blob, 0x80021EE8, 16)

    print("\n=== 2CF24 mode-7 (already ported?) ===")
    dump_window(blob, 0x8002CEE0, 20)

    chunk2 = load_chunk2()
    print("\n=== type-6 scan 0x12/0x55/0xAE/0x2A/0x89 ===")
    find_ops(chunk2, 0x2341C, {0x12, 0x55, 0xAE, 0x2A, 0x89, 0x00})

    print("\n=== type-6 +0xFAC ===")
    off = 0x2341C + 0xFAC
    word = struct.unpack_from("<I", chunk2, off)[0]
    print(f"  word={word:08X} op={word & 0x1FFF:#x}")


if __name__ == "__main__":
    main()
