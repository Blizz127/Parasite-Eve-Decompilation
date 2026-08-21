#!/usr/bin/env python3
"""Walk type-2 mailbox script for payloads 0xB / 0x17 / 0x1A."""
from __future__ import annotations

import hashlib
import pathlib
import struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
TYPE2 = 0x218D4
MB = 0xA8
PORTED = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
    0x0E, 0x11, 0x12, 0x14, 0x1A, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x22, 0x24,
    0x2A, 0x2E, 0x2F, 0x30, 0x31, 0x3F, 0x40, 0x41, 0x43, 0x4B, 0x4E, 0x52,
    0x53, 0x54, 0x59, 0x5A, 0x5E, 0x64, 0x65, 0x6A, 0x6B, 0x6F, 0x70, 0x77,
    0x79, 0x82, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8B, 0x94, 0x9B, 0x9C,
    0xA6, 0xAA, 0xAB, 0xAD, 0xB7, 0xB8, 0xC7, 0xCE, 0xD9, 0xDC, 0xE1, 0xEA,
    0xED,
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


def i32(x: int) -> int:
    x &= 0xFFFFFFFF
    return x - 0x100000000 if x >= 0x80000000 else x


def alu(sub: int, a: int, b: int) -> int:
    sa, sb = i32(a), i32(b)
    return {
        0: (a + b) & 0xFFFFFFFF,
        1: (a - b) & 0xFFFFFFFF,
        2: a | b,
        3: a & b,
        7: 1 if a == 0 else 0,
        0xB: 1 if a == b else 0,
        0xE: 1 if a != b else 0,
        0xA: 1 if sa < sb else 0,
        9: 1 if sb < sa else 0,
    }[sub]


def walk(blob: bytes, payload: int) -> None:
    print(f"\n=== mailbox payload {payload:#x} ===")
    cond: dict[int, int] = {}
    scratch: dict[int, int] = {}
    local: dict[int, int] = {}
    persist = {0x4A: 0x27}
    pc = MB
    seen = set()
    n = 0
    while n < 90:
        if pc in seen:
            print(f"  LOOP +{pc:#x} scratch12={scratch.get(0x12, 0)}")
            return
        seen.add(pc)
        word = struct.unpack_from("<I", blob, TYPE2 + pc)[0]
        word2 = struct.unpack_from("<I", blob, TYPE2 + pc + 4)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = kinds_of(word, word2, argc)
        imms = [struct.unpack_from("<I", blob, TYPE2 + pc + 8 + i * 4)[0] for i in range(argc)]
        flag = "PORTED" if op in PORTED else "NEW"
        print(f"  +{pc:04X} op={op:#06x} kinds={kinds} imms={[hex(x) for x in imms]} {flag}")
        if op not in PORTED:
            print(f"  *** UNPORTED +{pc:#x} op={op:#x}")
            return

        def read(kind: int, imm: int) -> int:
            return {0: imm, 1: local.get(imm, 0), 2: persist.get(imm, 0),
                    3: cond.get(imm, 0), 4: scratch.get(imm, 0)}[kind]

        def write(kind: int, imm: int, val: int) -> None:
            val &= 0xFFFFFFFF
            if kind == 1:
                local[imm] = val
            elif kind == 2:
                persist[imm] = val
            elif kind == 3:
                cond[imm] = val
            elif kind == 4:
                scratch[imm] = val
                print(f"    scratch[{imm:#x}]={val:#x}")

        nxt = pc + 8 + argc * 4
        if op == 0x1F:
            write(kinds[0], imms[0], payload)
        elif op == 0x00:
            if not imms:
                print("  END 0x00 argc0")
                return
            pc = imms[0] << 1
            n += 1
            continue
        elif op == 0x05:
            if read(kinds[0], imms[0]) == 0:
                pc = imms[1] << 1
                n += 1
                continue
        elif op == 0x09:
            sub = read(kinds[0], imms[0])
            a = read(kinds[2], imms[2])
            b = read(kinds[3], imms[3]) if argc > 3 else 0
            write(kinds[1], imms[1], alu(sub, a, b))
        elif op == 0x0A:
            write(kinds[0], imms[0], read(kinds[1], imms[1]))
        elif op == 0x01:
            print("  ACTOR END 0x01")
            return
        elif op == 0x20:
            print("  YIELD 0x20")
            return
        pc = nxt
        n += 1
    print("  WINDOW")


def main() -> None:
    blob = load_chunk2()
    for p in (0xB, 0x17, 0x1A, 0x3B, 0x84):
        walk(blob, p)


if __name__ == "__main__":
    main()
