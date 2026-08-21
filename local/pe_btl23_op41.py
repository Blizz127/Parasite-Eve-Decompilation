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
for op in (0x41, 0x2E, 0x4E, 0x2F, 0x30):
    fn = load_u32(data, 0x800910A0 + op * 4)
    print(f"table[{op:#x}] = {fn:#x}")
    if not fn:
        continue
    end = None
    for addr in range(fn, fn + 0x200, 4):
        if load_u32(data, addr) == 0x03E00008:
            end = addr + 8
            break
    if end:
        print(f"  {(end-fn)//4}w sha={window_sha(data, fn, end)}")
        jals = [hex(jal_target(load_u32(data, a))) for a in range(fn, end, 4) if load_u32(data, a) >> 26 == 3]
        print(f"  jals {jals}")
        for a in range(fn, min(fn + 64, end), 4):
            print(f"    {a:08X} {load_u32(data, a):08X}")
