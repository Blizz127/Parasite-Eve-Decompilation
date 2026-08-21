#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_8002FAA4 opcode 0xB7 formation (13 words).

Verifies the SHA-1-exact EXE window, 0xB7 wrapper, and body-relative
stores. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x8002FAA4
FUNC_END = 0x8002FAD8  # exclusive; next leaf also andi-index
WRAPPER = 0x80018A48
JAL_SITE = 0x80018A84
TABLE_SLOT = 0x8009137C  # D_800910A0[0xB7]
ACTOR = 0x8009D2F0

WORDS = [
    0x30A500FF, 0x00052900, 0x24A5001C, 0x93A80010,
    0x8C820000, 0x97A30014, 0x00451021, 0xA0400000,
    0xA0460001, 0xA0470002, 0xA0480003, 0x03E00008,
    0xA443000C,
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
    require((FUNC_END - FUNC) // 4 == 13, "13 words")
    require(len(WORDS) == 13, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require((WORDS[0] & 0xFFFF) == 0xFF, "andi index 0xFF")
    require((WORDS[1] >> 6) & 0x1F == 4, "sll 4 → *16")
    require((WORDS[2] & 0xFFFF) == 0x1C, "addiu +0x1C")
    require((WORDS[3] & 0xFFFF) == 0x10, "lbu st+0x10")
    require((WORDS[4] & 0xFFFF) == 0, "lw *a0 body")
    require((WORDS[5] >> 26) == 0x25, "lhu st+0x14")
    require((WORDS[7] >> 26) == 0x28, "sb +0")
    require((WORDS[8] & 0xFFFF) == 0x1, "sb +1")
    require((WORDS[9] & 0xFFFF) == 0x2, "sb +2")
    require((WORDS[10] & 0xFFFF) == 0x3, "sb +3")
    require(WORDS[11] == 0x03E00008, "jr $ra")
    require((WORDS[12] >> 26) == 0x29, "sh delay")
    require((WORDS[12] & 0xFFFF) == 0xC, "sh +0xC")
    require(load_u32(data, FUNC_END) == 0x30A500FF, "next leaf andi-index")
    require(load_u32(data, TABLE_SLOT) == WRAPPER, "jump table 0xB7")
    require(jal_target(load_u32(data, JAL_SITE)) == FUNC, "wrapper jal 2FAA4")
    lui = load_u32(data, 0x80018A78)
    lw = load_u32(data, 0x80018A7C)
    require(lui >> 16 == 0x3C04, "wrapper lui a0")
    hi = lui & 0xFFFF
    lo = lw & 0xFFFF
    signed = lo - 0x10000 if lo & 0x8000 else lo
    require(((hi << 16) + signed) & 0xFFFFFFFF == ACTOR, "wrapper D_8009D2F0")
    enc = 0x0C000000 | ((FUNC & 0x0FFFFFFF) >> 2)
    hits = []
    for i in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, i)[0] == enc:
            hits.append(0x80010000 + (i - 0x800))
    require(hits == [JAL_SITE], f"sole jal site, got {hits}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print("PASS: func_8002FAA4 13/13 words + 0xB7 wrapper + *actor+i*16+0x1C")
    return 0


if __name__ == "__main__":
    sys.exit(main())
