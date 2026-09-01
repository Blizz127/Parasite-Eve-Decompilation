# `func_80050260` — exact callback forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf 360 and Tier-2
continuation rung 19.

## Function hood and retail span

- File `[0x40A60,0x40A80)`, VRAM `[0x80050260,0x80050280)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x80050278`, followed by `nop` in the delay
  slot.
- There is no direct `jal` caller. Six independent sites construct the exact
  start and store it into callback slot `+0x88`: `0x8004C400–0x8004C40C`,
  `0x8004EB08–0x8004EB14`, `0x80044200–0x8004420C`,
  `0x80046FA4–0x80046FB0`, `0x80048774–0x8004877C`, and
  `0x8004E8BC–0x8004E8C4`.
- Preceding real `func_8005022C` ends at file `0x40A58/0x40A5C`;
  following real `func_80050280` begins immediately at file `0x40A80`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_REGISTRATIONS`; the generated
label is supported by six executable pointer registrations and is not
inferred from its name.

## Retail body

```text
40A60 80050260 E8FFBD27  addiu sp,sp,-0x18
40A64 80050264 1000BFAF  sw    ra,0x10(sp)
40A68 80050268 D855010C  jal   func_80055760
40A6C 8005026C 00000000  nop
40A70 80050270 1000BF8F  lw    ra,0x10(sp)
40A74 80050274 1800BD27  addiu sp,sp,0x18
40A78 80050278 0800E003  jr    ra
40A7C 8005027C 00000000  nop
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_80055760`; five registration sites also invoke it immediately after publishing this wrapper, while the sixth only publishes the callback |
| Stage-0 globals | none accessed directly by this wrapper; any state effects belong to the callee |
| Coloring pressure | none beyond preserving `$ra`; no argument or result needs a new register home |
| `$v0` liveness | dead after the call, matching the void source contract |
| Address retention | none |
| `-O` signal | canonical 24-byte call frame, empty call delay slot, and standard restore/teardown sequence are era `-O2` |
| Loop/back-edge | none in the wrapper |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
void func_80055760(void);

void func_80050260(void) {
    func_80055760();
}
```

Compile: era cc1, `-O2 -G0`; no maspsx gate.

## Single-leaf object and ROM comparison

```text
00000000 <func_80050260>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_80055760
   c: 00000000  nop
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c0155d8 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c0155d8 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0x408A8,0x41518)` = `0x0C70`:

```text
asm prefix: 0x40A60 - 0x408A8 = 0x01B8
C leaf:     0x40A80 - 0x40A60 = 0x0020
asm resume: 0x41518 - 0x40A80 = 0x0A98
closure:    0x01B8 + 0x0020 + 0x0A98 = 0x0C70
```

These are boundary-derived text sizes; no aligned object size was used for
the carve.

## Packed span and gates

All sixteen words below were read from both the rebuilt packed executable and
retail. Each pair is equal:

```text
40A50: 8fbf0010 = 8fbf0010  preceding restore ra
40A54: 27bd0018 = 27bd0018  preceding teardown
40A58: 03e00008 = 03e00008  preceding return
40A5C: 00000000 = 00000000  preceding delay
40A60: 27bdffe8 = 27bdffe8  leaf 1
40A64: afbf0010 = afbf0010  leaf 2
40A68: 0c0155d8 = 0c0155d8  leaf 3
40A6C: 00000000 = 00000000  leaf 4 / call delay
40A70: 8fbf0010 = 8fbf0010  leaf 5
40A74: 27bd0018 = 27bd0018  leaf 6
40A78: 03e00008 = 03e00008  leaf 7
40A7C: 00000000 = 00000000  leaf 8 / return delay
40A80: 27bdffe0 = 27bdffe0  following function entry
40A84: afb00018 = afb00018  following word 2
40A88: 00808021 = 00808021  following word 3
40A8C: 00002021 = 00002021  following word 4
PACKED_SPAN=EXACT
```

```text
build/extracted/disc1/SLUS_006.62:
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
build/disc1.candidate.exe:
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 360
```
