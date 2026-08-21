#!/usr/bin/env python3
"""Find lhu/sh of +8 near D300 / gp+0x590 (task+8)."""
from __future__ import annotations

import pathlib
import struct

EXE = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe")
text = EXE.read_bytes()[0x800 : 0x800 + 0x1EE000]


def w(i: int) -> int:
    return struct.unpack_from("<I", text, i)[0]


def main() -> None:
    for i in range(0, len(text) - 4, 4):
        word = w(i)
        # lhu/lh rt, 8(rs)
        if (word & 0xFC00FFFF) not in (0x94000008, 0x84000008):
            continue
        va = 0x80010000 + i
        window = text[max(0, i - 16) : i + 48]
        nearby = []
        for j in range(0, len(window) - 4, 4):
            x = struct.unpack_from("<I", window, j)[0]
            if (x & 0xFC00FFFF) in (0xA4000008, 0xA0000008):  # sh/sb +8
                nearby.append("sh+8")
            if (x & 0xFFFF) in (0x10, 0x50, 0xFFEF, 0xFFAF, 0xFFBF):
                nearby.append(f"{x:08x}")
            if x in (0x8F840590, 0x8F820590, 0x8F830590):
                nearby.append("lw D300")
        if nearby:
            print(f"{va:#010x} lhu+8 {nearby}")


if __name__ == "__main__":
    main()
