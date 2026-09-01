# `func_80087798` — parked MMIO address retention

`PARKED-MMIO-ADDRESS-RETENTION`. Two bounded era `-O2 -G0` phrasings were
tested. The rejected source is preserved in stash
`park volume func_80087798 MMIO address-retention`.

## Function hood and boundaries

Retail span `[0x77F98,0x77FBC)`, VRAM `0x80087798`, nine words. It ends in
`jr ra` with the second halfword store in the delay slot. Three exact direct
callers occur at `0x80087958`, `0x8008C780`, and `0x8008C960`.

The preceding word at `0x77F94` is the real nop delay slot of
`func_8008777C`; the following word at `0x77FBC` is the real first
`sll a0,a0,4` instruction of `func_800877BC`.
`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; hardware leaf |
| global access / Stage 0 | none; volatile MMIO at `0x1F801C00 + voice*0x10` |
| coloring pressure | shared computed address must be retained in `$a0` across two stores |
| `$v0` liveness | fixed base `0x1F801C00` only; retail transfers it into the `$a0` address |
| address retention | decisive: retail uses the same mutated `$a0` for offsets `0` and `2` |
| optimization signal | constant materialization and scheduling match era `-O2 -G0` |
| loop/back-edge owner | none |

The hardware layout proves a per-voice SPU register-pair write. Exact PsyQ
routine-name attribution is not claimed without symbols.

## Retail body

```text
3C021F80  lui   v0,0x1F80
34421C00  ori   v0,v0,0x1C00
00042100  sll   a0,a0,4
00822021  addu  a0,a0,v0
30A57FFF  andi  a1,a1,0x7FFF
30C67FFF  andi  a2,a2,0x7FFF
A4850000  sh    a1,0(a0)
03E00008  jr    ra
A4860002  sh    a2,2(a0)
```

## Attempt 1 — explicit volatile pointer

```c
void func_80087798(unsigned int a0, unsigned int a1, unsigned int a2) {
    volatile unsigned short *voice =
        (volatile unsigned short *)(0x1F801C00 + (a0 << 4));

    voice[0] = a1 & 0x7FFF;
    voice[1] = a2 & 0x7FFF;
}
```

## Attempt 2 — mutate the address argument

```c
void func_80087798(unsigned int a0, unsigned int a1, unsigned int a2) {
    a0 = 0x1F801C00 + (a0 << 4);

    *(volatile unsigned short *)a0 = a1 & 0x7FFF;
    *(volatile unsigned short *)(a0 + 2) = a2 & 0x7FFF;
}
```

Both phrasings produced byte-identical 12-word output:

```text
3C021F80  lui   v0,0x1F80
34421C00  ori   v0,v0,0x1C00
00042100  sll   a0,a0,4
00821021  addu  v0,a0,v0       # first mismatch: retail writes a0
30A57FFF  andi  a1,a1,0x7FFF
30C67FFF  andi  a2,a2,0x7FFF
A4450000  sh    a1,0(v0)
3C011F80  lui   at,0x1F80      # rematerialized second address
00810821  addu  at,a0,at
A4261C02  sh    a2,0x1C02(at)
03E00008  jr    ra
00000000  nop
```

The semantic stores and masks are correct, but the address DAG/register home
is not. The explicit source mutation is canonicalized away before allocation;
the second store is reconstructed as an indexed fixed-address access. Closing
this needs a genuinely new address-retention lever, not another spelling of
the same expression. No YAML/build/verifier integration was made, and the
matching-C count remains 308.
