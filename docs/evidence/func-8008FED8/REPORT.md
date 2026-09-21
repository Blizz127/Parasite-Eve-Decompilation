# func_8008FED8 — MATCHED (`LINK_EXACT`)

VRAM `0x8008FED8`, size `0x24` (9 words), file `0x806D8` in `asm/disc1/80098.s`.
era flags `-O2 -G0` (YAML default).

## Semantics

Stream rewind: clear bit 0 of the word at `arg0+0x38`, zero the halfword at
`arg0+0xE8`, then set bit `0x10` of the word at `arg0+0xF4`.

## Source

```c
void func_8008FED8(unsigned char *a0) {
    *(unsigned int *)(a0 + 0x38) &= ~1u;
    *(unsigned short *)(a0 + 0xE8) = 0;
    *(unsigned int *)(a0 + 0xF4) |= 0x10u;
}
```

`src/func_8008FED8.c`.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_8008FED8.c 0x8008FED8 0x24 -O2 -G0
ROM  .text 36 bytes  C .text 48 bytes  target 36
SIZE_MISMATCH C=0x30 ROM=0x24   (12 bytes trailing gas zero pad)
BYTE_EXACT (ignoring gas align pad)
python3 tools/analysis/era_link_check.py src/func_8008FED8.c 0x8008FED8 0x24 -O2 -G0
linked .text 48 bytes, target 0x24, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x806D8, c, func_8008FED8]` (asm resumes at
`0x806FC`).
