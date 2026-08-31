# `func_8004FFF8` — exact callback-registration wrapper

Outcome: **MATCHED**, natural phrasing 1, era `-O2 -G0`; matching-C leaf
**408**.

## Hood and screens

- File `[0x407F8,0x40820)`, VA `[0x8004FFF8,0x80050020)`, ten words.
- `0x3C69C/0x3C6A0` materializes the exact start and `0x3C6A4` stores it to
  callback field `0x30($v0)`.
- Canonical return is `0x40818/0x4081C`; real predecessor ends at `0x407F4`
  and already-matched real successor `func_80050020` starts at `0x40820`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

```text
407F8 27bdffe8  addiu sp,sp,-0x18
407FC afbf0010  sw    ra,0x10(sp)
40800 3c058005  lui   a1,%hi(func_80050D20)
40804 24a50d20  addiu a1,a1,%lo(func_80050D20)
40808 0c018e36  jal   func_800638D8
4080C 00000000  nop
40810 8fbf0010  lw    ra,0x10(sp)
40814 27bd0018  addiu sp,sp,0x18
40818 03e00008  jr    ra
4081C 00000000  nop
```

| screen | result |
|---|---|
| callee | one `func_800638D8`; forwards `$a0` and `func_80050D20` |
| Stage-0 globals | none |
| coloring | minimal |
| `$v0` | discarded result |
| retention | none |
| `-O` | compact era `-O2` shape |
| loop | none |

No gp access selects `-G0`.

## C, isolated object, and carve

```c
void func_80050D20(void);
void func_800638D8(int slot, void (*callback)(void));
void func_8004FFF8(int slot) { func_800638D8(slot, func_80050D20); }
```

```text
object: 27bdffe8 afbf0010 3c050000 24a50000 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
relocs: R_MIPS_HI16/R_MIPS_LO16 func_80050D20; R_MIPS_26 func_800638D8
ROM:    27bdffe8 afbf0010 3c058005 24a50d20 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
linked: 27bdffe8 afbf0010 3c058005 24a50d20 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
BYTE_EXACT=10/10; PHRASINGS_USED=1/2
```

No pins, asm, fabricated nops, or maspsx gates.

```text
old span [0x407F8,0x40820) = 0x28
C:        0x40820 - 0x407F8 = 0x28
resume:   none; next segment already starts at 0x40820
closure:  0x28 = 0x28
```

## Packed span and gates

```text
file    retail   candidate  role
407E8   1000bf8f 1000bf8f   predecessor restore
407EC   1800bd27 1800bd27   predecessor teardown
407F0   0800e003 0800e003   predecessor return
407F4   00000000 00000000   predecessor delay
407F8   e8ffbd27 e8ffbd27   leaf 1
407FC   1000bfaf 1000bfaf   leaf 2
40800   0580053c 0580053c   leaf 3
40804   200da524 200da524   leaf 4
40808   368e010c 368e010c   leaf 5
4080C   00000000 00000000   leaf 6
40810   1000bf8f 1000bf8f   leaf 7
40814   1800bd27 1800bd27   leaf 8
40818   0800e003 0800e003   leaf 9
4081C   00000000 00000000   leaf 10
40820   80200400 80200400   successor entry
40824   0a80013c 0a80013c   successor body
40828   21082400 21082400   successor body
4082C   8818228c 8818228c   successor body
PACKED_SPAN=EXACT

BUILD_RC=0; RESULT: EXACT MATCH
SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VERIFY_RC=0; compare: EXACT MATCH; matching-C count: 408
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
