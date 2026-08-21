#!/usr/bin/env python3
"""Walk m0005i / m0367i streams with persist[0x4A] after Watch."""
from __future__ import annotations

import hashlib
import pathlib
import struct

M0005I = {
    "name": "m0005i",
    "sha": "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b",
    "rel": 0x266A,
    "packed": 0x0600A921,
    "types": {0: 0x202C8, 1: 0x21678, 2: 0x218D4, 3: 0x227FC, 4: 0x22BFC, 5: 0x22D20, 6: 0x2341C},
}
M0367I = {
    "name": "m0367i",
    "sha": "ab9af4f446a6f9a1f8f79f1f80b862c4d516beddbede1229afab2dfc30b44b1e",
    "rel": 0x15050,
    "packed": 0x04E0AA21,
    "types": {1: 0x1B454},
}
PE_IMG_LBA = 1013
PORTED = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
    0x0E, 0x11, 0x12, 0x14, 0x1A, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x22, 0x24,
    0x2A, 0x2E, 0x2F, 0x30, 0x31, 0x3F, 0x40, 0x41, 0x43, 0x4B, 0x4E, 0x52,
    0x53, 0x54, 0x59, 0x5A, 0x5E, 0x64, 0x65, 0x6A, 0x6B, 0x6F, 0x70, 0x77,
    0x79, 0x82, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8B, 0x94, 0x9B, 0x9C,
    0xA6, 0xAA, 0xAB, 0xAD, 0xB7, 0xB8, 0xC7, 0xCE, 0xD9, 0xDC, 0xE1, 0xEA,
    0xED,
}


def load_chunk2(meta: dict) -> bytes:
    root = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
    pointer = (root / "local" / "pe_disc1.path").read_text().strip().splitlines()[0].strip()
    packed = meta["packed"]
    sec0, sec1, sec2 = packed & 0xFF, (packed >> 8) & 0xFFF, packed >> 20
    out = bytearray()
    with open(pointer, "rb") as fh:
        for i in range(sec2):
            fh.seek((PE_IMG_LBA + meta["rel"] + sec0 + sec1 + i) * 2352 + 24)
            out += fh.read(2048)
    data = bytes(out)
    assert hashlib.sha256(data).hexdigest() == meta["sha"]
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


def alu(sub: int, a: int, b: int) -> int:
    sa, sb = ctypes_i32(a), ctypes_i32(b)
    if sub == 0:
        return (a + b) & 0xFFFFFFFF
    if sub == 1:
        return (a - b) & 0xFFFFFFFF
    if sub == 2:
        return a | b
    if sub == 3:
        return a & b
    if sub == 4:
        return a ^ b
    if sub == 5:
        return 1 if (a or b) else 0
    if sub == 6:
        return 1 if (a and b) else 0
    if sub == 7:
        return 1 if a == 0 else 0
    if sub == 8:
        return (~a) & 0xFFFFFFFF
    if sub == 9:
        return 1 if sb < sa else 0
    if sub == 0xA:
        return 1 if sa < sb else 0
    if sub == 0xB:
        return 1 if a == b else 0
    if sub == 0xC:
        return 1 if sa >= sb else 0
    if sub == 0xD:
        return 1 if sa <= sb else 0
    if sub == 0xE:
        return 1 if a != b else 0
    if sub == 0x11:
        return (a << (b & 31)) & 0xFFFFFFFF
    raise SystemExit(f"unhandled alu {sub:#x}")


def ctypes_i32(x: int) -> int:
    x &= 0xFFFFFFFF
    return x - 0x100000000 if x >= 0x80000000 else x


def walk(label: str, blob: bytes, base: int, persist: dict[int, int], limit: int = 80,
         scratch0: dict[int, int] | None = None) -> None:
    print(f"\n=== {label} persist4A={persist.get(0x4A, 0):#x} ===")
    cond = [0] * 256
    scratch = [0] * 256
    local = [0] * 256
    if scratch0:
        for k, v in scratch0.items():
            scratch[k] = v
    pc = 0
    seen = set()
    n = 0
    while n < limit:
        if pc in seen:
            print(f"  LOOP at +{pc:#x}")
            return
        seen.add(pc)
        word = struct.unpack_from("<I", blob, base + pc)[0]
        word2 = struct.unpack_from("<I", blob, base + pc + 4)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = kinds_of(word, word2, argc)
        imms = [struct.unpack_from("<I", blob, base + pc + 8 + i * 4)[0] for i in range(argc)]
        flag = "PORTED" if op in PORTED else "NEW"
        print(f"  +{pc:04X} op={op:#06x} argc={argc} kinds={kinds} imms={[hex(x) for x in imms]} {flag}")
        if op not in PORTED:
            print(f"  *** FIRST UNPORTED +{pc:#x} op={op:#x}")
            return
        nxt = pc + 8 + argc * 4

        def read(kind: int, imm: int) -> int:
            if kind == 0:
                return imm
            if kind == 1:
                return local[imm]
            if kind == 2:
                return persist.get(imm, 0)
            if kind == 3:
                return cond[imm]
            if kind == 4:
                return scratch[imm]
            raise SystemExit(f"kind {kind}")

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
            elif kind == 0:
                pass
            else:
                raise SystemExit(f"write kind {kind}")

        if op == 0x00:
            pc = imms[0] << 1
            n += 1
            continue
        if op == 0x05:
            if read(kinds[0], imms[0]) == 0:
                pc = imms[1] << 1
                n += 1
                continue
        if op == 0x09:
            sub = read(kinds[0], imms[0])
            a = read(kinds[2], imms[2])
            b = read(kinds[3], imms[3]) if argc > 3 else 0
            write(kinds[1], imms[1], alu(sub, a, b))
        if op == 0x0A:
            write(kinds[0], imms[0], read(kinds[1], imms[1]))
        if op == 0x2A:
            cur = read(kinds[0], imms[0])
            write(kinds[0], imms[0], cur | (1 << (read(kinds[1], imms[1]) & 31)))
        if op == 0x31:
            print(f"  DEST {imms[0]:#010x} persist4A={persist.get(0x4A, 0):#x}")
            return
        if op == 0x20:
            print("  YIELD 0x20")
            return
        pc = nxt
        n += 1
    print("  WINDOW_END")


def main() -> None:
    m5 = load_chunk2(M0005I)
    m367 = load_chunk2(M0367I)
    walk("m0367i t1 persist26", m367, M0367I["types"][1], {0x4A: 0x26}, 40)
    walk("m0005i t1 persist27", m5, M0005I["types"][1], {0x4A: 0x27}, 80)
    walk("m0005i t6 persist27", m5, M0005I["types"][6], {0x4A: 0x27, 0xA: 0}, 80)
    walk("m0005i t0 persist27", m5, M0005I["types"][0], {0x4A: 0x27, 0: 0, 1: 5, 8: 0}, 80)
    walk("m0005i t0 scratch12=1", m5, M0005I["types"][0], {0x4A: 0x27, 0: 0, 1: 5, 8: 0}, 120,
         {0x12: 1})
    walk("m0005i t2 persist27", m5, M0005I["types"][2], {0x4A: 0x27, 0: 0, 1: 5, 8: 0}, 80)
    walk("m0005i t2 mailbox +0xA8", m5, M0005I["types"][2] + 0xA8, {0x4A: 0x27, 0: 0, 1: 5, 8: 0}, 80)
    walk("m0005i t3 persist27", m5, M0005I["types"][3], {0x4A: 0x27, 0: 0, 1: 5, 8: 0}, 40)
    walk("m0005i t5 persist27", m5, M0005I["types"][5], {0x4A: 0x27, 0: 0, 1: 5, 8: 0}, 40)


if __name__ == "__main__":
    main()
