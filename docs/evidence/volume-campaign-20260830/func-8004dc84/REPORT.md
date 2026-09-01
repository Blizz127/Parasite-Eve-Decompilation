# `func_8004DC84` — exact constant forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf 351 and Tier-2 probe rung
10.

## Function hood and retail span

- File `[0x3E484,0x3E4A4)`, VRAM `[0x8004DC84,0x8004DCA4)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra; nop` at `0x8004DC9C/0x8004DCA0`.
- One exact direct caller exists at `0x8004272C`; its next operations call
  `func_8004CDAC` and `func_80042910`.
- Preceding real `func_8004DAA4` ends at file `0x3E47C/0x3E480` with
  `jr ra; nop`. Following real `func_8004DCA4` starts immediately at
  `0x3E4A4` with a stack-frame prologue.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL`; this is not padding, data, or a tail
entry.

## Retail body

```text
3E484 8004DC84 E8FFBD27  addiu sp,sp,-0x18
3E488 8004DC88 1000BFAF  sw    ra,0x10(sp)
3E48C 8004DC8C CF8B010C  jal   func_80062F3C
3E490 8004DC90 2A000424  addiu a0,zero,0x2A
3E494 8004DC94 1000BF8F  lw    ra,0x10(sp)
3E498 8004DC98 1800BD27  addiu sp,sp,0x18
3E49C 8004DC9C 0800E003  jr    ra
3E4A0 8004DCA0 00000000  nop
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved asm callee, `func_80062F3C`; its body traverses GP-backed node/list state and calls `func_8006269C`, but no semantic name is asserted |
| Stage-0 globals | none read or written directly by this wrapper; state effects belong to the callee |
| Coloring pressure | none beyond preserving `$ra`; the only argument is constant `0x2A` |
| `$v0` liveness | none; wrapper return value is unused |
| Address retention | none |
| `-O` signal | canonical 24-byte one-call frame and constant-filled call delay slot are era `-O2` |
| Loop/back-edge | none in the wrapper |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
void func_80062F3C(int value);

void func_8004DC84(void) {
    func_80062F3C(0x2A);
}
```

Compile: era cc1, `-O2 -G0`; no maspsx gate.

## Single-leaf object and ROM comparison

```text
00000000 <func_8004DC84>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_80062F3C
   c: 2404002a  addiu a0,zero,42
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c018bcf 2404002a 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c018bcf 2404002a 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0x3E2A4,0x3F170)` = `0x0ECC`:

```text
asm prefix: 0x3E484 - 0x3E2A4 = 0x01E0
C leaf:     0x3E4A4 - 0x3E484 = 0x0020
asm resume: 0x3F170 - 0x3E4A4 = 0x0CCC
closure:    0x01E0 + 0x0020 + 0x0CCC = 0x0ECC
```

These sizes come from boundary arithmetic, not aligned object sizes.

## Packed span and gates

```text
3E474: 8fb00010 = 8fb00010  preceding epilogue
3E478: 27bd0028 = 27bd0028  preceding stack restore
3E47C: 03e00008 = 03e00008  preceding return
3E480: 00000000 = 00000000  preceding delay
3E484: 27bdffe8 = 27bdffe8  leaf 1
3E488: afbf0010 = afbf0010  leaf 2
3E48C: 0c018bcf = 0c018bcf  leaf 3
3E490: 2404002a = 2404002a  leaf 4 / call delay
3E494: 8fbf0010 = 8fbf0010  leaf 5
3E498: 27bd0018 = 27bd0018  leaf 6
3E49C: 03e00008 = 03e00008  leaf 7
3E4A0: 00000000 = 00000000  leaf 8 / return delay
3E4A4: 27bdffe8 = 27bdffe8  following function entry
3E4A8: afb00010 = afb00010  following function word 2
3E4AC: afbf0014 = afbf0014  following function word 3
3E4B0: 0c0177a2 = 0c0177a2  following call
PACKED_SPAN=EXACT
```

```text
Compare:  EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 351
```
