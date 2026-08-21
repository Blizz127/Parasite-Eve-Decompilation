#!/usr/bin/env python3
"""Size and head-dump unresolved/matching callees of func_80030894."""
from __future__ import annotations

import hashlib
import struct
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800

CALLEES = [
    0x800370DC, 0x80037140, 0x8005DADC,
    0x80077A64, 0x80077AA4, 0x80077B04, 0x80077B34,
    0x80077B64, 0x80077BA4, 0x80077BC4, 0x80077C44, 0x80077C64,
]

KNOWN_NEXT = {
    0x800370DC: 0x80037140,
    0x80037140: 0x800371A4,  # matching leaf after
    0x80077B64: 0x80077B84,
    0x80077BA4: 0x80077BC4,
    0x80077BC4: 0x80077BE4,
    0x80077C44: 0x80077C64,
    0x80077C64: 0x80077C84,
}


def exe_off(addr: int) -> int:
    return addr - LOAD + EXE_HDR


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def find_end(data: bytes, start: int) -> int:
    if start in KNOWN_NEXT:
        return KNOWN_NEXT[start]
    # scan forward for addiu sp,sp,-N prologue after a jr ra
    pc = start + 4
    last_jr = None
    limit = start + 0x800
    while pc < limit:
        w = load_u32(data, pc)
        if w == 0x03E00008:
            last_jr = pc
            delay = load_u32(data, pc + 4)
            nxt = pc + 8
            nw = load_u32(data, nxt)
            # typical next prologue addiu sp, sp, -imm (op=9, rs=29, rt=29)
            if ((nw >> 26) & 0x3F) == 9 and ((nw >> 21) & 0x1F) == 29 and ((nw >> 16) & 0x1F) == 29:
                return nxt
            # or another jr-sized leaf
        pc += 4
    return (last_jr + 8) if last_jr else start + 0x40


def dis_word(pc: int, w: int) -> str:
    op = (w >> 26) & 0x3F
    rs = (w >> 21) & 0x1F
    rt = (w >> 16) & 0x1F
    rd = (w >> 11) & 0x1F
    sa = (w >> 6) & 0x1F
    fn = w & 0x3F
    imm = w & 0xFFFF
    REG = [
        "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
        "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
        "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
        "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
    ]
    if op == 0 and fn == 8:
        return f"jr ${REG[rs]}"
    if op == 0 and fn == 0x21:
        return f"addu ${REG[rd]}, ${REG[rs]}, ${REG[rt]}"
    if op == 0 and fn == 0:
        return f"sll ${REG[rd]}, ${REG[rt]}, {sa}"
    if op == 3:
        tgt = ((w & 0x03FFFFFF) << 2) | 0x80000000
        return f"jal {tgt:#010x}"
    if op == 9:
        simm = imm - 0x10000 if imm & 0x8000 else imm
        return f"addiu ${REG[rt]}, ${REG[rs]}, {simm:#x}"
    if op == 13:
        return f"ori ${REG[rt]}, ${REG[rs]}, {imm:#x}"
    if op == 12:
        return f"andi ${REG[rt]}, ${REG[rs]}, {imm:#x}"
    if op == 15:
        return f"lui ${REG[rt]}, {imm:#x}"
    if op == 35:
        simm = imm - 0x10000 if imm & 0x8000 else imm
        return f"lw ${REG[rt]}, {simm:#x}(${REG[rs]})"
    if op == 43:
        simm = imm - 0x10000 if imm & 0x8000 else imm
        return f"sw ${REG[rt]}, {simm:#x}(${REG[rs]})"
    if op == 40:
        simm = imm - 0x10000 if imm & 0x8000 else imm
        return f"sb ${REG[rt]}, {simm:#x}(${REG[rs]})"
    if op == 41:
        simm = imm - 0x10000 if imm & 0x8000 else imm
        return f"sh ${REG[rt]}, {simm:#x}(${REG[rs]})"
    if op == 32:
        simm = imm - 0x10000 if imm & 0x8000 else imm
        return f"lb ${REG[rt]}, {simm:#x}(${REG[rs]})"
    if op == 36:
        simm = imm - 0x10000 if imm & 0x8000 else imm
        return f"lbu ${REG[rt]}, {simm:#x}(${REG[rs]})"
    if op == 4:
        simm = imm - 0x10000 if imm & 0x8000 else imm
        tgt = pc + 4 + (simm << 2)
        return f"beq ${REG[rs]}, ${REG[rt]}, {tgt:#010x}"
    if op == 5:
        simm = imm - 0x10000 if imm & 0x8000 else imm
        tgt = pc + 4 + (simm << 2)
        return f"bne ${REG[rs]}, ${REG[rt]}, {tgt:#010x}"
    return f"{w:#010x}"


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    data = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1

    for addr in CALLEES:
        end = find_end(data, addr)
        words = (end - addr) // 4
        print(f"\n=== {addr:#010x}  end={end:#010x}  words={words}  bytes={end-addr:#x} ===")
        pc = addr
        n = 0
        while pc < end and n < 40:
            w = load_u32(data, pc)
            print(f"{pc:08X}  {w:08X}  {dis_word(pc, w)}")
            pc += 4
            n += 1
        if end - addr > 40 * 4:
            print(f"  ... ({words - 40} more words)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
