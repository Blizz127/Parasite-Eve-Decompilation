# func_80067B40 — REPORT

VRAM `0x80067B40`, size `0x34`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80067B40.c 0x80067B40 0x34 -O2 -G0
```

```
linked .text 64 bytes, target 0x34, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

`D_800BCFFA = a0; D_800BCFFB = 0; D_800BCF88 = (D_800BCF88 & ~0xC00) | 0x400; return 0;`. The two byte stores go through `lui $at` absolute forms while the word read-modify-write stays in one `$v0` base pin; retail materializes `0xC01` as `addiu $a0,$zero,-0xC01`.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80067B40`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
