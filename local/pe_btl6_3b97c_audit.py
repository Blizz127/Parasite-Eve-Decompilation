#!/usr/bin/env python3
"""PE-BTL6 research: dump func_8003B97C words and COP2."""
from __future__ import annotations

import hashlib
import struct
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800
FN = 0x8003B97C
END = 0x8003BCE0
REGS = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]
C2D = [
    "VXY0", "VZ0", "VXY1", "VZ1", "VXY2", "VZ2", "RGB", "OTZ",
    "IR0", "IR1", "IR2", "IR3", "SXY0", "SXY1", "SXY2", "SXYP",
    "SZ0", "SZ1", "SZ2", "SZ3", "RGB0", "RGB1", "RGB2", "RES1",
    "MAC0", "MAC1", "MAC2", "MAC3", "IRGB", "ORGB", "LZCS", "LZCR",
]
C2C = [
    "R11R12", "R13R21", "R22R23", "R31R32", "R33", "TRX", "TRY", "TRZ",
    "L11L12", "L13L21", "L22L23", "L31L32", "L33", "RBK", "GBK", "BBK",
    "LR1LR2", "LR3LG1", "LG2LG3", "LB1LB2", "LB3", "RFC", "GFC", "BFC",
    "OFX", "OFY", "H", "DQA", "DQB", "ZSF3", "ZSF4", "FLAG",
]


def va2off(va: int) -> int:
    return va - LOAD + EXE_HDR


def word_at(exe: bytes, va: int) -> int:
    return struct.unpack_from("<I", exe, va2off(va))[0]


def simm16(imm: int) -> int:
    return imm - 0x10000 if imm >= 0x8000 else imm


def decode_gte(imm: int) -> str:
    op = imm & 0x3F
    sf = (imm >> 19) & 1
    mx = (imm >> 17) & 3
    v = (imm >> 15) & 3
    cv = (imm >> 13) & 3
    lm = (imm >> 10) & 1
    names = {
        0x01: "RTPS",
        0x06: "NCLIP",
        0x0C: "OP",
        0x10: "DPCS",
        0x11: "INTPL",
        0x12: "MVMVA",
        0x13: "NCDS",
        0x14: "CDP",
        0x16: "NCDT",
        0x1B: "NCCS",
        0x1C: "CC",
        0x1E: "NCS",
        0x20: "NCT",
        0x28: "SQR",
        0x29: "DCPL",
        0x2A: "DPCT",
        0x2D: "AVSZ3",
        0x2E: "AVSZ4",
        0x30: "RTPT",
        0x3D: "GPF",
        0x3E: "GPL",
        0x3F: "NCCT",
    }
    name = names.get(op, f"op{op:#x}")
    return f"{name} raw={imm:#x} sf={sf} mx={mx} v={v} cv={cv} lm={lm}"


def decode(w: int) -> str:
    op = w >> 26
    rs = (w >> 21) & 31
    rt = (w >> 16) & 31
    rd = (w >> 11) & 31
    sa = (w >> 6) & 31
    fn = w & 63
    imm = w & 0xFFFF
    simm = simm16(imm)
    if op == 0:
        special = {
            0x00: f"sll ${REGS[rd]}, ${REGS[rt]}, {sa}" if w else "nop",
            0x02: f"srl ${REGS[rd]}, ${REGS[rt]}, {sa}",
            0x03: f"sra ${REGS[rd]}, ${REGS[rt]}, {sa}",
            0x04: f"sllv ${REGS[rd]}, ${REGS[rt]}, ${REGS[rs]}",
            0x06: f"srlv ${REGS[rd]}, ${REGS[rt]}, ${REGS[rs]}",
            0x07: f"srav ${REGS[rd]}, ${REGS[rt]}, ${REGS[rs]}",
            0x08: f"jr ${REGS[rs]}",
            0x09: f"jalr ${REGS[rd]}, ${REGS[rs]}",
            0x10: f"mfhi ${REGS[rd]}",
            0x12: f"mflo ${REGS[rd]}",
            0x18: f"mult ${REGS[rs]}, ${REGS[rt]}",
            0x19: f"multu ${REGS[rs]}, ${REGS[rt]}",
            0x1A: f"div ${REGS[rs]}, ${REGS[rt]}",
            0x1B: f"divu ${REGS[rs]}, ${REGS[rt]}",
            0x20: f"add ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
            0x21: f"addu ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
            0x22: f"sub ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
            0x23: f"subu ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
            0x24: f"and ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
            0x25: f"or ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
            0x26: f"xor ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
            0x27: f"nor ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
            0x2A: f"slt ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
            0x2B: f"sltu ${REGS[rd]}, ${REGS[rs]}, ${REGS[rt]}",
        }
        return special.get(fn, f"special fn={fn:#x}")
    if op == 1:
        return f"regimm rt={rt} ${REGS[rs]}, {simm}"
    if op == 2:
        return f"j {((w & 0x03FFFFFF) << 2) | 0x80000000:#010x}"
    if op == 3:
        return f"jal {((w & 0x03FFFFFF) << 2) | 0x80000000:#010x}"
    if op == 4:
        return f"beq ${REGS[rs]}, ${REGS[rt]}, {simm}"
    if op == 5:
        return f"bne ${REGS[rs]}, ${REGS[rt]}, {simm}"
    if op == 6:
        return f"blez ${REGS[rs]}, {simm}"
    if op == 7:
        return f"bgtz ${REGS[rs]}, {simm}"
    if op == 8:
        return f"addi ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 9:
        return f"addiu ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 0x0A:
        return f"slti ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 0x0B:
        return f"sltiu ${REGS[rt]}, ${REGS[rs]}, {simm}"
    if op == 0x0C:
        return f"andi ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0D:
        return f"ori ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0E:
        return f"xori ${REGS[rt]}, ${REGS[rs]}, {imm:#x}"
    if op == 0x0F:
        return f"lui ${REGS[rt]}, {imm:#x}"
    if op == 0x10:
        return f"cop0 rs={rs} rt={rt} rd={rd}"
    if op == 0x12:
        cop_rs = rs
        if cop_rs == 0:
            return f"mfc2 ${REGS[rt]}, {C2D[rd]}"
        if cop_rs == 2:
            return f"cfc2 ${REGS[rt]}, {C2C[rd]}"
        if cop_rs == 4:
            return f"mtc2 ${REGS[rt]}, {C2D[rd]}"
        if cop_rs == 6:
            return f"ctc2 ${REGS[rt]}, {C2C[rd]}"
        if cop_rs == 0x10 or (w & 0x02000000):
            return f"cop2 {decode_gte(w & 0x1FFFFFF)}"
        return f"cop2 raw={w:#x}"
    loads = {
        0x20: "lb",
        0x21: "lh",
        0x23: "lw",
        0x24: "lbu",
        0x25: "lhu",
        0x28: "sb",
        0x29: "sh",
        0x2B: "sw",
        0x31: "lwc1",
        0x32: "lwc2",
        0x39: "swc1",
        0x3A: "swc2",
    }
    if op in loads:
        extra = ""
        if op in (0x32, 0x3A):
            extra = f" [{C2D[rt]}]"
        return f"{loads[op]} ${REGS[rt]}, {simm:#x}(${REGS[rs]}){extra}"
    return f"op={op:#x}"


def main() -> int:
    exe = (Path(__file__).resolve().parents[1] / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    print(f"func_8003B97C {(END - FN) // 4} words")
    for va in range(FN, END, 4):
        w = word_at(exe, va)
        print(f"{va:08X}  {w:08X}  {decode(w)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
