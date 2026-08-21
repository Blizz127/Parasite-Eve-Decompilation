#!/usr/bin/env python3
"""Live m0005i tables for 0x08 / 1AA78 / B0E70."""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
TYPE1_OFF = 0x21678
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048


def exe_off(addr: int) -> int:
    return addr - TADDR + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def find_disc(root: pathlib.Path) -> pathlib.Path | None:
    pointer = root / "local" / "pe_disc1.path"
    if pointer.is_file():
        line = pointer.read_text().strip().splitlines()[0].strip()
        path = pathlib.Path(line)
        if path.is_file():
            return path
    return None


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            out += fh.read(FORM1_USER)
    return bytes(out)


def rel24(chunk: bytes, packed: int) -> int:
    return struct.unpack_from("<I", chunk, (packed & 0x3FFFFF) + 4)[0] & 0x00FFFFFF


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    exe = root / "build" / "disc1.candidate.exe"
    data = exe.read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1
    disc = find_disc(root)
    assert disc is not None
    sec0 = PACKED & 0xFF
    sec1 = (PACKED >> 8) & 0xFFF
    sec2 = PACKED >> 20
    chunk2 = read_form1(disc, PE_IMG_LBA + REL + sec0 + sec1, sec2)
    print("chunk2", len(chunk2), hashlib.sha256(chunk2).hexdigest())
    assert hashlib.sha256(chunk2).hexdigest() == CHUNK2_SHA

    print("\n===== type-1 stream +0x090..+0x120")
    for i in range(0x090, 0x120, 4):
        w = struct.unpack_from("<I", chunk2, TYPE1_OFF + i)[0]
        print(
            f"  +{i:03X}  {w:08X}  op={w & 0x1FFF:#06x} "
            f"argc={(w >> 13) & 0xF} kinds={w >> 17:05x}"
        )

    hdr_off = struct.unpack_from("<I", chunk2, 4)[0] & 0x3FFFFF
    print(f"\n===== chunk2 hdr @{hdr_off:#x}")
    for off in range(0, 0x40, 4):
        w = struct.unpack_from("<I", chunk2, hdr_off + off)[0]
        print(f"  hdr+{off:02X} = {w:08X} count={w >> 22} off={w & 0x3FFFFF:#x}")

    word18 = struct.unpack_from("<I", chunk2, hdr_off + 0x18)[0]
    obj_off = rel24(chunk2, word18)
    print(f"\n===== 1A918 obj @{obj_off:#x} from hdr+0x18={word18:08X}")
    for off in range(0, 0x30, 2):
        h = struct.unpack_from("<H", chunk2, obj_off + off)[0]
        extra = ""
        if off % 4 == 0:
            w = struct.unpack_from("<I", chunk2, obj_off + off)[0]
            extra = f" word={w:08X}"
        print(f"  obj+{off:02X} h={h:04X}{extra}")

    count = struct.unpack_from("<H", chunk2, obj_off + 2)[0]
    print(f"lhu(+2) count={count}")
    print(f"+0x18={struct.unpack_from('<I', chunk2, obj_off + 0x18)[0]:08X}")
    print(f"+0x1C={struct.unpack_from('<I', chunk2, obj_off + 0x1C)[0]:08X}")
    print(f"+0x20={struct.unpack_from('<I', chunk2, obj_off + 0x20)[0]:08X}")
    print(f"+0x24={struct.unpack_from('<I', chunk2, obj_off + 0x24)[0]:08X}")
    for i in range(min(count, 8)):
        w = struct.unpack_from("<I", chunk2, obj_off + 0x28 + i * 4)[0]
        print(f"  +0x28[{i}] = {w:08X}")
        if w + 8 < len(chunk2):
            ent = obj_off + w if w < 0x100000 else w
            # raw offset before rebase
            if w < len(chunk2):
                eh0 = struct.unpack_from("<H", chunk2, obj_off + w)[0]
                eh2 = struct.unpack_from("<H", chunk2, obj_off + w + 2)[0]
                print(f"    as obj-rel: lh0={eh0:04X} lhu2={eh2:04X}")

    # 12574 list type script bases
    list_off = 0x202A4
    print("\n===== 12574 list type entries")
    n = struct.unpack_from("<I", chunk2, list_off + 4)[0]
    print("count", n)
    for i in range(n):
        w = struct.unpack_from("<I", chunk2, list_off + 8 + i * 4)[0]
        print(f"  list[{i}] = {w:08X}")

    # B0E70 writers in 6B4F8 / 6BECC
    print("\n===== overlay +0x198-family stores in 6B4F8")
    for addr in range(0x8006B4F8, 0x8006BD68, 4):
        w = load_u32(data, addr)
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm
        op = w >> 26
        if op == 0x2B and 0x180 <= (simm & 0xFFFF) <= 0x1C0:
            print(f"  {addr:08X} sw ? {simm}(rs={(w>>21)&31})")

    print("\n===== overlay +0x198-family stores in 6BECC window")
    for addr in range(0x8006BECC, 0x8006C400, 4):
        w = load_u32(data, addr)
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm
        op = w >> 26
        if op == 0x2B and (0x190 <= simm <= 0x1C4 or simm in (0x198, 0x19C, 0x1A0, 0x1A4, 0x1A8, 0x1AC, 0x1B0)):
            rt = (w >> 16) & 31
            rs = (w >> 21) & 31
            print(f"  {addr:08X} sw r{rt}, {simm}(r{rs})")

    # 35038 +0x1AC!=0 tail: find the beq after lw +0x1AC
    print("\n===== 35038 around +0x1AC test")
    for addr in range(0x80035380, 0x80035558, 4):
        w = load_u32(data, addr)
        print(f"  {addr:08X}  {w:08X}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
