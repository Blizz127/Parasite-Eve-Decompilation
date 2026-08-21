#!/usr/bin/env python3
from __future__ import annotations
import hashlib, os, pathlib, struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048

def exe_off(a):
    return a - 0x80010000 + 0x800

def load_u32(data, a):
    return struct.unpack_from("<I", data, exe_off(a))[0]

root = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
exe = (root / "build" / "disc1.candidate.exe").read_bytes()
assert hashlib.sha1(exe).hexdigest() == SHA1
print("jtbl 0x80010190:")
for i in range(7):
    print(f"  [{i}] {load_u32(exe, 0x80010190 + i*4):#x}")

pointer = (root / "local" / "pe_disc1.path").read_text().strip().splitlines()[0].strip()
disc = pathlib.Path(pointer)
sec0 = PACKED & 0xFF
sec1 = (PACKED >> 8) & 0xFFF
sec2 = PACKED >> 20
chunk2 = bytearray()
with disc.open("rb") as fh:
    for i in range(sec2):
        fh.seek((PE_IMG_LBA + REL + sec0 + sec1 + i) * SECTOR_RAW + FORM1_OFF)
        chunk2 += fh.read(FORM1_USER)
chunk2 = bytes(chunk2)
assert hashlib.sha256(chunk2).hexdigest() == CHUNK2_SHA

def decode_stream(label, off, nwords=40):
    print(f"\n=== {label} chunk2+{off:#x} ===")
    pc = 0
    words = [struct.unpack_from("<I", chunk2, off + i*4)[0] for i in range(nwords)]
    i = 0
    while i < nwords:
        w = words[i]
        op = w & 0xFF
        argc = (w >> 8) & 0xFF
        print(f"  +{pc:03X}  {w:08X}  op={op:#x} argc={argc}")
        i += 1
        pc += 4
        # operands: typically argc words follow? 17018 uses halfword stream.
        # Print next few raw words for context
        shown = 0
        while shown < max(argc, 1) and i < nwords and (words[i] & 0xFF) > 0x20 and shown < 8:
            # heuristic only; also print remaining argc words always
            break
        for _ in range(min(argc + 2, nwords - i)):
            print(f"         {words[i]:08X}")
            i += 1
            pc += 4
            shown += 1
            if shown >= argc:
                break

# Better: dump raw words and let me decode with known 17018 format
for label, off in [("type3", 0x227FC), ("type1+0x104", 0x21678 + 0x104), ("type5+0xE4", 0x22D20 + 0xE4), ("type0", 0x202C8)]:
    print(f"\n=== {label} ===")
    for i in range(24):
        w = struct.unpack_from("<I", chunk2, off + i*4)[0]
        print(f"  +{i*4:03X}  {w:08X}  op={w & 0xFF:#04x} b1={(w>>8)&0xFF:#04x} b2={(w>>16)&0xFF:#04x} b3={(w>>24)&0xFF:#04x}")
