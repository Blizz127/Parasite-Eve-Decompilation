#!/usr/bin/env python3
"""PE-BTL94 — M0367I persist 0x09 gate and 0xC1 (EXE only).

Watch persist[0x4A]==0x26 opens the first listed 0x08 only.
0x5E / 0xCD / 0x29E have no dest-enter writers. Do not force them.
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
WIN_C1 = "eac1de420911dc96a667651d70d09d2fb2aa93afdc3f254f3a125b2d806dd666"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x800910A0 + 0x09 * 4) == 0x80012850, "table[0x09]")
    require(load_u32(blob, 0x800910A0 + 0x05 * 4) == 0x8001731C, "table[0x05]")
    require(load_u32(blob, 0x800910A0 + 0x08 * 4) == 0x8001735C, "table[0x08]")
    require(load_u32(blob, 0x800910A0 + 0x0A * 4) == 0x800173F4, "table[0x0A]")
    require(load_u32(blob, 0x800910A0 + 0xC1 * 4) == 0x80019AC0, "table[0xC1]")
    require(load_u32(blob, 0x80019AD4) == 0x34420400, "0xC1 ori +0x98 0x400")
    require(load_u32(blob, 0x80019AE0) == 0x24020001, "0xC1 v0=1")
    require(window_sha(blob, 0x80019AC0, 0x80019AE4) == WIN_C1, "0xC1 sha")
    require(load_u32(blob, 0x80012A20) == 0x00431026, "0x09 eq xor")
    require(load_u32(blob, 0x80012A2C) == 0x2C420001, "0x09 eq sltiu 1")
    print("PASS: persist 0x09 / 0x05 / 0x08 / 0x0A and 0xC1 19AC0 |=0x400")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
