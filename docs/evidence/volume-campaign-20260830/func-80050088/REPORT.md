# `func_80050088` — exact callback-unregister wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf 359 and Tier-2
continuation rung 18.

## Function hood and retail span

- File `[0x40888,0x408A8)`, VRAM `[0x80050088,0x800500A8)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x800500A0`, followed by `nop` in the delay
  slot.
- There is no direct `jal` caller. The exact start is constructed at
  `0x80048E4C–0x80048E50` and stored at `0x80048E54` into callback slot
  `+0x30` of the record returned by `func_8006322C`.
- Preceding real `func_80050060` ends at file `0x40880/0x40884`; following
  real `func_800500A8` begins immediately at file `0x408A8`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_REGISTRATION`; the generated
label is supported by executable pointer provenance and is not inferred from
its name.

## Retail body

```text
40888 80050088 E8FFBD27  addiu sp,sp,-0x18
4088C 8005008C 1000BFAF  sw    ra,0x10(sp)
40890 80050090 368E010C  jal   func_800638D8
40894 80050094 21280000  addu  a1,zero,zero
40898 80050098 1000BF8F  lw    ra,0x10(sp)
4089C 8005009C 1800BD27  addiu sp,sp,0x18
408A0 800500A0 0800E003  jr    ra
408A4 800500A4 00000000  nop
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_800638D8`; sibling `func_80050060` passes a real callback as argument 2, while this leaf passes null |
| Stage-0 globals | none accessed directly by this wrapper; callback-list and rendering state effects belong to the callee |
| Coloring pressure | none beyond preserving `$ra`; `$a0` flows through and null `$a1` fills the call delay slot |
| `$v0` liveness | dead after the call, matching the void source contract |
| Address retention | none |
| `-O` signal | canonical 24-byte frame, useful null call-delay slot, and post-call restore/teardown sequence are era `-O2` |
| Loop/back-edge | none in the wrapper |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
void func_800638D8(void *object, void *callback);

void func_80050088(void *object) {
    func_800638D8(object, 0);
}
```

Compile: era cc1, `-O2 -G0`; no maspsx gate.

## Single-leaf object and ROM comparison

```text
00000000 <func_80050088>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_800638D8
   c: 00002821  move  a1,zero
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c018e36 00002821 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c018e36 00002821 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0x40838,0x41518)` = `0x0CE0`:

```text
asm prefix: 0x40888 - 0x40838 = 0x0050
C leaf:     0x408A8 - 0x40888 = 0x0020
asm resume: 0x41518 - 0x408A8 = 0x0C70
closure:    0x0050 + 0x0020 + 0x0C70 = 0x0CE0
```

These are boundary-derived text sizes; no aligned object size was used for
the carve.

## Packed span and gates

All sixteen words below were read from both the rebuilt packed executable and
retail. Each pair is equal:

```text
40878: 8fbf0010 = 8fbf0010  preceding restore ra
4087C: 27bd0018 = 27bd0018  preceding teardown
40880: 03e00008 = 03e00008  preceding return
40884: 00000000 = 00000000  preceding delay
40888: 27bdffe8 = 27bdffe8  leaf 1
4088C: afbf0010 = afbf0010  leaf 2
40890: 0c018e36 = 0c018e36  leaf 3
40894: 00002821 = 00002821  leaf 4 / call delay
40898: 8fbf0010 = 8fbf0010  leaf 5
4089C: 27bd0018 = 27bd0018  leaf 6
408A0: 03e00008 = 03e00008  leaf 7
408A4: 00000000 = 00000000  leaf 8 / return delay
408A8: 27bdffe8 = 27bdffe8  following function entry
408AC: afb00010 = afb00010  following word 2
408B0: 00808021 = 00808021  following word 3
408B4: 24040002 = 24040002  following word 4
PACKED_SPAN=EXACT
```

```text
build/extracted/disc1/SLUS_006.62:
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
build/disc1.candidate.exe:
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 359
```
