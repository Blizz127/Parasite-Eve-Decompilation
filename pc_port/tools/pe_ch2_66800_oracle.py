#!/usr/bin/env python3
"""Independent EXE oracle for func_80066800 (opcode 0x82 view apply)."""

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x80066800
WORDS = [
    0x3C02800B, 0x8C421624, 0x27BDFFE0, 0xAFB00010,
    0x3C10800B, 0x8E101624, 0xAFB10014, 0x00808821,
    0xAFBF0018, 0x8C42001C, 0x3C03800C, 0x8C63CFA8,
    0x02028021, 0x00111040, 0x00511021, 0x00021080,
    0x00511021, 0x00021080, 0x02028021, 0x96020000,
    0x00000000, 0xAC620000, 0x96040000, 0x0C01E409,
    0x00000000, 0x3C03800C, 0x8C63CFA4, 0x96020002,
    0x00000000, 0xA4620000, 0x3C03800C, 0x8C63CFA4,
    0x96020004, 0x00000000, 0xA4620002, 0x3C03800C,
    0x8C63CFA4, 0x96020006, 0x00000000, 0xA4620004,
    0x3C03800C, 0x8C63CFA4, 0x96020008, 0x00000000,
    0xA4620006, 0x3C03800C, 0x8C63CFA4, 0x9602000A,
    0x00000000, 0xA4620008, 0x3C03800C, 0x8C63CFA4,
    0x9602000C, 0x00000000, 0xA462000A, 0x3C03800C,
    0x8C63CFA4, 0x9602000E, 0x00000000, 0xA462000C,
    0x3C03800C, 0x8C63CFA4, 0x96020010, 0x00000000,
    0xA462000E, 0x3C03800C, 0x8C63CFA4, 0x96020012,
    0x00000000, 0xA4620010, 0x3C03800C, 0x8C63CFA4,
    0x8E020014, 0x00000000, 0xAC620014, 0x3C03800C,
    0x8C63CFA4, 0x8E020018, 0x00000000, 0xAC620018,
    0x3C03800C, 0x8C63CFA4, 0x8E02001C, 0x00000000,
    0xAC62001C, 0x3C02800C, 0x8C42CF88, 0x3C01800C,
    0xA031CFFD, 0x34420080, 0x3C01800C, 0xAC22CF88,
    0x00001021, 0x8FBF0018, 0x8FB10014, 0x8FB00010,
    0x27BD0020, 0x03E00008, 0x00000000,
]


def require(ok: bool, msg: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {msg}")


def off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def word(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, off(addr))[0]


def jal_target(value: int) -> int:
    return ((value & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    data = (root / "build" / "disc1.candidate.exe").read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")
    require(len(WORDS) == 99, "oracle word count")
    require([word(data, FUNC + i * 4) for i in range(99)] == WORDS,
            "99/99 function words")
    require(word(data, 0x800912A8) == 0x80018E58,
            "opcode 0x82 table entry")
    require(jal_target(word(data, 0x80018E6C)) == FUNC, "handler jal")
    require(jal_target(word(data, 0x80067834)) == FUNC, "677FC jal")
    require(jal_target(WORDS[23]) == 0x80079024, "SetGeomScreen call")
    require(WORDS[21] == 0xAC620000, "H guest store")
    require(WORDS[88] == 0xA031CFFD, "view-index byte store")
    require(WORDS[91] == 0xAC22CF88, "camera flag word store")
    require(sum(1 for w in WORDS if (w >> 26) == 0x29) == 9,
            "nine rotation halfword stores")
    print("PASS: func_80066800 99/99 words + opcode 0x82 view stores")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
