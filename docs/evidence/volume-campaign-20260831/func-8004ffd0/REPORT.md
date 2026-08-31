# `func_8004FFD0` — exact callback-registration wrapper

Outcome: **MATCHED**, natural phrasing 1, era `-O2 -G0`; matching-C leaf
**407**.

## Hood and screens

- File `[0x407D0,0x407F8)`, VA `[0x8004FFD0,0x8004FFF8)`, ten words.
- `0x35CF4/0x35CF8` materializes its exact start; `0x35D00` stores it to
  callback field `0x30($a0)`.
- Canonical return is `0x407F0/0x407F4`; real wrappers end at `0x407CC` and
  start at `0x407F8`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

```text
407D0 27bdffe8  addiu sp,sp,-0x18
407D4 afbf0010  sw    ra,0x10(sp)
407D8 3c058005  lui   a1,%hi(func_80050D18)
407DC 24a50d18  addiu a1,a1,%lo(func_80050D18)
407E0 0c018e36  jal   func_800638D8
407E4 00000000  nop
407E8 8fbf0010  lw    ra,0x10(sp)
407EC 27bd0018  addiu sp,sp,0x18
407F0 03e00008  jr    ra
407F4 00000000  nop
```

| screen | result |
|---|---|
| callee | one `func_800638D8`; forwards `$a0` and `func_80050D18` |
| Stage-0 globals | none |
| coloring | minimal |
| `$v0` | discarded result |
| retention | none |
| `-O` | compact era `-O2` shape |
| loop | none |

No gp access selects `-G0`.

## C, isolated object, and carve

```c
void func_80050D18(void);
void func_800638D8(int slot, void (*callback)(void));
void func_8004FFD0(int slot) { func_800638D8(slot, func_80050D18); }
```

```text
object: 27bdffe8 afbf0010 3c050000 24a50000 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
relocs: R_MIPS_HI16/R_MIPS_LO16 func_80050D18; R_MIPS_26 func_800638D8
ROM:    27bdffe8 afbf0010 3c058005 24a50d18 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
linked: 27bdffe8 afbf0010 3c058005 24a50d18 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
BYTE_EXACT=10/10; PHRASINGS_USED=1/2
```

No pins, asm, fabricated nops, or maspsx gates.

```text
old span [0x407D0,0x40820) = 0x50
C:      0x407F8 - 0x407D0 = 0x28
resume: 0x40820 - 0x407F8 = 0x28
closure: 0x28 + 0x28 = 0x50
```

## Packed span and gates

```text
file    retail   candidate  role
407C0   1000bf8f 1000bf8f   predecessor restore
407C4   1800bd27 1800bd27   predecessor teardown
407C8   0800e003 0800e003   predecessor return
407CC   00000000 00000000   predecessor delay
407D0   e8ffbd27 e8ffbd27   leaf 1
407D4   1000bfaf 1000bfaf   leaf 2
407D8   0580053c 0580053c   leaf 3
407DC   180da524 180da524   leaf 4
407E0   368e010c 368e010c   leaf 5
407E4   00000000 00000000   leaf 6
407E8   1000bf8f 1000bf8f   leaf 7
407EC   1800bd27 1800bd27   leaf 8
407F0   0800e003 0800e003   leaf 9
407F4   00000000 00000000   leaf 10
407F8   e8ffbd27 e8ffbd27   successor entry
407FC   1000bfaf 1000bfaf   successor body
40800   0580053c 0580053c   successor body
40804   200da524 200da524   successor body
PACKED_SPAN=EXACT

BUILD_RC=0; RESULT: EXACT MATCH
SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VERIFY_RC=0; compare: EXACT MATCH; matching-C count: 407
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
