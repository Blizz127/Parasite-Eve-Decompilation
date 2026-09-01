# `func_800631C0` — exact matching-C leaf

Leaf 292, matched on the second bounded natural-C phrasing.

## Function hood and retail screens

Retail span `[0x539C0,0x539DC)`, VRAM `0x800631C0`, seven words. It ends in
`jr ra` plus `nop`; the preceding and following boundary words are real
instructions. Direct callers are `0x80045E6C` and `0x80045FC8`.
`FUNCTION_HOOD=PROVEN`.

Retail body:

```text
beqz a0,.L800631D4    ; delay: addu v0,zero,zero
lw   v0,0x48(a0)
nop
sltiu v0,v0,1
jr   ra               ; delay: nop
```

Screens: no callees, globals, loops, or address-retention pressure; the input
is a nullable pointer and the result is `$v0`-live from either path. The key
`-O` signal is the filled branch delay slot on the null path.

## C and flags

```c
unsigned int func_800631C0(const unsigned char *arg0) {
    unsigned int result = 0;
    if (arg0 != 0) {
        result = *(const unsigned int *)(arg0 + 0x48) < 1;
    }
    return result;
}
```

Compiled with the retail-era `era_compile ... -O2 -G0` flags. The first
natural phrasing used an early null return and left the branch delay slot as
`nop`; the explicit accumulator phrasing above reproduced retail exactly.

## Packed-span comparison

The seven packed words at `[0x539C0,0x539DC)` match in full:

```text
candidate: 04008010 21100000 4800828C 00000000 0100422C 0800E003 00000000
retail:   04008010 21100000 4800828C 00000000 0100422C 0800E003 00000000
```

## Carve and gates

The original `539C0` asm segment was split at the candidate and resumes at
`539DC`; the next existing C leaf begins at `55248`:

```text
candidate: 0x539DC - 0x539C0 = 0x1C
resume:    0x55248 - 0x539DC = 0x186C
closure:   0x1C + 0x186C = 0x1888
```

Build output: candidate SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, exactly equal to retail.
`scripts/verify_us.sh` passed; matching-C count is 292.
