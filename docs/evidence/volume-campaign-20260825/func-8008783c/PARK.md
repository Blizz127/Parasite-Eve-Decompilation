# `func_8008783C` — parked volatile-store scheduling

`PARKED-VOLATILE-STORE-SCHEDULING`. Two bounded era `-O2 -G0` phrasings
were tested. The rejected source is preserved in stash
`park volume func_8008783C volatile store scheduling`.

## Function hood and boundaries

Retail span `[0x7803C,0x78064)`, VRAM `0x8008783C`, ten words. It ends in
the canonical `jr ra` with `sh v0,0(a0)` in the return delay slot. The exact
direct caller is at `0x80087A74`.

The preceding real function completes at `0x78034/0x78038`; adjacent
`func_80087864` begins with a real `lui` at `0x78064`. There is no padding or
ambiguous ownership at either boundary.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLER`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; volatile MMIO read/modify/write leaf |
| written-global Stage 0 | no RAM global; writes per-voice SPU halfword at `0x1F801C08 + voice*0x10` |
| coloring pressure | low; retail retains computed register address in `$a0`, value in `$v0` |
| `$v0` liveness | new halfword value remains live through return |
| address retention | computed MMIO pointer remains in `$a0` from `lhu` through `sh` |
| optimization signal | `lui; ori` fixed address and return-delay-slot store select era `-O2 -G0` |
| loop/back-edge owner | none |

The hardware layout proves a per-voice SPU register update. The mask
`0xFF0F` replaces bits 4..7 with `a1 << 4`; no exact Psy-Q routine name is
claimed here.

## Retail body

```text
3C021F80  lui   v0,0x1F80
34421C08  ori   v0,v0,0x1C08
00042100  sll   a0,a0,4
00822021  addu  a0,a0,v0
94820000  lhu   v0,0(a0)
00052900  sll   a1,a1,4
3042FF0F  andi  v0,v0,0xFF0F
00451025  or    v0,v0,a1
03E00008  jr    ra
A4820000  sh    v0,0(a0)
```

## Attempt 1 — void volatile RMW

```c
void func_8008783C(int a0, unsigned int a1) {
    volatile unsigned short *reg =
        (volatile unsigned short *)(0x1F801C08 + (a0 << 4));

    *reg = (unsigned short)((*reg & 0xFF0F) | (a1 << 4));
}
```

The first eight words are exact. Era cc1 emits the volatile store before the
return and leaves the delay slot empty:

```text
3C021F80  lui   v0,0x1F80
34421C08  ori   v0,v0,0x1C08
00042100  sll   a0,a0,4
00822021  addu  a0,a0,v0
94820000  lhu   v0,0(a0)
00052900  sll   a1,a1,4
3042FF0F  andi  v0,v0,0xFF0F
00451025  or    v0,v0,a1
A4820000  sh    v0,0(a0)
03E00008  jr    ra
00000000  nop
```

## Attempt 2 — explicit returned value

```c
unsigned int func_8008783C(int a0, unsigned int a1) {
    volatile unsigned short *reg =
        (volatile unsigned short *)(0x1F801C08 + (a0 << 4));
    unsigned int value = (*reg & 0xFF0F) | (a1 << 4);

    *reg = (unsigned short)value;
    return value;
}
```

Retail leaves the computed value in `$v0`, so an explicit return was the
bounded delay-slot lever. It produces byte-identical output to attempt 1:
`sh; jr; nop`. The residual is only scheduling, not address formation,
register coloring, or semantics.

The existing `MASPSX_FILL_STORE_DELAY_SLOT` patch is not a lever for this
site: it is deliberately scoped to an absolute symbolic `sw` macro that is
opaque to cc1, while this is a direct volatile `sh 0(a0)` already emitted by
cc1. Extending that patch would require a separate toolchain investigation
and full regression; it is not justified ad hoc by one leaf.

No YAML/build/verifier integration was made. The executable remains exact and
the matching-C count remains 311.
