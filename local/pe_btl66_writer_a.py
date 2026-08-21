#!/usr/bin/env python3
"""List m0005i Writer A command-table records (hdr+0x10)."""
from __future__ import annotations

import hashlib
import pathlib
import struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921


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


def main() -> None:
    c2 = load_chunk2()
    hdr = struct.unpack_from("<I", c2, 4)[0] & 0x3FFFFF
    word = struct.unpack_from("<I", c2, hdr + 0x10)[0]
    count = word >> 22
    rec = word & 0x3FFFFF
    print(f"hdr={hdr:#x} count={count} rec={rec:#x}")
    hits = []
    for i in range(count):
        off = rec + i * 12
        ptr = struct.unpack_from("<I", c2, off + 4)[0] & 0xFFFFFF
        cmd = c2[off + 7]
        typ = c2[off + 0xB]
        frame = c2[ptr + 2] if ptr + 2 < len(c2) else None
        if typ == 2 or cmd == 0x17:
            hits.append((typ, cmd, ptr, frame))
        if typ == 2 and cmd == 0x17:
            print(f"HIT type2 cmd17 ptr={ptr:#x} byte2={frame}")
    print("type2 or cmd17", hits[:30], "n", len(hits))
    types = {}
    for i in range(count):
        off = rec + i * 12
        typ = c2[off + 0xB]
        types[typ] = types.get(typ, 0) + 1
    print("types", types)


if __name__ == "__main__":
    main()
