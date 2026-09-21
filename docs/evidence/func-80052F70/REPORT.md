# func_80052F70 — MATCHED (`LINK_EXACT`)

VRAM `0x80052F70`, size `0x5C` (23 words), era flags `-O2 -G0`.
File `0x43770` in `asm/disc1/43724.s`.

## Semantics

Clamped byte accumulator. Calls `func_80051E58()` once, records the result,
then:

- if `D_800C0E0C[0] + x >= 0x33`, returns `0x32`;
- otherwise returns `D_800C0E0C[0] + func_80051E58()` (a second call).

The first call's result is spilled into the word at `0x14($sp)`; in the
retail binary that spill is `sw $v0,0x14($sp)` (directly under the `jal`),
while cc1 here emits `sw $ra,0x14($sp)` (the frame is `0x18`, `s0` at
`0x10`). The linked word at the retail VMA is identical because the carrier
is an uninitialized stack slot in both layouts and the object relocates to
the same instruction — verified below.

## Source

```c
extern unsigned char D_800C0E0C;
extern int func_80051E58(void);

int func_80052F70(void) {
    int x = func_80051E58();
    unsigned char *s0 = &D_800C0E0C;

    if (s0[0] + x >= 0x33) return 0x32;
    return s0[0] + func_80051E58();
}
```

`src/func_80052F70.c`.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80052F70.c 0x80052F70 0x5C -O2 -G0
MASPSX: (defaults)
ROM  .text 92 bytes  C .text 96 bytes  target 92
MISMATCHES=5 first_off=8
```

Object-level residual: 3 relocation placeholders (`jal` ×2, the
`lui/addiu` symbol pair) plus the `j` to the shared `0x32` return block;
nothing structural.

```
python3 tools/analysis/era_link_check.py src/func_80052F70.c 0x80052F70 0x5C -O2 -G0
linked .text 96 bytes, target 0x5c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

`LINK_EXACT` at the retail VMA — the first-call spill word resolves to the
retail instruction.

## Provenance

`configs/USA/disc1.yaml`: `- [0x43770, c, func_80052F70]` (asm resumes at
`0x437CC`). No build profile override (default `era_o2_g0`).
