#!/usr/bin/env python3
from __future__ import annotations
import hashlib, pathlib, struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
LIST_OFF = 0x202A4
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048

root = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
disc = pathlib.Path(root.joinpath("local/pe_disc1.path").read_text().strip().splitlines()[0].strip())
sec0, sec1, sec2 = PACKED & 0xFF, (PACKED >> 8) & 0xFFF, PACKED >> 20
out = bytearray()
with disc.open("rb") as fh:
    for i in range(sec2):
        fh.seek((PE_IMG_LBA + REL + sec0 + sec1 + i) * SECTOR_RAW + FORM1_OFF)
        out += fh.read(FORM1_USER)
chunk2 = bytes(out)
assert hashlib.sha256(chunk2).hexdigest() == CHUNK2_SHA
off = LIST_OFF + struct.unpack_from("<I", chunk2, LIST_OFF + 28)[0]
print(f"type5 @{off:#x}")
for i in range(0, 0x100, 4):
    w = struct.unpack_from("<I", chunk2, off + i)[0]
    print(f"  +{i:03X} {w:08X} op={w & 0x1FFF:#06x} argc={(w>>13)&0xF} kinds={w>>17:05x}")
