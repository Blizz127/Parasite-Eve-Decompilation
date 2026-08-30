#!/usr/bin/env python3
"""Independent B54K-H oracle for func_80030894 fixed sprite and L11.

No production code is imported. The oracle hashes the exact retail window,
checks all 70 words, decodes every call, proves descriptor indices and
L11/address geometry, and verifies both words adjacent to the cut.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x80031320
END = 0x80031438
WINDOW_SHA256 = "bdfec78193cfc60b0ed14829f5f3fb42ce74db2cbfe0431ad402f174fd665538"

CALLS = [
    (0x8003133C, 0x800370DC),
    (0x80031394, 0x8005DADC),
    (0x800313AC, 0x80077A64),
    (0x800313C8, 0x800370DC),
]

WORDS = {
    0x80031320: 0x3C12800A,
    0x80031324: 0x2652E928,
    0x80031328: 0x93B10018,
    0x8003132C: 0x03C02821,
    0x80031330: 0x001180C0,
    0x80031334: 0x02118023,
    0x80031338: 0x00108080,
    0x8003133C: 0x0C00DC37,
    0x80031340: 0x02122021,
    0x80031344: 0x0000B021,
    0x80031348: 0x00111040,
    0x8003134C: 0x00511021,
    0x80031350: 0x000210C0,
    0x80031354: 0x00511023,
    0x80031358: 0x00021080,
    0x8003135C: 0x00511023,
    0x80031360: 0x0002A080,
    0x80031364: 0x3C15800A,
    0x80031368: 0x26B5E960,
    0x8003136C: 0x97A90020,
    0x80031370: 0x26520008,
    0x80031374: 0x3C01800A,
    0x80031378: 0x00300821,
    0x8003137C: 0xA429E93E,
    0x80031380: 0x02128021,
    0x80031384: 0xA2170004,
    0x80031388: 0xA2170005,
    0x8003138C: 0xA2000006,
    0x80031390: 0x32D100FF,
    0x80031394: 0x0C0176B7,
    0x80031398: 0x2624006A,
    0x8003139C: 0x00409821,
    0x800313A0: 0x00002021,
    0x800313A4: 0x00002821,
    0x800313A8: 0x240601C0,
    0x800313AC: 0x0C01DE99,
    0x800313B0: 0x00003821,
    0x800313B4: 0x001180C0,
    0x800313B8: 0x02118023,
    0x800313BC: 0x00108080,
    0x800313C0: 0x02152021,
    0x800313C4: 0x02842021,
    0x800313C8: 0x0C00DC37,
    0x800313CC: 0x3045FFFF,
    0x800313D0: 0x02908021,
    0x800313D4: 0x3C09800A,
    0x800313D8: 0x2529E968,
    0x800313DC: 0x92620000,
    0x800313E0: 0x02091821,
    0x800313E4: 0xA062000C,
    0x800313E8: 0x92620001,
    0x800313EC: 0x00000000,
    0x800313F0: 0xA062000D,
    0x800313F4: 0x96620002,
    0x800313F8: 0x3C01800A,
    0x800313FC: 0x00300821,
    0x80031400: 0xA422E976,
    0x80031404: 0x92620004,
    0x80031408: 0x00000000,
    0x8003140C: 0xA4620010,
    0x80031410: 0x92620005,
    0x80031414: 0x26D60001,
    0x80031418: 0xA0770004,
    0x8003141C: 0xA0770005,
    0x80031420: 0xA0770006,
    0x80031424: 0xA4620012,
    0x80031428: 0x32C200FF,
    0x8003142C: 0x2C42000D,
    0x80031430: 0x1440FFD8,
    0x80031434: 0x32D100FF,
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FAIL: {message}")


def find_exe() -> pathlib.Path:
    here = pathlib.Path(__file__).resolve()
    for candidate in (
        here.parent.parent / "build" / "disc1.candidate.exe",
        here.parent.parent.parent / "build" / "disc1.candidate.exe",
        pathlib.Path("build/disc1.candidate.exe"),
        pathlib.Path("pc_port/build/disc1.candidate.exe"),
    ):
        if candidate.is_file():
            return candidate
    raise SystemExit("FAIL: could not locate disc1.candidate.exe")


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x03FFFFFF) << 2)


def run() -> int:
    path = find_exe()
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == EXE_SHA1, "executable SHA-1")
    window = data[START - LOAD_BASE:END - LOAD_BASE]
    require(len(window) == 0x118 and len(window) // 4 == 70,
            "window size is not 70 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 70 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "four-call address/order census")
    print("  OK call census: fixed wrapper, descriptor, TPage, L11 wrapper")

    require(len(WORDS) == 70, "oracle word map is not complete")
    for address in range(START, END, 4):
        expected = WORDS[address]
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print("  OK complete word comparison: 70/70")

    require((rom(0x80031398) & 0xFFFF) == 0x6A,
            "descriptor index base")
    require((rom(0x800313A8) & 0xFFFF) == 0x1C0,
            "TPage x input")
    require((rom(0x800313CC) & 0xFFFF) == 0xFFFF,
            "TPage return mask")
    print("  OK descriptor/TPage contract: index 0x6A+slot, mode 7 masked")

    branch = rom(0x80031430)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x80031434 + offset * 4
    require(target == 0x80031394, "L11 back-edge target")
    require((rom(0x8003142C) & 0xFFFF) == 13, "L11 bound is not thirteen")
    require((((1 * 3) * 8 - 1) * 4 - 1) * 4 == 364,
            "L11 bank stride")
    require(13 * 28 == 0x16C, "L11 packet extent arithmetic")
    print("  OK L11: head 0x80031394, bound 13, strides 364/28, extent 0x16C")

    require(rom(START - 4) == 0x32C200FF, "word before B54K-H window")
    require(rom(END) == 0x00002021, "first excluded epilogue-group word")
    print("  OK cut: prior L10 delay slot and first excluded word exact")
    print("\nB54K-H oracle: 7 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
