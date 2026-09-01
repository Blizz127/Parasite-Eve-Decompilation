# `func_8003708C` — parked fixed-point multiply coloring

Outcome: `PARKED-FIXED-POINT-REGISTER-COLORING` after two bounded natural-C
phrasings. No integration; matching-C count remains 291.

## Function hood

Retail span `[0x2788C,0x278A8)`, VRAM `0x8003708C`, seven words. It ends in
canonical `jr ra` plus `or v0,v1,v0` in the delay slot. The previous boundary
at `0x27888` is the prior function's delay-slot `nop`; the following existing
leaf begins immediately at `0x278A8`. The pool scan found 65 unique direct
callers and real boundaries, so `FUNCTION_HOOD=PROVEN`.

## Retail semantics and screens

```text
mult a0,a1
mflo v0
mfhi v1
srl  v0,v0,16
sll  v1,v1,16
jr   ra
or   v0,v1,v0
```

This is the low 32 bits of the signed 64-bit product shifted right 16 bits,
with no callee, frame, global, or loop. The critical screen is the fixed-point
HI/LO combine and the exact result-register coloring (`mflo`→`v0`, `mfhi`→`v1`).

## Bounded attempts

Flags for both attempts: `era_compile ... -O2 -G0`.

Attempt 1:

```c
int func_8003708C(int a0, int a1) {
    return (int)(((long long)a0 * a1) >> 16);
}
```

The object produced `mult` and the correct combine operations, but allocated
HI/LO temporaries in `$a3/$a2` and added an extra signed-result operation in
the delay slot. The nonzero tail exceeded the 7-word body, so trim refused it.

Attempt 2:

```c
unsigned int func_8003708C(int a0, int a1) {
    return (unsigned int)(((long long)a0 * a1) >> 16);
}
```

The retry produced the same register coloring and nonzero overflow:

```text
ERROR: build/src/func_8003708C.c.o .text: bytes beyond 0x1C are not all zero
```

R6 parks after two iterations. The candidate and carve/build changes are in
stash `park volume func_8003708C fixed-point multiply coloring`; no integration
remains in the tree.

## Carve geometry recorded for the rejected integration

The attempted head carve was from `[0x26C48,0x278BC)`, size `0xC74`:

```text
asm prefix: 0x2788C - 0x26C48 = 0xC44
C leaf:     0x278A8 - 0x2788C = 0x1C
closure:    0xC44 + 0x1C = 0xC60 before the existing 370A8 boundary
```

The pre-existing `func_800370A8` begins at `0x278A8` and remains `0x14` bytes;
the complete attempted segment closure was therefore
`0xC44 + 0x1C + 0x14 = 0xC74`. The candidate's requested span itself is
exactly `[0x2788C,0x278A8)` and the following C leaf was retained unchanged.
