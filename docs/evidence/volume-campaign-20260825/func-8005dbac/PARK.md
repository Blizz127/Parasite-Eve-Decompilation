# `func_8005DBAC` — parked symbolic-address lifetime coloring

`PARKED-SYMBOLIC-ADDRESS-LIFETIME-COLORING`. Two bounded era `-O2 -G0`
source phrasings were tested. The final rejected source is preserved in stash
`park volume func_8005DBAC symbolic address lifetime coloring`.

## Function hood and boundaries

Retail span `[0x4E3AC,0x4E3F8)`, VRAM `[0x8005DBAC,0x8005DBF8)`, is
`0x4C` bytes / nineteen words. It ends in canonical `jr ra` with the final
address addition in the return delay slot.

A raw executable scan finds eleven direct callers:

```text
0x80051E00 0x800521BC 0x80052298 0x800522CC 0x800522FC 0x80052348
0x80052388 0x800523B0 0x80052434 0x8005CCE8 0x8005DA04
```

The preceding real `func_8005DB8C` owns `jr ra` plus its live addition delay
slot at `0x8005DBA4/0x8005DBA8`. The following real `func_8005DBF8` begins
immediately at `0x8005DBF8` with a symbolic-address `lui`. There is no
padding ambiguity.

`FUNCTION_HOOD=PROVEN_BY_11_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

The nineteen-word boundary corrects older pc-port commentary that counted
the following function's first `lui` at `0x8005DBF8` as a twentieth word.
That older native semantic proof remains useful, but not for span ownership.

## Proven semantics and screens

Existing B28 native tests and an independent instruction oracle prove:

```text
clamped = clamp(index, 0, 98)
return D_800A803C + ((unsigned int)&D_800A803C - 0x14) + clamped * 24
```

Here `D_800A803C` denotes the loaded 32-bit value; the address term is
`0x800A8028`.

| screen | result |
|---|---|
| callee buckets | no `jal`; true leaf |
| written-state Stage 0 | none; reads `D_800A803C` only |
| coloring pressure | retail copies the argument to `$v1`, uses `$v0` for clamp predicate/scaled index/result, then reuses dead `$a0` for both symbolic address and loaded value |
| `$v0` liveness | clamp predicate, `clamped*24`, address-adjusted offset, final result |
| address retention | critical: `$a0=&D_800A803C`, `$v1=$a0-0x14`, then `lw a0,0(a0)` retains both address-derived terms |
| optimization signal | strength reduction, local jumps, and live return-delay addition select era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

## Retail body

```text
00801821  addu   v1,a0,zero
04610003  bgez   v1,0x8005DBC0
00000000  nop
080176F4  j      0x8005DBD0
00001821  move   v1,zero
28620063  slti   v0,v1,99
14400002  bnez   v0,0x8005DBD0
00000000  nop
24030062  addiu  v1,zero,98
3C04800B  lui    a0,0x800B
2484803C  addiu  a0,a0,-0x7FC4
00031040  sll    v0,v1,1
00431021  addu   v0,v0,v1
000210C0  sll    v0,v0,3
2483FFEC  addiu  v1,a0,-20
8C840000  lw     a0,0(a0)
00431021  addu   v0,v0,v1
03E00008  jr     ra
00821021  addu   v0,a0,v0
```

## Attempt 1 — natural global/address expression

```c
extern unsigned int D_800A803C;

unsigned int func_8005DBAC(int index) {
    int clamped = index;

    if (clamped < 0) {
        clamped = 0;
    } else if (clamped >= 99) {
        clamped = 98;
    }

    return D_800A803C + (unsigned int)&D_800A803C - 0x14 + clamped * 24;
}
```

cc1 keeps the clamped value in `$a0`, consumes the global value/address
before scaling, and produces eighteen content words:

```text
04810003 00000000 08000008 00002021
28820063 14400002 00000000 24040062
3C020000 8C420000 3C030000 2463FFEC
00431021 00041840 00641821 000318C0
03E00008 00431021
```

The symbolic words carry ordinary HI16/LO16 relocations. Resolving them does
not affect the control flow, register homes, or one-word size deficit.

## Attempt 2 — explicit pointer, scaled term first

```c
extern unsigned int D_800A803C;

unsigned int func_8005DBAC(int index) {
    int clamped = index;
    unsigned int *base = &D_800A803C;

    if (clamped < 0) {
        clamped = 0;
    } else if (clamped >= 99) {
        clamped = 98;
    }

    return clamped * 24 + ((unsigned int)base - 0x14) + *base;
}
```

The explicit pointer changes the DAG, but cc1 hoists it into `$a1` before
the clamp and still leaves the clamped value in `$a0`. It remains eighteen
content words:

```text
3C050000 24A50000 04810003 00000000
0800000A 00002021 28820063 14400002
00000000 24040062 00041040 00441021
000210C0 2442FFEC 8CA30000 00451021
03E00008 00431021
```

Retail's extra word is not redundant padding: it is the initial
`move v1,a0` that establishes disjoint clamp and future address homes. The
two natural expression orders either avoid that copy or hoist the pointer;
neither reproduces retail's lifetime split. A third equivalent spelling
would violate the bounded phrasing rule.

No YAML/build/verifier integration was made. The accepted executable remains
exact, matching-C stays 334, and consecutive parks become one.
