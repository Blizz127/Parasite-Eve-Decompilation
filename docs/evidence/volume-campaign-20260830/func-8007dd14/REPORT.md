# `func_8007DD14` — exact slot-4 forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 369 and Tier-2 continuation rung 28.

## Function hood

- File `[0x6E514,0x6E538)`, VA `[0x8007DD14,0x8007DD38)`: nine words,
  ending in canonical `jr ra; nop`.
- Two unique direct callers target the exact start: `0x8007D184` and
  `0x800859B8`.
- Preceding real `func_8007DCAC` returns at `0x6E500/0x6E504`, followed by
  three explicit alignment nops. After this leaf are three more alignment
  nops, then real handwritten tail helper `func_8007DD44` at `0x6E544`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`; alignment is kept outside the leaf.

## Retail, screens, and C

```text
6E514 8007DD14 E8FFBD27  addiu sp,sp,-0x18
6E518 8007DD18 1000BFAF  sw    ra,0x10(sp)
6E51C 8007DD1C 21288000  addu  a1,a0,zero
6E520 8007DD20 3DCF010C  jal   func_80073CF4
6E524 8007DD24 04000424  addiu a0,zero,4
6E528 8007DD28 1000BF8F  lw    ra,0x10(sp)
6E52C 8007DD2C 1800BD27  addiu sp,sp,0x18
6E530 8007DD30 0800E003  jr    ra
6E534 8007DD34 00000000  nop
```

| Screen | Result |
|---|---|
| Callee | one unresolved `func_80073CF4` call; original argument becomes argument 2 and constant slot 4 becomes argument 1 |
| Stage-0 | no direct globals; effects belong to the callee |
| Coloring / `$v0` | only `$ra`; call result dead |
| Address retention | none |
| `-O` signal | era `-O2` argument shuffle and call-delay constant materialization |
| Loop | none |

```c
void func_80073CF4(int slot, unsigned int value);

void func_8007DD14(unsigned int value) {
    func_80073CF4(4, value);
}
```

Flags: era `-O2 -G0`; no maspsx gate.

## Single-leaf object

```text
00000000 <func_8007DD14>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 00802821  move  a1,a0
   c: 0c000000  jal   0                 R_MIPS_26 func_80073CF4
  10: 24040004  li    a0,4
  14: 8fbf0010  lw    ra,16(sp)
  18: 27bd0018  addiu sp,sp,24
  1c: 03e00008  jr    ra
  20: 00000000  nop

ROM: 27bdffe8 afbf0010 00802821 0c01cf3d 24040004 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 00802821 0c01cf3d 24040004 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=9/9
```

The standalone object has `0x0C` zero alignment bytes after the
boundary-derived `0x24` body; the guarded trim accepted only those zeros.

## Carve geometry

Former asm `[0x6CD60,0x6E6A4)` = `0x1944`:

```text
prefix: 0x6E514 - 0x6CD60 = 0x17B4
leaf:   0x6E538 - 0x6E514 = 0x0024
resume: 0x6E6A4 - 0x6E538 = 0x016C
close:  0x17B4 + 0x0024 + 0x016C = 0x1944
```

## Packed span and gates

```text
6E500: 03e00008 = 03e00008  preceding return
6E504: 27bd0008 = 27bd0008  preceding teardown delay
6E508: 00000000 = 00000000  preceding alignment 1
6E50C: 00000000 = 00000000  preceding alignment 2
6E510: 00000000 = 00000000  preceding alignment 3
6E514: 27bdffe8 = 27bdffe8  leaf 1
6E518: afbf0010 = afbf0010  leaf 2
6E51C: 00802821 = 00802821  leaf 3
6E520: 0c01cf3d = 0c01cf3d  leaf 4
6E524: 24040004 = 24040004  leaf 5
6E528: 8fbf0010 = 8fbf0010  leaf 6
6E52C: 27bd0018 = 27bd0018  leaf 7
6E530: 03e00008 = 03e00008  leaf 8
6E534: 00000000 = 00000000  leaf 9
6E538: 00000000 = 00000000  following alignment 1
6E53C: 00000000 = 00000000  following alignment 2
6E540: 00000000 = 00000000  following alignment 3
6E544: 240a00a0 = 240a00a0  following real helper entry
6E548: 01400008 = 01400008  following tail jump
6E54C: 240900ab = 240900ab  following delay slot
PACKED_SPAN=EXACT
```

```text
retail SHA-1:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 369
```
