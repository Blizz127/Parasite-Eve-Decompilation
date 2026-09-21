# func_8008FBD4 — MATCHED (`LINK_EXACT`)

VRAM `0x8008FBD4`, size `0x28` (10 words), file `0x803D4`.
era flags `-O2 -G0` (YAML default).

## Semantics

Stream read into a signed halfword: load the cursor from `*arg0`, store
`cursor+1` back, fetch the byte at the old cursor, sign-extend it, and store
the result at `arg0+0xDE`.

## Source

```c
void func_8008FBD4(int a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(short *)(a0 + 0xDE) = (signed char)c;
}
```

`src/func_8008FBD4.c`.

## Semantics note — keep the intermediate `unsigned char`

Retail reads with `lbu` and then sign-extends with a distinct `sll $v0,24` /
`sra $v0,24` pair instead of one `lb`. Loading straight into a signed
`int` makes cc1 fold the two into `lb` (`MISMATCHES=4`). Keeping the fetched
value in an `unsigned char c` and casting only at the store reproduces the
`sll`/`sra` pair. A non-void function that also `return`s `c` is wrong: the
return value would need `$v0` live across the store, changing the epilogue
(`MISMATCHES=3`), so retail is `void` here.

Twin of `func_8008FCBC` (same shape, store at `+0xE0`).

## Span correction — do NOT widen to the gas pad

An earlier carve paired this leaf with `- [0x8040C, asm]`, taking the span to
`0x38` to absorb the C object's 16-byte gas alignment pad. That was wrong:
retail's next word `0x803FC` begins the **distinct** asm function
`func_8008FBFC` (its body is `lw/nop/addiu/sw`, and it is followed by
`func_8008FC0C` at `0x8008FC0C`). Taking the pad into the span silently
removed `func_8008FBFC` from the build (4 words in the asm `80098.s` zone were
overshot; the `8040C` zone retained the copy, which is why the packed image
still looked byte-identical).

The carve is now the true `0x28`, asm resumes at `0x803FC`, and the C object's
16-byte align pad falls inside the asm unit because it is provably zero.

`tools/analysis/profile_necessity.py` surfaced this: with a `0x38` span,
`-O2 -G0` no longer reproduced the leaf (word `0x8008FBFC` went to zero).

## Commands

```
tools/analysis/era_leaf_match.sh src/func_8008FBD4.c 0x8008FBD4 0x28 -O2 -G0
ROM  .text 40 bytes  C .text 48 bytes  target 40
SIZE_MISMATCH C=0x30 ROM=0x28   (8 bytes trailing gas zero pad, zero)
BYTE_EXACT (ignoring gas align pad)
python3 tools/analysis/era_link_check.py src/func_8008FBD4.c 0x8008FBD4 0x28 -O2 -G0
linked .text 48 bytes, target 0x28, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x803D4, c, func_8008FBD4]`, asm resumes at
`0x803FC` (was `0x8040C`).
