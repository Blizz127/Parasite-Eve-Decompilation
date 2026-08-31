# `func_80052534` — exact signed-byte return wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 370 and Tier-2 continuation rung 29.

## Function hood

- File `[0x42D34,0x42D58)`, VA `[0x80052534,0x80052558)`: nine words,
  ending in canonical `jr ra` with stack teardown in the delay slot.
- Two unique direct callers target the exact start: `0x80043F38` and
  `0x800507C0`.
- Preceding real C leaf `func_80052524` ends immediately at `0x42D30`;
  following real `func_80052558` starts immediately at `0x42D58`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`.

## Retail, screens, and C

```text
42D34 80052534 E8FFBD27  addiu sp,sp,-0x18
42D38 80052538 1000BFAF  sw    ra,0x10(sp)
42D3C 8005253C 2084000C  jal   func_80021080
42D40 80052540 00000000  nop
42D44 80052544 00160200  sll   v0,v0,24
42D48 80052548 1000BF8F  lw    ra,0x10(sp)
42D4C 8005254C 03160200  sra   v0,v0,24
42D50 80052550 0800E003  jr    ra
42D54 80052554 1800BD27  addiu sp,sp,0x18
```

| Screen | Result |
|---|---|
| Callee | one unresolved `func_80021080` call |
| Stage-0 | no direct globals; effects belong to the callee |
| Coloring / `$v0` | callee result stays in `$v0`; signed-byte canonicalization is live to return |
| Address retention | none |
| `-O` signal | era `-O2` schedules restore between `sll` and `sra` and stack teardown in return delay |
| Loop | none |

```c
int func_80021080(void);

signed char func_80052534(void) {
    return func_80021080();
}
```

The signed return type is retail-proven by `sll 24; sra 24`. Flags: era
`-O2 -G0`; no maspsx gate.

## Single-leaf object

```text
00000000 <func_80052534>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_80021080
   c: 00000000  nop
  10: 00021600  sll   v0,v0,0x18
  14: 8fbf0010  lw    ra,16(sp)
  18: 00021603  sra   v0,v0,0x18
  1c: 03e00008  jr    ra
  20: 27bd0018  addiu sp,sp,24

ROM: 27bdffe8 afbf0010 0c008420 00000000 00021600 8fbf0010 00021603 03e00008 27bd0018
C:   27bdffe8 afbf0010 0c008420 00000000 00021600 8fbf0010 00021603 03e00008 27bd0018
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=9/9
```

The standalone object has only zero alignment beyond the boundary-derived
`0x24` body; guarded trim accepted those zeros.

## Carve geometry

Former asm `[0x42D34,0x42D7C)` = `0x0048`:

```text
leaf:   0x42D58 - 0x42D34 = 0x0024
resume: 0x42D7C - 0x42D58 = 0x0024
close:  0x0024 + 0x0024 = 0x0048
```

## Packed span and gates

```text
42D24: 3c02800c = 3c02800c  preceding leaf address high
42D28: 94420e32 = 94420e32  preceding load
42D2C: 03e00008 = 03e00008  preceding return
42D30: 00000000 = 00000000  preceding delay
42D34: 27bdffe8 = 27bdffe8  leaf 1
42D38: afbf0010 = afbf0010  leaf 2
42D3C: 0c008420 = 0c008420  leaf 3
42D40: 00000000 = 00000000  leaf 4
42D44: 00021600 = 00021600  leaf 5
42D48: 8fbf0010 = 8fbf0010  leaf 6
42D4C: 00021603 = 00021603  leaf 7
42D50: 03e00008 = 03e00008  leaf 8
42D54: 27bd0018 = 27bd0018  leaf 9
42D58: 27bdffe8 = 27bdffe8  following real prologue
42D5C: afbf0010 = afbf0010  following word 2
42D60: 0c008435 = 0c008435  following call
42D64: 00000000 = 00000000  following delay
PACKED_SPAN=EXACT
```

```text
retail SHA-1:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 370
```
