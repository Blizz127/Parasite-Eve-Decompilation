# `func_8006E454` — parked independent load scheduling

`PARKED-INDEPENDENT-LOAD-SCHEDULING`. Two bounded era `-O2 -G0` source
phrasings were tested. The final rejected source is preserved in stash
`park volume func_8006E454 independent load scheduling`.

## Function hood and boundaries

Retail span `[0x5EC54,0x5EC98)`, VRAM `[0x8006E454,0x8006E498)`, is
`0x44` bytes / seventeen words. It ends in canonical `jr ra` with the final
addition in the return delay slot.

A raw executable scan finds two direct callers, at `0x8005D958` and
`0x8006B560`. The preceding real `func_8006E3D4` owns `jr ra` plus its live
result-copy delay slot at `0x8006E44C/0x8006E450`. The following real
`func_8006E498` begins immediately with `addiu sp,sp,-8`. There is no padding
or tail-entry ambiguity.

`FUNCTION_HOOD=PROVEN_BY_2_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Proven semantics and screens

The BTL14 native implementation and independent oracle already prove this as
the decimal parser for field/package-name bytes 2–4: for example `M0005I`
returns 5, and `M0367I` returns 367.

| screen | result |
|---|---|
| callee buckets | no `jal`; true leaf |
| written-state Stage 0 | none; reads three argument bytes only |
| coloring pressure | `$v1` carries hundreds arithmetic, `$a1` carries tens, and `$v0` accumulates the scaled result; retail reuses `$v1` for the late ones byte |
| `$v0` liveness | strength-reduced `hundreds*100`, then adds `tens*10`, then the final digit-minus-`'0'` |
| address retention | `$a0` remains the source pointer until retail's late byte-4 load |
| optimization signal | multiply-by-100/10 strength reduction and a live return-delay addition select era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

Retail uses signed `lb` for all three bytes and computes:

```text
(name[2] - '0') * 100 + (name[3] - '0') * 10 + name[4] - '0'
```

## Retail body

```text
80830002  lb     v1,2(a0)
80850003  lb     a1,3(a0)
2463FFD0  addiu  v1,v1,-48
00031040  sll    v0,v1,1
00431021  addu   v0,v0,v1
000210C0  sll    v0,v0,3
00431021  addu   v0,v0,v1
00021080  sll    v0,v0,2
24A5FFD0  addiu  a1,a1,-48
00051880  sll    v1,a1,2
00651821  addu   v1,v1,a1
00031840  sll    v1,v1,1
00431021  addu   v0,v0,v1
80830004  lb     v1,4(a0)
2442FFD0  addiu  v0,v0,-48
03E00008  jr     ra
00431021  addu   v0,v0,v1
```

## Attempt 1 — natural named digits

```c
int func_8006E454(const char *name) {
    int hundreds = name[2] - '0';
    int tens = name[3] - '0';
    int ones = name[4];

    return hundreds * 100 + tens * 10 + ones - '0';
}
```

This compiler treats plain `char` as unsigned, selecting `lbu`. It also
hoists all three independent byte loads before either digit calculation:

```text
90830002 90850003 90840004 2463FFD0
24A5FFD0 00031040 00431021 000210C0
00431021 00021080 00051880 00651821
00031840 00431021 00441021 03E00008
2442FFD0
```

## Attempt 2 — explicit signed bytes and inline ones digit

```c
int func_8006E454(const signed char *name) {
    int hundreds = name[2] - '0';
    int tens = name[3] - '0';

    return hundreds * 100 + tens * 10 + name[4] - '0';
}
```

Explicit signedness recovers all three retail `lb` opcodes, but removing the
`ones` local does not alter scheduling. cc1 still hoists byte 4 into `$a0`
at word 2, then performs the same correct arithmetic:

```text
80830002 80850003 80840004 2463FFD0
24A5FFD0 00031040 00431021 000210C0
00431021 00021080 00051880 00651821
00031840 00431021 00441021 03E00008
2442FFD0
```

Both candidates are seventeen content words, and attempt 2 has the exact
signed semantics. Retail's distinguishing feature is deferring the
independent byte-4 load until after both scaled terms, enabling `$v1` reuse
and a different final delay-slot expression. A third equivalent spelling
would violate the bounded phrasing rule.

No YAML/build/verifier integration was made. The accepted executable remains
exact, matching-C stays 333, and consecutive parks become one.
