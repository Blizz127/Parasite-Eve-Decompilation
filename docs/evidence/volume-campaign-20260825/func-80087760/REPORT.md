# `func_80087760` — exact matching-C hardware halfword setter

Leaf 296, matched on the first bounded phrasing using the proven one-base
volatile MMIO idiom.

## Function hood and retail screens

Retail span `[0x77F60,0x77F7C)`, VRAM `0x80087760`, seven words. It ends in
`jr ra` plus `nop`; both boundaries are real instructions. The pool records
one direct caller/reference in the contiguous hardware-setter cluster.
`FUNCTION_HOOD=PROVEN`.

Retail body:

```text
lui at,0x1F80
sh  a0,0x1D94(at)
srl a0,a0,16
lui at,0x1F80
sh  a0,0x1D96(at)
jr  ra
nop
```

## C and flags

```c
void func_80087760(unsigned int a0) {
    volatile unsigned short *base = (volatile unsigned short *)0x1F800000;
    base[0xECA] = (unsigned short)a0;
    base[0xECB] = (unsigned short)(a0 >> 16);
}
```

Compiled with `era_compile ... -O2 -G0`; the proven one-base MMIO form
reproduced the `$at` materialization and both stores on the first attempt.

## Packed span and carve

```text
candidate: 801f013c 941d24a4 02240400 801f013c 961d24a4 0800e003 00000000
retail:   801f013c 941d24a4 02240400 801f013c 961d24a4 0800e003 00000000
```

```text
asm prefix: 0x77F60 - 0x77F44 = 0x1C
C leaf:     0x77F7C - 0x77F60 = 0x1C
resume asm: 0x77FBC - 0x77F7C = 0x40
closure:    0x1C + 0x1C + 0x40 = 0x78
```

Build candidate SHA-1:
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
`scripts/verify_us.sh` passed; matching-C count is 296.
