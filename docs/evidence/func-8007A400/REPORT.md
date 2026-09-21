# func_8007A400 — REPORT

VRAM `0x8007A400`, size `0x34`, era `-O2 -G0`, profile `era_o2_g0 + maspsx patch 4 (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`)`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_8007A400.c 0x8007A400 0x34 -O2 -G0
```

```
linked .text 64 bytes, target 0x34, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Bound-checked 32-bit table getter: `if (i >= 0x1C) return (unsigned int)D_800119CC; return D_8009AFDC[i];`. Two levers: **early-return polarity** (the `if (i < N) return table[i]; return fallback;` form gives a `bnez`-into-fallthrough, not retail's `beqz`-to-exit) and patch 5 for the `$at`-with-`%lo` table read.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x8007A400`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
