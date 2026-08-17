#!/usr/bin/env python3
"""Dump func_8003D050 after the +0x10 prefix (0x8003D0D4..0x8003D834)."""
from __future__ import annotations

import hashlib
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from pe_btl5_6c5bc_audit import (  # type: ignore
    SHA1,
    decode,
    fmt,
    jal_target,
    track,
    word_at,
)

FN = 0x8003D050
END = 0x8003D834
AFTER = 0x8003D0D4


def main() -> int:
    exe = (Path(__file__).resolve().parents[1] / "build" / "disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA1
    print(f"=== 3D050 remainder {AFTER:#x}..{END:#x} words={(END-AFTER)//4} ===")
    jals = []
    for va, w, d, snap in track(exe, AFTER, END):
        print(fmt(va, w, snap))
        if d["op"] == 0x03:
            jals.append((va, jal_target(w)))
    print("\n=== jals ===")
    for va, tgt in jals:
        print(f"  {va:#010x} -> {tgt:#010x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
