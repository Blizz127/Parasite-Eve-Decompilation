# `func_8004FF58` — exact callback-registration wrapper

Outcome: **MATCHED**, natural phrasing 1, era `-O2 -G0`; matching-C leaf
**404**.

## Hood and screens

- File `[0x40758,0x40780)`, VA `[0x8004FF58,0x8004FF80)`, ten words.
- `0x3B780/0x3B784` materializes the exact start; `0x3B78C` stores it to
  callback field `0x30($a0)` in a real registrar call's delay slot.
- Canonical return is `0x40778/0x4077C`; real neighbors end at `0x40754`
  and start at `0x40780`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

```text
40758 27bdffe8  addiu sp,sp,-0x18
4075C afbf0010  sw    ra,0x10(sp)
40760 3c058005  lui   a1,%hi(func_80050C70)
40764 24a50c70  addiu a1,a1,%lo(func_80050C70)
40768 0c018e36  jal   func_800638D8
4076C 00000000  nop
40770 8fbf0010  lw    ra,0x10(sp)
40774 27bd0018  addiu sp,sp,0x18
40778 03e00008  jr    ra
4077C 00000000  nop
```

| screen | result |
|---|---|
| callee | one `func_800638D8`; forwards `$a0` and `func_80050C70` |
| Stage-0 globals | none |
| coloring | minimal |
| `$v0` | discarded call result |
| address retention | none |
| `-O` | compact era `-O2` shape |
| loop | none |

No gp access selects `-G0`.

## C, object, and carve

```c
void func_80050C70(void);
void func_800638D8(int slot, void (*callback)(void));
void func_8004FF58(int slot) { func_800638D8(slot, func_80050C70); }
```

```text
object: 27bdffe8 afbf0010 3c050000 24a50000 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
relocs: R_MIPS_HI16/R_MIPS_LO16 func_80050C70; R_MIPS_26 func_800638D8
ROM:    27bdffe8 afbf0010 3c058005 24a50c70 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
linked: 27bdffe8 afbf0010 3c058005 24a50c70 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
BYTE_EXACT=10/10; PHRASINGS_USED=1/2
```

No pins, asm, fabricated nops, or maspsx gates.

```text
old span [0x40758,0x40820) = 0xC8
C:      0x40780 - 0x40758 = 0x28
resume: 0x40820 - 0x40780 = 0xA0
closure: 0x28 + 0xA0 = 0xC8
```

## Packed span and gates

```text
file    retail   candidate  role
40748   1000bf8f 1000bf8f   predecessor restore
4074C   1800bd27 1800bd27   predecessor teardown
40750   0800e003 0800e003   predecessor return
40754   00000000 00000000   predecessor delay
40758   e8ffbd27 e8ffbd27   leaf 1
4075C   1000bfaf 1000bfaf   leaf 2
40760   0580053c 0580053c   leaf 3
40764   700ca524 700ca524   leaf 4
40768   368e010c 368e010c   leaf 5
4076C   00000000 00000000   leaf 6
40770   1000bf8f 1000bf8f   leaf 7
40774   1800bd27 1800bd27   leaf 8
40778   0800e003 0800e003   leaf 9
4077C   00000000 00000000   leaf 10
40780   e8ffbd27 e8ffbd27   successor entry
40784   1000bfaf 1000bfaf   successor body
40788   0580053c 0580053c   successor body
4078C   b40ca524 b40ca524   successor body
PACKED_SPAN=EXACT

BUILD_RC=0; RESULT: EXACT MATCH
SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VERIFY_RC=0; compare: EXACT MATCH; matching-C count: 404
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
