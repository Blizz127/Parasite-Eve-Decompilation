# `func_80078C94` — parked aggregate-return coloring

`PARKED-AGGREGATE-RETURN-COLORING`. Two bounded era `-O2 -G0` phrasings
were tested. The rejected source is preserved in stash
`park volume func_80078C94 aggregate return coloring`.

## Function hood and boundaries

Retail span `[0x69494,0x694B8)`, VRAM `0x80078C94`, nine words. It ends in
the canonical `jr ra; nop`, and has an exact direct caller at `0x80031A24`.

The span is surrounded by explicit alignment: the preceding real function
`func_80078C34` returns at `0x69488`/`0x6948C`, followed by one alignment nop
at `0x69490`; three alignment nops at `0x694B8..0x694C0` precede the next real
function at `0x694C4`. The padding is outside this called span.
`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLER`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; 12-byte copy leaf |
| global access / Stage 0 | none |
| coloring pressure | three loaded words plus destination/result pointer |
| `$v0` liveness | retail assigns the return pointer only after all stores |
| address retention | retail retains `$a0` as destination base across all three stores |
| optimization signal | straight-line block copy; era `-O2 -G0` |
| loop/back-edge owner | none |

The placement next to libGTE and the `+0x14` destination shape are suggestive
of a matrix/vector helper, but no exact PsyQ routine name is claimed without
symbol or string proof.

## Retail body

```text
8CA80000  lw    t0,0(a1)
8CA90004  lw    t1,4(a1)
8CAA0008  lw    t2,8(a1)
AC880014  sw    t0,0x14(a0)
AC890018  sw    t1,0x18(a0)
AC8A001C  sw    t2,0x1C(a0)
00801021  addu  v0,a0,zero
03E00008  jr    ra
00000000  nop
```

## Attempt 1 — 12-byte aggregate assignment

```c
typedef struct {
    int x;
    int y;
    int z;
} Vec3;

void *func_80078C94(unsigned char *a0, const Vec3 *a1) {
    *(Vec3 *)(a0 + 0x14) = *a1;
    return a0;
}
```

This preserved the nine-word size, but moved the result/destination into
`$v0` at entry and allocated the three loads to `$v1/$a0/$a2`:

```text
00801021  addu  v0,a0,zero
8CA30000  lw    v1,0(a1)
8CA40004  lw    a0,4(a1)
8CA60008  lw    a2,8(a1)
AC430014  sw    v1,0x14(v0)
AC440018  sw    a0,0x18(v0)
AC46001C  sw    a2,0x1C(v0)
03E00008  jr    ra
00000000  nop
```

## Attempt 2 — scalar field copy

```c
int *func_80078C94(int *a0, const int *a1) {
    a0[5] = a1[0];
    a0[6] = a1[1];
    a0[7] = a1[2];
    return a0;
}
```

This kept the same early `$v0` return home and let reorg place the final store
in the return delay slot, shrinking to eight words:

```text
00801021  addu  v0,a0,zero
8CA30000  lw    v1,0(a1)
8CA40004  lw    a0,4(a1)
8CA50008  lw    a1,8(a1)
AC430014  sw    v1,0x14(v0)
AC440018  sw    a0,0x18(v0)
03E00008  jr    ra
AC45001C  sw    a1,0x1C(v0)
```

Retail's late result copy and `$a0` destination home survive neither ordinary
source shape. Closing this needs a new aggregate-copy allocation/scheduling
lever, not a third equivalent spelling. No YAML/build/verifier integration was
made, and the matching-C count remains 309.
