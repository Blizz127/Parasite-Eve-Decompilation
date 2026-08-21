#!/usr/bin/env python3
"""Find every m0005i op that writes scratch[0] (kind 4 imm 0)."""
from __future__ import annotations

import hashlib
import pathlib
import struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
BASES = {
    0: (0x202C8, 0x21678),
    1: (0x21678, 0x218D4),
    2: (0x218D4, 0x227FC),
    3: (0x227FC, 0x22BFC),
    4: (0x22BFC, 0x22D20),
    5: (0x22D20, 0x2341C),
    6: (0x2341C, 0x2341C + 0x2000),
}
# ops that write arg0
WRITE_OPS = {0x0A, 0x1D, 0x2A, 0x87}


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
    for typ, (start, end) in BASES.items():
        pc = 0
        span = end - start
        while pc + 8 <= span:
            word = struct.unpack_from("<I", blob, start + pc)[0]
            word2 = struct.unpack_from("<I", blob, start + pc + 4)[0]
            op = word & 0x1FFF
            argc = (word >> 13) & 0xF
            if argc > 12:
                break
            kinds = kinds_of(word, word2, argc)
            imms = [struct.unpack_from("<I", blob, start + pc + 8 + i * 4)[0] for i in range(argc)]
            if op in WRITE_OPS and kinds and kinds[0] == 4 and imms and imms[0] == 0:
                print(f"  type{typ} +{pc:#06x} op={op:#x} kinds={kinds} imms={[hex(x) for x in imms]}")
            # 0x09 that ORs/ANDs into a dest that is scratch? dest is usually cond
            pc += 8 + argc * 4


if __name__ == "__main__":
    main()
