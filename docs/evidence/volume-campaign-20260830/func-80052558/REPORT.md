# `func_80052558` — exact signed-byte return twin

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 371 and Tier-2 continuation rung 30.

## Function hood

- File `[0x42D58,0x42D7C)`, VA `[0x80052558,0x8005257C)`: nine words,
  ending in canonical `jr ra` with stack teardown in the delay slot.
- Three unique direct callers target the exact start: `0x80044A28`,
  `0x80045D74`, and `0x80050A04`.
- Preceding independently proven `func_80052534` and following already-C
  `func_8005257C` meet this leaf at the exact boundaries.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`.

## Retail, screens, and C

```text
42D58 80052558 E8FFBD27  addiu sp,sp,-0x18
42D5C 8005255C 1000BFAF  sw    ra,0x10(sp)
42D60 80052560 3584000C  jal   func_800210D4
42D64 80052564 00000000  nop
42D68 80052568 00160200  sll   v0,v0,24
42D6C 8005256C 1000BF8F  lw    ra,0x10(sp)
42D70 80052570 03160200  sra   v0,v0,24
42D74 80052574 0800E003  jr    ra
42D78 80052578 1800BD27  addiu sp,sp,0x18
```

| Screen | Result |
|---|---|
| Callee | one unresolved `func_800210D4` call |
| Stage-0 | no direct globals; effects belong to the callee |
| Coloring / `$v0` | callee result stays in `$v0`; signed-byte canonicalization is live to return |
| Address retention | none |
| `-O` signal | era `-O2` schedules restore between `sll`/`sra` and teardown in return delay |
| Loop | none |

```c
int func_800210D4(void);

signed char func_80052558(void) {
    return func_800210D4();
}
```

The signed return is retail-proven by `sll 24; sra 24`. Flags: era `-O2
-G0`; no maspsx gate.

## Single-leaf object

```text
00000000 <func_80052558>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_800210D4
   c: 00000000  nop
  10: 00021600  sll   v0,v0,0x18
  14: 8fbf0010  lw    ra,16(sp)
  18: 00021603  sra   v0,v0,0x18
  1c: 03e00008  jr    ra
  20: 27bd0018  addiu sp,sp,24

ROM: 27bdffe8 afbf0010 0c008435 00000000 00021600 8fbf0010 00021603 03e00008 27bd0018
C:   27bdffe8 afbf0010 0c008435 00000000 00021600 8fbf0010 00021603 03e00008 27bd0018
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=9/9
```

The standalone object's bytes beyond the boundary-derived `0x24` body are
zero alignment only and pass the guarded trim.

## Carve geometry

The active asm span was exactly `[0x42D58,0x42D7C)`:

```text
leaf: 0x42D7C - 0x42D58 = 0x0024
close: 0x0024 = 0x0024
```

No aligned object size and no zero padding are included in the YAML leaf.

## Packed span and gates

```text
42D48: 8fbf0010 = 8fbf0010  preceding restore
42D4C: 00021603 = 00021603  preceding sign extension
42D50: 03e00008 = 03e00008  preceding return
42D54: 27bd0018 = 27bd0018  preceding teardown delay
42D58: 27bdffe8 = 27bdffe8  leaf 1
42D5C: afbf0010 = afbf0010  leaf 2
42D60: 0c008435 = 0c008435  leaf 3
42D64: 00000000 = 00000000  leaf 4
42D68: 00021600 = 00021600  leaf 5
42D6C: 8fbf0010 = 8fbf0010  leaf 6
42D70: 00021603 = 00021603  leaf 7
42D74: 03e00008 = 03e00008  leaf 8
42D78: 27bd0018 = 27bd0018  leaf 9
42D7C: 3c02800a = 3c02800a  following real function entry
42D80: 8c42d1a0 = 8c42d1a0  following load
42D84: 00000000 = 00000000  following load-delay nop
42D88: 00021042 = 00021042  following shift
PACKED_SPAN=EXACT
```

```text
retail SHA-1:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 371
```
