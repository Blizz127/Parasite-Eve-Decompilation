#!/usr/bin/env python3
"""Third-pass PE-BTL0-BOSS-ID: tag 50/51/52, local[24], later 0x89."""

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
    find_func_end,
    scan_jals,
    va2off,
)

LOAD = 0x80010000
JT = 0x80010C90  # func_80030220 jump table; index = tag-40


def main() -> int:
    exe, pe_img = txt0.load_retail(DISC)

    print("=== jump table 0x80010C90 tag 40..124 ===")
    for tag in range(40, 125):
        idx = tag - 40
        dest = word_at(exe, JT + idx * 4)
        mark = ""
        if tag in (40, 41, 42, 43, 44, 45, 50, 51, 52, 60, 61, 62, 63):
            mark = " <<<"
        print(f"  tag {tag:3d} idx {idx:2d} -> 0x{dest:08X}{mark}")

    print("\n=== decode tag 50/51/52 targets ===")
    for tag in (50, 51, 52, 40, 41, 42, 43, 44, 45, 60, 61):
        dest = word_at(exe, JT + (tag - 40) * 4)
        print(f"\n-- tag {tag} dest 0x{dest:08X} --")
        print("\n".join(disasm_range(exe, dest, dest + 16)))

    table = pst0.parse_field_table(exe)
    rec = next(r for r in table if r["index"] == 4)
    pkg = pe_img[rec["start"] * 2048 : rec["end"] * 2048]
    extracted = pst0.extract_script_from_package(pkg, rec["meta"])
    _, sblob = extracted
    script = pst0.decode_script(sblob)

    print("\n=== all mode-1 (actor local) uses of index 24 across m0005i ===")
    for mod in script["modules"]:
        for c in mod["commands"]:
            for i, (mode, arg) in enumerate(zip(c["modes"], c["args"])):
                if mode == 1 and arg == 24:
                    print(f"  mod{mod['index']} {cmd_fmt(c)}  (arg[{i}] is local[24])")

    print("\n=== module2 after local[24] assign: 0x6F through first park ===")
    mod2 = script["modules"][2]
    started = False
    for c in mod2["commands"]:
        if c["offset"] == 0x1DCC:
            started = True
        if started:
            print(" ", cmd_fmt(c))
            if c["offset"] >= 0x2560:
                break

    print("\n=== later 0x89 full neighborhoods with persist tests ===")
    mod6 = script["modules"][6]
    cmds = mod6["commands"]
    for site in (0x350C, 0x3728, 0x414C):
        idx = next(i for i, c in enumerate(cmds) if c["offset"] == site)
        print(f"\n######## 0x89 +0x{site:04X} ########")
        for c in cmds[max(0, idx - 60) : idx + 25]:
            print(" ", cmd_fmt(c))

    print("\n=== module6 0x6F / 0x5A / 0x70 / 0xB7 / 0x0D / 0x1A ===")
    for c in mod6["commands"]:
        if c["opcode"] in (0x6F, 0x5A, 0x70, 0xB7, 0x0D, 0x1A, 0x89, 0x95, 0x96):
            print(" ", cmd_fmt(c))

    print("\n=== who opens message 45 (Battle VS Eve) on first-play scenes ===")
    scenes = [
        ("m0002i", 1),
        ("m0003i", 2),
        ("m0004i", 3),
        ("m0005i", 4),
        ("m0372i", 371),
        ("m0378i", 377),
        ("m0377i", 376),
        ("m0012i", 11),
        ("m0013i", 12),
        ("m0014i", 13),
        ("m0016i", 15),
    ]
    for name, idx in scenes:
        recs = next((r for r in table if r["index"] == idx), None)
        if recs is None:
            print(f"  {name} missing")
            continue
        p = pe_img[recs["start"] * 2048 : recs["end"] * 2048]
        ex = pst0.extract_script_from_package(p, recs["meta"])
        if not ex:
            print(f"  {name} no script")
            continue
        sc = pst0.decode_script(ex[1])
        hits = []
        for mod in sc["modules"]:
            for c in mod["commands"]:
                if c["opcode"] in (0x0D, 0x22) and c["args"] and c["args"][0] == 45:
                    hits.append(f"mod{mod['index']}+0x{c['offset']:04X} op=0x{c['opcode']:02X}")
        print(f"  {name} 0x0D/22 id=45: {hits or 'none'}")

    # opcode of 0x80019D84
    print("\n=== dispatch table entry for 0x80019D84 ===")
    dispatch = 0x800910A0
    for op in range(0x200):
        h = word_at(exe, dispatch + op * 4)
        if h in (0x80019D84, 0x80019DA0, 0x80034A5C, 0x80034E54, 0x800375E0):
            print(f"  opcode 0x{op:02X} handler=0x{h:08X}")

    # find function containing 0x80034A5C
    print("\n=== around 0x80034A5C and 0x80034E54 ===")
    print("\n".join(disasm_range(exe, 0x80034A30, 0x80034A90)))
    print("---")
    print("\n".join(disasm_range(exe, 0x80034E20, 0x80034E80)))

    # encoded Eve / Actress in EXE
    print("\n=== EXE search encoded Eve / Actress ===")
    # E=0x14 v=0x45 e=0x34 ; A=0x10 c=0x32 t=0x43 r=0x41 e=0x34 s=0x42 s=0x42
    eve = bytes((0x14, 0x45, 0x34))
    actress = bytes((0x10, 0x32, 0x43, 0x41, 0x34, 0x42, 0x42))
    text = exe[0x800:]
    for name, pat in (("Eve", eve), ("Actress", actress)):
        start = 0
        while True:
            pos = text.find(pat, start)
            if pos < 0:
                break
            va = LOAD + pos
            ctx = text[max(0, pos - 4) : pos + 16]
            print(f"  {name} at 0x{va:08X} ctx={ctx.hex()}")
            start = pos + 1

    # func_8002FA10 / 2FAA4
    print("\n=== func_8002FA10 ===")
    end = find_func_end(exe, 0x8002FA10, 0x200)
    print(f" size=0x{end-0x8002FA10:X} jals={[hex(j) for j in scan_jals(exe, 0x8002FA10, end)]}")
    print("\n".join(disasm_range(exe, 0x8002FA10, min(end, 0x8002FA10 + 0xC0))))
    print("\n=== func_8002FAA4 ===")
    end = find_func_end(exe, 0x8002FAA4, 0x200)
    print(f" size=0x{end-0x8002FAA4:X}")
    print("\n".join(disasm_range(exe, 0x8002FAA4, min(end, 0x8002FAA4 + 0x80))))

    # battle cluster readers of slot/actor offsets that tags 50/51/52 write
    print("\n=== dump first 16 jump-table words as sanity ===")
    for i in range(16):
        print(f"  [{i}] 0x{word_at(exe, JT + i*4):08X}")

    print("\nDONE3")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
