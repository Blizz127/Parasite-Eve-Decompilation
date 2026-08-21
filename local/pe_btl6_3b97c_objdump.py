#!/usr/bin/env python3
"""Dump live PE.IMG [428,434) recs used by 3B97C."""
from __future__ import annotations

import hashlib
import os
import struct
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BANK_LBA = 428
BANK_END = 434
BANK_SHA = "56b2db6d2b4a2b76e083851f29b311be68dd129fad5a578d359a33f04e7674e0"
OBJ_SHA = "fcf33e91064ee158da51cd51558ac094120a19755897680696a9d45a4e95fba4"
PE_IMG_LBA = 1013
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048


def find_disc(root: Path) -> Path | None:
    pointer = root / "local" / "pe_disc1.path"
    if pointer.is_file():
        line = pointer.read_text().strip().splitlines()[0].strip()
        path = Path(line)
        if path.is_file():
            return path
    env = os.environ.get("PE_DISC1_BIN", "").strip()
    if env:
        path = Path(env)
        if path.is_file():
            return path
    return None


def read_form1(disc: Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for sector in range(lba, lba + nsec):
            fh.seek(sector * SECTOR_RAW + FORM1_OFF)
            out.extend(fh.read(FORM1_USER))
    return bytes(out)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    disc = find_disc(root)
    assert disc is not None
    bank = read_form1(disc, PE_IMG_LBA + BANK_LBA, BANK_END - BANK_LBA)
    assert hashlib.sha256(bank).hexdigest() == BANK_SHA
    section = struct.unpack_from("<I", bank, 4)[0]
    rec = struct.unpack_from("<I", bank, section + 0x0C)[0] & 0x3FFFFF
    size, ptrw, _w8 = struct.unpack_from("<III", bank, rec)
    ptr = ptrw & 0xFFFFFF
    obj = bank[ptr : ptr + size]
    assert hashlib.sha256(obj).hexdigest() == OBJ_SHA
    print(f"size={size} obj+2={obj[2]} obj+6={struct.unpack_from('<H', obj, 6)[0]}")
    print(f"obj+8={struct.unpack_from('<H', obj, 8)[0]} +A={struct.unpack_from('<H', obj, 0xA)[0]}")
    print(f"+C={struct.unpack_from('<H', obj, 0xC)[0]} +E={struct.unpack_from('<H', obj, 0xE)[0]}")
    print(f"+0x18={struct.unpack_from('<H', obj, 0x18)[0]}")
    for i in range(2):
        off = 0x1C + i * 12
        recb = obj[off : off + 12]
        h0, h1, b4 = struct.unpack_from("<HHB", recb, 0)
        rest = recb[5:]
        print(f"rec{i} @+{off:#x}: lhu0={h0} lhu2={h1} byte4={b4} rest={rest.hex()}")
        print(f"  words={recb.hex()}")

    # dest+8 = obj+0x1C + count*12 = obj+0x1C+24
    dest8 = 0x1C + 2 * 12
    half = struct.unpack_from("<H", obj, 6)[0]
    print(f"dest+8 offset={dest8:#x} half={half} dest+8 size={half*8}")
    dest8_bytes = obj[dest8 : dest8 + half * 8]
    print(f"dest+8 first 64: {dest8_bytes[:64].hex()}")
    # rec1 index stream: dest+8 + (lhu0<<3)
    rec1_h0 = struct.unpack_from("<H", obj, 0x1C + 12)[0]
    rec1_h1 = struct.unpack_from("<H", obj, 0x1C + 14)[0]
    stream = dest8 + (rec1_h0 << 3)
    print(f"rec1 stream off={stream:#x} count={rec1_h1}")
    if rec1_h1:
        triples = obj[stream : stream + rec1_h1 * 8]
        print(f"stream bytes={triples.hex()}")
        for j in range(min(rec1_h1, 8)):
            a, b, c = struct.unpack_from("<hhh", obj, stream + 16 + j * 24) if False else (0, 0, 0)
        # ROM: a3 = dest8 + (h0<<3) + 22; then lh -16, -8, 0; a3 += 24
        a3 = stream + 22
        for j in range(min(rec1_h1, 6)):
            i0 = struct.unpack_from("<h", obj, a3 - 16 + j * 24)[0]
            i1 = struct.unpack_from("<h", obj, a3 - 8 + j * 24)[0]
            i2 = struct.unpack_from("<h", obj, a3 + j * 24)[0]
            print(f"  tri{j}: {i0}, {i1}, {i2}")

    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    # D_80091A58 table — dump first few 8-byte verts
    off = 0x80091A58 - 0x80010000 + 0x800
    print("D_80091A58 first 16 verts:")
    for i in range(16):
        vx, vy, vz, pad = struct.unpack_from("<hhhh", exe, off + i * 8)
        print(f"  [{i}] vx={vx} vy={vy} vz={vz} pad={pad}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
