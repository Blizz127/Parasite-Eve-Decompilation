#!/usr/bin/env python3
"""EXE-wide sh/sb/sw to offset 0x16."""
from __future__ import annotations

import pathlib
import struct

EXE = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe")
blob = EXE.read_bytes()
text = blob[0x800:0x800 + 0x1EE000]
for i in range(0, len(text) - 4, 4):
    w = struct.unpack_from("<I", text, i)[0]
    op = w >> 26
    off = w & 0xFFFF
    if off != 0x16:
        continue
    if op not in (0x28, 0x29, 0x2B):  # sb sh sw
        continue
    va = 0x80010000 + i
    base = (w >> 21) & 31
    rt = (w >> 16) & 31
    name = {0x28: "sb", 0x29: "sh", 0x2B: "sw"}[op]
    print(f"{va:#010x} {name} ${rt}, 0x16(${base}) {w:08x}")
