# `func_800824F0` — exact slot-3 forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 368 and Tier-2 continuation rung 27.

## Function hood

- File `[0x72CF0,0x72D14)`, VA `[0x800824F0,0x80082514)`: nine words,
  ending in canonical `jr ra; nop`.
- Seven direct `jal` callers target the exact start: `0x8007A2C8`,
  `0x80080F2C`, `0x800812B4`, `0x8008135C`, `0x8008139C`,
  `0x80081EDC`, and `0x80081EF4`.
- Preceding real `func_800824DC` ends immediately at `0x72CE8/0x72CEC` with
  `jr ra` and a live store delay slot. Following real `func_80082514` begins
  immediately at `0x72D14` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`.

## Retail, screens, and C

```text
72CF0 800824F0 E8FFBD27  addiu sp,sp,-0x18
72CF4 800824F4 1000BFAF  sw    ra,0x10(sp)
72CF8 800824F8 21288000  addu  a1,a0,zero
72CFC 800824FC 3DCF010C  jal   func_80073CF4
72D00 80082500 03000424  addiu a0,zero,3
72D04 80082504 1000BF8F  lw    ra,0x10(sp)
72D08 80082508 1800BD27  addiu sp,sp,0x18
72D0C 8008250C 0800E003  jr    ra
72D10 80082510 00000000  nop
```

| Screen | Result |
|---|---|
| Callee | one unresolved `func_80073CF4` call; original argument becomes argument 2 and constant slot 3 becomes argument 1 |
| Stage-0 | no direct globals; effects belong to the callee |
| Coloring / `$v0` | only `$ra`; call result dead |
| Address retention | none |
| `-O` signal | era `-O2` moves `$a0` to `$a1` before the call and fills its delay slot with constant 3 |
| Loop | none |

```c
void func_80073CF4(int slot, unsigned int value);

void func_800824F0(unsigned int value) {
    func_80073CF4(3, value);
}
```

Flags: era `-O2 -G0`; no maspsx gate.

## Single-leaf object

```text
00000000 <func_800824F0>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 00802821  move  a1,a0
   c: 0c000000  jal   0                 R_MIPS_26 func_80073CF4
  10: 24040003  li    a0,3
  14: 8fbf0010  lw    ra,16(sp)
  18: 27bd0018  addiu sp,sp,24
  1c: 03e00008  jr    ra
  20: 00000000  nop

ROM: 27bdffe8 afbf0010 00802821 0c01cf3d 24040003 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 00802821 0c01cf3d 24040003 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=9/9
```

GNU as aligns the standalone object to `0x30`; all `0x0C` bytes beyond the
boundary-derived `0x24` body are zero, and the guarded trim accepts only that
zero alignment.

## Carve geometry

Former asm `[0x72ABC,0x72D34)` = `0x0278`:

```text
prefix: 0x72CF0 - 0x72ABC = 0x0234
leaf:   0x72D14 - 0x72CF0 = 0x0024
resume: 0x72D34 - 0x72D14 = 0x0020
close:  0x0234 + 0x0024 + 0x0020 = 0x0278
```

## Packed span and gates

```text
72CE0: 24638ab8 = 24638ab8  preceding address low
72CE4: 8c620000 = 8c620000  preceding load
72CE8: 03e00008 = 03e00008  preceding return
72CEC: ac640000 = ac640000  preceding live delay store
72CF0: 27bdffe8 = 27bdffe8  leaf 1
72CF4: afbf0010 = afbf0010  leaf 2
72CF8: 00802821 = 00802821  leaf 3
72CFC: 0c01cf3d = 0c01cf3d  leaf 4
72D00: 24040003 = 24040003  leaf 5
72D04: 8fbf0010 = 8fbf0010  leaf 6
72D08: 27bd0018 = 27bd0018  leaf 7
72D0C: 03e00008 = 03e00008  leaf 8
72D10: 00000000 = 00000000  leaf 9
72D14: 27bdffe8 = 27bdffe8  following real prologue
72D18: afbf0010 = afbf0010  following word 2
72D1C: 0c020b37 = 0c020b37  following call
72D20: 00000000 = 00000000  following call delay
PACKED_SPAN=EXACT
```

```text
retail SHA-1:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 368
```
