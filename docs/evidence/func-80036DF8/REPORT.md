# func_80036DF8 — REPORT

VRAM `0x80036DF8`, size `0x3C`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80036DF8.c 0x80036DF8 0x3C -O2 -G0
```

```
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Five-word state seed with deliberately repeated symbols: `D_800A76A8 = 0; D_800A76A8 = 0x1499700; D_800A76A4 = 0; D_800A76A0 = 0; D_800A76A0 = 1;`. The globals must be `volatile` or cc1 eliminates the dead first store of each pair.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80036DF8`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
