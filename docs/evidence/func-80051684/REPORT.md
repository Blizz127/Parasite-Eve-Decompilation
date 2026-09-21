# func_80051684 — REPORT

VRAM `0x80051684`, size `0x30`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80051684.c 0x80051684 0x30 -O2 -G0
```

```
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Guarded `+0x8` word store through `D_8009D254->0`: the outer pointer is loaded, null-checked, dereferenced to the inner pointer, null-checked again, then `a0 << 16` is stored. The inner pointer must be a *fresh load* (not a reused value) — the double-null guard is load-bearing: `if (p) { p = *(unsigned char **)p; if (p) ... }`.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80051684`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
