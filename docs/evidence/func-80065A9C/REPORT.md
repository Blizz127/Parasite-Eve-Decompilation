# func_80065A9C — REPORT

VRAM `0x80065A9C`, size `0x38`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80065A9C.c 0x80065A9C 0x38 -O2 -G0
```

```
linked .text 64 bytes, target 0x38, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Indexed byte OR: `q = D_800B1624 + D_800B1624[0x10] + (a0<<4); *q = *q | (a1 & 0x30); return 0;`. `D_800B1624` must be `extern unsigned char *volatile` so retail's **two** independent `lui`/`lw` base loads survive (a non-volatile declaration CSEs them into one).

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80065A9C`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
