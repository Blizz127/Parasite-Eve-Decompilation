#!/usr/bin/env python3
"""Census 0x2A and dest tokens in m0005i actor scripts."""
from __future__ import annotations

import hashlib
import pathlib
import struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
BASES = {
    0: 0x202C8,
    1: 0x21678,
    2: 0x218D4,
    3: 0x227FC,
    4: 0x22BFC,
    5: 0x22D20,
    6: 0x2341C,
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


def walk(blob: bytes, off: int, span: int):
    pc = 0
    n = 0
    while pc + 8 <= span:
        word = struct.unpack_from("<I", blob, off + pc)[0]
        word2 = struct.unpack_from("<I", blob, off + pc + 4)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        if argc > 12:
            return
        kinds = kinds_of(word, word2, argc)
        imms = [struct.unpack_from("<I", blob, off + pc + 8 + i * 4)[0] for i in range(argc)]
        yield pc, op, kinds, imms
        if op == 0x00 and n > 2:
            # keep scanning; 0x00 is a goto, not end of file
            pass
        pc += 8 + argc * 4
        n += 1
        if n > 800:
            return


def decode_token(token: int) -> str:
    # charset at 0x800930B4 — read from EXE
    root = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    cs_off = 0x800930B4 - 0x80010000 + 0x800
    chars = exe[cs_off : cs_off + 32]
    out = []
    for i in range(6):
        shift = (5 - i) * 5 + 2
        idx = (token >> shift) & 0x1F
        ch = chars[idx]
        if 97 <= ch < 123:
            ch -= 32
        out.append(chr(ch))
    return "".join(out)


def main() -> None:
    chunk2 = load_chunk2()
    print("=== 0x2A sites ===")
    ends = {0: 0x21678, 1: 0x218D4, 2: 0x227FC, 3: 0x22BFC, 4: 0x22D20, 5: 0x2341C, 6: 0x2341C + 0x2000}
    for typ, base in BASES.items():
        span = ends[typ] - base
        for pc, op, kinds, imms in walk(chunk2, base, span):
            if op == 0x2A:
                print(f"  type{typ} +{pc:#x} kinds={kinds} imms={[hex(x) for x in imms]}")
            if op == 0x31 and imms:
                tok = imms[0]
                print(f"  type{typ} +{pc:#x} 0x31 {tok:#010x} -> {decode_token(tok)}")
            if op == 0x55:
                print(f"  type{typ} +{pc:#x} 0x55 imms={[hex(x) for x in imms]}")


if __name__ == "__main__":
    main()
