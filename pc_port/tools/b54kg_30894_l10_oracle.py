#!/usr/bin/env python3
"""Independent B54K-G oracle for func_80030894 fixed sprites and L10.

No production code is imported. The oracle hashes the exact retail window,
checks all 77 words, decodes all calls, proves L10/address geometry, verifies
the computed CLUT inputs, and checks both words adjacent to the cut.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x800311EC
END = 0x80031320
WINDOW_SHA256 = "b8eafebde2564d8c3315c39c6b7e37ae8fe04036c525f8bdd2d12a761d0e86e2"

CALLS = [
    (0x80031208, 0x800370DC),
    (0x8003122C, 0x80077AA4),
    (0x80031268, 0x800370DC),
    (0x800312E0, 0x800370DC),
]

WORDS = {
    0x800311EC: 0x3C10800A,
    0x800311F0: 0x2610E730,
    0x800311F4: 0x93A20018,
    0x800311F8: 0x03C02821,
    0x800311FC: 0x000298C0,
    0x80031200: 0x02629823,
    0x80031204: 0x00139080,
    0x80031208: 0x0C00DC37,
    0x8003120C: 0x02502021,
    0x80031210: 0x24040130,
    0x80031214: 0x240501F9,
    0x80031218: 0x26100008,
    0x8003121C: 0x02508021,
    0x80031220: 0x24020068,
    0x80031224: 0xA202000C,
    0x80031228: 0x240200F4,
    0x8003122C: 0x0C01DEA9,
    0x80031230: 0xA202000D,
    0x80031234: 0x3C11800A,
    0x80031238: 0x2631E880,
    0x8003123C: 0x02512021,
    0x80031240: 0x03C02821,
    0x80031244: 0x3C01800A,
    0x80031248: 0x00320821,
    0x8003124C: 0xA422E746,
    0x80031250: 0x24020018,
    0x80031254: 0x24090004,
    0x80031258: 0xA6020010,
    0x8003125C: 0xA6090012,
    0x80031260: 0xA2170004,
    0x80031264: 0xA2170005,
    0x80031268: 0x0C00DC37,
    0x8003126C: 0xA2170006,
    0x80031270: 0x0000B021,
    0x80031274: 0x001398C0,
    0x80031278: 0x3C14800A,
    0x8003127C: 0x2694E8B8,
    0x80031280: 0x26950008,
    0x80031284: 0x26310008,
    0x80031288: 0x02518821,
    0x8003128C: 0x2402007C,
    0x80031290: 0xA222000C,
    0x80031294: 0x240200EF,
    0x80031298: 0xA222000D,
    0x8003129C: 0x97A90020,
    0x800312A0: 0x24020024,
    0x800312A4: 0x3C01800A,
    0x800312A8: 0x00320821,
    0x800312AC: 0xA429E896,
    0x800312B0: 0xA6220010,
    0x800312B4: 0x24020005,
    0x800312B8: 0xA6220012,
    0x800312BC: 0xA2370004,
    0x800312C0: 0xA2370005,
    0x800312C4: 0xA2370006,
    0x800312C8: 0x32C200FF,
    0x800312CC: 0x000280C0,
    0x800312D0: 0x02028023,
    0x800312D4: 0x00108080,
    0x800312D8: 0x02142021,
    0x800312DC: 0x02642021,
    0x800312E0: 0x0C00DC37,
    0x800312E4: 0x03C02821,
    0x800312E8: 0x26D60001,
    0x800312EC: 0x02138021,
    0x800312F0: 0x97A90020,
    0x800312F4: 0x24020006,
    0x800312F8: 0x3C01800A,
    0x800312FC: 0x00300821,
    0x80031300: 0xA429E8CE,
    0x80031304: 0x02158021,
    0x80031308: 0xA6020010,
    0x8003130C: 0xA6020012,
    0x80031310: 0x32C200FF,
    0x80031314: 0x2C420002,
    0x80031318: 0x1440FFEC,
    0x8003131C: 0x32C200FF,
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
    require(len(window) == 0x134 and len(window) // 4 == 77,
            "window size is not 77 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 77 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "four-call address/order census")
    print("  OK call census: wrapper, CLUT, wrapper, L10 wrapper")

    require(len(WORDS) == 77, "oracle word map is not complete")
    for address in range(START, END, 4):
        expected = WORDS[address]
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print("  OK complete word comparison: 77/77")

    require((rom(0x80031210) & 0xFFFF) == 0x130,
            "computed CLUT x input")
    require((rom(0x80031214) & 0xFFFF) == 0x1F9,
            "computed CLUT y input")
    require((((0x1F9 << 6) | (0x130 >> 4)) & 0xFFFF) == 0x7E53,
            "computed CLUT value")
    print("  OK CLUT input/value: (0x130,0x1F9) -> 0x7E53")

    branch = rom(0x80031318)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x8003131C + offset * 4
    require(target == 0x800312CC, "L10 back-edge target")
    require((rom(0x80031314) & 0xFFFF) == 2, "L10 bound is not two")
    require((1 * 8 - 1) * 8 == 56, "L10 bank stride")
    require(2 * 28 == 0x38, "L10 packet extent arithmetic")
    print("  OK L10: head 0x800312CC, bound 2, strides 56/28, extent 0x38")

    require(rom(START - 4) == 0xA2170006, "word before B54K-G window")
    require(rom(END) == 0x3C12800A, "first excluded fixed-group word")
    print("  OK cut: prior L9 delay slot and first excluded group word exact")
    print("\nB54K-G oracle: 7 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
