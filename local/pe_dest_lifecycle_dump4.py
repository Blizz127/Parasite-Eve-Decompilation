#!/usr/bin/env python3
"""CE2 table, CE2=10 bank, +0x154, 1220C loop, type4 0x01, mode JT."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
HDR = 0x800
PE_IMG_LBA = 1013
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")


def exe_off(addr: int) -> int:
    return addr - TADDR + HDR


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def load_u16(data: bytes, addr: int) -> int:
    return struct.unpack_from("<H", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def find_disc() -> pathlib.Path:
    pointer = ROOT / "local" / "pe_disc1.path"
    return pathlib.Path(pointer.read_text().strip().splitlines()[0].strip())


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            out += fh.read(FORM1_USER)
    return bytes(out)


def walk12(blob: bytes, packed: int) -> list[dict]:
    count = packed >> 22
    rec = packed & 0x3FFFFF
    rows = []
    for i in range(count):
        off = rec + i * 12
        if off + 12 > len(blob):
            break
        size, ptrw, w8 = struct.unpack_from("<III", blob, off)
        ptr = ptrw & 0xFFFFFF
        payload = blob[ptr : ptr + size] if ptr + size <= len(blob) else b""
        rows.append(
            {
                "i": i,
                "size": size,
                "ptr": ptr,
                "idb": blob[off + 7],
                "ida": blob[off + 0xB],
                "sha256": hashlib.sha256(payload).hexdigest() if payload else "",
                "b0": blob[ptr] if ptr < len(blob) else None,
                "b1": blob[ptr + 1] if ptr + 1 < len(blob) else None,
                "b2": blob[ptr + 2] if ptr + 2 < len(blob) else None,
            }
        )
    return rows


def walk_script(blob: bytes, base: int, limit: int = 80) -> list[dict]:
    rows = []
    pc = base
    for _ in range(limit):
        if pc + 8 > len(blob):
            break
        word = struct.unpack_from("<I", blob, pc)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        span = 8 + argc * 4
        if pc + span > len(blob):
            break
        imms = [struct.unpack_from("<I", blob, pc + 8 + i * 4)[0] for i in range(argc)]
        rows.append({"rel": pc - base, "op": op, "argc": argc, "imms": imms})
        pc += span
    return rows


def main() -> int:
    exe = (ROOT / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    disc = find_disc()

    print("===== D_800930D8[0..40]")
    for i in range(41):
        h = load_u16(exe, 0x800930D8 + i * 2)
        print(f"  [{i:2d}] {h}")

    print("\n===== CE2+8 banks for CE2=10 and CE2=14")
    for ce2 in (10, 11, 12, 13, 14):
        lo = load_u16(exe, 0x800930D8 + (ce2 + 8) * 2)
        hi = load_u16(exe, 0x800930D8 + (ce2 + 9) * 2)
        nsec = hi - lo
        print(f"  CE2={ce2} +8 table[{ce2+8}]={lo} table[{ce2+9}]={hi} nsec={nsec}")
        if 0 < nsec <= 64:
            blob = read_form1(disc, PE_IMG_LBA + lo, nsec)
            sha = hashlib.sha256(blob).hexdigest()
            hdr = struct.unpack_from("<I", blob, 4)[0] & 0x3FFFFF
            recs0c = walk12(blob, struct.unpack_from("<I", blob, hdr + 0x0C)[0])
            recs10 = walk12(blob, struct.unpack_from("<I", blob, hdr + 0x10)[0])
            print(f"    size={len(blob)} sha={sha} hdr={hdr:#x}")
            print(f"    0C n={len(recs0c)} idA/idB={[(r['ida'], r['idb']) for r in recs0c]}")
            print(
                f"    10 n={len(recs10)} idA={sorted({r['ida'] for r in recs10})} "
                f"idB={sorted({r['idb'] for r in recs10})}"
            )
            if recs0c:
                r = recs0c[0]
                print(f"    0C[0] ptr={r['ptr']:#x} size={r['size']} sha={r['sha256']}")

    print("\n===== CE2+3 banks")
    for ce2 in (10, 14):
        lo = load_u16(exe, 0x800930D8 + (ce2 + 3) * 2)
        hi = load_u16(exe, 0x800930D8 + (ce2 + 4) * 2)
        print(f"  CE2={ce2} +3 table[{ce2+3}]={lo} table[{ce2+4}]={hi} nsec={hi-lo}")

    # 1220C after 123F0
    print("\n===== 1220C after 123F0")
    for addr in range(0x800123F0, 0x80012480, 4):
        w = load_u32(exe, addr)
        op = w >> 26
        fn = w & 63
        extra = ""
        if op == 3:
            extra = f" jal {jal_target(w):#x}"
        if op == 2:
            extra = f" j {((w & 0x03FFFFFF) << 2) | 0x80000000:#x}"
        if op == 0 and fn == 8:
            extra = " jr"
        print(f"  {addr:08X}  {w:08X}{extra}")

    # +0x154 via lui 800b + sw
    print("\n===== overlay+0x154 materializations")
    for addr in range(0x80010000, 0x801EE000, 4):
        w = load_u32(exe, addr)
        if (w >> 26) != 0x2B:
            continue
        rs = (w >> 21) & 31
        soff = w & 0xFFFF
        if soff >= 0x8000:
            soff -= 0x10000
        # overlay base 0x800B0CD8 + 0x154 = 0x800B0E2C
        if rs == 28:
            continue
        for j in range(1, 8):
            ba = addr - j * 4
            lw = load_u32(exe, ba)
            if (lw >> 26) == 0x0F and ((lw >> 16) & 31) == rs:
                hi = lw & 0xFFFF
                dest = (hi << 16) + soff
                if dest == 0x800B0E2C:
                    print(f"  sw@{addr:#x} dest D_800B0E2C via lui@{ba:#x}")
            if (lw >> 26) == 9 and ((lw >> 16) & 31) == rs:
                # addiu rs
                pass

    # search addiu xx, overlay, 0x154 then sw
    print("\n===== addiu ..., 340 / 0x154")
    for addr in range(0x80010000, 0x801EE000, 4):
        w = load_u32(exe, addr)
        if (w >> 26) == 9 and (w & 0xFFFF) == 0x154:
            print(f"  {addr:08X}  addiu rt={(w>>16)&31} rs={(w>>21)&31} 0x154")

    # type4 continue for 0x01
    s0, s1, s2 = 33, 170, 78
    m367 = read_form1(disc, PE_IMG_LBA + 0x15050 + s0 + s1, s2)
    LIST = 0x1B314
    rel = struct.unpack_from("<I", m367, LIST + 8 + 4 * 4)[0]
    base = LIST + rel
    rows = walk_script(m367, base, 80)
    print(f"\n===== M0367I type4 0x2E sites from {base:#x}")
    for r in rows:
        if r["op"] in (0x2E, 0x2F, 0x9D, 0x01, 0x02, 0x30):
            print(f"  +{r['rel']:04X} op={r['op']:#06x} imms={[hex(x) for x in r['imms']]}")

    # mode JT 0x80010910
    print("\n===== JT 0x80010910 (gp+0x104 / mode9 cluster)")
    for i in range(8):
        tgt = load_u32(exe, 0x80010910 + i * 4)
        print(f"  [{i}] {tgt:#010x}")

    # D2F0
    print(f"\n===== D_8009D2F0 = {load_u32(exe, 0x8009D2F0):#x} (bss, runtime ptr)")
    print(f"  table[0x9D]={load_u32(exe, 0x800910A0 + 0x9D*4):#x}")

    # 6B4F8 chunk hashes for M0367I
    rel = load_u32(exe, 0x80093378 + 366 * 8)
    packed = load_u32(exe, 0x80093378 + 366 * 8 + 4)
    t0, t1, t2 = packed & 0xFF, (packed >> 8) & 0xFFF, packed >> 20
    c0 = read_form1(disc, PE_IMG_LBA + rel, t0)
    c1 = read_form1(disc, PE_IMG_LBA + rel + t0, t1)
    c2 = read_form1(disc, PE_IMG_LBA + rel + t0 + t1, t2)
    print("\n===== M0367I chunks")
    print(f"  c0 {len(c0)} {hashlib.sha256(c0).hexdigest()}")
    print(f"  c1 {len(c1)} {hashlib.sha256(c1).hexdigest()}")
    print(f"  c2 {len(c2)} {hashlib.sha256(c2).hexdigest()}")
    print(f"  lba c0={PE_IMG_LBA+rel} c1={PE_IMG_LBA+rel+t0} c2={PE_IMG_LBA+rel+t0+t1}")

    # 3F074 return / is it really every tick: find jr ra of 3F074
    print("\n===== 3F074 jr/jals after 3F250")
    for addr in range(0x8003F250, 0x8003F3C4, 4):
        w = load_u32(exe, addr)
        if (w >> 26) == 3:
            print(f"  {addr:08X} jal {jal_target(w):#x}")
        if w == 0x03E00008:
            print(f"  {addr:08X} jr ra")

    # overlay+0xEC value after 6B35C: confirm no store
    print("\n===== 6B35C stores to +0x0A/+0x0B/+0xEC/+0x154")
    for addr in range(0x8006B35C, 0x8006B4F8, 4):
        w = load_u32(exe, addr)
        op = w >> 26
        if op in (0x28, 0x29, 0x2B):
            soff = w & 0xFFFF
            if soff >= 0x8000:
                soff -= 0x10000
            if soff in (10, 11, 236, 340, 0x0A, 0x0B):
                print(f"  {addr:08X} op={op:02x} off={soff}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
