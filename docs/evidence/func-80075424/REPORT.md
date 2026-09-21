# func_80075424 — matching C leaf

VRAM `0x80075424`, size `0xC0` (48 words), era `-O2 -G0` plus maspsx patch 3
(`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`). Source: `src/func_80075424.c`.
YAML carve `0x65C24` in `configs/USA/disc1.yaml`.

## Semantics

Build a display record from `a0`:

- gate/log `(&D_8001193C, a0)` when `D_8009574E >= 2`;
- `func_80075EE0(a0 + 0x1C, a0)` (body init);
- **force the low 24 bits** of the word at `a0 + 0x1C` to 1:
  `((int *)a0)[7] |= 0xFFFFFF;`
- push `D_80095744->f(0x8)(D_80095744->v[6], a0 + 0x1C, 0x40, 0)` — the
  5-word struct-pointer form (`v1 + 2` slot, `v1[6]` argument);
- copy the 0x5C-byte `D_800957B8` template over the record via
  `func_80071A34(&D_800957B8, a0, 0x14)` and return `a0`.

Writing the OR through the base pointer (`((int *)a0)[7] |= 0xFFFFFF`)
rather than through the local `s0` is what keeps the `lui $a0,0xFF`
mask constant in `$a0` and the load/store displacement on `s1` (the
`s0`-based form schedules the mask into `$v1` and splits the load/store).

## Commands

```
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 tools/analysis/era_leaf_match.sh \
    src/func_80075424.c 0x80075424 0xC0 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 python3 tools/analysis/era_link_check.py \
    src/func_80075424.c 0x80075424 0xC0 -O2 -G0
```

## Object accounting

`MISMATCHES=10` (size already `0xc0`), all HI16/LO16/J26 relocation fields:

```
0x8007542c  3c128009  lui   s2,%hi(D_8009574E)
0x80075430  2652574e  addiu s2,s2,%lo(D_8009574E)
0x80075454  3c048001  lui   a0,%hi(D_8001193C)
0x80075458  2484193c  addiu a0,a0,%lo(D_8001193C)
0x8007545c  3c028009  lui   v0,%hi(D_80095748)
0x80075460  8c425748  lw    v0,%lo(D_80095748)(v0)
0x80075478  0c01d7b8  jal   func_80075EE0
0x80075494  3c038009  lui   v1,%hi(D_80095744)
0x80075498  8c635744  lw    v1,%lo(D_80095744)(v1)
0x800754c0  0c01c68d  jal   func_80071A34
```

No trailing pad (object is exactly `0xc0`).

## Link-level

`linked .text 192 bytes, target 0xc0, word mismatches=0, nonzero_pad=0` →
`LINK_EXACT`.
