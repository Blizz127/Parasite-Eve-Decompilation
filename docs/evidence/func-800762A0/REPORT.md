# func_800762A0 — MATCHED (`LINK_EXACT`)

VRAM `0x800762A0`, size `0x1C` (7 words), era flags `-O2 -G0`.
File `0x66AA0` in `asm/disc1/66970.s`.

## Semantics

GP0 `0xE5` draw-mode word packer: `((a1 & 0x7FF) << 11) | ((a0 & 0x7FF) |
0xE5000000)`.

## Source

```c
int func_800762A0(int a0, int a1) {
    int y = (a1 & 0x7FF) << 11;
    int x = (a0 & 0x7FF) | 0xE5000000;
    return y | x;
}
```

`src/func_800762A0.c`.

**Lever:** the evaluation order is load-bearing. Retail emits
`andi $a1` / `sll` / `lui $v1,0xE500` / `or` / `andi $a0` / `or`, i.e. the
`a1` half is fully materialized first and the `a0` base word is assembled
last. Writing the return as a flat `(a1...) | (a0...) | 0xE5000000` instead
forces cc1 to materialize the constant first (`MISMATCHES=6`). Splitting into
the named locals `y` (shifted half) then `x` (masked base) reproduces
retail's order.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_800762A0.c 0x800762A0 0x1C -O2 -G0
ROM  .text 28 bytes  C .text 32 bytes  target 28
SIZE_MISMATCH C=0x20 ROM=0x1c      (4 bytes of trailing gas zero pad)
```

The extra 4 bytes are `.text` alignment padding; the 28 retail words are
byte-identical.

```
python3 tools/analysis/era_link_check.py src/func_800762A0.c 0x800762A0 0x1C -O2 -G0
linked .text 32 bytes, target 0x1c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x66AA0, c, func_800762A0]` (asm resumes at
`0x66ABC`). No build profile override (default `era_o2_g0`).
