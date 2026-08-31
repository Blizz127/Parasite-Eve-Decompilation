# `func_800471BC` — exact callback-registration wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf **394**.

## Function hood first

- File `[0x379BC,0x379E4)`, VA `[0x800471BC,0x800471E4)`: `0x28`, ten
  words.
- Two independent exact-start references materialize `func_800471BC` at
  files `0x37650/0x37654` and `0x3F594/0x3F598`, then store it into callback
  field `+0x30` of separate allocated objects.
- Canonical return is `jr ra` at `0x379DC` with `nop` at `0x379E0`.
- Preceding real `func_80047040` ends at `0x379B4/0x379B8` with `jr ra; nop`.
- Following real `func_800471E4` starts at `0x379E4` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_TWO_INDEPENDENT_EXACT_START_CALLBACK_STORES_AND_CANONICAL_RETURN`.

## Retail body and screens

```text
379BC 800471BC 27BDFFE8  addiu sp,sp,-0x18
379C0 800471C0 AFBF0010  sw    ra,0x10(sp)
379C4 800471C4 3C058004  lui   a1,%hi(func_80047040)
379C8 800471C8 24A57040  addiu a1,a1,%lo(func_80047040)
379CC 800471CC 0C018E36  jal   func_800638D8
379D0 800471D0 00000000  nop
379D4 800471D4 8FBF0010  lw    ra,0x10(sp)
379D8 800471D8 27BD0018  addiu sp,sp,0x18
379DC 800471DC 03E00008  jr    ra
379E0 800471E0 00000000  nop
```

| screen | result |
|---|---|
| Callee bucket | one direct unresolved callee, `func_800638D8`; incoming slot `$a0` is forwarded and `func_80047040` is supplied as callback `$a1` |
| Stage-0 globals | none in the wrapper; callback ownership is proven by the exact-start stores in its object users |
| Coloring pressure | minimal; `$a0` is untouched and the symbolic callback occupies `$a1` |
| `$v0` liveness | callee result discarded; void wrapper |
| Address retention | none |
| `-O` signal | compact frame and stable symbolic function-pointer materialization select era `-O2` |
| Loop/back-edge | none |

No gp-relative access appears, selecting `-G0`.

## Minimal C and single-leaf comparison

```c
void func_80047040(void);
void func_800638D8(int slot, void (*callback)(void));

void func_800471BC(int slot) {
    func_800638D8(slot, func_80047040);
}
```

```text
00000000 <func_800471BC>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 3c050000  lui   a1,0            R_MIPS_HI16 func_80047040
   c: 24a50000  addiu a1,a1,0        R_MIPS_LO16 func_80047040
  10: 0c000000  jal   0              R_MIPS_26 func_800638D8
  14: 00000000  nop
  18: 8fbf0010  lw    ra,16(sp)
  1c: 27bd0018  addiu sp,sp,24
  20: 03e00008  jr    ra
  24: 00000000  nop

ROM: 27bdffe8 afbf0010 3c058004 24a57040 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 3c058004 24a57040 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_HI16/R_MIPS_LO16 func_80047040; R_MIPS_26 func_800638D8
BYTE_EXACT=10/10
PHRASINGS_USED=1/2
```

Compile flags: era GCC 2.7.2 `-O2 -G0`; no pins, inline/file-scope assembly,
fabricated nops, or maspsx gates.

## Carve geometry

The former active asm span was `[0x340EC,0x3C708)` = `0x861C`:

```text
asm prefix: 0x379BC - 0x340EC = 0x38D0
C leaf:     0x379E4 - 0x379BC = 0x0028
asm resume: 0x3C708 - 0x379E4 = 0x4D24
closure:    0x38D0 + 0x0028 + 0x4D24 = 0x861C
```

The carve creates `379E4.s`; sizes derive only from retail boundaries.

## Packed span and gates

```text
file    VA        retail   candidate  role
379AC 800471AC  1000b08f 1000b08f   preceding s0 restore
379B0 800471B0  1800bd27 1800bd27   preceding teardown
379B4 800471B4  0800e003 0800e003   preceding return
379B8 800471B8  00000000 00000000   preceding delay
379BC 800471BC  e8ffbd27 e8ffbd27   leaf 1
379C0 800471C0  1000bfaf 1000bfaf   leaf 2
379C4 800471C4  0480053c 0480053c   leaf 3
379C8 800471C8  4070a524 4070a524   leaf 4
379CC 800471CC  368e010c 368e010c   leaf 5
379D0 800471D0  00000000 00000000   leaf 6
379D4 800471D4  1000bf8f 1000bf8f   leaf 7
379D8 800471D8  1800bd27 1800bd27   leaf 8
379DC 800471DC  0800e003 0800e003   leaf 9
379E0 800471E0  00000000 00000000   leaf 10
379E4 800471E4  e8ffbd27 e8ffbd27   following real entry
379E8 800471E8  0100023c 0100023c   following body
379EC 800471EC  2410a200 2410a200   following body
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
C conversion: Phase 5HD-12850 — 394 leaves

grep matching-C count: 394
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
