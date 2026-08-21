#!/usr/bin/env python3
"""CE2=10 clip hashes, 6A904, 1220C loop, mode9 start."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
HDR = 0x800
PE_IMG_LBA = 1013
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")


def exe_off(a: int) -> int:
    return a - TADDR + HDR


def load_u32(data: bytes, a: int) -> int:
    return struct.unpack_from("<I", data, exe_off(a))[0]


def jal_target(w: int) -> int:
    return ((w & 0x03FFFFFF) << 2) | 0x80000000


def find_disc() -> pathlib.Path:
    return pathlib.Path((ROOT / "local/pe_disc1.path").read_text().strip().splitlines()[0].strip())


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * 2352 + 24)
            out += fh.read(2048)
    return bytes(out)


def walk12(blob: bytes, packed: int) -> list[dict]:
    count = packed >> 22
    rec = packed & 0x3FFFFF
    rows = []
    for i in range(count):
        off = rec + i * 12
        size, ptrw, w8 = struct.unpack_from("<III", blob, off)
        ptr = ptrw & 0xFFFFFF
        payload = blob[ptr : ptr + size] if ptr + size <= len(blob) else b""
        rows.append(
            {
                "idb": blob[off + 7],
                "ida": blob[off + 0xB],
                "ptr": ptr,
                "size": size,
                "sha256": hashlib.sha256(payload).hexdigest() if payload else "",
                "b0": blob[ptr],
                "b1": blob[ptr + 1],
                "b2": blob[ptr + 2],
            }
        )
    return rows


def main() -> int:
    exe = (ROOT / "build/disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    disc = find_disc()
    ce10 = read_form1(disc, PE_IMG_LBA + 288, 28)
    hdr = struct.unpack_from("<I", ce10, 4)[0] & 0x3FFFFF
    recs = walk12(ce10, struct.unpack_from("<I", ce10, hdr + 0x10)[0])
    print("===== CE2=10 Writer B rows")
    for r in recs:
        print(
            f"  idA={r['ida']} cmd={r['idb']:#04x} ptr={r['ptr']:#x} size={r['size']} "
            f"enc={r['b0']} bones-1={r['b1']} frames={r['b2']} sha={r['sha256']}"
        )
    recs0c = walk12(ce10, struct.unpack_from("<I", ce10, hdr + 0x0C)[0])
    print("===== CE2=10 hdr+0x0C")
    for r in recs0c:
        print(f"  idA={r['ida']} idB={r['idb']} ptr={r['ptr']:#x} size={r['size']} sha={r['sha256']}")

    ce14 = read_form1(disc, PE_IMG_LBA + 396, 32)
    h14 = struct.unpack_from("<I", ce14, 4)[0] & 0x3FFFFF
    r14 = walk12(ce14, struct.unpack_from("<I", ce14, h14 + 0x10)[0])
    c15_14 = next(r for r in r14 if r["idb"] == 0x15)
    c15_10 = next(r for r in recs if r["idb"] == 0x15)
    print(f"\n===== cmd 0x15 compare")
    print(f"  CE2=14 {c15_14['size']} {c15_14['sha256']} frames={c15_14['b2']}")
    print(f"  CE2=10 {c15_10['size']} {c15_10['sha256']} frames={c15_10['b2']}")
    print(f"  same={c15_14['sha256']==c15_10['sha256']}")

    print("\n===== 6A8E0..6A920 +0x154 writer")
    for addr in range(0x8006A8E0, 0x8006A930, 4):
        w = load_u32(exe, addr)
        extra = f" jal {jal_target(w):#x}" if w >> 26 == 3 else ""
        print(f"  {addr:08X}  {w:08X}{extra}")

    print("\n===== 1220C jals and j")
    for addr in range(0x8001220C, 0x80012500, 4):
        w = load_u32(exe, addr)
        if w >> 26 == 3:
            print(f"  {addr:08X} jal {jal_target(w):#x}")
        if w >> 26 == 2:
            print(f"  {addr:08X} j {((w & 0x03FFFFFF)<<2)|0x80000000:#x}")
        if w == 0x03E00008:
            print(f"  {addr:08X} jr ra")

    # function containing 2B278: walk back to addiu sp
    print("\n===== walk back from 2B24C for mode9 func start")
    addr = 0x8002B24C
    while addr > 0x8002B000:
        w = load_u32(exe, addr)
        if (w >> 26) == 9 and ((w >> 16) & 31) == 29 and (w & 0xFFFF) >= 0x8000:
            print(f"  possible start {addr:#x} {w:08X}")
            break
        addr -= 4
    print(f"  stopped at {addr:#x}")

    # 6BECC return v0=1 sites
    print("\n===== 6C1CC a0=0 restore sites already known")
    print("  6C258 restores CE2 from +0xEB")

    # window shas
    def wsha(a, b):
        return hashlib.sha256(exe[exe_off(a):exe_off(b)]).hexdigest()

    print("\n===== extra SHAs")
    print("  6BE4C", wsha(0x8006BE4C, 0x8006BECC))
    print("  3F074_6B35C_jal", hex(load_u32(exe, 0x8003F07C)))
    print("  6B7B8", hex(load_u32(exe, 0x8006B7B8)))
    print("  6B7B0", hex(load_u32(exe, 0x8006B7B0)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
