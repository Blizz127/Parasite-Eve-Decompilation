# func_80065B70 — MATCHED (`LINK_EXACT`)

VRAM `0x80065B70`, size `0xC8` (50 words), file `0x56370` in
`asm/disc1/561C8.s`. era flags `-O2 -G0` (YAML default).

## Semantics

Initialises the `0x800BCF88` BGM/SE control block: sets `D_800BCF88 = 0x70`,
`D_800BCFFC = 0x60`, `D_800BCFFE = 0x180`, three 0xFF bytes, stores the two
arguments into `D_800BCFA4`/`D_800BCFA8`, and zeroes the remaining sixteen
slots. Returns 0.

## Source

```c
extern int D_800BCF88;
extern char D_800BCFFC;
extern short D_800BCFFE;
extern char D_800BD027, D_800BD026, D_800BD025;
extern int D_800BCF8C, D_800BCF90, D_800BCF94, D_800BCF98, D_800BCF9C, D_800BCFA0;
extern int D_800BCFA4, D_800BCFA8, D_800BCFAC, D_800BCFB0, D_800BCFB4;
extern char D_800BCFFD;
extern short D_800BD022, D_800BD020;
extern char D_800BD024;
extern int D_800BD028;
int func_80065B70(int a0, int a1) {
    D_800BCF88 = 0x70;
    D_800BCFFC = 0x60;
    D_800BCFFE = 0x180;
    D_800BD027 = 0xFF;
    D_800BD026 = 0xFF;
    D_800BD025 = 0xFF;
    D_800BCF8C = 0;
    D_800BCF90 = 0;
    D_800BCF94 = 0;
    D_800BCF98 = 0;
    D_800BCF9C = 0;
    D_800BCFA0 = 0;
    D_800BCFA4 = a0;
    D_800BCFA8 = a1;
    D_800BCFAC = 0;
    D_800BCFB0 = 0;
    D_800BCFB4 = 0;
    D_800BCFFD = 0;
    D_800BD022 = 0;
    D_800BD020 = 0;
    D_800BD024 = 0;
    D_800BD028 = 0;
    return 0;
}
```

`src/func_80065B70.c`.

## Lever — a tail-call target carries no prologue

Retail's final `func_80065B44` is a bare
`li $t2,0x70 / jr $t2 / move $a0,...` jump into this entry, and the block
has **no frame and no `$ra` save at all**. cc1 reproduces that only for a
function it can prove has no returning callers in the TU (leaf + `return`
in the last statement); the resulting MIPS is the straight-line
`li $v0,N / lui $at / sw|sb|sh %lo($at)` sequence retail carries, every
store absolute via `$at`. Interleaved word/short/char declarations are
load-bearing because the store width drives the `sw`/`sh`/`sb` opcode and
the `lui $at` reuse pattern.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80065B70.c 0x80065B70 0xC8 -O2 -G0
ROM  .text 200 bytes  C .text 208 bytes  target 200
SIZE_MISMATCH C=0xd0 ROM=0xc8   (8 bytes trailing gas zero pad)
MISMATCHES=44 first_off=4  (all absolute %hi/%lo reloc placeholders)
python3 tools/analysis/era_link_check.py src/func_80065B70.c 0x80065B70 0xC8 -O2 -G0
linked .text 208 bytes, target 0xc8, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x56370, c, func_80065B70]` (asm resumes at
`0x56438`, the start of `func_80065B70`'s neighbour `func_80065C38`).
