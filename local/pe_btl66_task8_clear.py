#!/usr/bin/env python3
"""Find TEXT sites that clear task+8 bits 0x10 / 0x40 / 0x50."""
from __future__ import annotations

import pathlib
import struct

EXE = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe")
TADDR = 0x80010000
HDR = 0x800


def main() -> None:
    blob = EXE.read_bytes()
    text = blob[HDR : HDR + 0x1EE000]
    masks = {0xFFEF: "clear 0x10", 0xFFBF: "clear 0x40", 0xFFAF: "clear 0x50",
             0xFFDF: "clear 0x20", 0xFFFB: "clear 4"}
    for i in range(0, len(text) - 4, 4):
        w = struct.unpack_from("<I", text, i)[0]
        if (w >> 26) != 0x0C:  # andi is SPECIAL? No, andi is 0x0C
            pass
        op = w >> 26
        imm = w & 0xFFFF
        if op == 0x0C and imm in masks:  # andi
            va = TADDR + i
            print(f"{va:#010x} andi {imm:#06x} {masks[imm]} word={w:08x}")


if __name__ == "__main__":
    main()
