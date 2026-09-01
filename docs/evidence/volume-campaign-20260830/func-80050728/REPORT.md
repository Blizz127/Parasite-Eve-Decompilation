# `func_80050728` — exact offset callback wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf 363 and Tier-2
continuation rung 22.

## Function hood and retail span

- File `[0x40F28,0x40F48)`, VRAM `[0x80050728,0x80050748)`: eight words.
- Canonical `jr ra; nop` return at `0x80050740/0x80050744`.
- There is no direct `jal` caller. `0x8004F2EC–0x8004F2F0` constructs this
  exact start in `$a1`, then `0x8004F2F4` passes it to callback registrar
  `func_800638D8`.
- Preceding real `func_80050708` ends at file `0x40F20/0x40F24`;
  following real `func_80050748` begins immediately at `0x40F48`.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_REGISTRATION`.

## Retail body

```text
40F28 80050728 E8FFBD27  addiu sp,sp,-0x18
40F2C 8005072C 1000BFAF  sw    ra,0x10(sp)
40F30 80050730 1593010C  jal   func_80064C54
40F34 80050734 5D008424  addiu a0,a0,0x5D
40F38 80050738 1000BF8F  lw    ra,0x10(sp)
40F3C 8005073C 1800BD27  addiu sp,sp,0x18
40F40 80050740 0800E003  jr    ra
40F44 80050744 00000000  nop
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_80064C54` |
| Stage-0 globals | none directly accessed; any state effect belongs to the callee |
| Coloring pressure | none beyond `$ra`; incoming `$a0` is mutated in place |
| `$v0` liveness | dead after the call |
| Address retention | none |
| `-O` signal | era `-O2` places the sole argument adjustment in the call delay slot and emits the canonical 24-byte frame |
| Loop/back-edge | none |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
void func_80064C54(int value);

void func_80050728(int value) {
    func_80064C54(value + 0x5D);
}
```

Compile: era cc1, `-O2 -G0`; no maspsx gate.

## Single-leaf object and ROM comparison

```text
00000000 <func_80050728>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_80064C54
   c: 2484005d  addiu a0,a0,93
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c019315 2484005d 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c019315 2484005d 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0x40F28,0x41518)` = `0x05F0`:

```text
C leaf:     0x40F48 - 0x40F28 = 0x0020
asm resume: 0x41518 - 0x40F48 = 0x05D0
closure:    0x0020 + 0x05D0 = 0x05F0
```

The sizes come only from span boundaries, never aligned object sizes.

## Packed span and gates

```text
40F18: 8fbf0010 = 8fbf0010  preceding restore
40F1C: 27bd0018 = 27bd0018  preceding teardown
40F20: 03e00008 = 03e00008  preceding return
40F24: 00000000 = 00000000  preceding delay
40F28: 27bdffe8 = 27bdffe8  leaf 1
40F2C: afbf0010 = afbf0010  leaf 2
40F30: 0c019315 = 0c019315  leaf 3
40F34: 2484005d = 2484005d  leaf 4 / call delay
40F38: 8fbf0010 = 8fbf0010  leaf 5
40F3C: 27bd0018 = 27bd0018  leaf 6
40F40: 03e00008 = 03e00008  leaf 7
40F44: 00000000 = 00000000  leaf 8 / return delay
40F48: 27bdffe0 = 27bdffe0  following entry
40F4C: afb10014 = afb10014  following word 2
40F50: 00808821 = 00808821  following word 3
40F54: afb00010 = afb00010  following word 4
PACKED_SPAN=EXACT
```

```text
retail SHA-1:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 363
```
