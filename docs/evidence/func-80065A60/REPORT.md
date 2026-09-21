# func_80065A60 — MATCHED (`LINK_EXACT`)

VRAM `0x80065A60`, size `0x3C` (15 words), file `0x56260` in
`asm/disc1/561C8.s`. era flags `-O2 -G0` (YAML default).

## Semantics

Row byte insert at a per-row offset: stores `a2` at
`row + *(int *)(row + 0xC) + (a1 << 1) + 1` where
`row = D_800B1624 + *(int *)(D_800B1624 + 0x10) + (a0 << 4)`; returns 0.

## Source

```c
extern unsigned char *volatile D_800B1624;
int func_80065A60(unsigned int a0, unsigned int a1, unsigned char a2) {
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;
    q += *(unsigned int *)(p + 0x10);
    q += a0 << 4;
    q += *(unsigned int *)(q + 0xC);
    a1 = (a1 << 1) + (unsigned int)q;
    *(unsigned char *)(a1 + 1) = a2;
    return 0;
}
```

`src/func_80065A60.c`.

## Lever — fold the byte index into the base register

Retail computes the final address as `a1 = (a1 << 1) + row` in `$a1` and
stores `sb $a2, 0x1($a1)` **in the `jr` delay slot**. A natural
`*(q + (a1 << 1) + 1)` keeps the sum in `$v0` and stores `sb $a2,1($v0)`
16 bytes earlier, pushing the epilogue past ROM size. Writing the
index-plus-base sum back into the argument (`a1 = (a1 << 1) + (unsigned
int)q`) makes cc1 place the sum in `$a1` and leave the `+1` displacement
on the store — byte-identical, 15 words.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80065A60.c 0x80065A60 0x3C -O2 -G0
ROM  .text 60 bytes  C .text 64 bytes  target 60
SIZE_MISMATCH C=0x40 ROM=0x3c   (4 bytes trailing gas zero pad)
MISMATCHES=4  (two `lui %hi` + two `lw %lo` placeholders)
python3 tools/analysis/era_link_check.py src/func_80065A60.c 0x80065A60 0x3C -O2 -G0
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x56260, c, func_80065A60]` (asm resumes at
`0x5629C`, immediately before the previously-carved `func_80065A9C`).
