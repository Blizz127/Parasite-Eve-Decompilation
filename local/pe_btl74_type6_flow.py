#!/usr/bin/env python3
"""Find type-6 branches that target +0xFAC / +0x1850 / +0x190."""
from __future__ import annotations

import hashlib
import pathlib
import struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
BASE = 0x2341C
SPAN = 0x2000


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


def main() -> None:
    blob = load_chunk2()
    pc = 0
    want = {0xFAC, 0x1850, 0x190, 0x1E8, 0x138, 0x630, 0x77C, 0x1138, 0x688, 0xD08, 0xBA8, 0xCA0}
    while pc + 8 <= SPAN:
        word = struct.unpack_from("<I", blob, BASE + pc)[0]
        word2 = struct.unpack_from("<I", blob, BASE + pc + 4)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        if argc > 12:
            break
        imms = [struct.unpack_from("<I", blob, BASE + pc + 8 + i * 4)[0] for i in range(argc)]
        kinds = kinds_of(word, word2, argc)
        if op in (0x00, 0x05) and imms:
            tgt = (imms[0] if op == 0x00 else imms[-1]) << 1
            if tgt in want or abs(tgt - 0xFAC) < 8 or abs(tgt - 0x1850) < 8:
                print(f"  +{pc:#06x} op={op:#x} imm={imms[-1]:#x} -> +{tgt:#x}")
        if op == 0x12:
            print(f"  +{pc:#06x} 0x12 imm={imms[0]:#x} kinds={kinds}")
        if op == 0x55:
            print(f"  +{pc:#06x} 0x55 imm={imms}")
        pc += 8 + argc * 4


if __name__ == "__main__":
    main()
