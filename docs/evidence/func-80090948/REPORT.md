# func_80090948 — MATCHED (`LINK_EXACT`)

VRAM `0x80090948`, size `0x28` (10 words), file `0x81148` in `asm/disc1/80EE4.s`.
era flags `-O2 -G0` (YAML default).

## Semantics

Stream byte advance: load the cursor from `*arg0`, store `cursor+1` back,
then broadcast the fetched byte into the halfwords at `arg0+0x56`, `arg0+0x58`
and `arg0+0xD0`, and zero the halfword at `arg0+0xD2`.

## Source

```c
void func_80090948(int a0) {
    unsigned int v;
    unsigned char *p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    v = *p;
    *(unsigned short *)(a0 + 0xD2) = 0;
    *(unsigned short *)(a0 + 0x58) = v;
    *(unsigned short *)(a0 + 0x56) = v;
    *(unsigned short *)(a0 + 0xD0) = v;
}
```

`src/func_80090948.c`.

## Semantics note — the `unsigned int` value type is load-bearing

Retail has **no** `andi $v0,$v0,0xFF` after the `lbu`. Typing the fetched
value as `unsigned char` makes cc1 insert a dead zero-extension before the
three `sh` stores and emits `SIZE_MISMATCH C=0x30`/`MISMATCHES=4`. Typing it
`unsigned int` (the `lbu` already zero-extends) reproduces retail exactly.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80090948.c 0x80090948 0x28 -O2 -G0
ROM  .text 40 bytes  C .text 48 bytes  target 40
SIZE_MISMATCH C=0x30 ROM=0x28   (8 bytes trailing gas zero pad)
BYTE_EXACT (ignoring gas align pad)
python3 tools/analysis/era_link_check.py src/func_80090948.c 0x80090948 0x28 -O2 -G0
linked .text 48 bytes, target 0x28, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x81148, c, func_80090948]` (asm resumes at
`0x81170`).
