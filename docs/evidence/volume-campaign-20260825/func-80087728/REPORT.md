# `func_80087728` — exact matching-C hardware halfword setter

Leaf 293, matched on the second bounded natural-C phrasing.

## Function hood and retail screens

Retail span `[0x77F28,0x77F44)`, VRAM `0x80087728`, seven words. It ends in
`jr ra` plus `nop`; the surrounding words are real instructions. Direct
callers include `0x800857C4` and `0x8008993C`. `FUNCTION_HOOD=PROVEN`.

Retail body:

```text
lui at,0x1F80
sh  a0,0x1D8C(at)
srl a0,a0,16
lui at,0x1F80
sh  a0,0x1D8E(at)
jr  ra
nop
```

Screen: no callees, globals, loops, or return-value liveness. This is a
volatile hardware/MMIO setter in the same accepted family as `func_800877BC`.
The decisive codegen pressure is fixed-address materialization and `$at`
store addressing.

## C and bounded attempts

Attempt 1 used direct fixed-address stores:

```c
void func_80087728(unsigned int a0) {
    *(volatile unsigned short *)0x1F801D8C = (unsigned short)a0;
    *(volatile unsigned short *)0x1F801D8E = (unsigned short)(a0 >> 16);
}
```

It emitted two pointer materializations in `$v0/$v1` and an eight-word body;
the trim guard rejected nonzero bytes beyond `0x1C`.

Attempt 2 used one volatile base with halfword indices:

```c
void func_80087728(unsigned int a0) {
    volatile unsigned short *base = (volatile unsigned short *)0x1F800000;
    base[0xEC6] = (unsigned short)a0;
    base[0xEC7] = (unsigned short)(a0 >> 16);
}
```

Flags: retail-era `era_compile ... -O2 -G0`. This phrasing reproduced the
retail `$at` form exactly.

## Packed-span comparison and carve

```text
candidate: 801f013c 8c1d24a4 02240400 801f013c 8e1d24a4 0800e003 00000000
retail:   801f013c 8c1d24a4 02240400 801f013c 8e1d24a4 0800e003 00000000
```

The original `77C28` asm segment closes around the candidate as:

```text
asm prefix: 0x77F28 - 0x77C28 = 0x300
C leaf:     0x77F44 - 0x77F28 = 0x1C
tail through next accepted boundary: 0x77FBC - 0x77F44 = 0x78
closure:    0x300 + 0x1C + 0x78 = 0x394
```

The tail contains the intervening retail functions `87744`, `87760`,
`8777C`, and `87798`; the accepted `func_800877BC` carve begins at the
closing boundary `0x77FBC`, followed by the existing `77FD4` asm resume.

Build candidate SHA-1:
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
`scripts/verify_us.sh` passed; matching-C count is 293.
