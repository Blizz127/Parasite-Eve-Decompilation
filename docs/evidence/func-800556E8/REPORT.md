# func_800556E8 — REPORT

VRAM `0x800556E8`, size `0x3C`, era `-O2 -G8`, profile `era_o2_g8_symbol_at_temp`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  MASPSX_SYMBOL_AT_TEMP=1 python3 tools/analysis/era_link_check.py src/func_800556E8.c 0x800556E8 0x3C -O2 -G8
```

```
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Signed range-gated short-table getter: reject `a0 < 0` (the raw `bltz`) and `a0 >= D_8009D040` (gp-relative at `0x2D0($gp)`), else `D_800A1D9C[a0]`. The two-guard early-return shape emits retail's double `addu $v0,$zero,$zero` zero-inits; `-G8` makes the limit gp-relative; patch 5 gives the `$at`-with-`%lo` table read.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x800556E8`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
