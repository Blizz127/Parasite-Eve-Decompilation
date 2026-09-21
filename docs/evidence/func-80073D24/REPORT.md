# func_80073D24 — REPORT

VRAM `0x80073D24`, size `0x34`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80073D24.c 0x80073D24 0x34 -O2 -G0
```

```
linked .text 64 bytes, target 0x34, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Tail-dispatch through `D_8009566C->f+0x14` **with arguments** `(4, a0)`. Distinct from the argument-less twins: here the prototype *has* two parameters, so cc1 materializes `$a0 = 4` and `$a1 = a0` in the `jalr` delay-slot window exactly as retail.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80073D24`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
