#!/usr/bin/env python3
"""Pin remaining lifecycle EXE words before oracle edit."""
from __future__ import annotations

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
HDR = 0x800
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")


def exe_off(a: int) -> int:
    return a - TADDR + HDR


def lu32(data: bytes, a: int) -> int:
    return struct.unpack_from("<I", data, exe_off(a))[0]


def lu16(data: bytes, a: int) -> int:
    return struct.unpack_from("<H", data, exe_off(a))[0]


def jal_target(w: int) -> int:
    return ((w & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> None:
    exe = (ROOT / "build/disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    print("6B4F8 words", (0x8006BD68 - 0x8006B4F8) // 4)
    print("6B4F8 sha", window_sha(exe, 0x8006B4F8, 0x8006BD68))
    print("6BECC words", (0x8006C1CC - 0x8006BECC) // 4)
    print("6BECC sha", window_sha(exe, 0x8006BECC, 0x8006C1CC))
    print("6BE4C sha", window_sha(exe, 0x8006BE4C, 0x8006BECC))
    print("0x9D 19450..19484", window_sha(exe, 0x80019450, 0x80019484))
    print("0x31 17BB4..17C00", window_sha(exe, 0x80017BB4, 0x80017C00))
    print("mode7 2CEE0..2CF2C", window_sha(exe, 0x8002CEE0, 0x8002CF2C))
    print("mode9 2B24C..2B298", window_sha(exe, 0x8002B24C, 0x8002B298))
    print("mode10 2BC44..2BC7C", window_sha(exe, 0x8002BC44, 0x8002BC7C))
    print("WriterA 6B84C..6B880", window_sha(exe, 0x8006B84C, 0x8006B880))
    print("3F074 jal 6BECC", hex(jal_target(lu32(exe, 0x8003F20C))))
    print("3F074 jal 6BE4C", hex(jal_target(lu32(exe, 0x8003F204))))
    print("3F074 jal 125E0", hex(jal_target(lu32(exe, 0x8003F27C))))
    print("122D8?", hex(lu32(exe, 0x800122D8)))
    print("hdr+1 lbu", hex(lu32(exe, 0x8006B7B0)))
    print("CE2 sb", hex(lu32(exe, 0x8006B7B8)))
    print("CE2 table +8 for 10", lu16(exe, 0x800930D8 + 18 * 2), lu16(exe, 0x800930D8 + 19 * 2))
    print("CE2 table +3 for 10", lu16(exe, 0x800930D8 + 13 * 2), lu16(exe, 0x800930D8 + 14 * 2))
    print("CE2 table +8 for 14", lu16(exe, 0x800930D8 + 22 * 2), lu16(exe, 0x800930D8 + 23 * 2))
    print("JT 113B0")
    for i in range(7):
        print(f"  [{i}]", hex(lu32(exe, 0x800113B0 + i * 4)))
    print("6BE4C window words")
    for addr in range(0x8006BE4C, 0x8006BECC, 4):
        w = lu32(exe, addr)
        print(f"  {addr:08X} {w:08X}")
    print("35038 +0x1AC/+0x1B0 stores")
    for addr in range(0x80035038, 0x80035280, 4):
        w = lu32(exe, addr)
        if (w & 0xFC000000) == 0xAC000000:
            rt = (w >> 16) & 31
            off = w & 0xFFFF
            off = off - 0x10000 if off >= 0x8000 else off
            if off in (0x1AC, 0x1B0, 0x1B4, 0x198, 0xE70) or rt == 0:
                print(f"  {addr:08X} sw rt={rt} off={off:#x} {w:08X}")
    print("0x9D body")
    for addr in range(0x80019450, 0x80019488, 4):
        print(f"  {addr:08X} {lu32(exe, addr):08X}")
    print("0x31 D280 stores")
    for addr in range(0x80017BB4, 0x80017C04, 4):
        print(f"  {addr:08X} {lu32(exe, addr):08X}")
    print("mode7 around 2CF24")
    for addr in range(0x8002CEE0, 0x8002CF30, 4):
        print(f"  {addr:08X} {lu32(exe, addr):08X}")
    print("mode10 around 2BC74")
    for addr in range(0x8002BC44, 0x8002BC80, 4):
        print(f"  {addr:08X} {lu32(exe, addr):08X}")
    print("6B4F8 last 8")
    for addr in range(0x8006BD48, 0x8006BD68, 4):
        print(f"  {addr:08X} {lu32(exe, addr):08X}")
    print("6C0D8..6C1A0")
    for addr in range(0x8006C0D8, 0x8006C1A4, 4):
        print(f"  {addr:08X} {lu32(exe, addr):08X}")
    # unique D28C stores of 7/9/10
    gp_sw = []
    for addr in range(0x80010000, 0x80010000 + 0x1EE000, 4):
        w = lu32(exe, addr)
        if (w & 0xFC1F0000) == 0xAF800000 and (w & 0xFFFF) == 0x051C:
            gp_sw.append((addr, w, (w >> 16) & 31))
    print("gp+0x51C stores", len(gp_sw))
    for addr, w, rt in gp_sw:
        prev = lu32(exe, addr - 4)
        print(f"  {addr:08X} sw rt={rt} prev={prev:08X}")


if __name__ == "__main__":
    main()
