# func_8007FBF0 — MATCHED (`LINK_EXACT`)

VRAM `0x8007FBF0`, size `0x18` (6 words), file `0x703F0` in `asm/disc1/6FFC8.s`.
era flags `-O2 -G0` + **maspsx patch 4** (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`,
profile `era_o2_g0_symbol_load_dest_temp`).

## Semantics

Integer getter: returns `D_8009B574[a0]`.

## Source

```c
extern int D_8009B574[];

int func_8007FBF0(int a0) {
    return D_8009B574[a0];
}
```

`src/func_8007FBF0.c`.

## Lever — patch 4 dest-register temp

Retail uses the **destination register** as the address temp, keeping `%lo`
on the load (`lui $v0,%hi`. / `addu $v0,$v0,$a0` / `lw $v0,%lo($v0)`); the
default and patch-5 `$at` paths both mismatch. Verified: default
`word mismatches=5`, patch 5 (`MASPSX_SYMBOL_AT_TEMP=1`) `=3`, patch 4
(`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`) `=0`.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_8007FBF0.c 0x8007FBF0 0x18 -O2 -G0
ROM  .text 24 bytes  C .text 32 bytes  target 24
SIZE_MISMATCH C=0x20 ROM=0x18   (8 bytes trailing gas zero pad)
MISMATCHES=5  (HI16/LO16 placeholders + the two address words)
MASPSX_SYMBOL_LOAD_DEST_TEMP=1 python3 tools/analysis/era_link_check.py \
  src/func_8007FBF0.c 0x8007FBF0 0x18 -O2 -G0
linked .text 32 bytes, target 0x18, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x703F0, c, func_8007FBF0]` (followed by the
already-matched `func_8007FC08` at `0x70408`). Registered in
`configs/USA/disc1_build_profiles.json` under
`era_o2_g0_symbol_load_dest_temp`.
