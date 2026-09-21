# func_80077D30 — MATCHED (`LINK_EXACT`)

VRAM `0x80077D30`, size `0x90` (36 words), file `0x68530` in
`asm/disc1/68530.s`. era flags `-O2 -G0` + **maspsx patch 5**
(`MASPSX_SYMBOL_AT_TEMP=1`, profile `era_o2_g0_symbol_at_temp`).

## Semantics

Mirrored 0x1000-step short-table lookup: the step is folded into `[0,0x1000]`
and the quarter/octant quadrant selects one of the four `D_80095xxx` tables
with a sign. Callee of `func_80077CF4` / `func_80077DC4`; on-path fan-in 3.

## Source

```c
extern short D_8009489C[];
extern short D_8009589C[];

int func_80077D30(int a0) {
    if (a0 < 0x801) {
        if (a0 < 0x401)
            return D_8009589C[a0];
        return D_8009589C[0x800 - a0];
    }
    if (a0 < 0xC01)
        return -D_8009489C[a0];
    return -D_8009589C[0x1000 - a0];
}
```

`src/func_80077D30.c`.

## Load-bearing profile — the patch-5 lever

Every branch is an indexed symbolic `lh` over one of four tables. GNU as's
legacy expansion for `lh $v0,SYM($at)` emits the four-word absolute form
(`lui $at` / `ori` / `addu` / `lh 0x0($at)`), which drops the `%lo`
displacement and does not match retail's three-word

```
lui   $at,%hi(D_8009589C)
addu  $at,$at,$v0
lh    $v0,%lo(D_8009589C)($at)
```

`MASPSX_SYMBOL_AT_TEMP=1` (patch 5) keeps `$at` as the address temp and
preserves the `%lo` displacement.

```
default            MISMATCHES=33
patch 5            MISMATCHES=11   (reloc-only at object level)
patch 4            MISMATCHES=15
```

## Commands

```
MASPSX_SYMBOL_AT_TEMP=1 \
  tools/analysis/era_leaf_match.sh src/func_80077D30.c 0x80077D30 0x90 -O2 -G0
MASPSX_SYMBOL_AT_TEMP=1 \
  python3 tools/analysis/era_link_check.py src/func_80077D30.c 0x80077D30 0x90 -O2 -G0
linked .text 144 bytes, target 0x90, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x68530, c, func_80077D30]`. Profile
`era_o2_g0_symbol_at_temp`; `tools/analysis/profile_necessity.py --only
func_80077D30` reports OK (default does not reproduce it).
