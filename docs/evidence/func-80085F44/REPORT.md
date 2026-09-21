# func_80085F44 — MATCHED (`LINK_EXACT`)

VRAM `0x80085F44`, size `0x24` (9 words), file `0x76744` in
`asm/disc1/765E8.s`. era flags `-O2 -G0` (YAML default).

## Semantics

Guarded setter: if `a0` differs from the `D_8009B434` global, store it.

```c
extern int D_8009B434;

void func_80085F44(int a0) {
    if (a0 != D_8009B434) {
        D_8009B434 = a0;
    }
}
```

`src/func_80085F44.c`.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80085F44.c 0x80085F44 0x24 -O2 -G0
ROM  .text 36 bytes  C .text 48 bytes  target 36
SIZE_MISMATCH C=0x30 ROM=0x24   (12 bytes trailing gas zero pad)
MISMATCHES=4  (the `lui`/`lw` and `lui`/`sw` HI16/LO16 placeholders)
python3 tools/analysis/era_link_check.py src/func_80085F44.c 0x80085F44 0x24 -O2 -G0
linked .text 48 bytes, target 0x24, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x76744, c, func_80085F44]` (asm resumes at
`0x76768`). The guard is inverted in the C source to match retail's
`beq a0,v0,skip` polarity.
