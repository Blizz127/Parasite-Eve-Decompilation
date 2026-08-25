# `func_80083790` — parked arithmetic association/scheduling

`PARKED-ARITHMETIC-ASSOCIATION-SCHEDULING`. Two bounded era `-O2 -G0`
source phrasings were tested. The rejected source is preserved in stash
`park volume func_80083790 arithmetic association scheduling`.

## Function hood and boundaries

Retail span `[0x73F90,0x73FC8)`, VRAM `[0x80083790,0x800837C8)`, is
`0x38` bytes / fourteen words. It ends in canonical `jr ra` with the final
base addition in the return delay slot. The executable has one exact direct
caller at `0x80083738` (`jal 0x80083790`).

The preceding real `func_80083644` ends at `0x80083788/0x8008378C` with
`jr ra; addiu sp,sp,0x18`. The following real `func_800837C8` begins at
`0x800837C8` with `addiu sp,sp,-0x20`. There is no padding or ambiguous
ownership at either boundary.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLER_AND_CANONICAL_RETURN`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; pure arithmetic leaf |
| written-state Stage 0 | no stores and no global state |
| coloring pressure | `$v0` owns the first offset and result, `$a1` owns the second byte, `$v1` owns the second offset, and loaded base remains in `$a0` through return |
| `$v0` liveness | first byte → rounded/scaled term → combined offset → final result |
| address retention | input pointer is consumed by three loads, then `$a0` is reused to retain the loaded base for the return delay slot |
| optimization signal | in-place argument homes and a live arithmetic return delay slot select era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

The function computes a base-relative offset from bytes `+0xE3/+0xE9` and
word `+0xEC`. No semantic field names are assigned without caller/type
provenance.

## Retail body

```text
908200E3  lbu   v0,0xE3(a0)
908500E9  lbu   a1,0xE9(a0)
8C8400EC  lw    a0,0xEC(a0)
24420001  addiu v0,v0,1
00021043  sra   v0,v0,1
00021080  sll   v0,v0,2
00051880  sll   v1,a1,2
00651821  addu  v1,v1,a1
24630003  addiu v1,v1,3
30630FFC  andi  v1,v1,0xFFC
24630004  addiu v1,v1,4
00431021  addu  v0,v0,v1
03E00008  jr    ra
00441021  addu  v0,v0,a0
```

Equivalent arithmetic is:

```text
base + (((first + 1) >> 1) << 2)
     + (((second * 5 + 3) & 0xFFC) + 4)
```

## Attempt 1 — base-first expression

```c
int func_80083790(unsigned char *arg0) {
    int first = arg0[0xE3];
    int second = arg0[0xE9];
    int base = *(int *)(arg0 + 0xEC);

    return base + (((first + 1) >> 1) << 2) +
           (((second * 5 + 3) & 0xFFC) + 4);
}
```

This emits the exact fourteen-word size but associates `base`, the first
term, and `+4` together. GCC therefore loads base into `$v1`, reloads the
second byte into `$a0`, and schedules the two expression halves in the
opposite register/dataflow shape. Five of fourteen words match.

```text
908200E3 8C8300EC 908400E9 24420001
00021043 00021080 00621821 24630004
00041080 00441021 24420003 30420FFC
03E00008 00621021
```

## Attempt 2 — explicit offset before base

```c
int func_80083790(unsigned char *arg0) {
    int first = arg0[0xE3];
    int second = arg0[0xE9];
    int base = *(int *)(arg0 + 0xEC);
    int offset;

    offset = (((first + 1) >> 1) << 2) +
             (((second * 5 + 3) & 0xFFC) + 4);
    return offset + base;
}
```

This recovers all load order and register homes, both main arithmetic DAGs,
the combined offset, and the base addition in the return delay slot. Nine of
fourteen words are exact. The sole residual mechanism is placement of an
independent constant addition:

```text
retail:    first term
           second*5 + 3
           mask second
           second += 4
           combine

candidate: first term
           first += 4
           second*5 + 3
           mask second
           combine
```

Full attempt-2 candidate:

```text
908200E3 908500E9 8C8400EC 24420001
00021043 00021080 24420004 00051880
00651821 24630003 30630FFC 00431021
03E00008 00441021
```

The mismatch is words 6–10 only; all addresses, arithmetic, final register,
and function size agree. Closing this requires a source or scheduling lever
that prevents associative constant folding onto the first term. A third
equivalent spelling would violate the bounded-phrasing rule.

No YAML/build/verifier integration was made. The accepted executable remains
exact, matching-C stays 329, and consecutive parks become one.
