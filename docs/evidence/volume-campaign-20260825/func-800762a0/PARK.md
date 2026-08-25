# `func_800762A0` — parked packed-command register coloring

Outcome: `PARKED-REGISTER-COLORING` after two bounded natural-C phrasings.
No integration; the matching-C count remains 291.

## Function hood

Retail span `[0x66AA0,0x66ABC)`, VRAM `0x800762A0`, seven words. It ends in
the canonical `jr ra` with an `or v0,a1,v0` delay slot. Direct `jal` callers
are at `0x80075C24`, `0x80075D54`, and `0x80075F4C`; the words immediately
before and after the span are real instructions. `FUNCTION_HOOD=PROVEN`.

## Retail semantics and screens

```text
andi a1,a1,0x7FF
sll  a1,a1,11
andi v0,a0,0x7FF
lui  v1,0xE500
or   v0,v0,v1
jr   ra
or   v0,a1,v0
```

The helper packs two 11-bit fields into an `E5` GPU command word. There is no
callee, global, loop, or small-data access; the decisive screen is the
register-coloring choice that keeps the first masked/shifted input in `$a1`.

## Bounded attempts

Both attempts used the retail-era flags `era_compile ... -O2 -G0`.

Attempt 1:

```c
unsigned int func_800762A0(unsigned int a0, unsigned int a1) {
    return ((a1 & 0x7FFu) << 11) | (a0 & 0x7FFu) | 0xE5000000u;
}
```

Attempt 2:

```c
unsigned int func_800762A0(unsigned int a0, unsigned int a1) {
    a1 = (a1 & 0x7FFu) << 11;
    a0 &= 0x7FFu;
    return a1 | a0 | 0xE5000000u;
}
```

Both produced the same residual: the value equivalent to retail's first
`andi`/`sll` is colored through `$v0` rather than `$a1`. The full second build
was otherwise stable but differed in 13 bytes. Retail and candidate packed
spans were:

```text
candidate: ff07a230 c0120200 ff078430 25104400 00e5033c 0800e003 25104300
retail:   ff07a530 c02a0500 ff078230 00e5033c 25104300 0800e003 2510a200
```

First mismatch: file `0x66AA2`, candidate byte `0xA2`, retail byte `0xA5`.
Candidate full-build SHA-1 was `986754e64e4ad20c58a97f0a5f5cb08c276b9d68`;
the retail/original SHA-1 remained
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Carve geometry recorded for rejected integration

```text
asm prefix: 0x66AA0 - 0x654C8 = 0x15D8
C leaf:     0x66ABC - 0x66AA0 = 0x1C
resume asm: 0x66B3C - 0x66ABC = 0x80
closure:    0x15D8 + 0x1C + 0x80 = 0x1674
```

The candidate source, YAML/build integration, and retry object are preserved
in stash `park volume func_800762A0 register-coloring residual`; the accepted
tree contains no integration or matching-count change.
