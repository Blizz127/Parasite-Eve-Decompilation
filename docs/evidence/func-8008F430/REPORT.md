# func_8008F430 — REPORT

VRAM `0x8008F430`, size `0x40`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_8008F430.c 0x8008F430 0x40 -O2 -G0
```

```
linked .text 64 bytes, target 0x40, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

16-bit relative cursor jump: advance the cursor by 2 and then by the sign-extended little-endian halfword just read, storing the cursor three times. A named `q = p + 2` local keeps the second `sw` *before* the `hi` load and the third `sw` after the `addu`, exactly as retail.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x8008F430`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
