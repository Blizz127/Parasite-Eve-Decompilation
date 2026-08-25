# `func_8005D970` — exact matching-C signed-threshold selector

Leaf 309, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x4E170,0x4E194)`, VRAM `0x8005D970`, nine words. It ends in
the canonical `jr ra; nop`. The exact direct caller is at `0x80044150`.

The preceding word at `0x4E16C` is the real nop return-delay slot of
`func_8005D940`; the following word at `0x4E194` is the real first
`addiu v0,zero,0x32` instruction of `func_8005D994`.
`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLER`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; state-selection leaf |
| written-global Stage 0 | `D_8009D0D4` (`$gp+0x364`) has no other executable load/store reference; write-only in the executable census |
| input global | `D_8009CDAC` (`$gp+0x3C`), initialized to `0x123`; signed threshold proven by `slti 0x12C` |
| coloring pressure | low: condition in `$v0`, selected result in `$v1` |
| `$v0` liveness | load and boolean threshold only; no return value |
| address retention | none; two independent gp-relative relocations |
| optimization signal | load-delay nop plus branch-delay default value; era `-O2 -G8` |
| loop/back-edge owner | none |

The output's higher-level role is not named: the executable proves only that
it receives `4` when the signed input is below 300 and `-4` otherwise.

## C and flags

```c
extern int D_8009CDAC;
extern int D_8009D0D4;

void func_8005D970(void) {
    int value = -4;

    if (D_8009CDAC < 300) {
        value = 4;
    }

    D_8009D0D4 = value;
}
```

Compiled with `era_compile ... -O2 -G8`. The two `R_MIPS_GPREL16`
relocations normalize as follows:

```text
D_8009CDAC - _gp = 0x3C
D_8009D0D4 - _gp = 0x364
```

There are no pins, inline assembly, or special maspsx switches.

## Full single-leaf comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `8F82003C` | `8F82003C` | `lw v0,0x3C(gp)` |
| 1 | `00000000` | `00000000` | `nop` |
| 2 | `2842012C` | `2842012C` | `slti v0,v0,300` |
| 3 | `10400002` | `10400002` | `beqz v0,+2` |
| 4 | `2403FFFC` | `2403FFFC` | `addiu v1,zero,-4` |
| 5 | `24030004` | `24030004` | `addiu v1,zero,4` |
| 6 | `AF830364` | `AF830364` | `sw v1,0x364(gp)` |
| 7 | `03E00008` | `03E00008` | `jr ra` |
| 8 | `00000000` | `00000000` | `nop` |

## Carve and packed-span proof

The prior asm span was `[0x4CC98,0x4E2DC)`, size `0x1644`:

```text
prefix asm: 0x4E170 - 0x4CC98 = 0x14D8
C leaf:     0x4E194 - 0x4E170 = 0x24
resume asm: 0x4E2DC - 0x4E194 = 0x148
closure:    0x14D8 + 0x24 + 0x148 = 0x1644
```

Full packed span, file `0x4E170..0x4E193`:

```text
retail:    3c00828f000000002c01422802004010fcff032404000324640383af0800e00300000000
candidate: 3c00828f000000002c01422802004010fcff032404000324640383af0800e00300000000
```

The packed executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. `scripts/verify_us.sh` exits
zero and reports 309 matching-C leaves.
