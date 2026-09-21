# func_800858E8 — REPORT

VRAM `0x800858E8`, size `0x30`, era `-O2 -G0`, profile `era_o2_g0 + maspsx patch 4 (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`)`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_800858E8.c 0x800858E8 0x30 -O2 -G0
```

```
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

`D_8009B7CC[1] |= D_8009B7D4[a0 & 0xFFFF]; return (a0 & 0xFFFF) < 3;`. Two levers: a **signed** `int i` keeps retail's `slti` (an `unsigned` index gives `sltiu`), and maspsx patch 5 (`MASPSX_SYMBOL_AT_TEMP=1`) yields retail's `$at`-with-`%lo` table read.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x800858E8`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
