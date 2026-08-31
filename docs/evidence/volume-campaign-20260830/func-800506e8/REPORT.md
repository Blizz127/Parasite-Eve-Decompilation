# `func_800506E8` — exact registered callback wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf 361 and Tier-2
continuation rung 20.

## Function hood and retail span

- File `[0x40EE8,0x40F08)`, VRAM `[0x800506E8,0x80050708)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x80050700`, followed by `nop` in the delay
  slot.
- There is no direct `jal` caller. `0x8004EC90–0x8004EC94` constructs this
  exact start and `0x8004EC98–0x8004EC9C` passes it to `func_800638D8` as
  the callback paired with the caller's object.
- Preceding real `func_80050690` ends at file `0x40EE0/0x40EE4`;
  following real `func_80050708` begins immediately at file `0x40F08`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_REGISTRATION`; the generated
label is supported by executable pointer provenance and is not inferred from
its name.

## Retail body

```text
40EE8 800506E8 E8FFBD27  addiu sp,sp,-0x18
40EEC 800506EC 1000BFAF  sw    ra,0x10(sp)
40EF0 800506F0 AA62010C  jal   func_80058AA8
40EF4 800506F4 00000000  nop
40EF8 800506F8 1000BF8F  lw    ra,0x10(sp)
40EFC 800506FC 1800BD27  addiu sp,sp,0x18
40F00 80050700 0800E003  jr    ra
40F04 80050704 00000000  nop
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_80058AA8` |
| Stage-0 globals | none accessed directly by this wrapper; any state effects belong to the callee |
| Coloring pressure | none beyond preserving `$ra` |
| `$v0` liveness | dead after the call, matching the void source contract |
| Address retention | none |
| `-O` signal | canonical 24-byte call frame, empty call delay slot, and standard restore/teardown sequence are era `-O2` |
| Loop/back-edge | none in the wrapper |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
void func_80058AA8(void);

void func_800506E8(void) {
    func_80058AA8();
}
```

Compile: era cc1, `-O2 -G0`; no maspsx gate.

## Single-leaf object and ROM comparison

```text
00000000 <func_800506E8>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_80058AA8
   c: 00000000  nop
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c0162aa 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c0162aa 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0x40A80,0x41518)` = `0x0A98`:

```text
asm prefix: 0x40EE8 - 0x40A80 = 0x0468
C leaf:     0x40F08 - 0x40EE8 = 0x0020
asm resume: 0x41518 - 0x40F08 = 0x0610
closure:    0x0468 + 0x0020 + 0x0610 = 0x0A98
```

These are boundary-derived text sizes; no aligned object size was used for
the carve.

## Packed span and gates

All sixteen packed words, including both four-word boundaries, equal retail:

```text
40ED8: 8fb00010 = 8fb00010  preceding restore s0
40EDC: 27bd0018 = 27bd0018  preceding teardown
40EE0: 03e00008 = 03e00008  preceding return
40EE4: 00000000 = 00000000  preceding delay
40EE8: 27bdffe8 = 27bdffe8  leaf 1
40EEC: afbf0010 = afbf0010  leaf 2
40EF0: 0c0162aa = 0c0162aa  leaf 3
40EF4: 00000000 = 00000000  leaf 4 / call delay
40EF8: 8fbf0010 = 8fbf0010  leaf 5
40EFC: 27bd0018 = 27bd0018  leaf 6
40F00: 03e00008 = 03e00008  leaf 7
40F04: 00000000 = 00000000  leaf 8 / return delay
40F08: 27bdffe8 = 27bdffe8  following function entry
40F0C: afbf0010 = afbf0010  following word 2
40F10: 0c019315 = 0c019315  following call
40F14: 2484001f = 2484001f  following call delay
PACKED_SPAN=EXACT
```

```text
build/extracted/disc1/SLUS_006.62:
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
build/disc1.candidate.exe:
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 361
```
