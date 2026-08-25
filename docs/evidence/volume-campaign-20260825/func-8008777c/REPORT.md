# `func_8008777C` — exact matching-C hardware halfword setter

Leaf 297, matched on the first bounded phrasing using the proven one-base
volatile MMIO idiom.

## Function hood and retail screens

Retail span `[0x77F7C,0x77F98)`, VRAM `0x8008777C`, seven words. It ends in
`jr ra` plus `nop`; both boundaries are real instructions. The pool records
one direct caller/reference in the contiguous hardware-setter cluster.
`FUNCTION_HOOD=PROVEN`.

Retail body:

```text
lui at,0x1F80
sh  a0,0x1D90(at)
srl a0,a0,16
lui at,0x1F80
sh  a0,0x1D92(at)
jr  ra
nop
```

## C and flags

```c
void func_8008777C(unsigned int a0) {
    volatile unsigned short *base = (volatile unsigned short *)0x1F800000;
    base[0xEC8] = (unsigned short)a0;
    base[0xEC9] = (unsigned short)(a0 >> 16);
}
```

Compiled with `era_compile ... -O2 -G0`; the proven one-base MMIO form
reproduced the `$at` materialization and both stores on the first attempt.

## Packed span and carve

```text
candidate: 801f013c 901d24a4 02240400 801f013c 921d24a4 0800e003 00000000
retail:   801f013c 901d24a4 02240400 801f013c 921d24a4 0800e003 00000000
```

```text
asm prefix: 0x77F7C - 0x77F60 = 0x1C
C leaf:     0x77F98 - 0x77F7C = 0x1C
resume asm: 0x77FBC - 0x77F98 = 0x24
closure:    0x1C + 0x1C + 0x24 = 0x5C
```

Build candidate SHA-1:
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
`scripts/verify_us.sh` passed; matching-C count is 297.
