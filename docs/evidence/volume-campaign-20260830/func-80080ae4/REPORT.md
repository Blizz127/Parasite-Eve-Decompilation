# `func_80080AE4` — exact zero-status wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf 350 and Tier-2 probe rung
9.

## Function hood and retail span

- File `[0x712E4,0x71304)`, VRAM `[0x80080AE4,0x80080B04)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x80080AFC`, with frame teardown in its live
  delay slot at `0x80080B00`.
- Three exact direct callers exist at `0x80081040`, `0x80081EEC`, and
  `0x8008245C`.
- Preceding real `func_80080AC4` ends at file `0x712DC/0x712E0`; following
  real `func_80080B04` begins immediately at file `0x71304`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`; this is not padding, data, or a tail
entry.

## Retail body

```text
712E4 80080AE4 E8FFBD27  addiu sp,sp,-0x18
712E8 80080AE8 1000BFAF  sw    ra,0x10(sp)
712EC 80080AEC D1EF010C  jal   func_8007BF44
712F0 80080AF0 00000000  nop
712F4 80080AF4 1000BF8F  lw    ra,0x10(sp)
712F8 80080AF8 0100422C  sltiu v0,v0,1
712FC 80080AFC 0800E003  jr    ra
71300 80080B00 1800BD27  addiu sp,sp,0x18
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_8007BF44`; it performs an ordered hardware transaction, polls completion/status bits, and returns zero on its normal completion path |
| Stage-0 globals | none accessed directly by the wrapper; hardware/global effects belong to the callee |
| Coloring pressure | none beyond preserving `$ra`; arguments flow through `$a0/$a1` unchanged |
| `$v0` liveness | callee result remains in `$v0` and is canonicalized in place with `sltiu v0,v0,1` |
| Address retention | none |
| `-O` signal | canonical 24-byte one-call frame, in-place boolean canonicalization, and teardown delay slot are era `-O2` |
| Loop/back-edge | none in wrapper; polling loop belongs to callee |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
int func_8007BF44(void *buffer, int value);

int func_80080AE4(void *buffer, int value) {
    return func_8007BF44(buffer, value) == 0;
}
```

Compile: era cc1, `-O2 -G0`; no maspsx gate.

## Single-leaf object and ROM comparison

```text
00000000 <func_80080AE4>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_8007BF44
   c: 00000000  nop
  10: 8fbf0010  lw    ra,16(sp)
  14: 2c420001  sltiu v0,v0,1
  18: 03e00008  jr    ra
  1c: 27bd0018  addiu sp,sp,24

ROM: 27bdffe8 afbf0010 0c01efd1 00000000 8fbf0010 2c420001 03e00008 27bd0018
C:   27bdffe8 afbf0010 0c01efd1 00000000 8fbf0010 2c420001 03e00008 27bd0018
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0x71150,0x714C8)` = `0x0378`:

```text
asm prefix: 0x712E4 - 0x71150 = 0x0194
C leaf:     0x71304 - 0x712E4 = 0x0020
asm resume: 0x714C8 - 0x71304 = 0x01C4
closure:    0x0194 + 0x0020 + 0x01C4 = 0x0378
```

## Packed span and gates

```text
712D4: 8fbf0010 = 8fbf0010  preceding epilogue
712D8: 24020001 = 24020001  preceding result
712DC: 03e00008 = 03e00008  preceding return
712E0: 27bd0018 = 27bd0018  preceding teardown delay
712E4: 27bdffe8 = 27bdffe8  leaf 1
712E8: afbf0010 = afbf0010  leaf 2
712EC: 0c01efd1 = 0c01efd1  leaf 3
712F0: 00000000 = 00000000  leaf 4
712F4: 8fbf0010 = 8fbf0010  leaf 5
712F8: 2c420001 = 2c420001  leaf 6
712FC: 03e00008 = 03e00008  leaf 7
71300: 27bd0018 = 27bd0018  leaf 8 / return delay
71304: 27bdffe8 = 27bdffe8  following function entry
71308: afbf0010 = afbf0010  following function word 2
7130C: 0c01f011 = 0c01f011  following call
71310: 00000000 = 00000000  following call delay
PACKED_SPAN=EXACT
```

```text
Compare:  EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 350
```
