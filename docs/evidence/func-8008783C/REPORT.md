# func_8008783C — MATCHED (`LINK_EXACT`)

VRAM `0x8008783C`, size `0x28` (10 words), file `0x7803C` in `asm/disc1/7800C.s`.
era flags `-O2 -G0` (YAML default).

## Semantics

SPU voice pitch modulator. Reads the unsigned halfword register for voice
`a0` at `0x1F801C08 + (a0 << 4)`, clears the `0xFF0F` mask field and ORs in
`(a1 << 4)`, then stores it back. Returns the register base in `$v0`.

## Source

```c
void func_8008783C(int a0, int a1) {
    unsigned char *p = (unsigned char *)0x1F801C08 + (a0 << 4);
    unsigned short v = *(unsigned short *)p;
    v = (v & 0xFF0F) | (a1 << 4);
    *(unsigned short *)p = v;
}
```

`src/func_8008783C.c`.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_8008783C.c 0x8008783C 0x28 -O2 -G0
ROM  .text 40 bytes  C .text 48 bytes  target 40
SIZE_MISMATCH C=0x30 ROM=0x28   (8 bytes trailing gas zero pad)
BYTE_EXACT (ignoring gas align pad)
python3 tools/analysis/era_link_check.py src/func_8008783C.c 0x8008783C 0x28 -O2 -G0
linked .text 48 bytes, target 0x28, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x7803C, c, func_8008783C]` (asm resumes at
`0x78064`).
