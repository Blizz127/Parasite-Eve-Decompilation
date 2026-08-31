# `func_80050204` — exact callback-registration wrapper

Outcome: **MATCHED**, natural phrasing 1, era `-O2 -G0`; matching-C leaf
**411**.

## Hood and screens

- File `[0x40A04,0x40A2C)`, VA `[0x80050204,0x8005022C)`, ten words.
- `0x3FD1C/0x3FD20` materializes its exact start and `0x3FD28` stores it to
  callback field `0x30($s2)`.
- Canonical return is `0x40A24/0x40A28`; real predecessor ends at
  `0x409FC/0x40A00`, and real `func_8005022C` starts at `0x40A2C`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

```text
40A04 27bdffe8  addiu sp,sp,-0x18
40A08 afbf0010  sw    ra,0x10(sp)
40A0C 3c058005  lui   a1,%hi(func_80051060)
40A10 24a51060  addiu a1,a1,%lo(func_80051060)
40A14 0c018e36  jal   func_800638D8
40A18 00000000  nop
40A1C 8fbf0010  lw    ra,0x10(sp)
40A20 27bd0018  addiu sp,sp,0x18
40A24 03e00008  jr    ra
40A28 00000000  nop
```

| screen | result |
|---|---|
| callee | one `func_800638D8`; forwards `$a0` and already-matched `func_80051060` |
| Stage-0 globals | none |
| coloring | minimal |
| `$v0` | discarded result |
| retention | none |
| `-O` | compact era `-O2` shape |
| loop | none |

No gp access selects `-G0`.

## C, isolated object, and carve

```c
void func_80051060(void);
void func_800638D8(int slot, void (*callback)(void));
void func_80050204(int slot) { func_800638D8(slot, func_80051060); }
```

```text
object: 27bdffe8 afbf0010 3c050000 24a50000 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
relocs: R_MIPS_HI16/R_MIPS_LO16 func_80051060; R_MIPS_26 func_800638D8
ROM:    27bdffe8 afbf0010 3c058005 24a51060 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
linked: 27bdffe8 afbf0010 3c058005 24a51060 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
BYTE_EXACT=10/10; PHRASINGS_USED=1/2
```

No pins, asm, fabricated nops, or maspsx gates.

```text
old span [0x408A8,0x40A60) = 0x1B8
prefix: 0x40A04 - 0x408A8 = 0x15C
C:      0x40A2C - 0x40A04 = 0x028
resume: 0x40A60 - 0x40A2C = 0x034
closure: 0x15C + 0x028 + 0x034 = 0x1B8
```

## Packed span and gates

```text
file    retail   candidate  role
409F4   1000bf8f 1000bf8f   predecessor restore
409F8   1800bd27 1800bd27   predecessor teardown
409FC   0800e003 0800e003   predecessor return
40A00   00000000 00000000   predecessor delay
40A04   e8ffbd27 e8ffbd27   leaf 1
40A08   1000bfaf 1000bfaf   leaf 2
40A0C   0580053c 0580053c   leaf 3
40A10   6010a524 6010a524   leaf 4
40A14   368e010c 368e010c   leaf 5
40A18   00000000 00000000   leaf 6
40A1C   1000bf8f 1000bf8f   leaf 7
40A20   1800bd27 1800bd27   leaf 8
40A24   0800e003 0800e003   leaf 9
40A28   00000000 00000000   leaf 10
40A2C   e8ffbd27 e8ffbd27   successor entry
40A30   e801828f e801828f   successor gp load
40A34   0580053c 0580053c   successor body
40A38   d80aa524 d80aa524   successor body
PACKED_SPAN=EXACT

BUILD_RC=0; RESULT: EXACT MATCH
SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VERIFY_RC=0; compare: EXACT MATCH; matching-C count: 411
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
