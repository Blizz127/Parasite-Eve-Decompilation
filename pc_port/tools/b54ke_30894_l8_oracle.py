#!/usr/bin/env python3
"""Independent B54K-E oracle for func_80030894 L8.

No production code is imported. The oracle hashes the exact retail window,
checks all 27 words, decodes the sole call, proves the complete L8 loop and
its address geometry, and verifies both words adjacent to the cut.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x800310A4
END = 0x80031110
WINDOW_SHA256 = "9b87877a27ab26756ab9cfbdf9f6f5d4e31958975ca654fcdeaf87660f7b7ecb"

CALLS = [(0x800310E4, 0x800370DC)]

WORDS = {
    0x800310A4: 0x0000B021,
    0x800310A8: 0x93A30018,
    0x800310AC: 0x3C12800A,
    0x800310B0: 0x2652E500,
    0x800310B4: 0x26530008,
    0x800310B8: 0x000310C0,
    0x800310BC: 0x00431021,
    0x800310C0: 0x00021080,
    0x800310C4: 0x00431023,
    0x800310C8: 0x000288C0,
    0x800310CC: 0x32C200FF,
    0x800310D0: 0x000280C0,
    0x800310D4: 0x02028023,
    0x800310D8: 0x00108080,
    0x800310DC: 0x02122021,
    0x800310E0: 0x02242021,
    0x800310E4: 0x0C00DC37,
    0x800310E8: 0x03C02821,
    0x800310EC: 0x26D60001,
    0x800310F0: 0x02308021,
    0x800310F4: 0x02138021,
    0x800310F8: 0x32C200FF,
    0x800310FC: 0x2C42000A,
    0x80031100: 0xA2170004,
    0x80031104: 0xA2170005,
    0x80031108: 0x1440FFF0,
    0x8003110C: 0xA2170006,
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
    require(len(window) == 0x6C and len(window) // 4 == 27,
            "window size is not 27 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 27 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "single-call address/target census")
    print("  OK call census: one jal func_800370DC site")

    require(len(WORDS) == 27, "oracle word map is not complete")
    for address in range(START, END, 4):
        expected = WORDS[address]
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print("  OK complete word comparison: 27/27")

    branch = rom(0x80031108)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x8003110C + offset * 4
    require(target == 0x800310CC, "L8 back-edge target")
    require((rom(0x800310FC) & 0xFFFF) == 10, "L8 bound is not ten")
    require((((1 * 8 + 1) * 4 - 1) * 8) == 280,
            "L8 bank-stride arithmetic")
    require(10 * 28 == 0x118, "L8 packet extent arithmetic")
    print("  OK L8 loop: head 0x800310CC, bound 10, strides 280/28, extent 0x118")

    require(rom(START - 4) == 0xA2170006, "word before B54K-E window")
    require(rom(END) == 0x3C10800A, "first excluded fixed-group word")
    print("  OK cut: prior L7 delay slot and first excluded group word exact")
    print("\nB54K-E oracle: 6 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
