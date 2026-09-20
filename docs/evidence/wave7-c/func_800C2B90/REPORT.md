# wave7-c — func_800C2B90 (executed-path C leaf)

**Result: LANDED.** Commit `b6d0ea0b` on `agent/wave7-c`. Count transition
**902 -> 903**.

## Identity

| field | value |
| --- | --- |
| function | `func_800C2B90` |
| file offset | `0xB3390` |
| size | `0x17C` (95 words) |
| VRAM | `0x800C2B90` |
| splat source | `asm/disc1/B3390.s` |
| profile | default `era_o2_g0` (`-O2 -G0`), no new profile |

## Semantics

Weapon-slot record lookup + callback dispatch. Scans the 0x40 six-byte records
at `D_800F34F4` for the first whose byte `+1` is zero, recording its index
(`found = -1` when none). No record sets the "full" flag at `D_800E2248+0x40`
and returns 0. Otherwise, if `func_800C6CE0(slot)` is one of 3/4/5, the record
is published through `func_800C2D0C(found, code & 0xFF, sizes[code & 0xFF])`,
the `callbacks[code & 0xFF]` entry is invoked (when not `-1`) with
`(slot, rec, data)`, and the data pointer `D_800F3330 + rec->unk4` is returned.
Otherwise 0.

## Fresh-build authority

```
EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (903 registered C leaves)
plan 1310 spans = 903 c + 405 asm + 2 rodata
VERIFY_US=PASS
```

`try_leaf.py src/func_800C2B90.c 0xB3390 0x17C` before the carve:
`WORDS MATCH (+4 pad bytes, trimmed by the build)`.

## Levers

- `D_800F34F4` and `D_800F3330` are **integer bases** (they hold pointer
  values), so address arithmetic stays `addu`/`addiu` (same convention as the
  matched sibling `func_800C2E08`).
- The three `func_800C6CE0(slot)` calls are genuinely **separate short-circuit
  calls**; storing the kind in a local makes cc1 call once and compare three
  times, which is the wrong shape.
- `func_800C2D0C` is prototyped with an **`unsigned short` first parameter**,
  so the caller emits `andi $a0,$s1,0xFFFF` and cc1 fills it into the branch
  delay slots. An unprototyped call sign-extends instead.
- The four incoming arguments are copied into locals in the order
  **(slot, sizes, callbacks, code)**. cc1 emits the callee-saved save/move
  pairs in assignment order, reproducing retail's `$s2,$s3,$s4,$s0` prologue.
  Declaring them in ABI order leaves 6 words swapped in the prologue.

## Divergence history

First candidate (no prototype, ABI-order locals): 22 word diffs. Adding the
`func_800C2D0C` prototype fixed the delay-slot `andi` and dropped it to 6.
The save-order local copies closed the last 6.
