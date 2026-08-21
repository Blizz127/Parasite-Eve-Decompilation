#!/usr/bin/env python3
"""Fifth-pass: EXE battle-string bank, +0xB2 reader, 0x6F current-actor."""

from __future__ import annotations

import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from tools.research import pe_txt0_decode as txt0
from local.pe_btl0_boss_identity_scan import DISC, disasm_range, word_at, va2off, find_func_end, scan_jals

LOAD = 0x80010000
LETTER_BASE = 0x31
PUNCT = {0x0F: " ", 0x2B: "!", 0x2E: "'", 0x2F: ".", 0x4A: ",", 0x2D: "-"}


def decode_run(data: bytes) -> str:
    out = []
    for b in data:
        if b in PUNCT:
            out.append(PUNCT[b])
        elif 0x10 <= b <= 0x29 or 0x30 <= b <= 0x49:
            out.append(chr(b + LETTER_BASE))
        elif b == 0xF7:
            out.append("\\n")
        else:
            out.append(f"<{b:02X}>")
    return "".join(out)


def main() -> int:
    exe, pe_img = txt0.load_retail(DISC)

    print("=== EXE 0x80091080..0x80092000 as encoded strings ===")
    base = 0x80091080
    region = exe[va2off(base) : va2off(0x80092000)]
    i = 0
    while i < len(region):
        b = region[i]
        if b == 0:
            i += 1
            continue
        j = i
        while j < len(region) and region[j] != 0:
            j += 1
        blob = region[i:j]
        if blob and all(
            (0x10 <= c <= 0x29) or (0x30 <= c <= 0x49) or c in PUNCT or c in (0xF7, 0x01, 0x02, 0x2C, 0x2D, 0x4B)
            for c in blob
        ):
            print(f"  0x{base+i:08X} len={len(blob):2d} {decode_run(blob)!r}")
        i = j + 1 if j < len(region) else j

    print("\n=== around 0x8002FC14 (lh +0xB2 in battle cluster) ===")
    print("\n".join(disasm_range(exe, 0x8002FBE0, 0x8002FC80)))

    print("\n=== around 0x8003058C (lh +0xB4) ===")
    print("\n".join(disasm_range(exe, 0x80030550, 0x800305E0)))

    print("\n=== around 0x8001A8E0 (lh +0xB0/+0xB4) ===")
    print("\n".join(disasm_range(exe, 0x8001A8C0, 0x8001A940)))

    print("\n=== around 0x8001B000 ===")
    print("\n".join(disasm_range(exe, 0x8001AFE0, 0x8001B050)))

    print("\n=== 0x6F handler 0x80018954 ===")
    end = find_func_end(exe, 0x80018954, 0x80)
    print("\n".join(disasm_range(exe, 0x80018954, end)))
    print(" jals", [hex(j) for j in scan_jals(exe, 0x80018954, end)])

    # mode-1 binder
    print("\n=== mode-1 binder 0x8001718C ===")
    print("\n".join(disasm_range(exe, 0x8001718C, 0x800171BC)))

    print("\n=== pointers near 0x80091490 (string table?) ===")
    for va in range(0x80091400, 0x80091690, 4):
        w = word_at(exe, va)
        if 0x80091000 <= w < 0x80092000:
            print(f"  0x{va:08X} -> 0x{w:08X}")

    print("\nDONE5")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
