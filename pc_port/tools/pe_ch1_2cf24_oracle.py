#!/usr/bin/env python3
"""PE-BTL2 independent oracle: func_8002CF24 mode-7 named cut.

Verifies the SHA-1-exact EXE window that stores 7 into D_8009D28C
via sw $v0, 0x51C($gp). Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x8002CF24
FUNC_END = 0x8002CF2C
GP = 0x8009CD70
MODE = 0x8009D28C
WORDS = [
    0x24020007,  # addiu $v0, $zero, 7
    0xAF82051C,  # sw    $v0, 0x51C($gp)
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


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
    require((FUNC_END - FUNC) // 4 == 2, "2 words")
    require(len(WORDS) == 2, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")
    require(WORDS[0] == 0x24020007, "addiu $v0, 7")
    sw = WORDS[1]
    require((sw >> 26) == 0x2B, "sw")
    require(((sw >> 16) & 0x1F) == 2, "sw $v0")
    require(((sw >> 21) & 0x1F) == 28, "sw to $gp")
    require((sw & 0xFFFF) == 0x51C, "sw gp+0x51C")
    require(GP + 0x51C == MODE, "gp+0x51C is D_8009D28C")
    require(load_u32(data, FUNC_END) == 0x24020046, "exclusive end addiu 70")
    jal = 0x0C000000 | ((FUNC & 0x0FFFFFFF) >> 2)
    hits = [
        0x80010000 + (i - 0x800)
        for i in range(0, len(data) - 4, 4)
        if struct.unpack_from("<I", data, i)[0] == jal
    ]
    require(hits == [], f"unexpected jal to inlined cut {hits}")
    print("PASS: func_8002CF24_mode7_cut 2/2 words + gp+0x51C D_8009D28C=7")
    return 0


if __name__ == "__main__":
    sys.exit(main())
