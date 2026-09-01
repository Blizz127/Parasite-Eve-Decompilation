# `func_80050060` — exact callback-registration wrapper

Outcome: **MATCHED**, natural phrasing 1, era `-O2 -G0`; matching-C leaf
**410**.

## Hood and screens

- File `[0x40860,0x40888)`, VA `[0x80050060,0x80050088)`, ten words.
- `0x39480/0x39484` materializes its exact start; branch-delay instruction
  `0x3948C` stores it to callback field `0x30($s4)`.
- Canonical return is `0x40880/0x40884`; real predecessor ends at `0x4085C`
  and already-matched real successor `func_80050088` starts at `0x40888`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

```text
40860 27bdffe8  addiu sp,sp,-0x18
40864 afbf0010  sw    ra,0x10(sp)
40868 3c058005  lui   a1,%hi(func_80050E70)
4086C 24a50e70  addiu a1,a1,%lo(func_80050E70)
40870 0c018e36  jal   func_800638D8
40874 00000000  nop
40878 8fbf0010  lw    ra,0x10(sp)
4087C 27bd0018  addiu sp,sp,0x18
40880 03e00008  jr    ra
40884 00000000  nop
```

| screen | result |
|---|---|
| callee | one `func_800638D8`; forwards `$a0` and `func_80050E70` |
| Stage-0 globals | none |
| coloring | minimal |
| `$v0` | discarded result |
| retention | none |
| `-O` | compact era `-O2` shape |
| loop | none |

No gp access selects `-G0`.

## C, object, carve, and packed span

```c
void func_80050E70(void);
void func_800638D8(int slot, void (*callback)(void));
void func_80050060(int slot) { func_800638D8(slot, func_80050E70); }
```

```text
object: 27bdffe8 afbf0010 3c050000 24a50000 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
relocs: R_MIPS_HI16/R_MIPS_LO16 func_80050E70; R_MIPS_26 func_800638D8
ROM:    27bdffe8 afbf0010 3c058005 24a50e70 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
linked: 27bdffe8 afbf0010 3c058005 24a50e70 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
BYTE_EXACT=10/10; PHRASINGS_USED=1/2
old span [0x40860,0x40888)=0x28; C consumes all 0x28; no resume
```

No pins, asm, fabricated nops, or maspsx gates.

```text
file    retail   candidate  role
40850   1000bf8f 1000bf8f   predecessor restore
40854   1800bd27 1800bd27   predecessor teardown
40858   0800e003 0800e003   predecessor return
4085C   00000000 00000000   predecessor delay
40860   e8ffbd27 e8ffbd27   leaf 1
40864   1000bfaf 1000bfaf   leaf 2
40868   0580053c 0580053c   leaf 3
4086C   700ea524 700ea524   leaf 4
40870   368e010c 368e010c   leaf 5
40874   00000000 00000000   leaf 6
40878   1000bf8f 1000bf8f   leaf 7
4087C   1800bd27 1800bd27   leaf 8
40880   0800e003 0800e003   leaf 9
40884   00000000 00000000   leaf 10
40888   e8ffbd27 e8ffbd27   successor entry
4088C   1000bfaf 1000bfaf   successor body
40890   368e010c 368e010c   successor call
40894   21280000 21280000   successor delay
PACKED_SPAN=EXACT
BUILD_RC=0; RESULT: EXACT MATCH
SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VERIFY_RC=0; compare: EXACT MATCH; matching-C count: 410
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
