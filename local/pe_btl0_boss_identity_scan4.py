#!/usr/bin/env python3
"""Fourth-pass: persist[10] flow, +0xB0 readers, 5DC4C strings, EXE strings."""

from __future__ import annotations

import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from tools.research import pe_pst0_scan as pst0
from tools.research import pe_txt0_decode as txt0
from local.pe_btl0_boss_identity_scan import (
    DISC,
    disasm_range,
    word_at,
    cmd_fmt,
    va2off,
    find_func_end,
    scan_jals,
)

LOAD = 0x80010000
LETTER_BASE = 0x31
PUNCT = {0x0F: " ", 0x2B: "!", 0x2E: "'", 0x2F: ".", 0x4A: ","}


def decode_ff_string(data: bytes, start: int, limit: int = 64) -> str:
    out = []
    i = start
    while i < start + limit and i < len(data):
        b = data[i]
        if b == 0xFF or b == 0x00:
            break
        if b in PUNCT:
            out.append(PUNCT[b])
        elif 0x10 <= b <= 0x29 or 0x30 <= b <= 0x49:
            out.append(chr(b + LETTER_BASE))
        elif b == 0xF7:
            out.append("\\n")
        else:
            out.append(f"<{b:02X}>")
        i += 1
    return "".join(out)


def scan_lh_offset(exe: bytes, off: int, lo: int, hi: int) -> list[int]:
    """Find lh/lhu/sh of signed immediate `off` in [lo, hi)."""
    hits = []
    for va in range(lo, hi, 4):
        w = word_at(exe, va)
        op = w >> 26
        if op not in (0x21, 0x25, 0x29):  # lh, lhu, sh
            continue
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm
        if simm == off:
            hits.append(va)
    return hits


def main() -> int:
    exe, pe_img = txt0.load_retail(DISC)
    table = pst0.parse_field_table(exe)
    rec = next(r for r in table if r["index"] == 4)
    pkg = pe_img[rec["start"] * 2048 : rec["end"] * 2048]
    extracted = pst0.extract_script_from_package(pkg, rec["meta"])
    script = pst0.decode_script(extracted[1])

    print("=== persist[10] (0x0A) accesses on m0005i ===")
    for mod in script["modules"]:
        for c in mod["commands"]:
            rows = pst0.classify_access(c)
            for row in rows:
                if row.get("index") == 10:
                    print(f"  mod{mod['index']} {cmd_fmt(c)}  {row}")
            # also raw mode2 arg 10
            for i, (mode, arg) in enumerate(zip(c["modes"], c["args"])):
                if mode == 2 and arg == 10:
                    print(f"  mod{mod['index']} {cmd_fmt(c)}  (arg[{i}] persist[10])")

    print("\n=== persist[18]/[0x12] accesses ===")
    for mod in script["modules"]:
        for c in mod["commands"]:
            for i, (mode, arg) in enumerate(zip(c["modes"], c["args"])):
                if mode == 2 and arg in (0x12, 0x0A, 0x50, 0x54, 0x64):
                    print(f"  mod{mod['index']} persist[{arg}] {cmd_fmt(c)}")

    print("\n=== module0 first-play persist[1]==4 arm: 0x0D and persist ===")
    mod0 = script["modules"][0]
    for c in mod0["commands"]:
        if c["offset"] > 0x1400:
            break
        if c["opcode"] in (0x0D, 0x22, 0x31, 0x1C, 0x0A, 0x09) or 2 in c["modes"][: c["argc"]]:
            print(" ", cmd_fmt(c))

    print("\n=== lh/lhu/sh +0xB0/+0xB2/+0xB4 in EXE ===")
    for off, name in ((176, "+0xB0 tag50"), (178, "+0xB2 tag51"), (180, "+0xB4 tag52")):
        hits = scan_lh_offset(exe, off, 0x8001220C, 0x80091080)
        print(f"  {name} count={len(hits)} pcs={[hex(p) for p in hits[:24]]}")

    print("\n=== lh/sh +0xB0 in battle cluster 0x80029800-0x80031000 ===")
    for off in (176, 178, 180):
        hits = scan_lh_offset(exe, off, 0x80029800, 0x80031000)
        print(f"  {off} {[hex(p) for p in hits]}")

    print("\n=== func_8002F7D8 ===")
    end = find_func_end(exe, 0x8002F7D8, 0x300)
    print(f" size=0x{end-0x8002F7D8:X} jals={[hex(j) for j in scan_jals(exe, 0x8002F7D8, end)]}")
    print("\n".join(disasm_range(exe, 0x8002F7D8, min(end, 0x8002F7D8 + 0x120))))

    print("\n=== opcode 0xD1 uses on m0005i ===")
    for mod in script["modules"]:
        for c in mod["commands"]:
            if c["opcode"] == 0xD1:
                print(" ", cmd_fmt(c))

    print("\n=== EXE encoded strings near 0x80091080..0x80093000 ===")
    off0 = va2off(0x80091080)
    off1 = va2off(0x80093000)
    region = exe[off0:off1]
    i = 0
    while i < len(region) - 4:
        b = region[i]
        if 0x10 <= b <= 0x29:
            j = i
            chars = []
            while j < len(region):
                c = region[j]
                if 0x10 <= c <= 0x29 or 0x30 <= c <= 0x49:
                    chars.append(chr(c + LETTER_BASE))
                elif c in PUNCT:
                    chars.append(PUNCT[c])
                else:
                    break
                j += 1
            if len(chars) >= 4 and (j >= len(region) or region[j] in (0x00, 0xFF)):
                print(f"  0x{0x80091080 + i:08X} {''.join(chars)!r}")
            i = max(i + 1, j)
        else:
            i += 1

    # scan more of data for "Battle" / "Eve" encoded
    print("\n=== EXE/PE search encoded 'Battle' and ' Eve' ===")
    battle = bytes((0x11, 0x30, 0x43, 0x43, 0x3B, 0x34))  # Battle
    eve_sp = bytes((0x14, 0x45, 0x34))  # Eve
    for label, blob, base in (("EXE", exe[0x800:], LOAD), ("PE.IMG first 4MB", pe_img[: 4 * 1024 * 1024], 0)):
        for name, pat in (("Battle", battle), ("Eve", eve_sp)):
            start = 0
            n = 0
            while n < 8:
                pos = blob.find(pat, start)
                if pos < 0:
                    break
                ctx = decode_ff_string(blob, max(0, pos - 8), 32)
                print(f"  {label} {name} +0x{pos:X} va_or_off=0x{base+pos:X} ctx={ctx!r}")
                start = pos + 1
                n += 1

    # dump func_8005DC4C table from a loaded package? It's boot archive at 0x800A8028
    # Locate PE.IMG cycle-A payload: B26 said guest 0x800A8028.
    # Search PE.IMG for a count-prefixed string table that decodes as items.
    print("\n=== try decode 5DC4C-like tables mentioning Eve/Actress ===")
    # search pe_img for encoded Actress / Eve as standalone FF-terminated
    actress = bytes((0x10, 0x32, 0x43, 0x41, 0x34, 0x42, 0x42))  # Actress
    for name, pat in (("Actress", actress), ("EveFF", bytes((0x14, 0x45, 0x34, 0xFF)))):
        start = 0
        n = 0
        while n < 12:
            pos = pe_img.find(pat, start)
            if pos < 0:
                break
            ctx = decode_ff_string(pe_img, pos, 40)
            print(f"  PE.IMG {name} +0x{pos:X} ctx={ctx!r}")
            start = pos + 1
            n += 1

    # 0xED 2601 — any other 2601?
    print("\n=== 0xED and numeric 2601 / 1332-class on m0005i ===")
    for mod in script["modules"]:
        for c in mod["commands"]:
            if c["opcode"] == 0xED or any(a in (2601, 1332, 1333, 1334, 49, 50) for a in c["args"]):
                print(f"  mod{mod['index']} {cmd_fmt(c)}")

    # who opens 45 across a wider scene set (indices 0-30)
    print("\n=== 0x0D id=45 on table indices 0-30 ===")
    for recs in table:
        if recs["index"] > 30:
            break
        p = pe_img[recs["start"] * 2048 : recs["end"] * 2048]
        ex = pst0.extract_script_from_package(p, recs["meta"])
        if not ex:
            continue
        sc = pst0.decode_script(ex[1])
        for mod in sc["modules"]:
            for c in mod["commands"]:
                if c["opcode"] == 0x0D and c["args"] and c["args"][0] == 45:
                    print(f"  {recs['name']} mod{mod['index']}+0x{c['offset']:04X}")

    print("\nDONE4")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
