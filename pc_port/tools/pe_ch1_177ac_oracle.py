#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_800177AC opcode 0x1F poll (7 words).

Verifies the SHA-1-exact EXE window, jump-table slot, and gp current-task
contract. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x800177AC
FUNC_END = 0x800177C8  # exclusive; next leaf starts addiu $sp
TABLE_SLOT = 0x8009111C  # D_800910A0[0x1F]
TASK = 0x8009D300  # 0x590($gp)
GP = 0x8009CD70

WORDS = [
    0x8F820590,  # lw    $v0, 0x590($gp)      D_8009D300
    0x8C830000,  # lw    $v1, 0($a0)          dest ptr
    0x8C420014,  # lw    $v0, 0x14($v0)       task+0x14
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
    require((WORDS[0] >> 26) == 0x23, "lw gp")
    require((WORDS[0] & 0xFFFF) == 0x590, "lw 0x590($gp)")
    require(GP + 0x590 == TASK, "current task D_8009D300")
    require((WORDS[1] >> 26) == 0x23, "lw dest ptr")
    require((WORDS[1] & 0xFFFF) == 0, "lw 0($a0)")
    require((WORDS[2] >> 26) == 0x23, "lw payload")
    require((WORDS[2] & 0xFFFF) == 0x14, "lw task+0x14")
    require(WORDS[3] == 0, "nop")
    require((WORDS[4] >> 26) == 0x2B, "sw dest")
    require((WORDS[4] & 0xFFFF) == 0, "sw 0($v1)")
    require(WORDS[5] == 0x03E00008, "jr $ra")
    require(WORDS[6] == 0x24020001, "return 1")
    require(load_u32(data, TABLE_SLOT) == FUNC, "jump table 0x1F")
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
    print("PASS: func_800177AC 7/7 words + 0x1F table + D_8009D300 + no jal")
    return 0


if __name__ == "__main__":
    sys.exit(main())
