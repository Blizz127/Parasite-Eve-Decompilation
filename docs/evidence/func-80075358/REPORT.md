# func_80075358 — matching C leaf

VRAM `0x80075358`, size `0x5C` (23 words), era `-O2 -G0` plus maspsx patch 3
(`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`). Source: `src/func_80075358.c`.
YAML carve `0x65B58` in `configs/USA/disc1.yaml`.

## Semantics

Two display-handler pushes through `D_80095744`:

```
v0 = D_80095744;
s1 = a0[3];
(*f)(v0 + 15)(0);                 /* 0x3C slot, argument 0 */
v0 = D_80095744;
(*f)(v0 + 5)(a0 + 4, s1);         /* 0x14 slot, (body, flag byte) */
```

Retail reloads `D_80095744` between the two calls (the intervening `jalr`
clobbers `$v0`). The function is declared **`void`** — retail's tail is the
plain `lw ra / lw s1 / lw s0 / jr ra / addiu sp,sp,0x20`, with no result
register. The reported object size `0x60` is one word (a `nop`) of gas
16-byte section alignment after the real `0x5C` body; the code words match
exactly.

## Commands

```
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 tools/analysis/era_leaf_match.sh \
    src/func_80075358.c 0x80075358 0x5C -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 python3 tools/analysis/era_link_check.py \
    src/func_80075358.c 0x80075358 0x5C -O2 -G0
```

## Object accounting

`SIZE_MISMATCH C=0x60 ROM=0x5c`, `MISMATCHES=4`, all HI16/LO16 relocation
fields:

```
0x80075364  3c028009  lui  v0,%hi(D_80095744)
0x80075368  8c425744  lw   v0,%lo(D_80095744)(v0)
0x80075384  3c028009  lui  v0,%hi(D_80095744)
0x80075388  8c425744  lw   v0,%lo(D_80095744)(v0)
```

The `0x4` trailing pad is gas alignment and is zero.

## Link-level

`linked .text 96 bytes (0x60), target 0x5c, word mismatches=0,
nonzero_pad=0` → `LINK_EXACT`. The link check trims to the function symbol
and compares the first 0x5C bytes; the trailing one word is zero pad.
