# func_80058E08 — REPORT

VRAM `0x80058E08`, size `0x3C`, era `-O2 -G8`, profile `era_o2_g8_symbol_at_temp`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  MASPSX_SYMBOL_AT_TEMP=1 python3 tools/analysis/era_link_check.py src/func_80058E08.c 0x80058E08 0x3C -O2 -G8
```

```
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Twin of `func_800556E8`: limit `D_8009D044` at `0x2D4($gp)`, table `D_800A1E00`.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80058E08`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
