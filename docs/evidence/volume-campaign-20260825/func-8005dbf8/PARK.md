# `func_8005DBF8` — parked address-register coloring

Outcome: `PARKED-ADDRESS-REGISTER-COLORING` after the two allowed natural-C
phrasing iterations. No YAML/build integration; matching-C count remains 290.

## Function hood

Retail span: file `[0x4E3F8,0x4E410)`, VRAM `0x8005DBF8`, six words. The body
ends in canonical `jr ra` plus `addu v0,v1,v0` in the delay slot. The previous
boundary is `func_8005DBAC`'s `addu v0,a0,v0` delay-slot instruction at
`0x4E3F4`; the following boundary is `func_8005DC10` at `0x4E410`.

Eight unique direct callers target the exact start: `0x80043BB0`,
`0x8004B760`, `0x8004B7BC`, `0x8004B99C`, `0x8004B9C0`, `0x8004BBE0`,
`0x8004BBF4`, and `0x8005D9DC`. `FUNCTION_HOOD=PROVEN`.

## Retail semantics and screens

```text
8005DBF8: lui   v0,%hi(D_800A8040)
8005DBFC: addiu v0,v0,%lo(D_800A8040)
8005DC00: lw    v1,0(v0)
8005DC04: addiu v0,v0,-0x18
8005DC08: jr    ra
8005DC0C: addu  v0,v1,v0
```

The function returns the word at `D_800A8040` plus the address of that symbol
minus `0x18`. It has no callees, no stack frame, no loop, and no caller-written
global. The critical pressure is register coloring: retail keeps the symbolic
address in `$v0`, loads the word into `$v1`, then reuses `$v0` for the offset.

## Bounded C attempts

Both attempts used the justified era flags `-O2 -G0`; no pins or inline
assembly were used.

Attempt 1:

```c
extern unsigned int D_800A8040;
unsigned int func_8005DBF8(void) {
    return D_800A8040 + (unsigned int)&D_800A8040 - 0x18;
}
```

The integrated build produced the correct six-word body length but differed in
the address/load register coloring. It loaded through `$v1` and formed the
address in `$v0`; retail does the reverse. Full-build evidence:

```text
RESULT: NON-MATCH (9/2025472 bytes differ, 0.0004%)
first mismatch @ file 0x4E3FA: cand=0x03 orig=0x02
```

Attempt 2, pointer-local phrasing:

```c
extern unsigned int D_800A8040;
unsigned int func_8005DBF8(void) {
    unsigned int *base = &D_800A8040;
    return *base + (unsigned int)base - 0x18;
}
```

This changed the object shape rather than the register coloring. The object
has nonzero text beyond the required `0x18` bytes, so the trim guard correctly
refused to truncate it:

```text
ERROR: build/src/func_8005DBF8.c.o .text: bytes beyond 0x18 are not all zero
```

R6 therefore parks the leaf after two iterations. The candidate source and
carve/build changes are preserved in stash `park volume func_8005DBF8
address-register coloring`; the tree was restored to the 290-leaf base.

## Carve geometry recorded for the rejected integration

The attempted split was from prior asm span `[0x4CC98,0x4E914)`, size `0x1C7C`:

```text
prefix:  0x4E3F8 - 0x4CC98 = 0x1760
C leaf:  0x4E410 - 0x4E3F8 = 0x18
resume:  0x4E914 - 0x4E410 = 0x504
closure: 0x1760 + 0x18 + 0x504 = 0x1C7C
```

Because neither phrasing matched, these boundaries remain unintegrated.
