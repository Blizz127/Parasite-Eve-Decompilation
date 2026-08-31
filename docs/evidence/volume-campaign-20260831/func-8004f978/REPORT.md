# `func_8004F978` — exact callback-registration wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf **402**.

## Function hood first

- File `[0x40178,0x401A0)`, VA `[0x8004F978,0x8004F9A0)`: ten words.
- `0x3A1F4/0x3A1F8` independently materializes the exact start and the branch
  delay at `0x3A200` stores it to callback field `0x30($s0)`.
- It ends at `0x40198/0x4019C` with canonical `jr ra; nop`.
- Real predecessor `func_8004F950` ends at `0x40170/0x40174`; real successor
  `func_8004F9A0` starts at `0x401A0` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_INDEPENDENT_EXACT_START_CALLBACK_STORE_AND_CANONICAL_RETURN`.

## Retail body and screens

```text
40178 8004F978 27BDFFE8  addiu sp,sp,-0x18
4017C 8004F97C AFBF0010  sw    ra,0x10(sp)
40180 8004F980 3C058005  lui   a1,%hi(func_800509E0)
40184 8004F984 24A509E0  addiu a1,a1,%lo(func_800509E0)
40188 8004F988 0C018E36  jal   func_800638D8
4018C 8004F98C 00000000  nop
40190 8004F990 8FBF0010  lw    ra,0x10(sp)
40194 8004F994 27BD0018  addiu sp,sp,0x18
40198 8004F998 03E00008  jr    ra
4019C 8004F99C 00000000  nop
```

| screen | result |
|---|---|
| Callee bucket | one direct `func_800638D8`, forwarding `$a0` and callback `func_800509E0` |
| Stage-0 globals | none |
| Coloring pressure | minimal |
| `$v0` liveness | discarded call result; void wrapper |
| Address retention | none |
| `-O` signal | compact era `-O2` frame/materialization |
| Loop/back-edge | none |

No gp access appears, selecting `-G0`.

## C and isolated object

```c
void func_800509E0(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004F978(int slot) {
    func_800638D8(slot, func_800509E0);
}
```

```text
object: 27bdffe8 afbf0010 3c050000 24a50000 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
relocs: word 3 R_MIPS_HI16 func_800509E0; word 4 R_MIPS_LO16 func_800509E0; word 5 R_MIPS_26 func_800638D8
ROM:    27bdffe8 afbf0010 3c058005 24a509e0 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
linked: 27bdffe8 afbf0010 3c058005 24a509e0 0c018e36 00000000 8fbf0010 27bd0018 03e00008 00000000
BYTE_EXACT=10/10
PHRASINGS_USED=1/2
```

Flags: era GCC 2.7.2 `-O2 -G0`; no pins, asm, fabricated nops, or maspsx
behavior gates.

## Carve and packed span

The leaf consumed the head of active `[0x40178,0x40820)`:

```text
C:      0x401A0 - 0x40178 = 0x028
resume: 0x40820 - 0x401A0 = 0x680
closure: 0x028 + 0x680 = 0x6A8
```

```text
file    retail   candidate  role
40168   1000bf8f 1000bf8f   predecessor restore
4016C   1800bd27 1800bd27   predecessor teardown
40170   0800e003 0800e003   predecessor return
40174   00000000 00000000   predecessor delay
40178   e8ffbd27 e8ffbd27   leaf 1
4017C   1000bfaf 1000bfaf   leaf 2
40180   0580053c 0580053c   leaf 3
40184   e009a524 e009a524   leaf 4
40188   368e010c 368e010c   leaf 5
4018C   00000000 00000000   leaf 6
40190   1000bf8f 1000bf8f   leaf 7
40194   1800bd27 1800bd27   leaf 8
40198   0800e003 0800e003   leaf 9
4019C   00000000 00000000   leaf 10
401A0   e8ffbd27 e8ffbd27   successor entry
401A4   1000b0af 1000b0af   successor body
401A8   21808000 21808000   successor body
401AC   1400bfaf 1400bfaf   successor body
PACKED_SPAN=EXACT
```

```text
BUILD_RC=0; RESULT: EXACT MATCH
SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VERIFY_RC=0; Split verification (Phase 4E): OK; compare: EXACT MATCH
matching-C count: 402
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
