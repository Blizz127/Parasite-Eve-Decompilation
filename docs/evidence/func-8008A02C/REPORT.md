# func_8008A02C — REPORT

VRAM `0x8008A02C`, size `0x3C`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_8008A02C.c 0x8008A02C 0x3C -O2 -G0
```

```
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

16-word-stride pair incrementer: `d = a1 - *a0; do { *a0 += d; *(a0+1) += d; a0 += 0x10; a2--; } while (a2);` with a parallel `p = a0 + 1` pointer. The pre-loop `subu` of the difference and the `do/while (a2)` back-edge (`bnez`) are both load-bearing.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x8008A02C`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
