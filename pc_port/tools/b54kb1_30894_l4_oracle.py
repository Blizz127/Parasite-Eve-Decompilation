#!/usr/bin/env python3
"""Independent B54K-B1 oracle for func_80030894 bank-0 L4 continuation.

This verifier imports no production code. It checks the SHA-1-exact retail
executable window 0x80030AC4..0x80030C9C, its call order, literal address
materializations, L4 loop edge/bound, selected store words, and the first
excluded L5 instruction.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x80030AC4
END = 0x80030C9C
WINDOW_SHA256 = "79fa2086406d087e2781025634ec533d50e262f3543f5916eedcdd383da8f0f6"

CALLS = [
    (0x80030AD0, 0x80077A64),
    (0x80030AF4, 0x80037140),
    (0x80030B18, 0x80077B04),
    (0x80030B30, 0x80077C44),
    (0x80030B78, 0x80077BC4),
    (0x80030B94, 0x800370DC),
    (0x80030C58, 0x800370DC),
]

WORDS = {
    0x80030AC4: 0x00002021,  # move a0,zero
    0x80030AC8: 0x00002821,
    0x80030ACC: 0x00003021,
    0x80030AD4: 0x00003821,
    0x80030AD8: 0x3C11800A,  # D_8009E068
    0x80030ADC: 0x2631E068,
    0x80030B0C: 0x24020030,
    0x80030B10: 0xA0820004,
    0x80030B14: 0xA0820005,
    0x80030B1C: 0xA0820006,
    0x80030B24: 0x3C02800A,  # D_8009E098
    0x80030B28: 0x2442E098,
    0x80030B44: 0x3C02800B,  # D_800B00E8
    0x80030B48: 0x244200E8,
    0x80030B88: 0x3C11800B,  # D_800B6920
    0x80030B8C: 0x26316920,
    0x80030BA4: 0x3C14800A,  # D_8009E0F0
    0x80030BA8: 0x2694E0F0,
    0x80030BD8: 0x3C01800B,  # D_800B6936
    0x80030BE0: 0xA4296936,
    0x80030C10: 0xA2000004,
    0x80030C14: 0xA2040005,
    0x80030C18: 0xA203000C,
    0x80030C20: 0xA2000014,
    0x80030C34: 0xA2230004,
    0x80030C40: 0x32C200FF,
    0x80030C44: 0x000280C0,
    0x80030C48: 0x02028023,
    0x80030C4C: 0x00108080,
    0x80030C6C: 0x24020006,
    0x80030C70: 0x3C01800A,  # D_8009E106
    0x80030C78: 0xA429E106,
    0x80030C80: 0xA6020010,
    0x80030C84: 0x2402000A,
    0x80030C88: 0xA6020012,
    0x80030C90: 0x2C420004,
    0x80030C94: 0x1440FFEB,
    0x80030C98: 0x32C200FF,
    0x80030C9C: 0x0000B021,  # first excluded: move s6,zero
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
    require(len(window) == 0x1D8 and len(window) // 4 == 118,
            "window size is not 118 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 118 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "seven-call address/order census")
    print("  OK call census: seven jal sites in retail order")

    for address, expected in WORDS.items():
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print(f"  OK selected literal words: {len(WORDS)}")

    branch = rom(0x80030C94)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x80030C98 + offset * 4
    require(target == 0x80030C44, "L4 back-edge target")
    require((rom(0x80030C90) & 0xFFFF) == 4, "L4 bound is not four")
    require(rom(END) == 0x0000B021, "first excluded L5 word")
    print("  OK L4 loop: head 0x80030C44, bound 4, cut before L5")
    print("\nB54K-B1 oracle: 4 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
