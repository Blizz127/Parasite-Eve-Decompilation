#!/usr/bin/env python3
"""PE-BTL121 — TID 406 is index+387 via 512AC(1); 26824 publishes BE834."""
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

    require(load_u32(blob, 0x80011170 + 1 * 4) == 0x8005130C, "512AC jtbl[1]")
    require(load_u32(blob, 0x8005130C) == 0x8CE20000, "case1 lw *a3")
    require(load_u32(blob, 0x80051314) == 0x24420183, "delay addiu 387")
    require(load_u32(blob, 0x800514CC) == 0xAF8202A0, "sw D010")
    require(load_u32(blob, 0x80011170 + 10 * 4) == 0x800514A0, "jtbl[10]")
    require(load_u32(blob, 0x800514A4) == 0x240203E8, "case10 li 1000")

    require(load_u32(blob, 0x80057BB8) == 0x24040001, "57B70 li a0,1")
    require(load_u32(blob, 0x80057BBC) == 0x27A50010, "57B70 a1=sp+16")
    require(jal_target(load_u32(blob, 0x80057BD0)) == 0x800512AC, "57B70 jal 512AC")
    require(load_u32(blob, 0x80057BD4) == 0xAFB10010, "delay sw s1,16(sp)")
    require(jal_target(load_u32(blob, 0x80046DE4)) == 0x80057B70, "46DBC jal 57B70")
    require(load_u32(blob, 0x80046DE0) == 0x8F840244, "a0=gp+0x244")
    require(load_u32(blob, 0x80046C54) == 0xAF820244, "46C20 sw v0,gp+0x244")

    require(load_u32(blob, 0x8002684C) == 0x8610D2A4, "26824 lh D2A4")
    require(load_u32(blob, 0x8002685C) == 0x2A020183, "slti 387")
    require(load_u32(blob, 0x80026864) == 0x2A020197, "slti 407")
    require(load_u32(blob, 0x800108AC + 13 * 4) == 0x8002692C, "jtbl tid406")
    require(load_u32(blob, 0x800269CC) == 0xA6020004, "sh v0, slot+4")
    require(load_u32(blob, 0x80020F7C) == 0xA420E834, "20F7C sh 0 BE834")
    require(load_u32(blob, 0x80026FA0) == 0xA420E834, "26FA0 sh 0 BE834")

    require(load_u32(blob, 0x80016828) == 0x24020196, "15DAC li 406 is bne")
    require(load_u32(blob, 0x80022C64) == 0x24020196, "22394 li 406 is compare")
    require(load_u32(blob, 0x80022CA8) == 0x24020196, "22394 second compare")
    require(load_u32(blob, 0x80027A58) == 0x24020196, "27A58 li 406 is compare")

    print(
        "PASS: 512AC(1)=*a1+387→D010; 57B70(index); "
        "26824 D2A4→BE834; TEXT sh BE834 are zeros; li 406 are compares"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
