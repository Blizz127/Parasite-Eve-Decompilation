# func_8007A434 — REPORT

VRAM `0x8007A434`, size `0x34`, era `-O2 -G0`, profile `era_o2_g0 + maspsx patch 4 (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`)`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_8007A434.c 0x8007A434 0x34 -O2 -G0
```

```
linked .text 64 bytes, target 0x34, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Twin of `func_8007A400` with bound `7` and table `D_8009B05C`; same two levers.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x8007A434`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
