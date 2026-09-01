# `func_8004B534` — exact callback-registration wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf **397**.

## Function hood first

- File `[0x3BD34,0x3BD5C)`, VA `[0x8004B534,0x8004B55C)`: `0x28`, ten words.
- Files `0x3B9A8/0x3B9AC` materialize the exact start and its owner stores it
  into callback field `+0x30`.
- Canonical return is `jr ra` at `0x3BD54`, with `nop` at `0x3BD58`.
- Preceding real `func_8004B394` ends at `0x3BD2C/0x3BD30` with `jr ra; nop`.
- Following real `func_8004B55C` starts at `0x3BD5C` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

## Retail body and screens

```text
3BD34 8004B534 27BDFFE8  addiu sp,sp,-0x18
3BD38 8004B538 AFBF0010  sw    ra,0x10(sp)
3BD3C 8004B53C 3C058005  lui   a1,%hi(func_80050438)
3BD40 8004B540 24A50438  addiu a1,a1,%lo(func_80050438)
3BD44 8004B544 0C018E36  jal   func_800638D8
3BD48 8004B548 00000000  nop
3BD4C 8004B54C 8FBF0010  lw    ra,0x10(sp)
3BD50 8004B550 27BD0018  addiu sp,sp,0x18
3BD54 8004B554 03E00008  jr    ra
3BD58 8004B558 00000000  nop
```

| screen | result |
|---|---|
| Callee bucket | one direct unresolved callee, `func_800638D8`; forwards slot `$a0` and callback `func_80050438` in `$a1` |
| Stage-0 globals | none in the wrapper |
| Coloring pressure | minimal; unchanged `$a0`, symbolic callback in `$a1` |
| `$v0` liveness | callee result discarded; void wrapper |
| Address retention | none |
| `-O` signal | compact call frame and symbolic callback materialization select era `-O2` |
| Loop/back-edge | none |

No gp-relative access appears, selecting `-G0`.

## Minimal C and single-leaf comparison

```c
void func_80050438(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004B534(int slot) {
    func_800638D8(slot, func_80050438);
}
```

```text
00000000 <func_8004B534>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 3c050000  lui   a1,0            R_MIPS_HI16 func_80050438
   c: 24a50000  addiu a1,a1,0        R_MIPS_LO16 func_80050438
  10: 0c000000  jal   0              R_MIPS_26 func_800638D8
  14: 00000000  nop
  18: 8fbf0010  lw    ra,16(sp)
  1c: 27bd0018  addiu sp,sp,24
  20: 03e00008  jr    ra
  24: 00000000  nop

ROM: 27bdffe8 afbf0010 3c058005 24a50438 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 3c058005 24a50438 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_HI16/R_MIPS_LO16 func_80050438; R_MIPS_26 func_800638D8
BYTE_EXACT=10/10
PHRASINGS_USED=1/2
```

Compile flags: era GCC 2.7.2 `-O2 -G0`; no pins, inline/file-scope assembly,
fabricated nops, or maspsx gates.

## Carve geometry

The prior active asm span was `[0x37CD0,0x3C708)` = `0x4A38`:

```text
asm prefix: 0x3BD34 - 0x37CD0 = 0x4064
C leaf:     0x3BD5C - 0x3BD34 = 0x0028
asm resume: 0x3C708 - 0x3BD5C = 0x09AC
closure:    0x4064 + 0x0028 + 0x09AC = 0x4A38
```

The carve creates `3BD5C.s`; sizes derive only from retail boundaries.

## Packed span and gates

```text
file    VA        retail   candidate  role
3BD24 8004B524  1000b08f 1000b08f   preceding s0 restore
3BD28 8004B528  2000bd27 2000bd27   preceding teardown
3BD2C 8004B52C  0800e003 0800e003   preceding return
3BD30 8004B530  00000000 00000000   preceding delay
3BD34 8004B534  e8ffbd27 e8ffbd27   leaf 1
3BD38 8004B538  1000bfaf 1000bfaf   leaf 2
3BD3C 8004B53C  0580053c 0580053c   leaf 3
3BD40 8004B540  3804a524 3804a524   leaf 4
3BD44 8004B544  368e010c 368e010c   leaf 5
3BD48 8004B548  00000000 00000000   leaf 6
3BD4C 8004B54C  1000bf8f 1000bf8f   leaf 7
3BD50 8004B550  1800bd27 1800bd27   leaf 8
3BD54 8004B554  0800e003 0800e003   leaf 9
3BD58 8004B558  00000000 00000000   leaf 10
3BD5C 8004B55C  e8ffbd27 e8ffbd27   following real entry
3BD60 8004B560  1000bfaf 1000bfaf   following prologue
3BD64 8004B564  0580053c 0580053c   following body
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
C conversion: Phase 5HD-12850 — 397 leaves

grep matching-C count: 397
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
