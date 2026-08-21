#!/usr/bin/env python3
"""PE-BTL82 — HP-field writer census + 0xAE + type-3 door hop.

Does not import production C. Does not emit hp_mutated.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800
AE_SHA = "a0eb25f922ee3ac811b0dd66b37bbef3dcbf67d46084f8ae04e0856831407e2b"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_sites(blob: bytes, target: int) -> list[int]:
    want = 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)
    text = blob[HDR : HDR + 0x1EE000]
    return [TADDR + i for i in range(0, len(text), 4)
            if struct.unpack_from("<I", text, i)[0] == want]


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def load_chunk2() -> bytes:
    pointer = (ROOT / "local" / "pe_disc1.path").read_text().strip().splitlines()[0].strip()
    sec0, sec1, sec2 = PACKED & 0xFF, (PACKED >> 8) & 0xFFF, PACKED >> 20
    out = bytearray()
    with open(pointer, "rb") as fh:
        for i in range(sec2):
            fh.seek((PE_IMG_LBA + REL + sec0 + sec1 + i) * 2352 + 24)
            out += fh.read(2048)
    data = bytes(out)
    require(hashlib.sha256(data).hexdigest() == CHUNK2_SHA, "chunk2")
    return data


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")

    require(load_u32(blob, 0x80029418) == 0xA4A6000C, "293F4 clamp sh +0x0C")
    require(load_u32(blob, 0x80029444) == 0xA4A2000E, "293F4 copy sh +0x0E")
    require(load_u32(blob, 0x8001E93C) == 0x00431023, "1E940 subu HP")
    require(load_u32(blob, 0x8001E940) == 0xA482000C, "1E940 sh +0x0C")
    require(load_u32(blob, 0x8001F700) == 0x00501023, "1F704 subu HP")
    require(load_u32(blob, 0x8001F704) == 0xA482000C, "1F704 sh +0x0C")
    require(load_u32(blob, 0x8002020C) == 0x00431023, "20210 subu +0x38")
    require(load_u32(blob, 0x80020210) == 0xA4A2000C, "20210 sh +0x0C")
    require(load_u32(blob, 0x8001F4B0) == 0xA440000C, "1F4B0 sh zero +0x0C")
    require(load_u32(blob, 0x8002AE7C) == 0xA4A2000C, "2AE60 restore sh +0x0C")
    require(jal_sites(blob, 0x8001D340) == [0x8002A4FC], "1D340 sole jal")
    require(load_u32(blob, 0x80029464) == 0xA38204D4, "293F4(1) sb 4D4")
    require(load_u32(blob, 0x80029488) == 0xA38004D4, "293F4(0) clear 4D4")
    require(load_u32(blob, 0x8001F41C) == 0xAC22D28C, "1D340 mode=3")
    require(jal_sites(blob, 0x80019D20) == [], "19D20 no jal")
    require(jal_sites(blob, 0x8002CEE0) == [], "2CEE0 no jal")

    require(load_u32(blob, 0x800910A0 + 0xAE * 4) == 0x80019728, "table 0xAE")
    require((0x80019748 - 0x80019728) // 4 == 8, "0xAE 8w")
    require(window_sha(blob, 0x80019728, 0x80019748) == AE_SHA, "0xAE sha")
    require(load_u32(blob, 0x80019734) == 0x34420004, "ori 4")
    require(load_u32(blob, 0x8001973C) == 0xAC22D2E8, "sw D2E8")

    chunk2 = load_chunk2()
    require((struct.unpack_from("<I", chunk2, 0x227FC + 0xEC)[0] & 0x1FFF) == 0x31,
            "type-3 first 0x31")
    require(struct.unpack_from("<I", chunk2, 0x227FC + 0xF4)[0] == 0xA8000248,
            "type-3 first dest M0004I")
    require((struct.unpack_from("<I", chunk2, 0x227FC + 0x1D8)[0] & 0x1FFF) == 0x31,
            "type-3 second 0x31")
    require(struct.unpack_from("<I", chunk2, 0x227FC + 0x1E0)[0] == 0xA8000248,
            "type-3 second dest M0004I")
    require((struct.unpack_from("<I", chunk2, 0x227FC + 0xC8)[0] & 0x1FFF) == 0x85,
            "0x85 at +0xC8")
    require((struct.unpack_from("<I", chunk2, 0x227FC + 0xD4)[0] & 0x1FFF) == 0x9C,
            "0x9C after 0x85")

    print(
        "PASS: HP writers classified; 1D340 DAMAGE 4D4-gated; "
        "0xAE 8/8; type-3 0x85→0x9C→0x31 M0004I; no hp_mutated"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
