# func_8009071C — REPORT

VRAM `0x8009071C`, size `0x38`, era `-O2 -G0`, profile `era_o2_g0`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_8009071C.c 0x8009071C 0x38 -O2 -G0
```

```
linked .text 64 bytes, target 0x38, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

Ring-buffer push: `idx = (idx + 1) & 3` at `a0+0xCE`, store `a0[0]` at `a0 + 4 + (idx<<2)`, re-read the index and zero the halfword at `a0 + 0x62 + (idx2<<1)`. The second address needs a **named `unsigned int t` local** (`t = (h2 << 1) + (unsigned int)a0`) — an in-line `a0 + (h2<<1) + 0x62` puts the operands in the other order.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x8009071C`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
