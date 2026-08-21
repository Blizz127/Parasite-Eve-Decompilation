#!/usr/bin/env python3
"""Find lw +0x98 / andi 0x200 sites and nearby +0x16."""
from __future__ import annotations

import pathlib
import struct

EXE = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe")
blob = EXE.read_bytes()
text = blob[0x800:0x800 + 0x1EE000]


def w(i: int) -> int:
    return struct.unpack_from("<I", text, i)[0]


for i in range(0, len(text) - 4, 4):
    word = w(i)
    if (word & 0xFC00FFFF) != 0x8C000098:  # lw rt, 0x98(rs)
        continue
    va = 0x80010000 + i
    window = text[i:i + 0x40]
    has200 = False
    has16 = False
    for j in range(0, len(window) - 4, 4):
        x = struct.unpack_from("<I", window, j)[0]
        if (x & 0xFFFF) == 0x200 and (x >> 26) == 0x0C:
            has200 = True
        if (x & 0xFFFF) == 0x16 and (x >> 26) in (0x21, 0x25, 0x29):
            has16 = True
    if has200 or has16:
        print(f"{va:#010x} lw+0x98 200={has200} 16={has16}")
