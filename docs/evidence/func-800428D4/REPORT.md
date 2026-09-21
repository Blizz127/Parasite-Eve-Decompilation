# func_800428D4 — REPORT

VRAM `0x800428D4`, size `0x3C`, era `-O2 -G0`, profile `era_o2_g0_symbol_at_temp`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  MASPSX_SYMBOL_AT_TEMP=1 python3 tools/analysis/era_link_check.py src/func_800428D4.c 0x800428D4 0x3C -O2 -G0
```

```
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Strided byte setter: `*(unsigned char *)((unsigned char *)&D_800A0ED5 + (D_800A1860 - 1) * 0x418) = 0xD;`. Taking `&D_800A0ED5` as a data symbol (not an integer literal) is what gives retail's `lui $at`/`addu`/`sb %lo` sequence; maspsx patch 5 supplies the `$at`-with-`%lo` indexed store.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x800428D4`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
