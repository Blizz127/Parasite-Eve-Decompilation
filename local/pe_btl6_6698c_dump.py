#!/usr/bin/env python3
"""Dump func_8006698C (215 words, zero callees) and 3D050 live-skip facts."""
from __future__ import annotations

import hashlib
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from pe_btl5_6c5bc_audit import SHA1, decode, find_fn_end, fmt, jal_target, track, word_at

FN = 0x8006698C
END = 0x80066CE8


def main() -> int:
    exe = (Path(__file__).resolve().parents[1] / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    print(f"=== 6698C {FN:#x}..{END:#x} words={(END-FN)//4} ===")
    jals = []
    stores = []
    for va, w, d, snap in track(exe, FN, END):
        print(fmt(va, w, snap))
        if d["op"] == 0x03:
            jals.append((va, jal_target(w)))
    print(f"\njals={jals or 'NONE'}")

    for name, start in (("3D94C", 0x8003D94C), ("794C4", 0x800794C4), ("3C5D8", 0x8003C5D8)):
        end = find_fn_end(exe, start, start + 0x2000)
        print(f"{name} {start:#x}..{end:#x} words={(end-start)//4}")
        j = []
        for va in range(start, end, 4):
            w = word_at(exe, va)
            if w >> 26 == 3:
                j.append(hex(jal_target(w)))
        print(f"  jals={j or 'NONE'} prologue {fmt(start, word_at(exe, start))}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
