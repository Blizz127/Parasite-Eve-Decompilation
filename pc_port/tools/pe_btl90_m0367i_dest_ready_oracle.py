#!/usr/bin/env python3
"""PE-BTL90 — M0367I dest-ready pins (EXE only).

Canonical ready is 6B35C + 6B4F8 + 6BECC==0 + 6C5BC==0 + 125E0.
Modes 7/9/10 are not dest-ready. Type-1 clip is not required.
Does not import production C.
"""
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
WIN_6B35C = "1106cb2a94fa877af5a067e36a3c24d08f186694b112c2139d2869f7cf341f24"
WIN_6BE4C = "fb4091da59b98ce270dd0a50c7edf8574fe5bf2117fa6a1e0cb3f5dc9e71306b"
CE210_15 = "e1cb9dfd14eafe873e4768722b39f7fd51e763ea5147279d6a09ad401c1ba40e"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def load_u16(blob: bytes, va: int) -> int:
    return struct.unpack_from("<H", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(jal_target(load_u32(blob, 0x8003F07C)) == 0x8006B35C, "jal 6B35C")
    require(jal_target(load_u32(blob, 0x8003F088)) == 0x8006B4F8, "jal 6B4F8")
    require(jal_target(load_u32(blob, 0x8003F204)) == 0x8006BE4C, "jal 6BE4C")
    require(jal_target(load_u32(blob, 0x8003F20C)) == 0x8006BECC, "jal 6BECC")
    require(jal_target(load_u32(blob, 0x8003F22C)) == 0x8006C5BC, "jal 6C5BC")
    require(jal_target(load_u32(blob, 0x8003F27C)) == 0x800125E0, "jal 125E0")
    require(load_u32(blob, 0x8006B7B8) == 0xA2C2000A, "sb CE2")
    require(load_u32(blob, 0x8006B804) == 0xAC620198, "hdr+0x0C B0E70")
    require(load_u32(blob, 0x8006C118) == 0xAE820198, "6C118 B0E70[0]")
    require(load_u32(blob, 0x800351E0) == 0xAE2001B0, "35038 +0x1B0=0")
    require(load_u32(blob, 0x8002CF24) == 0x24020007, "mode7 li 7")
    require(load_u32(blob, 0x8002B278) == 0x24030009, "mode9 li 9")
    require(load_u32(blob, 0x8002BC74) == 0x2402000A, "mode10 li 10")
    require(load_u16(blob, 0x800930D8 + 18 * 2) == 288, "CE2=10 start")
    require(load_u16(blob, 0x800930D8 + 19 * 2) == 316, "CE2=10 end")
    require(window_sha(blob, 0x8006B35C, 0x8006B4F8) == WIN_6B35C, "6B35C sha")
    require(window_sha(blob, 0x8006BE4C, 0x8006BECC) == WIN_6BE4C, "6BE4C sha")
    require(CE210_15.startswith("e1cb9dfd"), "CE2=10 0x15 pin")
    print("PASS: M0367I dest-ready is 6B35C/6B4F8/6BECC/6C5BC/125E0; not mode 7")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
