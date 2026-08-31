# `func_80050CF8` — exact constant callback wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 366 and Tier-2 continuation rung 25.

## Function hood

- File `[0x414F8,0x41518)`, VA `[0x80050CF8,0x80050D18)`: eight words,
  ending in canonical `jr ra; nop`.
- No direct `jal` caller. `0x8004FFB0–0x8004FFB4` constructs the exact start
  and `0x8004FFB8` passes it in `$a1` to callback registrar `func_800638D8`.
- Real predecessor `func_80050CB4` ends at `0x414F0/0x414F4`; already-proven
  real `func_80050D18` starts immediately at `0x41518`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_REGISTRATION`.

## Retail, screens, and C

```text
414F8 80050CF8 E8FFBD27  addiu sp,sp,-0x18
414FC 80050CFC 1000BFAF  sw    ra,0x10(sp)
41500 80050D00 1593010C  jal   func_80064C54
41504 80050D04 08000424  addiu a0,zero,8
41508 80050D08 1000BF8F  lw    ra,0x10(sp)
4150C 80050D0C 1800BD27  addiu sp,sp,0x18
41510 80050D10 0800E003  jr    ra
41514 80050D14 00000000  nop
```

| Screen | Result |
|---|---|
| Callee | one unresolved `func_80064C54` call |
| Stage-0 | no direct globals; effects belong to callee |
| Coloring / `$v0` | only `$ra`; call result dead |
| Address retention | none |
| `-O` signal | `-O2` materializes constant 8 in the call delay slot |
| Loop | none |

```c
void func_80064C54(int value);

void func_80050CF8(void) {
    func_80064C54(8);
}
```

Flags: era `-O2 -G0`; no maspsx gate.

## Object and ROM

```text
00000000 <func_80050CF8>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_80064C54
   c: 24040008  li    a0,8
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c019315 24040008 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c019315 24040008 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve

Former asm `[0x41470,0x41518)` = `0x00A8`:

```text
prefix: 0x414F8 - 0x41470 = 0x0088
leaf:   0x41518 - 0x414F8 = 0x0020
close:  0x0088 + 0x0020 = 0x00A8
```

Boundary arithmetic only; no aligned object size.

## Packed span and gates

```text
414E8: 8fb00010 = 8fb00010  preceding restore s0
414EC: 27bd0018 = 27bd0018  preceding teardown
414F0: 03e00008 = 03e00008  preceding return
414F4: 00000000 = 00000000  preceding delay
414F8: 27bdffe8 = 27bdffe8  leaf 1
414FC: afbf0010 = afbf0010  leaf 2
41500: 0c019315 = 0c019315  leaf 3
41504: 24040008 = 24040008  leaf 4
41508: 8fbf0010 = 8fbf0010  leaf 5
4150C: 27bd0018 = 27bd0018  leaf 6
41510: 03e00008 = 03e00008  leaf 7
41514: 00000000 = 00000000  leaf 8
41518: 03e00008 = 03e00008  following `func_80050D18` entry
4151C: 00000000 = 00000000  following return delay
41520: 27bdffd8 = 27bdffd8  next real prologue
41524: afb1001c = afb1001c  next real word 2
PACKED_SPAN=EXACT
```

```text
retail SHA-1:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 366
```
