#!/usr/bin/env python3
"""PE-BTL2 independent oracle: opcode 0x55 func_800144FC → jal 0x80029810.

Verifies the SHA-1-exact handler, D_800B0CD8+0xF4 jump table, sole
jal of func_80029810, and that 29810 jals the HP cut with a0=0.
Does not import production C. Does not claim 0x55 completion.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x800144FC
FUNC_END = 0x80014694  # exclusive; next leaf
TABLE = 0x800910A0
JT = 0x800100A0
GP = 0x8009CD70
JAL_29810 = 0x8001461C
INIT = 0x80029810
HP_JAL = 0x80029910
HP_DELAY = 0x80029914
HP_CUT = 0x800293F4

WORDS = [
    0x27BDFFE0, 0xAFB10014, 0x00808821, 0xAFB00010,
    0x3C10800B, 0x26100CD8, 0xAFBF0018, 0x920300F4,
    0x00000000, 0x2C62003C, 0x1040004C, 0x00031080,
    0x3C018001, 0x00220821, 0x8C2200A0, 0x00000000,
    0x00400008, 0x00000000, 0x9202000E, 0x00000000,
    0x30420003, 0x14400002, 0x24020037, 0xA20200F4,
    0x8E020000, 0x3C030080, 0x00431025, 0x08005198,
    0xAE020000, 0x3C02800B, 0x8C420CD8, 0x3C030040,
    0x00431024, 0x14400004, 0x24020038, 0x0C010BB7,
    0x00000000, 0x24020038, 0x08005146, 0xA20200F4,
    0x0C01B583, 0x24040001, 0x24030001, 0x1043002D,
    0x3C030040, 0x3C02800B, 0x8C420CD8, 0x00000000,
    0x00431024, 0x14400004, 0x24020039, 0x0C010BC8,
    0x00000000, 0x24020039, 0x08005146, 0xA20200F4,
    0x0C01A453, 0x24040001, 0x24030001, 0x1043001D,
    0x2402003A, 0x08005146, 0xA20200F4, 0x3C02800A,
    0x8C42D1A0, 0x00000000, 0x34420002, 0x3C01800A,
    0xAC22D1A0, 0x8E220000, 0x00000000, 0x90440000,
    0x0C00A604, 0x00000000, 0x2402003B, 0x08005198,
    0xA20200F4, 0x9202000E, 0x00000000, 0x30420003,
    0x14400008, 0x3C03FF7F, 0x8E020000, 0x3463FFFF,
    0xA20000F4, 0x00431024, 0xAE020000, 0x0800519F,
    0x24020001, 0x00001021, 0x8F830090, 0x8F840590,
    0x2463FFF4, 0xAF830090, 0x24030001, 0xAC830010,
    0x8FBF0018, 0x8FB10014, 0x8FB00010, 0x27BD0020,
    0x03E00008, 0x00000000,
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x3FFFFFF) << 2) | 0x80000000


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    require((FUNC_END - FUNC) // 4 == 102, "102 words")
    require(len(WORDS) == 102, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")
    require(load_u32(data, TABLE + 0x55 * 4) == FUNC, "D_800910A0[0x55]")
    require(WORDS[4] == 0x3C10800B and WORDS[5] == 0x26100CD8, "s0=D_800B0CD8")
    require((WORDS[7] >> 26) == 0x24 and (WORDS[7] & 0xFFFF) == 0xF4,
            "lbu +0xF4")
    require(WORDS[9] == 0x2C62003C, "sltiu 0x3C")
    require(WORDS[14] == 0x8C2200A0, "lw JT 0x800100A0")
    require(load_u32(data, JT + 0) == 0x80014544, "state 0")
    for i in range(1, 0x37):
        require(load_u32(data, JT + i * 4) == 0x80014658, f"state {i:#x} default")
    require(load_u32(data, JT + 0x37 * 4) == 0x80014570, "state 0x37")
    require(load_u32(data, JT + 0x38 * 4) == 0x8001459C, "state 0x38")
    require(load_u32(data, JT + 0x39 * 4) == 0x800145DC, "state 0x39")
    require(load_u32(data, JT + 0x3A * 4) == 0x800145F8, "state 0x3A")
    require(load_u32(data, JT + 0x3B * 4) == 0x80014630, "state 0x3B")
    require(jal_target(load_u32(data, JAL_29810)) == INIT, "jal 29810")
    require(WORDS[72] == 0x0C00A604, "encoded jal 29810")
    jal = 0x0C000000 | ((INIT & 0x0FFFFFFF) >> 2)
    hits = [
        0x80010000 + (i - 0x800)
        for i in range(0, len(data) - 4, 4)
        if struct.unpack_from("<I", data, i)[0] == jal
    ]
    require(hits == [JAL_29810], f"sole jal 29810 {hits}")
    require(WORDS[89] == 0x00001021, "return 0 park")
    require(WORDS[90] == 0x8F830090, "lw gp+0x90")
    require(WORDS[92] == 0x2463FFF4, "rewind gp+0x90 by 12")
    require(WORDS[93] == 0xAF830090, "sw rewinded pc")
    require(jal_target(load_u32(data, HP_JAL)) == HP_CUT, "29810 jal 293F4")
    require(load_u32(data, HP_DELAY) == 0x00002021, "293F4 a0=0")
    require(WORDS[-2] == 0x03E00008, "jr $ra")
    print("PASS: func_800144FC 102/102 + JT + sole jal 29810 + 293F4(0)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
