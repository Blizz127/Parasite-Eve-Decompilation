#!/usr/bin/env python3
"""PE-BTL114 — CE54 writer is 24A3C case 9; +0x1A is 1A4AC's +0x18 high half."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x80024A3C) == 0x938304EC, "24A3C lbu D25C")
    require(load_u32(blob, 0x80010824 + 9 * 4) == 0x80024F48, "jtbl[9]")
    require(load_u32(blob, 0x80024F54) == 0x9083000F, "case9 lbu +0x0F")
    require(load_u32(blob, 0x80024F58) == 0x9482001A, "case9 lhu +0x1A")
    require(load_u32(blob, 0x80024F90) == 0x24020001, "li 1")
    require(load_u32(blob, 0x80024F94) == 0xA38200E4, "sb CE54")
    require(load_u32(blob, 0x80024F8C) == 0xA38200E5, "sb CE55")
    require(jal_target(load_u32(blob, 0x80021E6C)) == 0x80022394, "21DE0 22394")
    require(jal_target(load_u32(blob, 0x800229D8)) == 0x80024A3C, "22394 24A3C")
    require(load_u32(blob, 0x8001A520) == 0xAE070018, "1A4AC sw +0x18")
    require(load_u32(blob, 0x8002B1E8) == 0x9083000F, "phase2 lbu +0x0F")
    require(load_u32(blob, 0x8002B1EC) == 0x9482001A, "phase2 lhu +0x1A")
    require(load_u32(blob, 0x80024C7C) == 0x94820016, "case4 lhu +0x16")
    require(load_u32(blob, 0x80024D04) == 0x24050006, "case5 a1=6")
    require(load_u32(blob, 0x80024D1C) == 0x90C20252, "case6 lbu +0x252")
    require(load_u32(blob, 0x80024E88) == 0x9482001A, "case7 lhu +0x1A")
    require(load_u32(blob, 0x80024F10) == 0x9482001A, "case8 lhu +0x1A")
    require(load_u32(blob, 0x80024988) == 0x3C030008, "24250 lui 0x80000")
    require(load_u32(blob, 0x80024994) == 0xAC82004C, "24250 sw rec+0x4C")
    require(jal_target(load_u32(blob, 0x80022C9C)) == 0x80024250, "22394 24250")
    require(load_u32(blob, 0x800107D4 + 19 * 4) == 0x80024974, "jtbl[19]")
    require(load_u32(blob, 0x80051314) == 0x24420183, "5112C addiu 387")
    print("PASS: 24F94 is the CE54 store; 1A4AC copies +0x14 to +0x18")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
