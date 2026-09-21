# func_80087050 — status-bit wait / test (LINK_EXACT)

- Span: file `0x77850` / VRAM `0x80087050` / size `0x40` (16 words), in
  `asm/disc1/77850.s`.
- Profile: `era_o2_g0` (default).
- Source: `src/func_80087050.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_80087050.c 0x80087050 0x40 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_80087050.c 0x80087050 0x40 -O2 -G0
```

## Result

```
linked .text 64 bytes, target 0x40, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Notes / durable lever

Two load-bearing points:
1. `D_8009D2E0` must be `volatile`. Without it cc1 hoists the second load out of
   the loop and the test reuses a stale value (`bnez v0,.L` with a dead `beqz`).
2. The `a0 == 0` arm must be written **first** so the compiler emits the loop at
   the lower addresses with an explicit `j` past the `a0 != 0` arm to the shared
   epilogue; `if (a0) return …; while(…) {} return 0;` produces the mirror layout.
