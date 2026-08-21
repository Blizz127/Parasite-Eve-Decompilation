#!/usr/bin/env python3
"""Follow-up PE-BTL0-BOSS-ID research. Not production."""

from __future__ import annotations

import json
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
    find_func_end,
    scan_jals,
    scan_abs_stores,
    word_at,
    va2off,
    cmd_fmt,
)

LOAD = 0x80010000


def extract_record_at(blob: bytes, pos: int) -> dict | None:
    if pos + 3 > len(blob):
        return None
    if blob[pos] not in (0xFF, 0xF9) or blob[pos + 1] != 0xFE:
        return None
    mid = blob[pos + 2]
    i = pos + 3
    while i < len(blob):
        b = blob[i]
        if b == 0xFF:
            i += 1
            break
        if b == 0xF9:
            break
        if b == 0xFB:
            i += 1
            if i < len(blob):
                sub = blob[i]
                i += 1
                if sub == 7 and i < len(blob):
                    i += 1
            continue
        if b in (0xFC, 0xFD) and i + 1 < len(blob):
            i += 2
            continue
        i += 1
    rec = blob[pos:i]
    return {"id": mid, "off": pos, "raw": rec, "body": rec[3:], "marker": rec[:3]}


def all_markers_linear(blob: bytes) -> list[dict]:
    rows = []
    i = 0
    while i + 3 <= len(blob):
        if blob[i] in (0xFF, 0xF9) and blob[i + 1] == 0xFE:
            rec = extract_record_at(blob, i)
            if rec:
                rows.append(rec)
                i += max(3, len(rec["raw"]))
                continue
        i += 1
    return rows


def main() -> int:
    out = Path("local/btl0_boss")
    exe, pe_img = txt0.load_retail(DISC)

    table = pst0.parse_field_table(exe)
    rec = next(r for r in table if r["index"] == 4)
    pkg = pe_img[rec["start"] * 2048 : rec["end"] * 2048]
    streams = txt0.slot7_streams(pkg, rec["meta"])
    usa = streams[1]
    blob = usa["bytes"]

    # decode specific IDs used on m0005i
    m0005i_ids = [2, 3, 7, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 59, 60, 132]
    print("=== requested stream1 IDs ===")
    for mid in m0005i_ids:
        recd = txt0.extract_record(blob, mid)
        if recd is None:
            print(f"  id {mid} (0x{mid:02X}) NOT_FOUND")
            continue
        tokens = txt0.decode_tokens(recd["body"], exe)
        text = txt0.tokens_to_text(tokens)
        print(f"  id {mid:3d} 0x{mid:02X} off=+0x{recd['marker_offset']:X} marker={recd['marker'].hex()} text={text!r}")

    print("\n=== linear markers around 0x2C..0x40 ===")
    linear = all_markers_linear(blob)
    for r in linear:
        if 0x2A <= r["id"] <= 0x40 or r["id"] in (2, 3, 46, 50, 132):
            tokens = txt0.decode_tokens(r["body"], exe)
            print(f"  off=+0x{r['off']:X} id={r['id']} 0x{r['id']:02X} {txt0.tokens_to_text(tokens)!r}")

    # short name-like records (body < 24 bytes, mostly letters)
    print("\n=== short stream1 records (possible names) ===")
    for r in linear:
        tokens = txt0.decode_tokens(r["body"], exe)
        text = txt0.tokens_to_text(tokens)
        letters = [t for t in tokens if t["kind"] == "LETTER"]
        if 1 <= len(letters) <= 16 and len(r["body"]) <= 24:
            print(f"  id={r['id']:3d} 0x{r['id']:02X} off=+0x{r['off']:X} len={len(r['body'])} text={text!r}")

    # script: uses of local[24] and 49/50 after the roll
    extracted = pst0.extract_script_from_package(pkg, rec["meta"])
    rel, sblob = extracted
    script = pst0.decode_script(sblob)
    print("\n=== module2 commands that mention arg 24 or values 49/50 ===")
    mod2 = script["modules"][2]
    for c in mod2["commands"]:
        if 24 in c["args"] or 49 in c["args"] or 50 in c["args"]:
            print(" ", cmd_fmt(c))

    print("\n=== module6 0x89 neighborhoods ===")
    mod6 = script["modules"][6]
    sites = [c["offset"] for c in mod6["commands"] if c["opcode"] == 0x89]
    print("  0x89 sites", [hex(s) for s in sites])
    cmds = mod6["commands"]
    for site in sites:
        print(f"\n  ---- around +0x{site:04X} ----")
        # 25 cmds before and 40 after
        idx = next(i for i, c in enumerate(cmds) if c["offset"] == site)
        for c in cmds[max(0, idx - 25) : idx + 40]:
            print("   ", cmd_fmt(c))

    # persist tests near later 0x89
    print("\n=== module6 persist (mode2) accesses ===")
    for c in mod6["commands"]:
        if 2 in c["modes"][: c["argc"]] or c["opcode"] in (0x89, 0x94, 0x6F, 0x70, 0xB7, 0x5A, 0x0D, 0x1A):
            if c["opcode"] in (0x09, 0x0A, 0x89, 0x94, 0x6F, 0x70, 0xB7, 0x5A, 0x0D, 0x1A, 0x95, 0x96):
                print(" ", cmd_fmt(c))

    # disassemble func_8002FF78
    print("\n=== func_8002FF78 ===")
    start = 0x8002FF78
    end = find_func_end(exe, start, 0x800)
    print(f"  end=0x{end:08X} size=0x{end-start:X}")
    lines = disasm_range(exe, start, min(end, start + 0x400))
    (out / "dis_func_8002FF78.txt").write_text("\n".join(lines) + "\n")
    print("  jals", [hex(j) for j in scan_jals(exe, start, end)])
    print("  abs", [f"{s['kind']}@{s['addr']:08X}" for s in scan_abs_stores(exe, start, end)[:20]])

    # also 30220
    start2 = 0x80030220
    end2 = find_func_end(exe, start2, 0x800)
    print(f"\n=== func_80030220 end=0x{end2:08X} size=0x{end2-start2:X} ===")
    lines2 = disasm_range(exe, start2, min(end2, start2 + 0x400))
    (out / "dis_func_80030220.txt").write_text("\n".join(lines2) + "\n")
    print("  jals", [hex(j) for j in scan_jals(exe, start2, end2)])

    # 0x80019DA0 context (other 375E0 caller)
    print("\n=== around 0x80019DA0 (375E0 caller) ===")
    # walk backwards to function start: look for typical prologue
    fn = 0x80019DA0
    for va in range(0x80019DA0, 0x80019000, -4):
        w = word_at(exe, va)
        # addiu sp, sp, -N
        if (w >> 16) == 0x27BD and (w & 0x8000):
            fn = va
            break
    print(f"  guessed fn start 0x{fn:08X}")
    print("\n".join(disasm_range(exe, 0x80019D60, 0x80019E20)))

    # 0x80069F48
    print("\n=== around 0x80069F48 (37870 caller) ===")
    print("\n".join(disasm_range(exe, 0x80069DE0, 0x80069F80)))

    # func_8005DC4C prologue + first compares
    print("\n=== func_8005DC4C prologue ===")
    print("\n".join(disasm_range(exe, 0x8005DC4C, 0x8005DD40)))

    # search EXE data for encoded short words using letter map
    # e.g. look for sequences of letter codes that decode to TitleCase names
    print("\n=== EXE rodata letter-code runs (len 4..16, start uppercase) ===")
    text = exe[0x800:]
    # scan BSS-less rodata-ish: 0x80010000..0x80091080 is mixed; data after ~0x80091080
    # better: scan whole exe for 0x10-0x29 start then 0x30-0x49
    found = []
    i = 0
    while i < len(text) - 4:
        b = text[i]
        if 0x10 <= b <= 0x29:
            j = i + 1
            while j < len(text) and (0x10 <= text[j] <= 0x29 or 0x30 <= text[j] <= 0x49 or text[j] in (0x0F, 0x2B, 0x2E, 0x2F, 0x4A)):
                j += 1
            n = j - i
            if 4 <= n <= 16 and (j == len(text) or text[j] in (0x00, 0xFF)):
                raw = text[i:j]
                dec = []
                ok = True
                for ch in raw:
                    if 0x10 <= ch <= 0x29 or 0x30 <= ch <= 0x49:
                        dec.append(chr(ch + 0x31))
                    elif ch == 0x0F:
                        dec.append(" ")
                    else:
                        ok = False
                        break
                if ok:
                    s = "".join(dec)
                    if s[0].isupper() and s[1:].islower():
                        found.append((LOAD + i, s, raw.hex()))
            i = j
        else:
            i += 1
    print(f"  candidates={len(found)}")
    for va, s, hx in found[:80]:
        print(f"  0x{va:08X} {s!r} {hx}")

    print("\nDONE2")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
