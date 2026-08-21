#!/usr/bin/env python3
"""Independent EXE oracle for func_80065954 (opcode 0x75)."""

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x80065954
HANDLER = 0x80018B98
WORDS = [
    0x3C03800B, 0x8C631624, 0x3C02800B, 0x8C421624,
    0x8C630010, 0x00042100, 0x00431021, 0x10A00004,
    0x00441821, 0x90620000, 0x08019664, 0x34420006,
    0x90620000, 0x00000000, 0x304200F9, 0xA0620000,
    0x03E00008, 0x00001021,
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
    require([word(data, FUNC + i * 4) for i in range(18)] == WORDS,
            "18/18 function words")
    require(word(data, 0x80091274) == HANDLER, "opcode 0x75 table entry")
    require(jal_target(word(data, 0x80018BB0)) == FUNC, "handler jal")
    require(((WORDS[5] >> 6) & 0x1F) == 4, "slot stride 16")
    require((WORDS[11] & 0xFFFF) == 6, "enabled OR mask")
    require((WORDS[14] & 0xFFFF) == 0xF9, "disabled AND mask")
    require(WORDS[15] == 0xA0620000, "indexed byte store")
    print("PASS: func_80065954 18/18 words + opcode 0x75 handler")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
