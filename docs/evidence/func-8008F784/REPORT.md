# func_8008F784 — REPORT

VRAM `0x8008F784`, size `0x38`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_8008F784.c 0x8008F784 0x38 -O2 -G0
```

```
linked .text 64 bytes, target 0x38, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Cursor byte reader: `(c + 0x40) & 0xFF` shifted left 8 into `a0+0x76`, zero `a0+0x78`, set bits 0/1 at `a0+0xF4`. The `(c + 0x40) & 0xFF` intermediate keeps retail's `addiu`/`andi`/`sll` order; the `| 0xF4 |= 3` is a read-modify-write.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x8008F784`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
