#!/usr/bin/env python3
"""Decode live type-3/0/1/5 streams with 17018 word layout."""
from __future__ import annotations
import hashlib, pathlib, struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
PORTED = {
    0x00, 0x01, 0x02, 0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x11, 0x14, 0x24,
    0x1C, 0x1D, 0x1F, 0x20, 0x2E, 0x2F, 0x30, 0x3F, 0x40, 0x41,
    0x4E, 0x5E, 0x77, 0x84, 0x88, 0x9B, 0xCE, 0xD9, 0xE1, 0xEA, 0xED,
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

def decode(label: str, blob: bytes, off: int, limit: int = 80) -> None:
    print(f"\n=== {label} chunk2+{off:#x} ===")
    pc = 0
    n = 0
    while n < limit and pc + 8 <= len(blob) - off:
        word = struct.unpack_from("<I", blob, off + pc)[0]
        word2 = struct.unpack_from("<I", blob, off + pc + 4)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = kinds_of(word, word2, argc)
        imms = [struct.unpack_from("<I", blob, off + pc + 8 + i * 4)[0] for i in range(argc)]
        flag = "PORTED" if op in PORTED else "NEW"
        print(f"  +{pc:03X}  {word:08X}  op={op:#06x} argc={argc} kinds={kinds} {flag}")
        for i, imm in enumerate(imms):
            print(f"         arg{i} k{kinds[i]} {imm:#010x} ({imm})")
        if op not in PORTED:
            print(f"  *** FIRST UNPORTED at +{pc:#x} op={op:#x}")
            return
        if op in (0x02, 0x30, 0x01, 0x20) and n > 0:
            # yield-ish; keep going a bit for type3 region loop
            pass
        pc += 8 + argc * 4
        n += 1

chunk2 = load_chunk2()
decode("type3 from +0xF8", chunk2, 0x227FC + 0xF8, 40)
decode("type3 from +0", chunk2, 0x227FC, 20)
decode("type0 from +0", chunk2, 0x202C8, 16)
decode("type1 from +0x104", chunk2, 0x21678 + 0x104, 16)
decode("type5 from +0xE4", chunk2, 0x22D20 + 0xE4, 16)
decode("type3 from +0x1E4 (double-miss)", chunk2, 0x227FC + 0x1E4, 24)
decode("type5 from +0x128 (scratch miss)", chunk2, 0x22D20 + 0x128, 60)
decode("type5 from +0x294 (after 0xD9 ALU)", chunk2, 0x22D20 + 0x128 + 0x16C, 24)
decode("type5 from +0x2C4 (0x24 both arms)", chunk2, 0x22D20 + 0x2C4, 40)
decode("type5 from +0x6F0 (0x11 miss)", chunk2, 0x22D20 + 0x6F0, 32)
decode("type1 from +0x148 (4A skip)", chunk2, 0x21678 + 0x148, 8)
decode("type6 from +0x1E8", chunk2, 0x2341C + 0x1E8, 8)
