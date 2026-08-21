#!/usr/bin/env python3
"""Find lhu +0x16; addiu 1; sh +0x16 sequences."""
from __future__ import annotations

import pathlib
import struct

EXE = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe")
blob = EXE.read_bytes()
text = blob[0x800:0x800 + 0x1EE000]


def w(i: int) -> int:
    return struct.unpack_from("<I", text, i)[0]


for i in range(0, len(text) - 16, 4):
    a, b, c, d = w(i), w(i + 4), w(i + 8), w(i + 12)
    # lhu/lh rt, 0x16(rs)
    op_a = a >> 26
    if op_a not in (0x21, 0x25):  # lh lhu
        continue
    if (a & 0xFFFF) != 0x16:
        continue
    rt = (a >> 16) & 31
    # addiu same rt, 1  OR addiu after nop
    words = [b, c, d]
    va = 0x80010000 + i
    for j, x in enumerate(words):
        if (x >> 26) == 9 and ((x >> 16) & 31) == rt and ((x >> 21) & 31) == rt:
            imm = x & 0xFFFF
            if imm in (1, 0xFFFF):
                print(f"{va:#010x} lh/lhu +0x16 then addiu {imm:#x} at +{(j+1)*4}")
                break
