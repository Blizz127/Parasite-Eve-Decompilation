# func_80076B58 — REPORT

VRAM `0x80076B58`, size `0x40`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80076B58.c 0x80076B58 0x40 -O2 -G0
```

```
linked .text 64 bytes, target 0x40, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

`*D_80095854 = 0x4000000;` then a `do { *D_80095850 = *a0++; } while (--c != -1);` word copy guarded by `if (a1)`. Writing the induction as `c = a1 - 1` and comparing to `-1` reproduces retail's `addiu $a2,$a1,-1` preheader and `addiu $a1,$zero,-1` loop sentinel.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80076B58`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
