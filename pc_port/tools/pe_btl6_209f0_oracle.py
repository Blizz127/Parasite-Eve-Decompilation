#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 209F0 after 293F4.

Pins SHA-1-exact EXE. 161 words 0x800209F0..0x80020C70. Copies
rodata 0x800106A4 / 0x800106D4 onto $sp, then D278-local sb
+0x12=4 .. +0x19=11, jal 6C4C4(lh(*(D278+0x68)+6)).
29810 sets D278=*D254 = 0x6F slot body (D_800109B0 copy).
109B0+0x68 is 0. Actor+0x68 (D254/D2F0) is a different field.
Does not invent a pointer or a 6C4C4 argument. Does not import
production C.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
START = 0x800209F0
END = 0x80020C74
SHA = "e7766a6237150448a4affe0090e84f2c0964e7803d0e573b284a8919f9564246"
RODATA_A = 0x800106A4
RODATA_B = 0x800106D4
FN_6C4C4 = 0x8006C4C4
HP_TMPL = 0x80010928
SLOT_TMPL = 0x800109B0


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((END - START) // 4 == 161, "161 words")
    require(window_sha(data, START, END) == SHA, "209F0 sha")
    require(load_u32(data, START) == 0x27BDFFA0, "addiu sp,-96")
    require(load_u32(data, 0x800209F8) == 0x3C068001, "lui 0x8001")
    require(load_u32(data, 0x800209FC) == 0x24C606A4, "addiu 0x800106A4")
    require(RODATA_A == 0x800106A4, "rodata A")
    require(load_u32(data, 0x80020AD4) == 0x8C84D278, "lw D278")
    require(0x800A0000 - 0x2D88 == 0x8009D278, "D278")
    require(load_u32(data, 0x80020ADC) == 0x24C606D4, "addiu 0x800106D4")
    require(RODATA_B == 0x800106D4, "rodata B")
    require(load_u32(data, 0x80020B08) == 0x8C820068, "lw record+0x68")
    require(load_u32(data, 0x80020B0C) == 0, "nop, no beqz")
    require(load_u32(data, 0x80020B10) == 0x90450006, "lbu 6(obj) no nullcheck")
    require(load_u32(data, 0x80020B3C) == 0xA0820012, "sb +0x12")
    require(load_u32(data, 0x80020B38) == 0x24020004, "li 4")
    require(load_u32(data, 0x80020B54) == 0x24020005, "li 5")
    require(load_u32(data, 0x80020B58) == 0xA0620013, "sb +0x13")
    require(load_u32(data, 0x80020B64) == 0x24020006, "li 6")
    require(load_u32(data, 0x80020B68) == 0xA0620014, "sb +0x14")
    require(load_u32(data, 0x80020B74) == 0x24020007, "li 7")
    require(load_u32(data, 0x80020B78) == 0xA0620017, "sb +0x17")
    require(load_u32(data, 0x80020B84) == 0x24020008, "li 8")
    require(load_u32(data, 0x80020B88) == 0xA0620015, "sb +0x15")
    require(load_u32(data, 0x80020B94) == 0x24020009, "li 9")
    require(load_u32(data, 0x80020B98) == 0xA0620018, "sb +0x18")
    require(load_u32(data, 0x80020BA4) == 0x2402000A, "li 10")
    require(load_u32(data, 0x80020BA8) == 0xA0620016, "sb +0x16")
    require(load_u32(data, 0x80020BB4) == 0x2402000B, "li 11")
    require(load_u32(data, 0x80020BB8) == 0xA0620019, "sb +0x19")
    require(jal_target(load_u32(data, 0x80020C5C)) == FN_6C4C4, "jal 6C4C4")
    require(load_u32(data, 0x80020C58) == 0x84440006, "lh *(+0x68)+6")
    require(load_u32(data, SLOT_TMPL + 0x68) == 0, "109B0 slot-body +0x68 is 0")
    require(load_u32(data, HP_TMPL + 0x68) == 0, "10928 HP tmpl +0x68 is 0")
    require(jal_target(load_u32(data, 0x80029918)) == START, "29810 jal 209F0")
    require(load_u32(data, 0x8002F664) == 0x24C60928, "2F658 src 10928")
    require(load_u32(data, 0x8002F668) == 0x24C80070, "2F658 copy 0x70")
    require(load_u32(data, 0x8002F7EC) == 0x24C609B0, "2F7D8 src 109B0")
    require(data[exe_off(SLOT_TMPL + 0x68) : exe_off(SLOT_TMPL + 0x6C)]
            == b"\x00\x00\x00\x00", "109B0+0x68 bytes 0")
    require(data[exe_off(RODATA_B) : exe_off(RODATA_B) + 9]
            == bytes([0x00, 0x0A, 0x08, 0x0A, 0x08, 0x08, 0x04, 0x0A, 0x14]),
            "106D4 scale")
    require(load_u32(data, 0x80020D1C) == 0x8C84D254, "20D28 base is D254")
    require(load_u32(data, 0x80020D28) == 0xAC800068, "20D28 zeros actor+0x68")
    require(load_u32(data, 0x80020D74) == 0x8C84D254, "20D7C base is D254")
    require(load_u32(data, 0x80020D7C) == 0xAC800068, "20D7C zeros actor+0x68")
    require(load_u32(data, 0x800350DC) == 0xAE200068, "35038 zeros actor+0x68")
    require(load_u32(data, 0x80014504) == 0x00808821, "144FC s1=a0")
    require(load_u32(data, 0x80014610) == 0x8E220000, "0x3A lw 0(s1)")
    require(load_u32(data, 0x80014618) == 0x90440000, "0x3A lbu 0(v0)")
    require(load_u32(data, 0x80030640) == 0x3C04800A, "30640 lui")
    require(load_u32(data, 0x80030644) == 0x8C84D278, "30640 lw D278")
    require(load_u32(data, 0x80030654) == 0x8C820068, "30640 lw +0x68")
    require(load_u32(data, 0x8003065C) == 0x8C420010, "30640 lw obj+0x10")
    require(load_u32(data, 0x80030668) == 0x10400018, "30640 beqz skip")

    print(
        "PASS: 209F0 161w; D278=slot body 109B0+0x68=0; actor+0x68 "
        "is motion; 0x3A lbu(*arg0); no invented ptr"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
