# func_800906E4 — MATCHED (`LINK_EXACT`)

VRAM `0x800906E4`, size `0x38` (14 words), file `0x80EE4` in
`asm/disc1/80EE4.s`. era flags `-O2 -G0` + `MASPSX_SYMBOL_AT_TEMP=1`
(profile `era_o2_g0_symbol_at_temp`).

## Context — carved out of the oversized `func_800906B4` span

This function was previously **swallowed** by the `func_800906B4` `c` span,
declared `0x68` while its C compiles to `0x30`. The deep preflight reported:

```
FAIL [deep-size] func_800906B4: declared span 0x68 (0x80EB4->0x80F1C)
exceeds compiled .text 0x30 by 0x38
```

Retail confirms the real boundary: `func_800906B4` ends with
`jr $ra / sh $v1,0x116($a0)` at `0x800906DC`/`0x800906E0`, and
`func_800906E4` runs `0x38` bytes through `jr $ra` at `0x80090714`.

## Semantics

Clears bit `0x200` at `a0+0x38`; indexes the `D_800B290C` byte table with
`*(unsigned short *)(a0 + 0x5A) << 6`; ORs `0x4400` into `a0+0xF4`; stores the
table byte at `a0+0x116` (the `sh` lands in the `jr` delay slot).

## Source

```c
extern unsigned char D_800B290C[];
void func_800906E4(unsigned char *a0) {
    unsigned int v = *(unsigned int *)(a0 + 0x38);
    unsigned char byte;
    unsigned int f;
    v &= ~0x200u;
    *(unsigned int *)(a0 + 0x38) = v;
    byte = D_800B290C[*(unsigned short *)(a0 + 0x5A) << 6];
    f = *(unsigned int *)(a0 + 0xF4) | 0x4400;
    *(unsigned int *)(a0 + 0xF4) = f;
    *(unsigned short *)(a0 + 0x116) = byte;
}
```

`src/func_800906E4.c`.

## Levers

1. **Patch 5 (`MASPSX_SYMBOL_AT_TEMP=1`) is required.** Retail's indexed
   symbolic load is the 3-word `$at` form with the `%lo` displacement kept
   (`lui $at / addu $at,$at,$v1 / lbu $v1,%lo($at)`). Without it maspsx emits
   the 4-word `lui`/`addiu`/`addu`/`lbu 0($at)` and the object is `0x40`, not
   `0x38` — the same `deep-pad` defect class.
2. **Inline the index shift in the subscript.** Writing
   `D_800B290C[*(unsigned short *)(a0 + 0x5A) << 6]` matches retail's
   `lhu v1,0x5A / sll v1,v1,6`; a named `idx` local reorders the loads
   (`MISMATCHES=10`).
3. **Named result/flag locals** (`v`, `f`) keep the `and`/`ori` in `$v0` and
   the store in the `jr` delay slot (`MISMATCHES` drops to the 2 `%lo`
   placeholders).

## Commands

```
tools/analysis/era_leaf_match.sh src/func_800906E4.c 0x800906E4 0x38 -O2 -G0
ROM  .text 56 bytes  C .text 64 bytes  target 56
SIZE_MISMATCH C=0x40 ROM=0x38
MISMATCHES=2 first_off=28 vram=0x80090700   (4-word fallback form)
MASPSX_SYMBOL_AT_TEMP=1 ...  -> 3-word form, BYTE_EXACT ignoring pad
python3 tools/analysis/era_link_check.py src/func_800906E4.c 0x800906E4 0x38 -O2 -G0
linked .text 64 bytes, target 0x38, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x80EE4, c, func_800906E4]` immediately after the
corrected `- [0x80EB4, c, func_800906B4]`. Profile assignment added to
`configs/USA/disc1_build_profiles.json`
(`era_o2_g0_symbol_at_temp`: `func_800906E4`).
