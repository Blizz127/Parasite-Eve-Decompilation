# `func_800474A8` — exact callback-registration wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf **396**.

## Function hood first

- File `[0x37CA8,0x37CD0)`, VA `[0x800474A8,0x800474D0)`: `0x28`, ten
  words.
- Files `0x37C34/0x37C38` materialize the exact start and file `0x37C40`
  stores it into callback field `+0x30`.
- Canonical return is `jr ra` at `0x37CC8`, with `nop` at `0x37CCC`.
- Preceding real `func_800473E4` ends at `0x37CA0/0x37CA4` with `jr ra; nop`.
- Following real `func_800474D0` starts at `0x37CD0` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

## Retail body and screens

```text
37CA8 800474A8 27BDFFE8  addiu sp,sp,-0x18
37CAC 800474AC AFBF0010  sw    ra,0x10(sp)
37CB0 800474B0 3C058005  lui   a1,%hi(func_80050308)
37CB4 800474B4 24A50308  addiu a1,a1,%lo(func_80050308)
37CB8 800474B8 0C018E36  jal   func_800638D8
37CBC 800474BC 00000000  nop
37CC0 800474C0 8FBF0010  lw    ra,0x10(sp)
37CC4 800474C4 27BD0018  addiu sp,sp,0x18
37CC8 800474C8 03E00008  jr    ra
37CCC 800474CC 00000000  nop
```

| screen | result |
|---|---|
| Callee bucket | one direct unresolved callee, `func_800638D8`; incoming slot `$a0` is forwarded and `func_80050308` is callback `$a1` |
| Stage-0 globals | none; this wrapper only forwards a callback and slot |
| Coloring pressure | minimal; unchanged `$a0`, symbolic function pointer in `$a1` |
| `$v0` liveness | callee result discarded; void wrapper |
| Address retention | none |
| `-O` signal | compact frame and stable callback materialization select era `-O2` |
| Loop/back-edge | none |

No gp-relative access appears, selecting `-G0`.

## Minimal C and single-leaf comparison

```c
void func_80050308(void);
void func_800638D8(int slot, void (*callback)(void));

void func_800474A8(int slot) {
    func_800638D8(slot, func_80050308);
}
```

```text
00000000 <func_800474A8>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 3c050000  lui   a1,0            R_MIPS_HI16 func_80050308
   c: 24a50000  addiu a1,a1,0        R_MIPS_LO16 func_80050308
  10: 0c000000  jal   0              R_MIPS_26 func_800638D8
  14: 00000000  nop
  18: 8fbf0010  lw    ra,16(sp)
  1c: 27bd0018  addiu sp,sp,24
  20: 03e00008  jr    ra
  24: 00000000  nop

ROM: 27bdffe8 afbf0010 3c058005 24a50308 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 3c058005 24a50308 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_HI16/R_MIPS_LO16 func_80050308; R_MIPS_26 func_800638D8
BYTE_EXACT=10/10
PHRASINGS_USED=1/2
```

Compile flags: era GCC 2.7.2 `-O2 -G0`; no pins, inline/file-scope assembly,
fabricated nops, or maspsx gates.

## Carve geometry

The prior active asm span was `[0x37B54,0x3C708)` = `0x4BB4`:

```text
asm prefix: 0x37CA8 - 0x37B54 = 0x0154
C leaf:     0x37CD0 - 0x37CA8 = 0x0028
asm resume: 0x3C708 - 0x37CD0 = 0x4A38
closure:    0x0154 + 0x0028 + 0x4A38 = 0x4BB4
```

The carve creates `37CD0.s`; sizes derive only from retail boundaries.

## Packed span and gates

```text
file    VA        retail   candidate  role
37C98 80047498  1000b08f 1000b08f   preceding s0 restore
37C9C 8004749C  2000bd27 2000bd27   preceding teardown
37CA0 800474A0  0800e003 0800e003   preceding return
37CA4 800474A4  00000000 00000000   preceding delay
37CA8 800474A8  e8ffbd27 e8ffbd27   leaf 1
37CAC 800474AC  1000bfaf 1000bfaf   leaf 2
37CB0 800474B0  0580053c 0580053c   leaf 3
37CB4 800474B4  0803a524 0803a524   leaf 4
37CB8 800474B8  368e010c 368e010c   leaf 5
37CBC 800474BC  00000000 00000000   leaf 6
37CC0 800474C0  1000bf8f 1000bf8f   leaf 7
37CC4 800474C4  1800bd27 1800bd27   leaf 8
37CC8 800474C8  0800e003 0800e003   leaf 9
37CCC 800474CC  00000000 00000000   leaf 10
37CD0 800474D0  e8ffbd27 e8ffbd27   following real entry
37CD4 800474D4  0100023c 0100023c   following body
37CD8 800474D8  2410a200 2410a200   following body
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
C conversion: Phase 5HD-12850 — 396 leaves

grep matching-C count: 396
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
