# func_80018718 — REPORT

VRAM `0x80018718`, size `0x3C`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80018718.c 0x80018718 0x3C -O2 -G0
```

```
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

`D_800A76C4` bit-2 gate writing 1/0 through `a0[0]`, always returning 1. The `if/else` (not a ternary) keeps retail's `beqz`-to-else layout with the store in the `j` delay slot.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80018718`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
