# `func_8004F950` — exact callback-registration wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf **401**.

## Function hood first

- File `[0x40150,0x40178)`, VA `[0x8004F950,0x8004F978)`: `0x28`, ten words.
- Among multiple independent references, `0x3E33C/0x3E340` materializes the
  exact start and `0x3E344` stores it to callback field `0x30($s1)`.
- Canonical return is `jr ra` at `0x40170`, with `nop` at `0x40174`.
- Preceding real `func_8004F910` ends at `0x40148/0x4014C`; following real
  `func_8004F978` starts at `0x40178` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_INDEPENDENT_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

```text
3E33C 8004DB3C 3C028005  lui   v0,%hi(func_8004F950)
3E340 8004DB40 2442F950  addiu v0,v0,%lo(func_8004F950)
3E344 8004DB44 AE220030  sw    v0,0x30(s1)
```

## Retail body and screens

```text
40150 8004F950 27BDFFE8  addiu sp,sp,-0x18
40154 8004F954 AFBF0010  sw    ra,0x10(sp)
40158 8004F958 3C058005  lui   a1,%hi(func_800509A8)
4015C 8004F95C 24A509A8  addiu a1,a1,%lo(func_800509A8)
40160 8004F960 0C018E36  jal   func_800638D8
40164 8004F964 00000000  nop
40168 8004F968 8FBF0010  lw    ra,0x10(sp)
4016C 8004F96C 27BD0018  addiu sp,sp,0x18
40170 8004F970 03E00008  jr    ra
40174 8004F974 00000000  nop
```

| screen | result |
|---|---|
| Callee bucket | one direct callee, `func_800638D8`; forwards slot `$a0` and callback `func_800509A8` in `$a1` |
| Stage-0 globals | none in the wrapper |
| Coloring pressure | minimal; unchanged `$a0`, symbolic callback in `$a1` |
| `$v0` liveness | callee result discarded; void wrapper |
| Address retention | none |
| `-O` signal | compact frame and callback materialization select era `-O2` |
| Loop/back-edge | none |

No gp-relative access appears, selecting `-G0`.

## Minimal C and single-leaf comparison

```c
void func_800509A8(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004F950(int slot) {
    func_800638D8(slot, func_800509A8);
}
```

```text
00000000 <func_8004F950>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 3c050000  lui   a1,0            R_MIPS_HI16 func_800509A8
   c: 24a50000  addiu a1,a1,0        R_MIPS_LO16 func_800509A8
  10: 0c000000  jal   0              R_MIPS_26 func_800638D8
  14: 00000000  nop
  18: 8fbf0010  lw    ra,16(sp)
  1c: 27bd0018  addiu sp,sp,24
  20: 03e00008  jr    ra
  24: 00000000  nop

ROM: 27bdffe8 afbf0010 3c058005 24a509a8 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 3c058005 24a509a8 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_HI16/R_MIPS_LO16 func_800509A8; R_MIPS_26 func_800638D8
BYTE_EXACT=10/10
PHRASINGS_USED=1/2
```

Compile flags: era GCC 2.7.2 `-O2 -G0`; no pins, inline/file-scope assembly,
fabricated nops, or maspsx gates.

## Carve geometry

The prior active asm span was `[0x40038,0x40820)` = `0x7E8`:

```text
asm prefix: 0x40150 - 0x40038 = 0x118
C leaf:     0x40178 - 0x40150 = 0x028
asm resume: 0x40820 - 0x40178 = 0x6A8
closure:    0x118 + 0x028 + 0x6A8 = 0x7E8
```

All sizes derive from retail boundaries, not aligned object sizes.

## Packed span and gates

```text
file    VA        retail   candidate  role
40140 8004F940  1000b08f 1000b08f   preceding restore
40144 8004F944  1800bd27 1800bd27   preceding teardown
40148 8004F948  0800e003 0800e003   preceding return
4014C 8004F94C  00000000 00000000   preceding delay
40150 8004F950  e8ffbd27 e8ffbd27   leaf 1
40154 8004F954  1000bfaf 1000bfaf   leaf 2
40158 8004F958  0580053c 0580053c   leaf 3
4015C 8004F95C  a809a524 a809a524   leaf 4
40160 8004F960  368e010c 368e010c   leaf 5
40164 8004F964  00000000 00000000   leaf 6
40168 8004F968  1000bf8f 1000bf8f   leaf 7
4016C 8004F96C  1800bd27 1800bd27   leaf 8
40170 8004F970  0800e003 0800e003   leaf 9
40174 8004F974  00000000 00000000   leaf 10
40178 8004F978  e8ffbd27 e8ffbd27   following real entry
4017C 8004F97C  1000bfaf 1000bfaf   following body
40180 8004F980  0580053c 0580053c   following body
40184 8004F984  e009a524 e009a524   following body
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
C conversion: Phase 5HD-12850 — 401 leaves
grep matching-C count: 401
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
