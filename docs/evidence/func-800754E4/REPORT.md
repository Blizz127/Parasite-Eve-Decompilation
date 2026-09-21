# func_800754E4 — matching C leaf

VRAM `0x800754E4`, size `0xD8` (54 words), era `-O2 -G0` plus maspsx patch 3
(`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`). Source: `src/func_800754E4.c`.
YAML carve `0x65CE4` in `configs/USA/disc1.yaml`.

## Semantics

`func_80075424`'s twin, with `a0`'s low 24 bits **spliced** into the record
word instead of OR-ed with 1:

- gate/log `(&D_80011954, (void *)a0, a1)` when `D_8009574E >= 2`;
- `func_80075EE0(a1 + 0x1C, a1)`;
- `s0 = (char *)a1 + 0x1C;`
  `*(int *)s0 = (*(int *)s0 & 0xFF000000) | (a0 & 0xFFFFFF);`
  — the load/store go through `a1` with the `0x1C` displacement
  (`((int *)a1)[7]`), matching retail's `lw v0,0x1C(s1)` /
  `sw v0,0x1C(s1)`;
- push `D_80095744->f(0x8)(D_80095744->v[6], s0, 0x40, 0)`;
- copy the `D_800957B8` 0x14-byte template via
  `func_80071A34(&D_800957B8, a1, 0x14)`. Returns `void` (retail ends with
  the `func_80071A34` call, no `move v0`).

The OR-through-`a1` element form (`((int *)a1)[7] = (((int *)a1)[7] &
0xFF000000) | (a0 & 0xFFFFFF)`) is the lever: the `s0`-based form had cc1
sink the un-masked high-half `lui`/`lw` into the loop-free middle and split
the store.

## Commands

```
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 tools/analysis/era_leaf_match.sh \
    src/func_800754E4.c 0x800754E4 0xD8 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 python3 tools/analysis/era_link_check.py \
    src/func_800754E4.c 0x800754E4 0xD8 -O2 -G0
```

## Object accounting

`SIZE_MISMATCH C=0xe0 ROM=0xd8`, `MISMATCHES=10`, all HI16/LO16/J26
relocation fields:

```
0x800754f4  3c138009  lui   s3,%hi(D_8009574E)
0x800754f8  2673574e  addiu s3,s3,%lo(D_8009574E)
0x8007551c  3c048001  lui   a0,%hi(D_80011954)
0x80075520  24841954  addiu a0,a0,%lo(D_80011954)
0x80075528  3c028009  lui   v0,%hi(D_80095748)
0x8007552c  8c425748  lw    v0,%lo(D_80095748)(v0)
0x80075544  0c01d7b8  jal   func_80075EE0
0x8007556c  3c038009  lui   v1,%hi(D_80095744)
0x80075570  8c635744  lw    v1,%lo(D_80095744)(v1)
0x80075598  0c01c68d  jal   func_80071A34
```

The `0x8` trailing pad is gas alignment and is zero.

## Link-level

`linked .text 224 bytes (0xe0), target 0xd8, word mismatches=0,
nonzero_pad=0` → `LINK_EXACT`.
