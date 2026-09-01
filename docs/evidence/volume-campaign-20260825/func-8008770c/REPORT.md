# `func_8008770C` — exact matching-C hardware halfword setter

Leaf 294, matched on the first bounded phrasing using the proven one-base
volatile MMIO idiom.

## Function hood and retail screens

Retail span `[0x77F0C,0x77F28)`, VRAM `0x8008770C`, seven words. It ends in
`jr ra` plus `nop`; both boundaries are real instructions. The pool records
one direct caller/reference, and the function is part of the contiguous
hardware-setter cluster. `FUNCTION_HOOD=PROVEN`.

Retail body:

```text
lui at,0x1F80
sh  a0,0x1D88(at)
srl a0,a0,16
lui at,0x1F80
sh  a0,0x1D8A(at)
jr  ra
nop
```

Screen: no callees, globals, loops, or return-value pressure. This is the
adjacent MMIO twin of `func_80087728`.

## C and flags

```c
void func_8008770C(unsigned int a0) {
    volatile unsigned short *base = (volatile unsigned short *)0x1F800000;
    base[0xEC4] = (unsigned short)a0;
    base[0xEC5] = (unsigned short)(a0 >> 16);
}
```

Compiled with `era_compile ... -O2 -G0`. The one-base pointer keeps the
`0x1F80` materialization in `$at` and offsets both stores, as required by the
retail form.

## Packed span and carve

```text
candidate: 801f013c 881d24a4 02240400 801f013c 8a1d24a4 0800e003 00000000
retail:   801f013c 881d24a4 02240400 801f013c 8a1d24a4 0800e003 00000000
```

```text
asm prefix: 0x77F0C - 0x77C28 = 0x2E4
C leaf:     0x77F28 - 0x77F0C = 0x1C
closure:    0x2E4 + 0x1C = 0x300
```

The next leaf `func_80087728` begins immediately at `0x77F28`.

Build candidate SHA-1:
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
`scripts/verify_us.sh` passed; matching-C count is 294.
