#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_8002FF78 opcode 0x5A Aya tagged setter.

Verifies the SHA-1-exact EXE window, 0x5A wrapper branch, and store
map. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x8002FF78
FUNC_END = 0x8003010C  # exclusive; jr+nop
WRAPPER = 0x80018164
JAL_SITES = [0x80018194, 0x80018200]
TABLE_SLOT = 0x80091208  # D_800910A0[0x5A]
AYA = 0x8009D254
GLOBAL_SH = 0x800942EC

WORDS = [
    0x308400FF, 0x3C02800A, 0x8C42D254, 0x2403000B,
    0x8C460000, 0x10830049, 0x2882000C, 0x10400018,
    0x24020003, 0x1082003B, 0x28820004, 0x10400009,
    0x24020001, 0x10820033, 0x28820002, 0x10400033,
    0x00000000, 0x1080002D, 0x00000000, 0x0800C041,
    0x00000000, 0x24020005, 0x10820032, 0x28820005,
    0x1440002E, 0x24020006, 0x10820030, 0x2402000A,
    0x10820030, 0x00000000, 0x0800C041, 0x00000000,
    0x2402001F, 0x10820037, 0x28820020, 0x10400010,
    0x2402000E, 0x1082002D, 0x2882000F, 0x10400005,
    0x2402000C, 0x10820027, 0x00000000, 0x0800C041,
    0x00000000, 0x24020012, 0x10820026, 0x2402001E,
    0x10820026, 0x00000000, 0x0800C041, 0x00000000,
    0x24020021, 0x10820027, 0x28820021, 0x14400023,
    0x24020022, 0x10820025, 0x240200FF, 0x10820025,
    0x00000000, 0x0800C041, 0x00000000, 0x0800C041,
    0xACC50000, 0x0800C041, 0xA4C50004, 0x0800C041,
    0xA4C50006, 0x0800C041, 0xACC50008, 0x0800C041,
    0xA4C5000C, 0x0800C041, 0xA4C5000E, 0x0800C041,
    0xA4C50010, 0x0800C041, 0xA4C5001C, 0x0800C041,
    0xA4C5001E, 0x0800C041, 0xA4C50020, 0x0800C041,
    0xA4C50022, 0x0800C041, 0xA4C50026, 0x0800C041,
    0xA4C50050, 0x0800C041, 0xA0C50056, 0x0800C041,
    0xA0C50057, 0x0800C041, 0xA4C50058, 0x0800C041,
    0xA0C5005E, 0x3C018009, 0xA42542EC, 0x03E00008,
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


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def verify_words(data: bytes) -> None:
    require((FUNC_END - FUNC) // 4 == 101, "101 words")
    require(len(WORDS) == 101, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require((WORDS[0] & 0xFFFF) == 0xFF, "andi tag 0xFF")
    lui = WORDS[1]
    lw = WORDS[2]
    hi = lui & 0xFFFF
    lo = lw & 0xFFFF
    signed = lo - 0x10000 if lo & 0x8000 else lo
    require(((hi << 16) + signed) & 0xFFFFFFFF == AYA, "lw D_8009D254")
    require(WORDS[4] == 0x8C460000, "lw dest *aya")
    require(WORDS[-2] == 0x03E00008, "jr $ra")
    require(WORDS[-1] == 0, "nop delay")
    require(WORDS[64] == 0xACC50000, "tag0 sw +0")
    require(WORDS[66] == 0xA4C50004, "tag1 sh +4")
    require(WORDS[90] == 0xA0C50056, "tag31 sb +0x56")
    require(WORDS[98] == 0xA42542EC, "tag255 sh D_800942EC")
    sh_imm = WORDS[98] & 0xFFFF
    require((0x80090000 + sh_imm) == GLOBAL_SH, "D_800942EC")
    require(load_u32(data, TABLE_SLOT) == WRAPPER, "jump table 0x5A")
    require(jal_target(load_u32(data, JAL_SITES[0])) == FUNC, "0x5A jal")
    require(jal_target(load_u32(data, 0x800181B4)) == 0x80030220,
            "0x5A else jal 30220")
    enc = 0x0C000000 | ((FUNC & 0x0FFFFFFF) >> 2)
    hits = []
    for i in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, i)[0] == enc:
            hits.append(0x80010000 + (i - 0x800))
    require(hits == JAL_SITES, f"jal sites, got {hits}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print("PASS: func_8002FF78 101/101 words + 0x5A wrapper + Aya tag map")
    return 0


if __name__ == "__main__":
    sys.exit(main())
