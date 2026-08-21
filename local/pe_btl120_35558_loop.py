#!/usr/bin/env python3
from __future__ import annotations
import hashlib, pathlib, struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

def u32(data, addr):
    return struct.unpack_from("<I", data, addr - 0x80010000 + 0x800)[0]

exe = (pathlib.Path("build/disc1.candidate.exe")).read_bytes()
assert hashlib.sha1(exe).hexdigest() == SHA1
for pc in range(0x80035970, 0x800359B8, 4):
    w = u32(exe, pc)
    print(f"  {pc:08X}  {w:08X}")
