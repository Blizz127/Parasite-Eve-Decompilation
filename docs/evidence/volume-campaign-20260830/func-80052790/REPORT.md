# `func_80052790` — exact GP state/boolean forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G8`; matching-C
leaf 372 and Tier-2 continuation rung 31.

## Function hood

- File `[0x42F90,0x42FB4)`, VA `[0x80052790,0x800527B4)`: nine words,
  ending in canonical `jr ra; nop`.
- Three unique direct callers target the exact start: `0x8004AFEC`,
  `0x8005C438`, and `0x8005D910`.
- The preceding real function ends at `0x42F88/0x42F8C`; already-matched GP
  getter `func_800527B4` starts immediately at `0x42FB4`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`.

## Screens and semantics

```text
42F90 80052790 E8FFBD27  addiu sp,sp,-0x18
42F94 80052794 B00284AF  sw    a0,0x2B0(gp)
42F98 80052798 1000BFAF  sw    ra,0x10(sp)
42F9C 8005279C CA19020C  jal   func_80086728
42FA0 800527A0 0100842C  sltiu a0,a0,1
42FA4 800527A4 1000BF8F  lw    ra,0x10(sp)
42FA8 800527A8 1800BD27  addiu sp,sp,0x18
42FAC 800527AC 0800E003  jr    ra
42FB0 800527B0 00000000  nop
```

| Screen | Result |
|---|---|
| Callee | one unresolved `func_80086728` call; receives `value == 0` |
| Stage-0 | sole direct writer found for `D_8009D020`; already-matched `func_800527B4` is its GP-relative reader/getter |
| Coloring / `$v0` | no live result; boolean is formed in-place in `$a0` |
| Address retention | none; retail directly uses `_gp + 0x2B0` |
| `-O` signal | `-G8` is proven by the GP-relative store; era `-O2` fills the call delay with `sltiu` |
| Loop | none |

With `_gp = 0x8009CD70`, `_gp + 0x2B0 = 0x8009D020`.

```c
extern int D_8009D020;

void func_80086728(int enabled);

void func_80052790(int value) {
    D_8009D020 = value;
    func_80086728(value == 0);
}
```

Flags: era `-O2 -G8`; no maspsx gate.

## Single-leaf object

```text
00000000 <func_80052790>:
   0: 27bdffe8  addiu sp,sp,-24
   4: af840000  sw    a0,0(gp)           R_MIPS_GPREL16 D_8009D020
   8: afbf0010  sw    ra,16(sp)
   c: 0c000000  jal   0                  R_MIPS_26 func_80086728
  10: 2c840001  sltiu a0,a0,1
  14: 8fbf0010  lw    ra,16(sp)
  18: 27bd0018  addiu sp,sp,24
  1c: 03e00008  jr    ra
  20: 00000000  nop

ROM: 27bdffe8 af8402b0 afbf0010 0c0219ca 2c840001 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 af8402b0 afbf0010 0c0219ca 2c840001 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_GPREL16 plus R_MIPS_26
BYTE_EXACT=9/9
```

The standalone object has zero alignment only beyond the boundary-derived
`0x24` body; guarded trim accepted those zeros.

## Carve geometry

Former asm `[0x42D94,0x42FB4)` = `0x0220`:

```text
prefix: 0x42F90 - 0x42D94 = 0x01FC
leaf:   0x42FB4 - 0x42F90 = 0x0024
close:  0x01FC + 0x0024 = 0x0220
```

## Packed span and gates

```text
42F80: 8fbf0010 = 8fbf0010  preceding restore
42F84: 27bd0018 = 27bd0018  preceding teardown
42F88: 03e00008 = 03e00008  preceding return
42F8C: 00000000 = 00000000  preceding delay
42F90: 27bdffe8 = 27bdffe8  leaf 1
42F94: af8402b0 = af8402b0  leaf 2
42F98: afbf0010 = afbf0010  leaf 3
42F9C: 0c0219ca = 0c0219ca  leaf 4
42FA0: 2c840001 = 2c840001  leaf 5
42FA4: 8fbf0010 = 8fbf0010  leaf 6
42FA8: 27bd0018 = 27bd0018  leaf 7
42FAC: 03e00008 = 03e00008  leaf 8
42FB0: 00000000 = 00000000  leaf 9
42FB4: 8f8202b0 = 8f8202b0  following getter entry
42FB8: 03e00008 = 03e00008  following getter return
42FBC: 00000000 = 00000000  following getter delay
42FC0: 03e00008 = 03e00008  next real leaf entry
PACKED_SPAN=EXACT
```

```text
retail SHA-1:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 372
```
