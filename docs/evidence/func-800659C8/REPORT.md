# func_800659C8 — MATCHED (`LINK_EXACT`)

VRAM `0x800659C8`, size `0x30` (12 words), file `0x561C8` in
`asm/disc1/561C8.s`. era flags `-O2 -G0` (YAML default).

## Semantics

Row halfword insert: stores `a1 >> 8` at
`D_800B1624 + *(int *)(D_800B1624 + 0x10) + (a0 << 4) + 8`; returns 0.

## Source

```c
extern unsigned char *volatile D_800B1624;
int func_800659C8(unsigned int a0, unsigned int a1) {
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;
    a1 >>= 8;
    q += *(unsigned int *)(p + 0x10);
    q += a0 << 4;
    *(unsigned short *)(q + 8) = a1;
    return 0;
}
```

`src/func_800659C8.c`.

## Lever — pointer-to-volatile base defeats the second-load CSE

Retail loads `D_800B1624` twice (two independent `lui`/`lw` pairs). A plain
`extern unsigned char *D_800B1624` lets cc1 CSE the second load
(`MISMATCHES=10`); the same `volatile` lever proven by `func_8006599C`
reduces this to the four link-time HI16/LO16 placeholders. The `>> 8` is
scheduled before the first `addu` because it is a pure value operation
(no memory dependence), matching retail's slot.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_800659C8.c 0x800659C8 0x30 -O2 -G0
ROM  .text 48 bytes  C .text 48 bytes  target 48
MISMATCHES=4  first_off=4  (two `lui %hi` + two `lw %lo` placeholders)
python3 tools/analysis/era_link_check.py src/func_800659C8.c 0x800659C8 0x30 -O2 -G0
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x561C8, c, func_800659C8]` (asm resumes at
`0x561F8`, immediately before the unregistered `func_800659F8`).

`func_800659F8` (same cluster, `0x68`) was attempted and **not** carved: it
carries an unexplained 8-byte frame (`addiu $sp,$sp,-8` +
`addiu $sp,$sp,8`) that no C phrasing reproduced; every build was either
8 bytes short (no frame) or 8 bytes long (16-byte frame) versus the
56-byte body. It is left as asm.
