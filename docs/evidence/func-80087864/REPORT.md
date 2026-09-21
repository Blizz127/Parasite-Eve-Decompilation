# func_80087864 — REPORT

VRAM `0x80087864`, size `0x28`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80087864.c 0x80087864 0x28 -O2 -G0
```

```
linked .text 48 bytes, target 0x28, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Read-modify-write of the low nibble of SPU voice a0's halfword at `0x1F801C08 + (a0<<4)`: clear the 0xF0F field via `& 0xFFF0`, OR in `a1`, store back. Single expression; the natural `*p = (*p & 0xFFF0) | a1;` gives retail's `lhu`/`andi`/`or`/`sh` with the store in the `jr` delay slot.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80087864`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
