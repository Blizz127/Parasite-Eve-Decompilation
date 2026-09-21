# func_80076B44 — MATCHED (`LINK_EXACT`)

VRAM `0x80076B44`, size `0x14` (5 words), era flags `-O2 -G0` + **maspsx
patch 4** (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`).

## Semantics

Byte-table reader: returns `D_800A3348[a0]`.

## Source

```c
extern unsigned char D_800A3348[];
unsigned char func_80076B44(int a0) {
    return D_800A3348[a0];
}
```

`src/func_80076B44.c`.

## Lever — new maspsx patch 4 (`symbol_load_dest_temp`)

Retail (5 words):

```
lui   $v0,%hi(D_800A3348)
addu  $v0,$v0,$a0
lbu   $v0,%lo(D_800A3348)($v0)
jr    $ra
nop
```

cc1 emits the bare `lbu $2,D_800A3348($4)` pseudo. Every prior maspsx path
expands it with **`$at`** as the address temp (either the 4-word
`lui/addiu/addu/op-0` legacy form, or the patch-2 3-word `$at` form). Retail
here uses the **destination register** as the temp *and keeps the `%lo`
displacement on the load* — the naive GNU-as expansion. Neither existing
knob produces it (verified: `MISMATCHES=5` legacy, `3` with
`MASPSX_THREE_WORD_SYMBOL_STORE`, and the two still carry the wrong opcode
at word 2).

Patch 4 (tracked, `tools/era/maspsx/maspsx/__init__.py`, durable test
`tools/era/maspsx/tests/test_symbol_load_dest_temp.py`, registered in
`scripts/setup_era.sh` `MASPSX_TRACKED` + `.gitignore` negation) emits:

```
lui   $d,%hi(SYM)
addu  $d,$d,$b
op    $d,%lo(SYM)($d)
```

Default OFF; opt-in per leaf via the profile
`era_o2_g0_symbol_load_dest_temp`. ROM survey: 69 dest-temp sites in disc1
text (54 `lw`, 7 `lbu`, 5 `lh`, 2 `lhu`, 1 `lb`); the shape is reusable.

## Commands

```
MASPSX_SYMBOL_LOAD_DEST_TEMP=1 \
  tools/analysis/era_leaf_match.sh src/func_80076B44.c 0x80076B44 0x14 -O2 -G0
ROM  .text 20 bytes  C .text 32 bytes  target 20
SIZE_MISMATCH C=0x20 ROM=0x14     (12 bytes trailing gas zero pad)
MISMATCHES=2 (both relocation placeholders: lui %hi, %lo)
python3 tools/analysis/era_link_check.py src/func_80076B44.c 0x80076B44 0x14 -O2 -G0
linked .text 32 bytes, target 0x14, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x67344, c, func_80076B44]` (asm resumes at
`0x67358`). Profile `era_o2_g0_symbol_load_dest_temp`.
