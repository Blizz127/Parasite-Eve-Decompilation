#!/usr/bin/env python3
"""Decode M0367I type-1 stream after dest hop."""
from __future__ import annotations

import hashlib
import pathlib
import struct

CHUNK2_SHA = "ab9af4f446a6f9a1f8f79f1f80b862c4d516beddbede1229afab2dfc30b44b1e"
PE_IMG_LBA = 1013
REL = 0x15050
PACKED = 0x04E0AA21
TYPE1 = 0x1B454
PORTED = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
    0x0E, 0x11, 0x12, 0x14, 0x1A, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x22, 0x24,
    0x2A, 0x2E, 0x2F, 0x30, 0x31, 0x3F, 0x40, 0x41, 0x43, 0x4B, 0x4E, 0x52,
    0x53, 0x54, 0x59, 0x5A, 0x5E, 0x64, 0x65, 0x6A, 0x6B, 0x6F, 0x70, 0x77,
    0x79, 0x82, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8B, 0x94, 0x9B, 0x9C,
    0xA6, 0xAA, 0xAB, 0xAD, 0xB7, 0xC7, 0xCE, 0xD9, 0xDC, 0xE1, 0xEA, 0xED,
}


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
    assert hashlib.sha256(data).hexdigest() == CHUNK2_SHA, hashlib.sha256(data).hexdigest()
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


def decode(label: str, blob: bytes, off: int, limit: int = 80) -> None:
    print(f"\n=== {label} chunk2+{off:#x} ===")
    pc = 0
    n = 0
    first_new = None
    while n < limit and pc + 8 <= len(blob) - off:
        word = struct.unpack_from("<I", blob, off + pc)[0]
        word2 = struct.unpack_from("<I", blob, off + pc + 4)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = kinds_of(word, word2, argc)
        imms = [struct.unpack_from("<I", blob, off + pc + 8 + i * 4)[0] for i in range(argc)]
        flag = "PORTED" if op in PORTED else "NEW"
        print(f"  +{pc:04X}  {word:08X}  op={op:#06x} argc={argc} kinds={kinds} {flag}")
        for i, imm in enumerate(imms):
            print(f"          arg{i} k{kinds[i]} {imm:#010x} ({imm})")
        if op not in PORTED and first_new is None:
            first_new = (pc, op)
            print(f"  *** FIRST UNPORTED at +{pc:#x} op={op:#x}")
        pc += 8 + argc * 4
        n += 1
    if first_new:
        print(f"FIRST_UNPORTED +{first_new[0]:#x} op={first_new[1]:#x}")
    else:
        print("NO_UNPORTED_IN_WINDOW")


def main() -> None:
    chunk2 = load_chunk2()
    print(f"chunk2 len={len(chunk2):#x}")
    decode("type1 from +0", chunk2, TYPE1, 40)
    decode("type1 from +0x1C8", chunk2, TYPE1 + 0x1C8, 50)
    decode("type1 from +0", chunk2, TYPE1, 120)


if __name__ == "__main__":
    main()
