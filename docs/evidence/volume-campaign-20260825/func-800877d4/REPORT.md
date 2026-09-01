# `func_800877D4` — exact matching-C per-voice SPU setter

Leaf 298, matched on the first bounded natural-C phrasing.

## Function hood and retail screens

Retail span `[0x77FD4,0x77FF0)`, VRAM `0x800877D4`, seven words. It ends in
`jr ra` plus `nop`; both boundaries are real instructions. A direct caller is
at `0x80087984`. `FUNCTION_HOOD=PROVEN`.

```text
sll  a0,a0,4
srl  a1,a1,3
lui  at,0x1F80
addu at,a0,at
sh   a1,0x1C06(at)
jr   ra
nop
```

This is not the preceding paired-control-register shape. It scales a voice
index by the SPU voice-record stride (`0x10`), shifts the input by three, and
writes one per-voice halfword register. There are no callees, globals, loops,
or return-value pressure.

## C and flags

```c
void func_800877D4(int a0, unsigned int a1) {
    *(volatile unsigned short *)(0x1F801C06 + (a0 << 4)) =
        (unsigned short)(a1 >> 3);
}
```

Compiled with `era_compile ... -O2 -G0`; the natural indexed volatile store
matched all seven words on the first attempt.

## Packed span and carve

```text
candidate: 00210400 c2280500 801f013c 21088100 061c25a4 0800e003 00000000
retail:   00210400 c2280500 801f013c 21088100 061c25a4 0800e003 00000000
```

```text
C leaf:     0x77FF0 - 0x77FD4 = 0x1C
resume asm: 0x7B31C - 0x77FF0 = 0x332C
closure:    0x1C + 0x332C = 0x3348
```

Build candidate SHA-1:
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
`scripts/verify_us.sh` passed; matching-C count is 298.
