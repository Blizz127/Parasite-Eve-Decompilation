# func_80077DC4 — MATCHED (`LINK_EXACT`)

VRAM `0x80077DC4`, size `0xA0` (40 words), file `0x685C4` in
`asm/disc1/68530.s`. era flags `-O2 -G0` + **maspsx patch 5**
(`MASPSX_SYMBOL_AT_TEMP=1`, profile `era_o2_g0_symbol_at_temp`).

## Semantics

Sign-folded wrapper around the same mirrored 0x1000-step short-table lookup:
negate if `a0 < 0`, mask to 12 bits, then select the quarter/octant table with
a sign. **On-path fan-in 17** — the highest remaining leaf in the
`route_coverage.py` queue at the time of this match.

## Source

```c
extern short D_8009409C[];
extern short D_8009509C[];
extern short D_8009589C[];

int func_80077DC4(int a0) {
    if (a0 < 0)
        a0 = -a0;
    a0 &= 0xFFF;
    if (a0 < 0x801) {
        if (a0 < 0x401)
            return D_8009589C[0x400 - a0];
        return -D_8009509C[a0];
    }
    if (a0 < 0xC01)
        return -D_8009589C[0xC00 - a0];
    return D_8009409C[a0];
}
```

`src/func_80077DC4.c`.

Note the index expressions are written with the constant **first**
(`0x400 - a0`), matching retail's `lui $v0,0x400` / `subu $v0,$v0,$a0` operand
order; a `0x400 - a0` written as `-(a0 - 0x400)` folds differently.

## Load-bearing profile — the patch-5 lever

Same indexed symbolic `lh`-with-`%at` shape as `func_80077D30`: retail keeps
the `%lo` displacement in the three-word form.

```
default            MISMATCHES=31
patch 5            MISMATCHES=11   (reloc-only at object level)
patch 4            MISMATCHES=15
```

## Commands

```
MASPSX_SYMBOL_AT_TEMP=1 \
  tools/analysis/era_leaf_match.sh src/func_80077DC4.c 0x80077DC4 0xA0 -O2 -G0
MASPSX_SYMBOL_AT_TEMP=1 \
  python3 tools/analysis/era_link_check.py src/func_80077DC4.c 0x80077DC4 0xA0 -O2 -G0
linked .text 160 bytes, target 0xa0, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x685C4, c, func_80077DC4]`. Profile
`era_o2_g0_symbol_at_temp`; `tools/analysis/profile_necessity.py --only
func_80077DC4` reports OK.
