#!/usr/bin/env python3
"""Sixth-pass: 6DCE4, global 0x0D id=45, persist[10] bit0 writers."""

from __future__ import annotations

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
    scan_abs_stores,
)

LOAD = 0x80010000


def main() -> int:
    exe, pe_img = txt0.load_retail(DISC)

    print("=== func_8006DCE4 ===")
    end = find_func_end(exe, 0x8006DCE4, 0x200)
    print(f" size=0x{end-0x8006DCE4:X} jals={[hex(j) for j in scan_jals(exe, 0x8006DCE4, end)]}")
    print(" abs", [f"{s['kind']}@{s['addr']:08X}" for s in scan_abs_stores(exe, 0x8006DCE4, end)[:16]])
    print("\n".join(disasm_range(exe, 0x8006DCE4, min(end, 0x8006DCE4 + 0x80))))

    print("\n=== around 0x80087EB0 ( +0xB0 / +0xB4 ) ===")
    print("\n".join(disasm_range(exe, 0x80087E90, 0x80087F20)))

    print("\n=== 0x0D id=45 across entire field table ===")
    table = pst0.parse_field_table(exe)
    found = 0
    for recs in table:
        p = pe_img[recs["start"] * 2048 : recs["end"] * 2048]
        if not p:
            continue
        ex = pst0.extract_script_from_package(p, recs["meta"])
        if not ex:
            continue
        sc = pst0.decode_script(ex[1])
        for mod in sc.get("modules", []):
            for c in mod["commands"]:
                if c["opcode"] == 0x0D and c["args"] and c["args"][0] == 45:
                    print(f"  {recs['name']} idx={recs['index']} mod{mod['index']}+0x{c['offset']:04X}")
                    found += 1
    print(f"  total={found}")

    print("\n=== 0x0D ids 45-57 on all maps (count) ===")
    counts = {i: [] for i in range(45, 58)}
    counts[132] = []
    for recs in table:
        p = pe_img[recs["start"] * 2048 : recs["end"] * 2048]
        ex = pst0.extract_script_from_package(p, recs["meta"]) if p else None
        if not ex:
            continue
        sc = pst0.decode_script(ex[1])
        for mod in sc.get("modules", []):
            for c in mod["commands"]:
                if c["opcode"] == 0x0D and c["args"] and c["args"][0] in counts:
                    counts[c["args"][0]].append(f"{recs['name']}+m{mod['index']}+0x{c['offset']:04X}")
    for mid, sites in counts.items():
        print(f"  id {mid:3d} count={len(sites)} {sites[:6]}")

    rec = next(r for r in table if r["index"] == 4)
    pkg = pe_img[rec["start"] * 2048 : rec["end"] * 2048]
    script = pst0.decode_script(pst0.extract_script_from_package(pkg, rec["meta"])[1])

    print("\n=== persist[10] WRITES and opcode 0x2A on m0005i ===")
    for mod in script["modules"]:
        for c in mod["commands"]:
            if c["opcode"] == 0x2A or (
                c["opcode"] == 0x0A and c["modes"] and c["modes"][0] == 2 and c["args"] and c["args"][0] == 10
            ):
                print(f"  mod{mod['index']} {cmd_fmt(c)}")

    print("\n=== 0x2A handler ===")
    h = word_at(exe, 0x800910A0 + 0x2A * 4)
    print(f" handler=0x{h:08X}")
    end = find_func_end(exe, h, 0x80)
    print("\n".join(disasm_range(exe, h, end)))

    print("\nDONE6")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
