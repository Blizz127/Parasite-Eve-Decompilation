#!/usr/bin/env python3
"""Functions that both load and store actor+0x16."""
from __future__ import annotations

import pathlib
import struct

EXE = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe")
blob = EXE.read_bytes()
text = blob[0x800:0x800 + 0x1EE000]
loads = []
stores = []
for i in range(0, len(text) - 4, 4):
    w = struct.unpack_from("<I", text, i)[0]
    op = w >> 26
    off = w & 0xFFFF
    if off != 0x16:
        continue
    va = 0x80010000 + i
    if op in (0x21, 0x25):  # lh lhu
        loads.append(va)
    if op == 0x29:  # sh
        stores.append(va)

print("lh/lhu +0x16 count", len(loads))
print("sh +0x16 count", len(stores))
# pair nearby
for s in stores:
    near = [l for l in loads if abs(l - s) < 0x80]
    if near:
        print(f"store {s:#010x} near loads {[hex(x) for x in near]}")
