# `func_80077CB4` — exact packet-chain merge helper

Leaf 329, matched on the second and final bounded phrasing.

## Function hood and boundary ownership

Retail span `[0x684B4,0x684EC)`, VRAM `[0x80077CB4,0x80077CEC)`, is
`0x38` bytes / fourteen words. It ends in canonical `jr ra; nop` at
`0x80077CE4/0x80077CE8`.

The executable contains eleven direct `jal 0x80077CB4` references:

```text
0x80037110  0x80037174  0x80037318  0x800373C0
0x80037D70  0x80037EB8  0x80038134  0x800383EC
0x80038528  0x80038668  0x800387E8
```

The preceding real `func_80077C84` ends with `jr ra` and its store delay
slot at `0x80077CA8/0x80077CAC`. One alignment nop at `0x80077CB0`
precedes this function. The function owns the nop at `0x80077CE8` because
it is the architectural delay slot of its return. Two separate alignment
nops follow at `0x80077CEC/0x80077CF0`; real `func_80077CF4` then starts
with `addiu sp,sp,-24`.

`FUNCTION_HOOD=PROVEN_BY_11_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; true leaf |
| written-state Stage 0 | argument-relative only: success updates `head[3]` and clears the first word of `tail`; no global writer |
| coloring pressure | `$a0` and `$a1` retain the two addresses, `$v1` retains the new length, and `$v0` transitions from sum/predicate to return accumulator |
| `$v0` liveness | byte sum → signed range predicate → explicit result; retail fills the guard delay slot with `move v0,zero` |
| address retention | fixed argument offsets only; no symbolic/global address |
| optimization signal | load-delay nop plus filled branch/jump delay slots and merged result select era GCC `-O2 -G0` |
| loop/back-edge owner | none |

The two input bytes undergo ordinary integer promotion. Retail's signed
`slti`, rather than `sltiu`, proves that the local length is a signed `int`.

## Bounded phrasing record

Attempt 1 used an unsigned length and early returns. It emitted `sltiu`,
reversed the success branch, and required an extra control-flow word.

Attempt 2 used the retail-proven signed length and an explicit result
accumulator. That recovered `slti`, the `beqz` success guard, the
`move v0,zero` guard delay slot, and the success-path `sw` in the local
jump delay slot. No pins, inline assembly, or special maspsx switch was used.

## Final C and flags

```c
int func_80077CB4(unsigned char *head, unsigned char *tail) {
    int length = head[3] + tail[3] + 1;
    int result;

    if (length < 17) {
        head[3] = length;
        *(unsigned int *)tail = 0;
        result = 0;
    } else {
        result = -1;
    }

    return result;
}
```

Compiler: era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`. Those flags are
justified by retail's era load-delay nop, its filled MIPS-I delay slots, and
the exact accumulator scheduling above.

## Full fourteen-word comparison

The object-local jump has word `0800000C` plus `R_MIPS_26 .text`; after
normalizing that relocation to linked VRAM it is retail `0801DF39`.

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `90820003` | `90820003` | `lbu v0,3(a0)` |
| 1 | `90A30003` | `90A30003` | `lbu v1,3(a1)` |
| 2 | `00000000` | `00000000` | `nop` |
| 3 | `00431021` | `00431021` | `addu v0,v0,v1` |
| 4 | `24430001` | `24430001` | `addiu v1,v0,1` |
| 5 | `28620011` | `28620011` | `slti v0,v1,17` |
| 6 | `10400004` | `10400004` | `beqz v0,+4` |
| 7 | `00001021` | `00001021` | `move v0,zero` |
| 8 | `A0830003` | `A0830003` | `sb v1,3(a0)` |
| 9 | `0801DF39` | `0801DF39` | `j 0x80077CE4` (relocation normalized) |
| 10 | `ACA00000` | `ACA00000` | `sw zero,0(a1)` |
| 11 | `2402FFFF` | `2402FFFF` | `li v0,-1` |
| 12 | `03E00008` | `03E00008` | `jr ra` |
| 13 | `00000000` | `00000000` | `nop` (return delay slot) |

Single-leaf object:

```text
00000000 <func_80077CB4>:
   0: 90820003  lbu    v0,3(a0)
   4: 90a30003  lbu    v1,3(a1)
   8: 00000000  nop
   c: 00431021  addu   v0,v0,v1
  10: 24430001  addiu  v1,v0,1
  14: 28620011  slti   v0,v1,17
  18: 10400004  beqz   v0,2c
  1c: 00001021  move   v0,zero
  20: a0830003  sb     v1,3(a0)
  24: 0800000c  j      30
      24: R_MIPS_26 .text
  28: aca00000  sw     zero,0(a1)
  2c: 2402ffff  li     v0,-1
  30: 03e00008  jr     ra
  34: 00000000  nop
```

The trim guard proves all gas-alignment bytes beyond the `0x38` body are
zero.

## Carve geometry and packed-span proof

The prior asm span was `[0x68478,0x6AB24)`, size `0x26AC`:

```text
prefix asm:  0x684B4 - 0x68478 = 0x003C
C leaf:      0x684EC - 0x684B4 = 0x0038
resume asm:  0x6AB24 - 0x684EC = 0x2638
closure:     0x003C + 0x0038 + 0x2638 = 0x26AC
```

The prefix keeps the leading alignment nop; the resume keeps both trailing
alignment nops. Retail and packed candidate are identical across both real
boundaries:

```text
retail:
000684a4: 25106200 0800e003 040082ac 00000000
000684b4: 03008290 0300a390 00000000 21104300
000684c4: 01004324 11006228 04004010 21100000
000684d4: 030083a0 39df0108 0000a0ac ffff0224
000684e4: 0800e003 00000000 00000000 00000000
000684f4: e8ffbd27 05008004

candidate:
000684a4: 25106200 0800e003 040082ac 00000000
000684b4: 03008290 0300a390 00000000 21104300
000684c4: 01004324 11006228 04004010 21100000
000684d4: 030083a0 39df0108 0000a0ac ffff0224
000684e4: 0800e003 00000000 00000000 00000000
000684f4: e8ffbd27 05008004
```

## Correction to older evidence

`docs/evidence/pe-b54i-gpu-primitive-leaves/REPORT.md` originally described
this as thirteen words and treated `0x80077CE8` as external alignment. That
was incorrect boundary ownership: `0x80077CE8` executes as the `jr ra` delay
slot. The matching span and the corrected older report now count fourteen
words; only `0x80077CEC/0x80077CF0` are trailing padding.

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
329
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 329 leaves
```

Result: `MATCHED=14/14` after relocation normalization, second phrasing;
consecutive parks remain zero.
