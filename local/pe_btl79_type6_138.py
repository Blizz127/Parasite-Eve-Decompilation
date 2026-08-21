#!/usr/bin/env python3
"""Decode type-6 +0x138..+0x420 and +0xF80..+0x1140."""
from __future__ import annotations

import hashlib
import pathlib
import struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
BASE = 0x2341C


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


def decode(blob: bytes, start_pc: int, end_pc: int) -> None:
    print(f"\n=== type-6 {start_pc:#x}..{end_pc:#x} ===")
    pc = start_pc
    while pc + 8 <= end_pc:
        word = struct.unpack_from("<I", blob, BASE + pc)[0]
        word2 = struct.unpack_from("<I", blob, BASE + pc + 4)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = kinds_of(word, word2, argc)
        imms = [struct.unpack_from("<I", blob, BASE + pc + 8 + i * 4)[0] for i in range(argc)]
        extra = ""
        if op in (0x00, 0x05) and imms:
            tgt = (imms[0] if op == 0x00 else imms[-1]) << 1
            extra = f" -> +{tgt:#x}"
        print(f"  +{pc:04X}  op={op:#06x} argc={argc} kinds={kinds} imms={[hex(x) for x in imms]}{extra}")
        pc += 8 + argc * 4


def main() -> None:
    blob = load_chunk2()
    decode(blob, 0x138, 0x420)
    decode(blob, 0xF80, 0x1140)


if __name__ == "__main__":
    main()
