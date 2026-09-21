# func_8008FCBC — MATCHED (`LINK_EXACT`)

VRAM `0x8008FCBC`, size `0x28` (10 words), file `0x804BC` in
`asm/disc1/804BC.s`. era flags `-O2 -G0` (YAML default).

## Semantics

Twin of `func_8008FBD4`: load the cursor from `*arg0`, store `cursor+1`
back, fetch the byte at the old cursor, sign-extend, and store at `arg0+0xE0`.

## Source

```c
void func_8008FCBC(int a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(short *)(a0 + 0xE0) = (signed char)c;
}
```

`src/func_8008FCBC.c`.

## Semantics note

Same lever as `func_8008FBD4`: the fetched byte must stay an
`unsigned char c` so cc1 emits the retail `sll $v0,24` / `sra $v0,24` pair
(`lb` folding gives `MISMATCHES=4`), and the function is `void`.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_8008FCBC.c 0x8008FCBC 0x28 -O2 -G0
ROM  .text 40 bytes  C .text 48 bytes  target 40
SIZE_MISMATCH C=0x30 ROM=0x28   (8 bytes trailing gas zero pad)
BYTE_EXACT (ignoring gas align pad)
python3 tools/analysis/era_link_check.py src/func_8008FCBC.c 0x8008FCBC 0x28 -O2 -G0
linked .text 48 bytes, target 0x28, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x804BC, c, func_8008FCBC]` (asm resumes at
`0x804E4`).
