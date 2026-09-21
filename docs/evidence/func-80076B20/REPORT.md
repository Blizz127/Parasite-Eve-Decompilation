# func_80076B20 — MATCHED (`LINK_EXACT`)

VRAM `0x80076B20`, size `0x24` (9 words), era flags `-O2 -G0` + **maspsx
patch 4** (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`).

## Semantics

Writes `*D_80095854 = a0`, then indexes a byte table by the top byte of the
value: `D_800A3348[a0 >> 24] = a0`.

## Source

```c
extern unsigned int *D_80095854;
extern unsigned char D_800A3348[];
void func_80076B20(unsigned int a0) {
    *D_80095854 = a0;
    D_800A3348[a0 >> 24] = a0;
}
```

`src/func_80076B20.c`.

**Typing note:** `a0` is `unsigned int`, so cc1 emits `srl $v0,$a0,0x18` —
retail's word-exact `0x00041602`. A signed `int` produces the `sra`
`0x00041603` and diverges at the last word of the index sequence.

**Lever — patch 4 store arm.** The indexed store is cc1's bare
`sb $4,D_800A3348($2)` pseudo. Retail materializes the address with `$at`
*before* the return jump and schedules the store itself into the `jr` delay
slot:

```
lui   $at,%hi(D_800A3348)
addu  $at,$at,$v0
jr    $ra
sb    $a0,%lo(D_800A3348)($at)
```

Patch 4's store arm reproduces exactly this when the indexed symbolic store
immediately precedes a bare `j $31`. Before the arm, the object carried the
legacy `$at` sequence plus a `nop` slot and the `jr` — 2 linked word
mismatches. No pre-existing knob covered it (patch 1 does this only for the
*absolute* `sw $r,SYM` macro, not the indexed form; `sw`-only).

## Commands

```
MASPSX_SYMBOL_LOAD_DEST_TEMP=1 \
  tools/analysis/era_leaf_match.sh src/func_80076B20.c 0x80076B20 0x24 -O2 -G0
ROM  .text 36 bytes  C .text 48 bytes  target 36
SIZE_MISMATCH C=0x30 ROM=0x24      (12 bytes trailing gas zero pad)
MISMATCHES=4 (3 relocation placeholders + the jr/nop transposition)
python3 tools/analysis/era_link_check.py src/func_80076B20.c 0x80076B20 0x24 -O2 -G0
linked .text 48 bytes, target 0x24, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x67320, c, func_80076B20]`. Profile
`era_o2_g0_symbol_load_dest_temp`.
