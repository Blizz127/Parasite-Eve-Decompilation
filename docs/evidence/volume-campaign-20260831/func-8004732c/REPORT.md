# `func_8004732C` — exact callback-registration wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf **395**.

## Function hood first

- File `[0x37B2C,0x37B54)`, VA `[0x8004732C,0x80047354)`: `0x28`, ten
  words.
- Exact-start reference at files `0x37AB0/0x37AB4` materializes
  `func_8004732C` and stores it into callback field `+0x30` at `0x37ABC`.
- Canonical return is `jr ra` at `0x37B4C`, with `nop` at `0x37B50`.
- Preceding real `func_800471E4` ends at `0x37B24/0x37B28` with `jr ra; nop`.
- Following real `func_80047354` starts at `0x37B54` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

## Retail body and screens

```text
37B2C 8004732C 27BDFFE8  addiu sp,sp,-0x18
37B30 80047330 AFBF0010  sw    ra,0x10(sp)
37B34 80047334 3C058005  lui   a1,%hi(func_80050280)
37B38 80047338 24A50280  addiu a1,a1,%lo(func_80050280)
37B3C 8004733C 0C018E36  jal   func_800638D8
37B40 80047340 00000000  nop
37B44 80047344 8FBF0010  lw    ra,0x10(sp)
37B48 80047348 27BD0018  addiu sp,sp,0x18
37B4C 8004734C 03E00008  jr    ra
37B50 80047350 00000000  nop
```

| screen | result |
|---|---|
| Callee bucket | one direct unresolved callee, `func_800638D8`; incoming slot `$a0` is forwarded and `func_80050280` is supplied as callback `$a1` |
| Stage-0 globals | none in the wrapper; callback use is proven by its independent exact-start field store |
| Coloring pressure | minimal: unchanged `$a0` plus symbolic callback in `$a1` |
| `$v0` liveness | callee result discarded; void wrapper |
| Address retention | none |
| `-O` signal | compact frame and symbolic callback materialization select era `-O2` |
| Loop/back-edge | none |

No gp-relative access appears, selecting `-G0`.

## Minimal C and single-leaf comparison

```c
void func_80050280(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004732C(int slot) {
    func_800638D8(slot, func_80050280);
}
```

```text
00000000 <func_8004732C>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 3c050000  lui   a1,0            R_MIPS_HI16 func_80050280
   c: 24a50000  addiu a1,a1,0        R_MIPS_LO16 func_80050280
  10: 0c000000  jal   0              R_MIPS_26 func_800638D8
  14: 00000000  nop
  18: 8fbf0010  lw    ra,16(sp)
  1c: 27bd0018  addiu sp,sp,24
  20: 03e00008  jr    ra
  24: 00000000  nop

ROM: 27bdffe8 afbf0010 3c058005 24a50280 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 3c058005 24a50280 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_HI16/R_MIPS_LO16 func_80050280; R_MIPS_26 func_800638D8
BYTE_EXACT=10/10
PHRASINGS_USED=1/2
```

Compile flags: era GCC 2.7.2 `-O2 -G0`; no pins, inline/file-scope assembly,
fabricated nops, or maspsx gates.

## Carve geometry

The prior active asm span was `[0x379E4,0x3C708)` = `0x4D24`:

```text
asm prefix: 0x37B2C - 0x379E4 = 0x0148
C leaf:     0x37B54 - 0x37B2C = 0x0028
asm resume: 0x3C708 - 0x37B54 = 0x4BB4
closure:    0x0148 + 0x0028 + 0x4BB4 = 0x4D24
```

The carve creates `37B54.s`; sizes derive only from retail boundaries.

## Packed span and gates

```text
file    VA        retail   candidate  role
37B1C 8004731C  1000b08f 1000b08f   preceding s0 restore
37B20 80047320  1800bd27 1800bd27   preceding teardown
37B24 80047324  0800e003 0800e003   preceding return
37B28 80047328  00000000 00000000   preceding delay
37B2C 8004732C  e8ffbd27 e8ffbd27   leaf 1
37B30 80047330  1000bfaf 1000bfaf   leaf 2
37B34 80047334  0580053c 0580053c   leaf 3
37B38 80047338  8002a524 8002a524   leaf 4
37B3C 8004733C  368e010c 368e010c   leaf 5
37B40 80047340  00000000 00000000   leaf 6
37B44 80047344  1000bf8f 1000bf8f   leaf 7
37B48 80047348  1800bd27 1800bd27   leaf 8
37B4C 8004734C  0800e003 0800e003   leaf 9
37B50 80047350  00000000 00000000   leaf 10
37B54 80047354  e8ffbd27 e8ffbd27   following real entry
37B58 80047358  0100023c 0100023c   following body
37B5C 8004735C  2410a200 2410a200   following body
PACKED_SPAN=EXACT
```

```text
BUILD_RC=0
RESULT: EXACT MATCH
Compare: EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

VERIFY_RC=0
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 395 leaves

grep matching-C count: 395
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
