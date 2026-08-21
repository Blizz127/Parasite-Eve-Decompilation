#!/usr/bin/env python3
"""Dump opcode 0xB8 handler bounds, callees, and nearby table twins."""
from __future__ import annotations

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
EXE = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe")
TADDR = 0x80010000
HDR = 0x800
TABLE = 0x800910A0


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int | None:
    if (word >> 26) != 3:
        return None
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def jr_ra(word: int) -> bool:
    return word == 0x03E00008


def dis_word(word: int) -> str:
    op = word >> 26
    rs = (word >> 21) & 31
    rt = (word >> 20) & 31
    # keep compact
    if jal_target(word):
        return f"jal {jal_target(word):#010x}"
    if jr_ra(word):
        return "jr $ra"
    return f"{word:08x} op={op} rs={rs} rt={rt}"


def main() -> None:
    blob = EXE.read_bytes()
    assert hashlib.sha1(blob).hexdigest() == SHA1
    handler = load_u32(blob, TABLE + 0xB8 * 4)
    print(f"table[0xB8]={handler:#010x}")
    for op in range(0xB0, 0xC0):
        print(f"  table[{op:#x}]={load_u32(blob, TABLE + op * 4):#010x}")

    # walk until jr ra + delay, but stop at next known table entry if close
    neighbors = sorted({load_u32(blob, TABLE + i * 4) for i in range(0x200) if load_u32(blob, TABLE + i * 4) >= handler})
    nxt = neighbors[1] if len(neighbors) > 1 else handler + 0x200
    print(f"next table va {nxt:#010x} delta {(nxt-handler)//4}w")

    va = handler
    jals = []
    words = []
    while va < handler + 0x400:
        w = load_u32(blob, va)
        words.append((va, w))
        jt = jal_target(w)
        if jt:
            jals.append((va, jt))
        if jr_ra(w):
            end = va + 8
            break
        va += 4
    else:
        end = va
    print(f"bounds {handler:#010x}..{end:#010x} words={(end-handler)//4}")
    print("sha", hashlib.sha256(blob[exe_off(handler):exe_off(end)]).hexdigest())
    print("jals:")
    for va, jt in jals:
        print(f"  {va:#010x} -> {jt:#010x}")
    print("body:")
    for va, w in words:
        print(f"  {va:#010x}  {w:08x}  {dis_word(w)}")

    # callers of handler
    enc = 0x0C000000 | ((handler & 0x0FFFFFFC) >> 2)
    print(f"encoded jal {enc:08x}")
    hits = []
    text = blob[HDR : HDR + 0x1EE000]
    off = 0
    while True:
        i = text.find(struct.pack("<I", enc), off)
        if i < 0:
            break
        hits.append(0x80010000 + i)
        off = i + 4
    print("TEXT callers", [f"{h:#010x}" for h in hits])


if __name__ == "__main__":
    main()
