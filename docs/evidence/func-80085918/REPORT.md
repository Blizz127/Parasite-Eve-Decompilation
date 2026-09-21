# func_80085918 — REPORT

VRAM `0x80085918`, size `0x34`, era `-O2 -G0`, profile `era_o2_g0 + maspsx patch 4 (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`)`.
Status: **LINK_EXACT**.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_80085918.c 0x80085918 0x34 -O2 -G0
```

```
linked .text 64 bytes, target 0x34, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Why this shape

`D_8009B7CC[1] &= ~D_8009B7D4[a0 & 0xFFFF]; return 1;` — the clear-side twin of `func_800858E8`. Needs maspsx **patch 4** (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`) rather than patch 5: retail keeps the `%lo` on a *dest-register* address temp (`lui $v0,%hi` / `addu $v0,$v0,$a0` / `lw %lo($v0)`) because the load is the last use before the `nor`.

## Registration

- `configs/USA/disc1.yaml`: `c` span carved at `0x80085918`.
- Profile assignment recorded in `configs/USA/disc1_build_profiles.json`.
- `python3 tools/analysis/profile_necessity.py`: this leaf's profile is load-bearing.
