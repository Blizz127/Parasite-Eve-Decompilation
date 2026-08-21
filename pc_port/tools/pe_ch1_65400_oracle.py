#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_80065400 mailbox drain (117 words).

Verifies the SHA-1-exact EXE window, both jal-12700 sites, and the
sole field-tick caller. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x80065400
FUNC_END = 0x800655D4  # exclusive; next leaf starts here
SPAWN = 0x80012700
CALLER = 0x8003F4E8
TABLE = 0x800A3180
ACTORS = 0x8009D20C
GP_COUNT_OFF = 0x44

WORDS = [
    0x93820044, 0x27BDFFD8, 0xAFB3001C, 0x00009821,
    0xAFBF0020, 0xAFB20018, 0xAFB10014, 0x10400064,
    0xAFB00010, 0x326200FF, 0x00021840, 0x00621821,
    0x00031880, 0x3C02800A, 0x24423180, 0x00628821,
    0x92220004, 0x00000000, 0x10400029, 0x00000000,
    0x3C10800A, 0x8E10D20C, 0x00000000, 0x1200004E,
    0x00409021, 0x96020024, 0x00000000, 0x1452001A,
    0x00000000, 0x8E04019C, 0x00000000, 0x10800014,
    0x00000000, 0x0C0049C0, 0x00002821, 0x00401821,
    0x94620008, 0x00000000, 0x34420004, 0xA4620008,
    0x8E220008, 0x00000000, 0xAC62000C, 0x92220003,
    0x00000000, 0xAC620014, 0x8E0200A8, 0x00000000,
    0x10400002, 0xAC620024, 0xAC430028, 0xAE0300A8,
    0x08019537, 0x00008021, 0x8E100004, 0x00000000,
    0x1600FFE0, 0x00000000, 0x08019567, 0x26730001,
    0x3C10800A, 0x8E10D20C, 0x00000000, 0x12000026,
    0x00000000, 0x9203000C, 0x96220000, 0x00000000,
    0x1462001D, 0x00000000, 0x8E04019C, 0x00000000,
    0x10800019, 0x00000000, 0x9203000D, 0x92220002,
    0x00000000, 0x14620014, 0x00000000, 0x0C0049C0,
    0x00002821, 0x00401821, 0x94620008, 0x00000000,
    0x34420004, 0xA4620008, 0x8E220008, 0x00000000,
    0xAC62000C, 0x92220003, 0x00000000, 0xAC620014,
    0x8E0200A8, 0x00000000, 0x10400002, 0xAC620024,
    0xAC430028, 0xAE0300A8, 0x8E100004, 0x00000000,
    0x1600FFDC, 0x00000000, 0x26730001, 0x93830044,
    0x326200FF, 0x0043102B, 0x1440FF9F, 0x326200FF,
    0xA3800044, 0x8FBF0020, 0x8FB3001C, 0x8FB20018,
    0x8FB10014, 0x8FB00010, 0x27BD0028, 0x03E00008,
    0x00000000,
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


def lui_lw(lui: int, lw: int) -> int:
    imm = lw & 0xFFFF
    signed = imm - 0x10000 if imm & 0x8000 else imm
    return ((lui & 0xFFFF) << 16) + signed


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def verify_words(data: bytes) -> None:
    require((FUNC_END - FUNC) // 4 == 117, "117 words")
    require(len(WORDS) == 117, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require((WORDS[0] >> 26) == 0x24, "lbu count")
    require((WORDS[0] & 0xFFFF) == GP_COUNT_OFF, "count gp+0x44")
    require((WORDS[13] & 0xFFFF) == 0x800A, "lui 0x800A")
    require((WORDS[14] & 0xFFFF) == (TABLE & 0xFFFF), "addiu 0x3180")
    require((WORDS[16] & 0xFFFF) == 4, "lbu rec+4 extra")
    require(lui_lw(WORDS[20], WORDS[21]) == ACTORS, "serial-arm D_8009D20C")
    require((WORDS[25] & 0xFFFF) == 0x24, "lhu actor+0x24")
    require((WORDS[29] & 0xFFFF) == 0x19C, "lw actor+0x19C serial")
    require(WORDS[33] == 0x0C0049C0, "jal 12700 serial")
    require(jal_target(WORDS[33]) == SPAWN, "jal tgt serial")
    require(WORDS[34] == 0x00002821, "a1=0 serial")
    require(WORDS[38] == 0x34420004, "ori task+8,4 serial")
    require((WORDS[43] & 0xFFFF) == 3, "lbu payload serial")
    require(lui_lw(WORDS[60], WORDS[61]) == ACTORS, "typeid-arm D_8009D20C")
    require((WORDS[65] & 0xFFFF) == 0x0C, "lbu actor+0x0C")
    require((WORDS[66] & 0xFFFF) == 0, "lhu rec+0")
    require((WORDS[70] & 0xFFFF) == 0x19C, "lw actor+0x19C typeid")
    require((WORDS[74] & 0xFFFF) == 0x0D, "lbu actor+0x0D")
    require((WORDS[75] & 0xFFFF) == 2, "lbu rec+2")
    require(WORDS[79] == 0x0C0049C0, "jal 12700 typeid")
    require(jal_target(WORDS[79]) == SPAWN, "jal tgt typeid")
    require(WORDS[80] == 0x00002821, "a1=0 typeid")
    require((WORDS[108] & 0xFFFF) == GP_COUNT_OFF, "sb count 0")
    require((WORDS[108] >> 26) == 0x28, "sb op")
    require(WORDS[115] == 0x03E00008, "jr $ra")
    require(WORDS[116] == 0, "nop delay")
    require(load_u32(data, CALLER) == 0x0C019500, "sole jal encoding")
    require(jal_target(load_u32(data, CALLER)) == FUNC, "jal 65400")
    require(load_u32(data, CALLER + 4) == 0, "nop delay at 3F4E8")
    require((WORDS[54] & 0xFFFF) == 4, "next actor +4 serial")
    require((WORDS[98] & 0xFFFF) == 4, "next actor +4 typeid")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print("PASS: func_80065400 117/117 words + both jal 12700 + sole 3F4E8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
