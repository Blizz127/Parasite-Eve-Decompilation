#!/usr/bin/env python3
"""PE-CH1 oracle: opcode 0x31 handler and BTL1 normal token path."""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x80017BB4
FUNC_END = 0x80017C54
BTL1_TOKEN = 0xA80002C8
SPECIAL_TOKEN = 0xA9400048
DECODE = 0x8006E2D0
SPECIAL_CALL = 0x8006A25C

WORDS = [
    0x27BDFFE0, 0xAFB00018, 0x00808021, 0xAFBF001C,
    0x8E020000, 0x00000000, 0x8C450000, 0x0C01B8B4,
    0x27A40010, 0x8E030000, 0x3C02800A, 0x8C42D1A0,
    0x8C630000, 0x34422000, 0x3C01800A, 0xAC22D1A0,
    0x3C01800A, 0xAC23D280, 0x8E020000, 0x3C03A940,
    0x8C420000, 0x34630048, 0x1443000C, 0x00001021,
    0x3C04800A, 0x24847918, 0x8C830000, 0x240207D0,
    0x14620003, 0x00000000, 0x08005F0F, 0xAC800000,
    0x0C01A897, 0x00000000, 0x00001021, 0x8FBF001C,
    0x8FB00018, 0x27BD0020, 0x03E00008, 0x00000000,
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def simm16(value: int) -> int:
    return value - 0x10000 if value & 0x8000 else value


def branch_target(pc: int, word: int) -> int:
    return pc + 4 + simm16(word & 0xFFFF) * 4


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")
    require(len(WORDS) == 40, "40 words")
    require((FUNC_END - FUNC) // 4 == 40, "window size")
    for i, want in enumerate(WORDS):
        got = load(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x}")

    require(jal_target(WORDS[7]) == DECODE, "stack-local token decoder")
    require((WORDS[13] & 0xFFFF) == 0x2000, "D_8009D1A0 OR 0x2000")
    require((WORDS[15] & 0xFFFF) == 0xD1A0, "store D_8009D1A0")
    require((WORDS[17] & 0xFFFF) == 0xD280, "store D_8009D280")
    require(((WORDS[19] & 0xFFFF) << 16 | (WORDS[21] & 0xFFFF))
            == SPECIAL_TOKEN, "special token compare")
    require(branch_target(FUNC + 22 * 4, WORDS[22]) == 0x80017C40,
            "normal token branches to epilogue")
    require(jal_target(WORDS[32]) == SPECIAL_CALL, "special-only call")
    require(BTL1_TOKEN != SPECIAL_TOKEN, "BTL1 follows normal path")
    require(WORDS[-2:] == [0x03E00008, 0], "jr/nop")

    print("PASS: func_80017BB4 40/40 words + BTL1 0xA80002C8 "
          "normal D280/D1A0 path")
    return 0


if __name__ == "__main__":
    sys.exit(main())
