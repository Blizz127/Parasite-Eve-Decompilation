# func_800752AC — matching C leaf

VRAM `0x800752AC`, size `0xAC` (43 words), era `-O2 -G0` plus maspsx patch 3
(`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`). Source: `src/func_800752AC.c`.
YAML carve `0x65AAC` in `configs/USA/disc1.yaml`.

## Semantics

Gate/log wrapper around the `D_80095744->f(0x2C)` handler with the same
display-window seed as `func_800751E4`:

- if `D_8009574E >= 2`, log `(&D_80011910, a0, a1)` through `D_80095748`;
- `v0 = D_80095744; (*f)(v0 + 11)(a0, a1)` — the 0x2C slot reached through
  a **single loaded base** (`lw v0,D_80095744` then `lw v0,0x2C(v0)`);
- seed the window: `*(int*)&D_8009580C = ((unsigned)&D_800957F8 & 0xFFFFFF)
  | 0x4000000;` `*(int *)s0 = (unsigned)&D_8009580C & 0xFFFFFF;`
- return `s0` (a0).

`s0`/`s1` must **not** be pinned here: retail's `move s0,a0` /
`move s1,a1` save pair is plain (no `sw s2` extra frame slot), and the
natural allocator produces exactly `$16`/`$17`.

## Commands

```
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 tools/analysis/era_leaf_match.sh \
    src/func_800752AC.c 0x800752AC 0xAC -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 python3 tools/analysis/era_link_check.py \
    src/func_800752AC.c 0x800752AC 0xAC -O2 -G0
```

## Object accounting

`SIZE_MISMATCH C=0xb0 ROM=0xac`, `MISMATCHES=12`, all HI16/LO16 relocation
fields (literal zero in the relocatable object):

```
0x800752ac  3c028009  lui   v0,%hi(D_8009574E)
0x800752b0  9042574e  lbu   v0,%lo(D_8009574E)(v0)
0x800752d4  3c048001  lui   a0,%hi(D_80011910)
0x800752d8  24841910  addiu a0,%lo(D_80011910)
0x800752e0  3c028009  lui   v0,%hi(D_80095748)
0x800752e4  8c425748  lw    v0,%lo(D_80095748)(v0)
0x800752f4  3c028009  lui   v0,%hi(D_80095744)
0x800752f8  8c425744  lw    v0,%lo(D_80095744)(v0)
0x8007531c  3c058009  lui   a1,%hi(D_8009580C)
0x80075320  24a5580c  addiu a1,%lo(D_8009580C)
0x80075324  3c038009  lui   v1,%hi(D_800957F8)
0x80075328  246357f8  addiu v1,%lo(D_800957F8)
```

The `0x4` trailing pad is gas alignment and is zero.

## Link-level

`linked .text 176 bytes, target 0xac, word mismatches=0, nonzero_pad=0` →
`LINK_EXACT`.
