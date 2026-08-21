#!/usr/bin/env python3
"""Find writers of scratch[0x12] / B6A80+0x48 on m0005i streams and EXE."""
from __future__ import annotations

import hashlib
import pathlib
import struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
TYPES = {0: 0x202C8, 1: 0x21678, 2: 0x218D4, 3: 0x227FC, 4: 0x22BFC, 5: 0x22D20, 6: 0x2341C}
SPANS = {0: 0x13B0, 1: 0x218D4 - 0x21678, 2: 0x227FC - 0x218D4, 3: 0x22BFC - 0x227FC,
         4: 0x22D20 - 0x22BFC, 5: 0x2341C - 0x22D20, 6: 0x800}


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


def scan_scripts(blob: bytes) -> None:
    for typ, off in TYPES.items():
        pc = 0
        span = SPANS[typ]
        while pc + 8 <= span:
            word = struct.unpack_from("<I", blob, off + pc)[0]
            word2 = struct.unpack_from("<I", blob, off + pc + 4)[0]
            op = word & 0x1FFF
            argc = (word >> 13) & 0xF
            if argc > 8:
                pc += 4
                continue
            kinds = kinds_of(word, word2, argc)
            imms = [struct.unpack_from("<I", blob, off + pc + 8 + i * 4)[0] for i in range(argc)]
            hit = False
            if op == 0x0A and kinds and kinds[0] == 4 and imms[0] == 0x12:
                hit = True
            if op == 0x09 and len(kinds) > 1 and kinds[1] == 4 and imms[1] == 0x12:
                hit = True
            if op == 0x2A and kinds and kinds[0] == 4 and imms[0] == 0x12:
                hit = True
            if any(k == 4 and imm == 0x12 for k, imm in zip(kinds, imms)):
                print(f"  t{typ} +{pc:#06x} op={op:#x} kinds={kinds} imms={[hex(x) for x in imms]}")
            pc += 8 + argc * 4


def scan_exe() -> None:
    exe = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe").read_bytes()
    # gp-relative: B6A80 - 9CD70 = 0xE910. Unlikely via gp.
    # lui/addiu to 0x800B6A80 + 0x48
    text = exe[0x800:0x800 + 0x1EE000]
    # search addiu dest, reg, 0x6AC8 (6A80+48) after lui 0x800B
    hits = []
    for i in range(0, len(text) - 8, 4):
        w = struct.unpack_from("<I", text, i)[0]
        if w == 0x800B6AC8:
            hits.append(0x80010000 + i)
    print("imm 0x800B6AC8 sites", [hex(h) for h in hits])
    # lui 0x800B + addiu 0x6AC8
    for i in range(0, len(text) - 8, 4):
        w = struct.unpack_from("<I", text, i)[0]
        if (w & 0xFFE0FFFF) == 0x3C0B800B or (w >> 16) == 0x3C0B:
            pass
    # raw half 6AC8 as addiu imm
    addiu_hits = []
    for i in range(0, len(text) - 4, 4):
        w = struct.unpack_from("<I", text, i)[0]
        if (w & 0xFC000000) == 0x24000000 and (w & 0xFFFF) == 0x6AC8:
            addiu_hits.append(0x80010000 + i)
        if (w & 0xFC000000) == 0x8C000000 and (w & 0xFFFF) == 0x6AC8:
            addiu_hits.append(0x80010000 + i)
        if (w & 0xFC000000) == 0xAC000000 and (w & 0xFFFF) == 0x6AC8:
            addiu_hits.append(0x80010000 + i)
    print("imm16 0x6AC8 load/store/addiu", [hex(h) for h in addiu_hits[:40]], "count", len(addiu_hits))


def main() -> None:
    print("=== script scratch[0x12] uses ===")
    scan_scripts(load_chunk2())
    print("=== EXE ===")
    scan_exe()


if __name__ == "__main__":
    main()
