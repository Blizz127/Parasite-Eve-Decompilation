# `func_8004FFA8` — exact callback-registration wrapper

Outcome: **MATCHED**, natural phrasing 1, era `-O2 -G0`; matching-C leaf
**406**.

## Hood and screens

- File `[0x407A8,0x407D0)`, VA `[0x8004FFA8,0x8004FFD0)`, ten words.
- One of several independent references materializes its exact start at
  `0x38908/0x3890C` and stores it to callback field `0x30($s1)` at `0x38914`.
- Canonical return is `0x407C8/0x407CC`; real wrappers end at `0x407A4` and
  start at `0x407D0`.

`FUNCTION_HOOD=PROVEN_BY_INDEPENDENT_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

```text
407A8 27bdffe8  addiu sp,sp,-0x18
407AC afbf0010  sw    ra,0x10(sp)
407B0 3c058005  lui   a1,%hi(func_80050CF8)
407B4 24a50cf8  addiu a1,a1,%lo(func_80050CF8)
407B8 0c018e36  jal   func_800638D8
407BC 00000000  nop
407C0 8fbf0010  lw    ra,0x10(sp)
407C4 27bd0018  addiu sp,sp,0x18
407C8 03e00008  jr    ra
407CC 00000000  nop
```

| screen | result |
|---|---|
| callee | one `func_800638D8`; forwards `$a0` and `func_80050CF8` |
| Stage-0 globals | none |
| coloring | minimal |
| `$v0` | discarded result |
| retention | none |
| `-O` | compact era `-O2` shape |
| loop | none |

No gp access selects `-G0`.

## C, isolated object, and carve

```c
void func_80050CF8(void);
void func_800638D8(int slot, void (*callback)(void));
void func_8004FFA8(int slot) { func_800638D8(slot, func_80050CF8); }
```

```text
object: 27bdffe8 afbf0010 3c050000 24a50000 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
relocs: R_MIPS_HI16/R_MIPS_LO16 func_80050CF8; R_MIPS_26 func_800638D8
ROM:    27bdffe8 afbf0010 3c058005 24a50cf8 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
linked: 27bdffe8 afbf0010 3c058005 24a50cf8 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
BYTE_EXACT=10/10; PHRASINGS_USED=1/2
```

No pins, asm, fabricated nops, or maspsx gates.

```text
old span [0x407A8,0x40820) = 0x78
C:      0x407D0 - 0x407A8 = 0x28
resume: 0x40820 - 0x407D0 = 0x50
closure: 0x28 + 0x50 = 0x78
```

## Packed span and gates

```text
file    retail   candidate  role
40798   1000bf8f 1000bf8f   predecessor restore
4079C   1800bd27 1800bd27   predecessor teardown
407A0   0800e003 0800e003   predecessor return
407A4   00000000 00000000   predecessor delay
407A8   e8ffbd27 e8ffbd27   leaf 1
407AC   1000bfaf 1000bfaf   leaf 2
407B0   0580053c 0580053c   leaf 3
407B4   f80ca524 f80ca524   leaf 4
407B8   368e010c 368e010c   leaf 5
407BC   00000000 00000000   leaf 6
407C0   1000bf8f 1000bf8f   leaf 7
407C4   1800bd27 1800bd27   leaf 8
407C8   0800e003 0800e003   leaf 9
407CC   00000000 00000000   leaf 10
407D0   e8ffbd27 e8ffbd27   successor entry
407D4   1000bfaf 1000bfaf   successor body
407D8   0580053c 0580053c   successor body
407DC   180da524 180da524   successor body
PACKED_SPAN=EXACT

BUILD_RC=0; RESULT: EXACT MATCH
SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VERIFY_RC=0; compare: EXACT MATCH; matching-C count: 406
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
