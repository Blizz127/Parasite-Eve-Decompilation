#!/usr/bin/env python3
"""PE-BTL76 — classify HP stores, 4D4/1D340 gates, type-3 after 0x85, 0xAE."""
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


def store_sites(blob: bytes, gp_off: int, ops: tuple[int, ...]) -> list[int]:
    text = blob[HDR : HDR + 0x1EE000]
    out = []
    for i in range(0, len(text), 4):
        w = struct.unpack_from("<I", text, i)[0]
        if op(w) in ops and rs(w) == 28 and imm16(w) == gp_off:
            out.append(TADDR + i)
    return out


def lui_store_sites(blob: bytes, addr: int, ops: tuple[int, ...]) -> list[int]:
    """lui/addiu or lui/sb/sh/sw forming addr."""
    text = blob[HDR : HDR + 0x1EE000]
    out = []
    for i in range(0, len(text) - 4, 4):
        w = struct.unpack_from("<I", text, i)[0]
        if op(w) != 0x0F:
            continue
        w2 = struct.unpack_from("<I", text, i + 4)[0]
        if op(w2) not in ops:
            continue
        if rs(w2) != rt(w):
            continue
        base = (imm16(w) << 16) + simm16(w2)
        if base == addr:
            out.append(TADDR + i + 4)
    return out


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


def decode(blob: bytes, off: int, limit: int = 40) -> None:
    print(f"\n=== stream chunk2+{off:#x} ===")
    pc = 0
    n = 0
    while n < limit and pc + 8 <= len(blob) - off:
        word = struct.unpack_from("<I", blob, off + pc)[0]
        word2 = struct.unpack_from("<I", blob, off + pc + 4)[0]
        opc = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = kinds_of(word, word2, argc)
        imms = [struct.unpack_from("<I", blob, off + pc + 8 + i * 4)[0]
                for i in range(argc)]
        print(f"  +{pc:04X}  op={opc:#06x} argc={argc} kinds={kinds}")
        for i, imm in enumerate(imms):
            print(f"         arg{i} k{kinds[i]} {imm:#010x}")
        if opc == 0x00:
            break
        pc += 8 + argc * 4
        n += 1


def main() -> None:
    blob = EXE.read_bytes()
    assert hashlib.sha1(blob).hexdigest() == SHA1

    print("=== 4D4 (gp+0x4D4 = D_8009D244) stores ===")
    for va in store_sites(blob, 0x4D4, (0x28, 0x29, 0x2B)):
        print(f"  {va:08X}  {dis(load_u32(blob, va), va)}")
    print("lui-path:")
    for va in lui_store_sites(blob, GP + 0x4D4, (0x28, 0x29, 0x2B)):
        print(f"  {va:08X}  {dis(load_u32(blob, va), va)}")

    print("\n=== gp+0x104 (D_8009CE74) stores ===")
    for va in store_sites(blob, 0x104, (0x28, 0x29, 0x2B)):
        print(f"  {va:08X}  {dis(load_u32(blob, va), va)}")

    print("\n=== D28C mode stores (gp+0x51C) ===")
    for va in store_sites(blob, 0x51C, (0x28, 0x29, 0x2B)):
        print(f"  {va:08X}  {dis(load_u32(blob, va), va)}")
    print("lui-path D28C:")
    for va in lui_store_sites(blob, 0x8009D28C, (0x28, 0x29, 0x2B)):
        print(f"  {va:08X}  {dis(load_u32(blob, va), va)}")

    print("\n=== 1D340 jal sites ===")
    print([f"{x:#x}" for x in jal_sites(blob, 0x8001D340)])
    print("2AA98 jal sites:", [f"{x:#x}" for x in jal_sites(blob, 0x8002AA98)])
    print("2AE60 jal sites:", [f"{x:#x}" for x in jal_sites(blob, 0x8002AE60)])
    print("33A2C jal sites:", [f"{x:#x}" for x in jal_sites(blob, 0x80033A2C)])
    print("19D20 jal sites:", [f"{x:#x}" for x in jal_sites(blob, 0x80019D20)])

    print("\n=== 2AA98 prefix (JT gp+0x104) ===")
    dump_window(blob, 0x8002AA80, 40)

    print("\n=== 1E920 DAMAGE neighborhood ===")
    dump_window(blob, 0x8001E900, 28)
    print("\n=== 1F6E0 DAMAGE neighborhood ===")
    dump_window(blob, 0x8001F6D0, 24)
    print("\n=== 201F0 DAMAGE neighborhood ===")
    dump_window(blob, 0x800201E8, 28)
    print("\n=== 1F490 DEATH/CLEAR neighborhood ===")
    dump_window(blob, 0x8001F480, 20)
    print("\n=== 1F410 mode-3 store ===")
    dump_window(blob, 0x8001F400, 16)

    print("\n=== opcode 0xAE table ===")
    ae = load_u32(blob, 0x800910A0 + 0xAE * 4)
    print(f"D_800910A0[0xAE] = {ae:#x}")
    dump_window(blob, ae, 40)

    print("\n=== opcode 0x2A table ===")
    op2a = load_u32(blob, 0x800910A0 + 0x2A * 4)
    print(f"D_800910A0[0x2A] = {op2a:#x}")

    chunk2 = load_chunk2()
    print("\n=== type-3 from start through both 0x85 hops ===")
    decode(chunk2, 0x227FC, 30)
    print("\n=== type-6 from +0 ===")
    decode(chunk2, 0x2341C, 50)
    print("\n=== type-6 +0x1850 ===")
    decode(chunk2, 0x2341C + 0x1850, 12)


if __name__ == "__main__":
    main()
