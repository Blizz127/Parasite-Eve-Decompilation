#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_8002FA10 opcode 0x70 formation (37 words).

Verifies the SHA-1-exact EXE window, 0x70 wrapper, and body-relative
stores. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x8002FA10
FUNC_END = 0x8002FAA4  # exclusive; twin func_8002FAA4
WRAPPER = 0x8001897C
JAL_SITE = 0x80018A30
TABLE_SLOT = 0x80091260  # D_800910A0[0x70]
ACTOR = 0x8009D2F0

WORDS = [
    0x30A500FF, 0x00051900, 0x2463001C, 0x93AA0010,
    0x97AB0014, 0x93AC0018, 0x93AD001C, 0x93AE0020,
    0x93AF0024, 0x93A90028, 0x8C820000, 0x93A8002C,
    0x00431021, 0xA0400000, 0xA0460001, 0xA0470002,
    0xA04A0003, 0xA44B000C, 0xA049000E, 0xA048000F,
    0x8C820000, 0x00052880, 0x00451021, 0xA04C007C,
    0x8C820000, 0x00000000, 0x00451021, 0xA04D007D,
    0x8C820000, 0x00000000, 0x00451021, 0xA04E007E,
    0x8C820000, 0x00000000, 0x00451021, 0x03E00008,
    0xA04F007F,
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
    require((FUNC_END - FUNC) // 4 == 37, "37 words")
    require(len(WORDS) == 37, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require((WORDS[0] & 0xFFFF) == 0xFF, "andi index 0xFF")
    require((WORDS[1] >> 6) & 0x1F == 4, "sll 4 → *16")
    require((WORDS[2] & 0xFFFF) == 0x1C, "addiu +0x1C")
    require((WORDS[3] & 0xFFFF) == 0x10, "lbu st+0x10")
    require((WORDS[4] >> 26) == 0x25, "lhu st+0x14")
    require((WORDS[10] & 0xFFFF) == 0, "lw *a0 body")
    require((WORDS[13] >> 26) == 0x28, "sb +0")
    require((WORDS[17] >> 26) == 0x29, "sh +0xC")
    require((WORDS[17] & 0xFFFF) == 0xC, "sh 0xC")
    require((WORDS[21] >> 6) & 0x1F == 2, "sll 2 → *4")
    require((WORDS[23] & 0xFFFF) == 0x7C, "sb +0x7C")
    require((WORDS[27] & 0xFFFF) == 0x7D, "sb +0x7D")
    require((WORDS[31] & 0xFFFF) == 0x7E, "sb +0x7E")
    require(WORDS[35] == 0x03E00008, "jr $ra")
    require((WORDS[36] & 0xFFFF) == 0x7F, "sb +0x7F delay")
    require(load_u32(data, FUNC_END) == 0x30A500FF, "next leaf 2FAA4")
    require(load_u32(data, TABLE_SLOT) == WRAPPER, "jump table 0x70")
    require(jal_target(load_u32(data, JAL_SITE)) == FUNC, "wrapper jal 2FA10")
    imm = load_u32(data, 0x80018A28) & 0xFFFF
    signed = imm - 0x10000 if imm & 0x8000 else imm
    require((0x800A0000 + signed) & 0xFFFFFFFF == ACTOR, "wrapper D_8009D2F0")
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
    print("PASS: func_8002FA10 37/37 words + 0x70 wrapper + *actor+i*16+0x1C / +i*4+0x7C")
    return 0


if __name__ == "__main__":
    sys.exit(main())
