#!/usr/bin/env python3
"""Dump dest+0xC color flags and dest+0x88 writers near 3B97C."""
from __future__ import annotations

import hashlib
import os
import struct
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800
BANK_LBA = 428
BANK_END = 434
BANK_SHA = "56b2db6d2b4a2b76e083851f29b311be68dd129fad5a578d359a33f04e7674e0"
OBJ_SHA = "fcf33e91064ee158da51cd51558ac094120a19755897680696a9d45a4e95fba4"
PE_IMG_LBA = 1013


def va2off(va: int) -> int:
    return va - LOAD + EXE_HDR


def find_disc(root: Path) -> Path:
    line = (root / "local" / "pe_disc1.path").read_text().strip().splitlines()[0]
    return Path(line)


def read_form1(disc: Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for sector in range(lba, lba + nsec):
            fh.seek(sector * 2352 + 24)
            out.extend(fh.read(2048))
    return bytes(out)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    bank = read_form1(find_disc(root), PE_IMG_LBA + BANK_LBA, BANK_END - BANK_LBA)
    assert hashlib.sha256(bank).hexdigest() == BANK_SHA
    section = struct.unpack_from("<I", bank, 4)[0]
    rec = struct.unpack_from("<I", bank, section + 0x0C)[0] & 0x3FFFFF
    size, ptrw, _ = struct.unpack_from("<III", bank, rec)
    obj = bank[ptrw & 0xFFFFFF : (ptrw & 0xFFFFFF) + size]
    assert hashlib.sha256(obj).hexdigest() == OBJ_SHA

    dest8 = 0x1C + 24
    half = struct.unpack_from("<H", obj, 6)[0]
    destc = dest8 + (half << 3)
    print(f"dest+0xC off={destc:#x} half={half} color bytes={half*4}")
    colors = obj[destc : destc + half * 4]
    nonzero = [(i, colors[i : i + 4].hex()) for i in range(0, len(colors), 4) if any(colors[i : i + 4])]
    print(f"nonzero color recs: {len(nonzero)} / {half}")
    print("first 12 color recs:", colors[:48].hex())
    print("nonzero sample:", nonzero[:12])

    # rec1 index=1, t2=0..37; v0 = 1+t2; word = dest+0xC[(1+t2)*4]
    rec1_h0 = 1
    flags = []
    for t2 in range(38):
        off = destc + (rec1_h0 + t2) * 4
        b = obj[off : off + 12]  # three candidate words
        flags.append((t2, obj[off + 3], obj[off + 7] if off + 7 < destc + half * 4 else -1, obj[off + 11] if off + 11 < destc + half * 4 else -1, b[:12].hex()))
    live_hits = [x for x in flags if x[1] or (x[2] not in (0, -1)) or (x[3] not in (0, -1))]
    print(f"rec1 triples with any +3/+7/+B: {len(live_hits)}")
    print("first 8 flags:", flags[:8])

    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    # gp+0x30 = 0x8009CDA0 initial EXE word
    print(f"EXE D_8009CDA0={struct.unpack_from('<I', exe, va2off(0x8009CDA0))[0]:08X}")
    print(f"EXE D_800BD025={exe[va2off(0x800BD025):va2off(0x800BD025)+3].hex()}")

    # ctc2 into LCM rd=16..20 in 3A088 / 6698C / 3C5D8 / 3D050 / 794C4
    ranges = [
        (0x8003A088, 0x8003A6A8, "3A088"),
        (0x8006698C, 0x80066B60, "6698C"),
        (0x8003C5D8, 0x8003C638, "3C5D8"),
        (0x8003D050, 0x8003D834, "3D050"),
        (0x800794C4, 0x80079750, "794C4"),
        (0x8003B97C, 0x8003BCE0, "3B97C"),
        (0x8003D834, 0x8003D94C, "3D834"),
        (0x80077F7C, 0x80078040, "InitGeom"),
    ]
    for lo, hi, name in ranges:
        hits = []
        for va in range(lo, hi, 4):
            w = struct.unpack_from("<I", exe, va2off(va))[0]
            if (w >> 26) != 0x12:
                continue
            rs = (w >> 21) & 31
            rd = (w >> 11) & 31
            if rs == 6:
                hits.append((va, rd, w))
        if hits:
            print(f"{name} ctc2:", [(hex(v), rd) for v, rd, _ in hits])

    # dest+0x88 stores in 3D050 / 3C5D8 / 3A088
    for lo, hi, name in ranges:
        stores = []
        for va in range(lo, hi, 4):
            w = struct.unpack_from("<I", exe, va2off(va))[0]
            op = w >> 26
            imm = w & 0xFFFF
            simm = imm - 0x10000 if imm >= 0x8000 else imm
            if op in (0x28, 0x29, 0x2B) and simm in (0x88, 0x89, 0x8A):
                stores.append((hex(va), op, simm, f"{w:08X}"))
        if stores:
            print(f"{name} stores +0x88:", stores)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
