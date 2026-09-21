# func_80076BE0 — REPORT

VRAM `0x80076BE0`, size `0x30`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80076BE0.c 0x80076BE0 0x30 -O2 -G0
```

```
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

`*D_80095854 = a0 | 0x10000000; return *D_80095850 & 0xFFFFFF;`. Both globals are pointer globals; `0x10000000` is a lone `lui` and `0xFFFFFF` a `lui`/`ori` pair, exactly as retail.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80076BE0`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
