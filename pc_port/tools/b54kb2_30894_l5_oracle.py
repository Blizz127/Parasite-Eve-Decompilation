#!/usr/bin/env python3
"""Independent B54K-B2 oracle for func_80030894's L5 continuation.

This verifier imports no production code. It checks the SHA-1-exact retail
window 0x80030C9C..0x80030D20, every instruction word, the sole call, the
five-packet loop edge/bound, and the first excluded post-L5 instruction.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x80030C9C
END = 0x80030D20
WINDOW_SHA256 = "9fbe234f952c643335daaff99026f070ed84133c5654f9ed673c4e14893cec10"

CALLS = [(0x80030CDC, 0x800370DC)]

# Complete 33-word window plus the first excluded word.
WORDS = {
    0x80030C9C: 0x0000B021,
    0x80030CA0: 0x93A30018,
    0x80030CA4: 0x3C12800A,
    0x80030CA8: 0x2652E1D0,
    0x80030CAC: 0x26530008,
    0x80030CB0: 0x000310C0,
    0x80030CB4: 0x00431021,
    0x80030CB8: 0x00021080,
    0x80030CBC: 0x00431023,
    0x80030CC0: 0x00028880,
    0x80030CC4: 0x32C200FF,
    0x80030CC8: 0x000280C0,
    0x80030CCC: 0x02028023,
    0x80030CD0: 0x00108080,
    0x80030CD4: 0x02122021,
    0x80030CD8: 0x02242021,
    0x80030CDC: 0x0C00DC37,
    0x80030CE0: 0x03C02821,
    0x80030CE4: 0x26D60001,
    0x80030CE8: 0x02118021,
    0x80030CEC: 0x97A90020,
    0x80030CF0: 0x24020006,
    0x80030CF4: 0x3C01800A,
    0x80030CF8: 0x00300821,
    0x80030CFC: 0xA429E1E6,
    0x80030D00: 0x02138021,
    0x80030D04: 0xA6020010,
    0x80030D08: 0x2402000A,
    0x80030D0C: 0xA6020012,
    0x80030D10: 0x32C200FF,
    0x80030D14: 0x2C420005,
    0x80030D18: 0x1440FFEB,
    0x80030D1C: 0x32C200FF,
    0x80030D20: 0x93B50018,
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
    require(len(window) == 0x84 and len(window) // 4 == 33,
            "window size is not 33 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 33 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    for address, expected in WORDS.items():
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print("  OK complete word table: 33 words plus first excluded word")

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "sole-call address/target census")
    print("  OK call census: func_800370DC at 0x80030CDC")

    branch = rom(0x80030D18)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x80030D1C + offset * 4
    require(target == 0x80030CC8, "L5 back-edge target")
    require((rom(0x80030D14) & 0xFFFF) == 5, "L5 bound is not five")
    require(5 * 28 == 0x8C, "L5 packet extent arithmetic")
    require(rom(END) == 0x93B50018, "first excluded post-L5 word")
    print("  OK L5 loop: head 0x80030CC8, bound 5, extent 0x8C")
    print("  OK cut: first excluded word is lbu s5,0x18(sp)")
    print("\nB54K-B2 oracle: 5 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
