#!/usr/bin/env python3
"""Independent EXE oracle for func_800659C8 (opcode 0x7B)."""

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x800659C8
HANDLER = 0x80018C58
WORDS = [
    0x00042100, 0x3C03800B, 0x8C631624, 0x3C02800B,
    0x8C421624, 0x8C630010, 0x00052A02, 0x00431021,
    0x00441021, 0xA4450008, 0x03E00008, 0x00001021,
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
    require([word(data, FUNC + i * 4) for i in range(12)] == WORDS,
            "12/12 function words")
    require(word(data, 0x8009128C) == HANDLER, "opcode 0x7B table entry")
    require(jal_target(word(data, 0x80018C70)) == FUNC, "handler jal")
    require(((WORDS[0] >> 6) & 0x1F) == 4, "slot stride 16")
    require(((WORDS[6] >> 6) & 0x1F) == 8, "parameter shift 8")
    require(WORDS[9] == 0xA4450008, "slot+8 halfword store")
    print("PASS: func_800659C8 12/12 words + opcode 0x7B handler")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
