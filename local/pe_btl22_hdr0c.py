#!/usr/bin/env python3
from __future__ import annotations
import hashlib, pathlib, struct

CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
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
hdr = struct.unpack_from("<I", chunk2, 4)[0] & 0x3FFFFF
word = struct.unpack_from("<I", chunk2, hdr + 0x0C)[0]
count, off = word >> 22, word & 0x3FFFFF
print(f"hdr+0x0C count={count} off={off:#x}")
for i in range(count):
    rec = off + i * 12
    b = chunk2[rec:rec+12]
    ptr = struct.unpack_from("<I", b, 4)[0]
    print(f"  rec[{i}] @{rec:#x} bytes={b.hex()} idB={b[7]} ptr={ptr:08X} off24={ptr & 0xFFFFFF:#x}")

word18 = struct.unpack_from("<I", chunk2, hdr + 0x18)[0]
obj = struct.unpack_from("<I", chunk2, (word18 & 0x3FFFFF) + 4)[0] & 0xFFFFFF
print(f"obj size hint +0x760..+0x7D8")
print("row0", chunk2[obj+0x760:obj+0x76C].hex())
print("geom +0x98", chunk2[obj+0x98:obj+0x98+18].hex())
print("lhu+8", struct.unpack_from("<H", chunk2, obj+8)[0])
