#!/usr/bin/env python3
"""Step 3999C with table[5]=0x80094350 from the EXE."""
from __future__ import annotations

import pathlib
import struct

EXE = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe")
blob = EXE.read_bytes()


def w(va: int) -> int:
    return struct.unpack_from("<I", blob, va - 0x80010000 + 0x800)[0]


def main() -> None:
    print("table[5]", hex(w(0x800943C0 + 20)))
    rec = w(0x800943C0 + 20)
    print("rec", hex(rec), [hex(w(rec + i * 4)) for i in range(8)])
    print("table[17]", hex(w(0x800943C0 + 17 * 4)))
    print("table[21]", hex(w(0x800943C0 + 21 * 4)))
    rec21 = w(0x800943C0 + 21 * 4)
    print("rec21", hex(rec21), [hex(w(rec21 + i * 4)) for i in range(8)])
    rec17 = w(0x800943C0 + 17 * 4)
    print("rec17", hex(rec17), [hex(w(rec17 + i * 4)) for i in range(8)])


if __name__ == "__main__":
    main()
