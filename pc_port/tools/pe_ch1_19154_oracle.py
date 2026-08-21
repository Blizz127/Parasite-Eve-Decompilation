#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_80019154 opcode 0x94 mode read (7 words).

Verifies the SHA-1-exact EXE window, jump-table slot, and D_8009D28C
word-copy contract. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x80019154
FUNC_END = 0x80019170  # exclusive; next leaf starts addiu $sp
TABLE_SLOT = 0x800912F0  # D_800910A0[0x94]
MODE = 0x8009D28C
TWIN = 0x80017FF0
TABLE_89 = 0x800912C4  # D_800910A0[0x89]

WORDS = [
    0x8C830000,  # lw    $v1, 0($a0)          dest ptr
    0x3C02800A,  # lui   $v0, 0x800A
    0x8C42D28C,  # lw    $v0, -0x2D74($v0)    D_8009D28C
    0x00000000,  # nop
    0xAC620000,  # sw    $v0, 0($v1)          *dest
    0x03E00008,  # jr    $ra
    0x24020001,  # addiu $v0, $zero, 1
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


def verify_words(data: bytes) -> None:
    require((FUNC_END - FUNC) // 4 == 7, "7 words")
    require(len(WORDS) == 7, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require((WORDS[0] >> 26) == 0x23, "lw dest ptr")
    require((WORDS[0] & 0xFFFF) == 0, "lw 0($a0)")
    require((WORDS[1] >> 16) == 0x3C02, "lui $v0")
    require((WORDS[1] & 0xFFFF) == 0x800A, "lui 0x800A")
    imm = WORDS[2] & 0xFFFF
    signed = imm - 0x10000 if imm & 0x8000 else imm
    require((0x800A0000 + signed) & 0xFFFFFFFF == MODE, "lw D_8009D28C")
    require((WORDS[2] >> 26) == 0x23, "lw mode")
    require(WORDS[3] == 0, "nop")
    require((WORDS[4] >> 26) == 0x2B, "sw dest")
    require((WORDS[4] & 0xFFFF) == 0, "sw 0($v1)")
    require(WORDS[5] == 0x03E00008, "jr $ra")
    require(WORDS[6] == 0x24020001, "return 1")
    require(load_u32(data, TABLE_SLOT) == FUNC, "jump table 0x94")
    require(load_u32(data, TABLE_89) == TWIN, "0x89 twin 17FF0")
    require(load_u32(data, FUNC_END) == 0x27BDFFE8, "next leaf bound")
    enc = 0x0C000000 | ((FUNC & 0x0FFFFFFF) >> 2)
    hits = []
    for i in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, i)[0] == enc:
            hits.append(0x80010000 + (i - 0x800))
    require(hits == [], f"no jal sites, got {hits}")
    require(jal_target(enc) == FUNC, "jal encoding identity")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print("PASS: func_80019154 7/7 words + 0x94 table + D_8009D28C + 0x89 twin")
    return 0


if __name__ == "__main__":
    sys.exit(main())
