# `func_80082534` — exact no-argument forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf 358 and Tier-2
continuation rung 17.

## Function hood and retail span

- File `[0x72D34,0x72D54)`, VRAM `[0x80082534,0x80082554)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x8008254C`, followed by `nop` in the delay
  slot.
- One exact direct caller exists at `0x8003E95C`.
- Preceding real `func_80082514` ends at file `0x72D2C/0x72D30`; following
  real `func_80082554` begins immediately at file `0x72D54`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL`; this is not padding, data, or a tail
entry.

## Retail body

```text
72D34 80082534 E8FFBD27  addiu sp,sp,-0x18
72D38 80082538 1000BFAF  sw    ra,0x10(sp)
72D3C 8008253C 3C0B020C  jal   func_80082CF0
72D40 80082540 00000000  nop
72D44 80082544 1000BF8F  lw    ra,0x10(sp)
72D48 80082548 1800BD27  addiu sp,sp,0x18
72D4C 8008254C 0800E003  jr    ra
72D50 80082550 00000000  nop
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_80082CF0`; it performs a multi-step subsystem reset/initialization transaction and its result is unused |
| Stage-0 globals | none accessed directly by the wrapper; all state effects belong to the callee |
| Coloring pressure | none beyond preserving `$ra`; there are no arguments or retained values |
| `$v0` liveness | dead after the call, matching the void source contract |
| Address retention | none |
| `-O` signal | canonical 24-byte one-call frame and post-call restore/teardown sequence are era `-O2` |
| Loop/back-edge | none in the wrapper |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
void func_80082CF0(void);

void func_80082534(void) {
    func_80082CF0();
}
```

Compile: era cc1, `-O2 -G0`; no maspsx gate.

## Single-leaf object and ROM comparison

```text
00000000 <func_80082534>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_80082CF0
   c: 00000000  nop
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c020b3c 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c020b3c 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0x72ABC,0x734DC)` = `0x0A20`:

```text
asm prefix: 0x72D34 - 0x72ABC = 0x0278
C leaf:     0x72D54 - 0x72D34 = 0x0020
asm resume: 0x734DC - 0x72D54 = 0x0788
closure:    0x0278 + 0x0020 + 0x0788 = 0x0A20
```

These are boundary-derived text sizes; no aligned object size was used for
the carve.

## Packed span and gates

All sixteen words below were read from both the rebuilt packed executable and
retail. Each pair is equal:

```text
72D24: 8fbf0010 = 8fbf0010  preceding restore ra
72D28: 27bd0018 = 27bd0018  preceding teardown
72D2C: 03e00008 = 03e00008  preceding return
72D30: 00000000 = 00000000  preceding delay
72D34: 27bdffe8 = 27bdffe8  leaf 1
72D38: afbf0010 = afbf0010  leaf 2
72D3C: 0c020b3c = 0c020b3c  leaf 3
72D40: 00000000 = 00000000  leaf 4
72D44: 8fbf0010 = 8fbf0010  leaf 5
72D48: 27bd0018 = 27bd0018  leaf 6
72D4C: 03e00008 = 03e00008  leaf 7
72D50: 00000000 = 00000000  leaf 8 / return delay
72D54: 27bdffe8 = 27bdffe8  following function entry
72D58: afbf0010 = afbf0010  following word 2
72D5C: 0c020b6f = 0c020b6f  following call
72D60: 00000000 = 00000000  following call delay
PACKED_SPAN=EXACT
```

```text
build/extracted/disc1/SLUS_006.62:
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
build/disc1.candidate.exe:
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 358
```
