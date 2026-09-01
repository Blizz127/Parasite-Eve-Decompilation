# `func_80050038` — exact callback-registration wrapper

Outcome: **MATCHED**, natural phrasing 1, era `-O2 -G0`; matching-C leaf
**409**.

## Hood and screens

- File `[0x40838,0x40860)`, VA `[0x80050038,0x80050060)`, ten words.
- `0x38C14/0x38C18` materializes its exact start and `0x38C1C` stores it to
  callback field `0x30($a0)`.
- Canonical return is `0x40858/0x4085C`; real predecessor `func_80050020`
  ends at `0x40830/0x40834`, and real successor starts at `0x40860`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

```text
40838 27bdffe8  addiu sp,sp,-0x18
4083C afbf0010  sw    ra,0x10(sp)
40840 3c058005  lui   a1,%hi(func_80050DC0)
40844 24a50dc0  addiu a1,a1,%lo(func_80050DC0)
40848 0c018e36  jal   func_800638D8
4084C 00000000  nop
40850 8fbf0010  lw    ra,0x10(sp)
40854 27bd0018  addiu sp,sp,0x18
40858 03e00008  jr    ra
4085C 00000000  nop
```

| screen | result |
|---|---|
| callee | one `func_800638D8`; forwards `$a0` and `func_80050DC0` |
| Stage-0 globals | none |
| coloring | minimal |
| `$v0` | discarded result |
| retention | none |
| `-O` | compact era `-O2` shape |
| loop | none |

No gp access selects `-G0`.

## C, object, carve, and packed span

```c
void func_80050DC0(void);
void func_800638D8(int slot, void (*callback)(void));
void func_80050038(int slot) { func_800638D8(slot, func_80050DC0); }
```

```text
object: 27bdffe8 afbf0010 3c050000 24a50000 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
relocs: R_MIPS_HI16/R_MIPS_LO16 func_80050DC0; R_MIPS_26 func_800638D8
ROM:    27bdffe8 afbf0010 3c058005 24a50dc0 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
linked: 27bdffe8 afbf0010 3c058005 24a50dc0 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
BYTE_EXACT=10/10; PHRASINGS_USED=1/2
old span [0x40838,0x40888)=0x50; C 0x28 + resume 0x28 = 0x50
```

No pins, asm, fabricated nops, or maspsx gates.

```text
file    retail   candidate  role
40828   21082400 21082400   predecessor body
4082C   8818228c 8818228c   predecessor body
40830   0800e003 0800e003   predecessor return
40834   00000000 00000000   predecessor delay
40838   e8ffbd27 e8ffbd27   leaf 1
4083C   1000bfaf 1000bfaf   leaf 2
40840   0580053c 0580053c   leaf 3
40844   c00da524 c00da524   leaf 4
40848   368e010c 368e010c   leaf 5
4084C   00000000 00000000   leaf 6
40850   1000bf8f 1000bf8f   leaf 7
40854   1800bd27 1800bd27   leaf 8
40858   0800e003 0800e003   leaf 9
4085C   00000000 00000000   leaf 10
40860   e8ffbd27 e8ffbd27   successor entry
40864   1000bfaf 1000bfaf   successor body
40868   0580053c 0580053c   successor body
4086C   700ea524 700ea524   successor body
PACKED_SPAN=EXACT
BUILD_RC=0; RESULT: EXACT MATCH
SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VERIFY_RC=0; compare: EXACT MATCH; matching-C count: 409
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
