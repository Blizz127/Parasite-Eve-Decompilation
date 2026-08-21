#!/usr/bin/env python3
"""Dump m0005i type-3 0x77 vertices and type-0 0x0B/0xB8 pose imms."""
from __future__ import annotations

import hashlib
import pathlib
import struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921


def load_chunk2() -> bytes:
    root = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
    pointer = (root / "local" / "pe_disc1.path").read_text().strip().splitlines()[0].strip()
    sec0, sec1, sec2 = PACKED & 0xFF, (PACKED >> 8) & 0xFFF, PACKED >> 20
    out = bytearray()
    with open(pointer, "rb") as fh:
        for i in range(sec2):
            fh.seek((PE_IMG_LBA + REL + sec0 + sec1 + i) * 2352 + 24)
            out += fh.read(2048)
    data = bytes(out)
    assert hashlib.sha256(data).hexdigest() == CHUNK2_SHA
    return data


def kinds_of(word: int, word2: int, argc: int) -> list[int]:
    k = word >> 17
    out = []
    for i in range(argc):
        if i == 5:
            k = word2
        out.append(k & 7)
        k >>= 3
    return out


def decode(blob: bytes, off: int, limit: int = 200) -> None:
    print(f"\n=== type3 chunk2+{off:#x} ===")
    pc = 0
    n = 0
    while n < limit and pc + 8 <= len(blob) - off:
        word = struct.unpack_from("<I", blob, off + pc)[0]
        word2 = struct.unpack_from("<I", blob, off + pc + 4)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = kinds_of(word, word2, argc)
        imms = [struct.unpack_from("<I", blob, off + pc + 8 + i * 4)[0] for i in range(argc)]
        print(f"  +{pc:04X}  op={op:#06x} argc={argc} kinds={kinds}")
        for i, imm in enumerate(imms):
            print(f"         arg{i} k{kinds[i]} {imm:#010x} ({imm})")
        if op == 0x77 and argc >= 8:
            verts = []
            for i in range(0, 8, 2):
                x = imms[i] & 0xFFFF
                y = imms[i + 1] & 0xFFFF
                if x >= 0x8000:
                    x -= 0x10000
                if y >= 0x8000:
                    y -= 0x10000
                verts.append((x, y))
            print(f"         RECT {verts}")
        if op == 0x00:
            break
        pc += 8 + argc * 4
        n += 1


def decode0(blob: bytes, off: int, limit: int = 80) -> None:
    print(f"\n=== type0 chunk2+{off:#x} ===")
    pc = 0
    n = 0
    while n < limit and pc + 8 <= len(blob) - off:
        word = struct.unpack_from("<I", blob, off + pc)[0]
        word2 = struct.unpack_from("<I", blob, off + pc + 4)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = kinds_of(word, word2, argc)
        imms = [struct.unpack_from("<I", blob, off + pc + 8 + i * 4)[0] for i in range(argc)]
        mark = ""
        if op in (0x0B, 0xB8, 0x17, 0x3F, 0x20, 0x65, 0xAA, 0x40):
            mark = " <<<"
        print(f"  +{pc:04X}  op={op:#06x} argc={argc} kinds={kinds}{mark}")
        for i, imm in enumerate(imms):
            print(f"         arg{i} k{kinds[i]} {imm:#010x}")
        if op == 0x00:
            break
        pc += 8 + argc * 4
        n += 1


def main() -> None:
    chunk2 = load_chunk2()
    decode0(chunk2, 0x202C8 + 0x31C, 80)
    decode0(chunk2, 0x202C8 + 0x5F8, 40)


if __name__ == "__main__":
    main()
