# `func_8008780C` — parked independent-operation scheduling

`PARKED-INDEPENDENT-OP-SCHEDULING`. Two bounded era `-O2 -G0` source
phrasings were tested. The rejected source is preserved in stash
`park volume func_8008780C independent-op scheduling`.

## Function hood and boundaries

Retail span `[0x7800C,0x7803C)`, VRAM `[0x8008780C,0x8008783C)`, is
`0x30` bytes / twelve words. It ends in canonical `jr ra` with the final
halfword store in the return delay slot. Three exact direct callers occur at
`0x80087A18`, `0x8008C79C`, and `0x8008C97C`.

The preceding real `func_800877F0` ends at `0x80087804/0x80087808` with
`jr ra; nop`. The following real `func_8008783C` begins at `0x8008783C`
with `lui v0,0x1F80`. There is no padding or ambiguous ownership at either
boundary.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; MMIO bitfield-update leaf |
| written-state Stage 0 | no RAM global; writes per-voice SPU halfword at `0x1F801C08 + voice*0x10` |
| coloring pressure | retail mutates `$a0` into the register address, `$a2` into the upper mode field, `$a1` into the middle field, and combines through `$v0` |
| `$v0` liveness | fixed base, then loaded low byte, then final halfword value through the return-slot store |
| address retention | computed MMIO pointer stays in `$a0` from `lbu` through `sh` |
| optimization signal | fixed-address `lui; ori`, in-place argument transforms, and return-delay store select era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

The hardware layout proves a per-voice SPU control-register update. Retail
preserves the low byte, places `high` at bit 8, and places `(mode >> 2)` at
bit 15. No exact Psy-Q routine name is claimed without symbol provenance.

## Retail body

```text
3C021F80  lui   v0,0x1F80
34421C08  ori   v0,v0,0x1C08
00042100  sll   a0,a0,4
00822021  addu  a0,a0,v0
00063082  srl   a2,a2,2
000633C0  sll   a2,a2,15
00052A00  sll   a1,a1,8
90820000  lbu   v0,0(a0)
00C53025  or    a2,a2,a1
00461025  or    v0,v0,a2
03E00008  jr    ra
A4820000  sh    v0,0(a0)
```

## Attempt 1 — natural volatile expression

```c
void func_8008780C(int voice, unsigned int high, unsigned int mode) {
    volatile unsigned short *reg =
        (volatile unsigned short *)(0x1F801C08 + (voice << 4));

    *reg = (unsigned short)(*(volatile unsigned char *)reg |
                            (high << 8) | ((mode >> 2) << 15));
}
```

This proves the fixed-address and `$a0` address-retention shape, but the
left-associated expression schedules `$a1` before `$a2`, combines each field
directly into `$v0`, and volatility emits `sh; jr; nop`. It produces thirteen
semantic words versus retail's twelve:

```text
3C021F80 34421C08 00042100 00822021
00052A00 00063082 90820000 000633C0
00451025 00461025 A4820000 03E00008 00000000
```

## Attempt 2 — explicit retail dataflow and nonvolatile lvalue

```c
void func_8008780C(int voice, unsigned int high, unsigned int mode) {
    unsigned short *reg =
        (unsigned short *)(0x1F801C08 + (voice << 4));
    unsigned int value;

    mode = (mode >> 2) << 15;
    high <<= 8;
    value = *(unsigned char *)reg;
    mode |= high;
    value |= mode;
    *reg = (unsigned short)value;
}
```

This fixes the expression DAG, all register homes, body size, and the
`jr; sh` ending. Eight of twelve words are exact. The only residual is the
order of two independent two-instruction blocks:

```text
retail:    lui/ori base
           sll/addu a0 address
           srl/sll  a2 field
           sll a1 / lbu / or / or / jr / sh

candidate: lui/ori base
           srl/sll  a2 field
           sll/addu a0 address
           sll a1 / lbu / or / or / jr / sh
```

Full attempt-2 candidate:

```text
3C021F80 34421C08 00063082 000633C0
00042100 00822021 00052A00 90820000
00C53025 00461025 03E00008 A4820000
```

First mismatch is at `0x80087814`; mismatches are words 2–5 only. The
semantic DAG, addresses, registers, and ending all agree. Closing this needs
a new independent-operation scheduling lever; a third equivalent spelling
would violate the bounded-phrasing rule.

No YAML/build/verifier integration was made. The accepted executable remains
exact, matching-C remains 322, and the consecutive-park count becomes one.
