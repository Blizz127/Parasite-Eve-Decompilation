#!/usr/bin/env python3
"""Dump authentic type-0 / type-3 first-visit streams and 3E188 prefix."""
from __future__ import annotations

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
CHUNK2 = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
TADDR = 0x80010000
REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def exe_off(a: int) -> int:
    return a - TADDR + 0x800


def load_u32(data: bytes, a: int) -> int:
    return struct.unpack_from("<I", data, exe_off(a))[0]


def dis(word: int, addr: int) -> str:
    op = word >> 26
    rs, rt, rd, sa = (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31, (word >> 6) & 31
    fn = word & 63
    imm = word & 0xFFFF
    simm = imm - 0x10000 if imm >= 0x8000 else imm
    tgt = ((word & 0x03FFFFFF) << 2) | (addr & 0xF0000000)
    if op == 0:
        if word == 0:
            return "nop"
        names = {
            0: f"sll {REGS[rd]}, {REGS[rt]}, {sa}",
            2: f"srl {REGS[rd]}, {REGS[rt]}, {sa}",
            3: f"sra {REGS[rd]}, {REGS[rt]}, {sa}",
            8: f"jr {REGS[rs]}",
            9: f"jalr {REGS[rs]}",
            0x21: f"addu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x23: f"subu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x24: f"and {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x25: f"or {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2A: f"slt {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2B: f"sltu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
        }
        return names.get(fn, f"spec {fn:02x}")
    if op == 1:
        tgtb = addr + 4 + simm * 4
        return {0: f"bltz {REGS[rs]}, {tgtb:#x}", 1: f"bgez {REGS[rs]}, {tgtb:#x}"}.get(rt, f"regimm {rt}")
    ops = {
        2: f"j {tgt:#x}",
        3: f"jal {tgt:#x}",
        4: f"beq {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}",
        5: f"bne {REGS[rs]}, {REGS[rt]}, {addr + 4 + simm * 4:#x}",
        6: f"blez {REGS[rs]}, {addr + 4 + simm * 4:#x}",
        7: f"bgtz {REGS[rs]}, {addr + 4 + simm * 4:#x}",
        8: f"addi {REGS[rt]}, {REGS[rs]}, {simm}",
        9: f"addiu {REGS[rt]}, {REGS[rs]}, {simm}",
        0x0A: f"slti {REGS[rt]}, {REGS[rs]}, {simm}",
        0x0B: f"sltiu {REGS[rt]}, {REGS[rs]}, {simm}",
        0x0C: f"andi {REGS[rt]}, {REGS[rs]}, {imm:#x}",
        0x0D: f"ori {REGS[rt]}, {REGS[rs]}, {imm:#x}",
        0x0E: f"xori {REGS[rt]}, {REGS[rs]}, {imm:#x}",
        0x0F: f"lui {REGS[rt]}, {imm:#x}",
        0x12: f"cop2 {word & 0x3FFFFFF:07x}",
        0x20: f"lb {REGS[rt]}, {simm}({REGS[rs]})",
        0x21: f"lh {REGS[rt]}, {simm}({REGS[rs]})",
        0x23: f"lw {REGS[rt]}, {simm}({REGS[rs]})",
        0x24: f"lbu {REGS[rt]}, {simm}({REGS[rs]})",
        0x25: f"lhu {REGS[rt]}, {simm}({REGS[rs]})",
        0x28: f"sb {REGS[rt]}, {simm}({REGS[rs]})",
        0x29: f"sh {REGS[rt]}, {simm}({REGS[rs]})",
        0x2B: f"sw {REGS[rt]}, {simm}({REGS[rs]})",
    }
    return ops.get(op, f"op{op:02x} {word:08x}")


def decode_vm(data: bytes, off: int, nwords: int = 80) -> None:
    i = 0
    while i < nwords:
        word = struct.unpack_from("<I", data, off + i * 4)[0]
        word2 = struct.unpack_from("<I", data, off + (i + 1) * 4)[0] if i + 1 < nwords else 0
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = word >> 17
        print(f"  +{i * 4:04X} word={word:08X} op={op:#06x} argc={argc} kinds={kinds:05x} w2={word2:08X}")
        imms = []
        for a in range(argc):
            imm = struct.unpack_from("<I", data, off + (i + 2 + a) * 4)[0]
            kind = kinds & 7
            imms.append(f"k{kind}:{imm:#x}")
            kinds >>= 3
            if a == 4:
                kinds = word2
        if imms:
            print(f"         imms {imms}")
        i += 2 + argc


def main() -> None:
    root = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1

    print("=== 3E188 0x8003E188..0x8003E474 ===")
    for addr in range(0x8003E188, 0x8003E474, 4):
        w = load_u32(exe, addr)
        print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")

    print("\n=== 3A6A8 prefix ===")
    for addr in range(0x8003A6A8, 0x8003A6E0, 4):
        w = load_u32(exe, addr)
        print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")

    chunk_path = None
    for cand in [
        root / "local" / "btl0" / "chunk2.bin",
        root / "rom" / "image" / "chunk2.bin",
    ]:
        if cand.is_file():
            chunk_path = cand
            break
    if chunk_path is None:
        # Find PE.IMG / packed chunk2 via existing helper notes.
        disc = root / "local" / "pe_disc1.path"
        print(f"\nno chunk2.bin; disc pointer {disc} exists={disc.is_file()}")
        if disc.is_file():
            print(disc.read_text().strip())
        return

    chunk = chunk_path.read_bytes()
    print(f"\nchunk2 {chunk_path} sha={hashlib.sha256(chunk).hexdigest()}")
    print("=== type 0 script chunk2+0x202C8 ===")
    decode_vm(chunk, 0x202C8, 40)
    print("=== type 3 script chunk2+0x227FC ===")
    decode_vm(chunk, 0x227FC, 90)


if __name__ == "__main__":
    main()
