#!/usr/bin/env python3
from __future__ import annotations
import hashlib, pathlib, struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000

def exe_off(a):
    return a - TADDR + 0x800

def load_u32(data, a):
    return struct.unpack_from("<I", data, exe_off(a))[0]

def jal_target(w):
    return ((w & 0x03FFFFFF) << 2) | 0x80000000

def window_sha(data, s, e):
    return hashlib.sha256(data[exe_off(s):exe_off(e)]).hexdigest()

data = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe").read_bytes()
assert hashlib.sha1(data).hexdigest() == SHA1
for op in (0x9B, 0x0B, 0x5E, 0x77):
    fn = load_u32(data, 0x800910A0 + op * 4)
    print(f"table[{op:#x}] = {fn:#x}")
    if fn == 0:
        continue
    # find first jr ra
    end = None
    for addr in range(fn, fn + 0x400, 4):
        w = load_u32(data, addr)
        if w == 0x03E00008:
            end = addr + 8
            break
    if end:
        words = (end - fn) // 4
        print(f"  window {fn:#x}..{end:#x} {words}w sha={window_sha(data, fn, end)}")
        jals = []
        for addr in range(fn, end, 4):
            w = load_u32(data, addr)
            if w >> 26 == 3:
                jals.append(hex(jal_target(w)))
        print(f"  jals {jals}")
        # print first 24 words
        for addr in range(fn, min(fn + 96, end), 4):
            print(f"    {addr:08X} {load_u32(data, addr):08X}")
