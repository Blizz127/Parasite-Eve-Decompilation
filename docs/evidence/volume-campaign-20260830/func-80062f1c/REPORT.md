# `func_80062F1C` — exact node-removal forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf 347 and Tier-2 probe rung
6.

## Function hood and retail span

- File `[0x5371C,0x5373C)`, VRAM `[0x80062F1C,0x80062F3C)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra; nop` at `0x80062F34/0x80062F38`.
- A raw executable scan finds **57 unique direct `jal` targets** at the exact
  start. Representative callers are `0x80044768`, `0x80044C20`,
  `0x80045150`, `0x8004C2D8`, `0x8004D390`, and `0x8004F10C`.
- The preceding real `func_80062D2C` ends with `jr ra; nop` at file
  `0x53714/0x53718`.
- The following real `func_80062F3C` begins immediately at file `0x5373C`
  with its stack-frame prologue.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`. This is not padding, data, or a tail
entry.

## Retail body

```text
5371C 80062F1C E8FFBD27  addiu sp,sp,-0x18
53720 80062F20 1000BFAF  sw    ra,0x10(sp)
53724 80062F24 A789010C  jal   func_8006269C
53728 80062F28 00000000  nop
5372C 80062F2C 1000BF8F  lw    ra,0x10(sp)
53730 80062F30 1800BD27  addiu sp,sp,0x18
53734 80062F34 0800E003  jr    ra
53738 80062F38 00000000  nop
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_8006269C`; its body recursively detaches the supplied node from a GP-backed list, releases descendants, and clears references to it |
| Caller/reference census | 57 exact direct calls in the executable; no function-hood inference rests on the generated label |
| Stage-0 globals | none accessed by the wrapper; all global/list effects belong to the callee |
| Coloring pressure | none; the incoming node pointer remains naturally in `$a0` through the call |
| `$v0` liveness | wrapper specifies no result and does not consume or rewrite the callee's incidental `$v0`; retail likewise performs only epilogue operations after the call |
| Address retention | none |
| `-O` signal | canonical 24-byte one-call frame and ordinary epilogue are the established era `-O2` wrapper shape |
| Loop/back-edge | none in the wrapper; recursion and loops are wholly owned by the callee |
| Relocations | one normalized `R_MIPS_26` call relocation |

The wrapper adds no policy or state of its own. Its semantic contract is the
forwarding call; a more specific public name is not asserted without symbols.

## Minimal C and flags

```c
void func_8006269C(void *node);

void func_80062F1C(void *node) {
    func_8006269C(node);
}
```

Compile: era cc1, `-O2 -G0`; no three-word, store-delay, div, or dispatch
gate.

## Single-leaf object and ROM comparison

```text
00000000 <func_80062F1C>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_8006269C
   c: 00000000  nop
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c0189a7 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c0189a7 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0x534E4,0x53958)` = `0x0474`:

```text
asm prefix: 0x5371C - 0x534E4 = 0x0238
C leaf:     0x5373C - 0x5371C = 0x0020
asm resume: 0x53958 - 0x5373C = 0x021C
closure:    0x0238 + 0x0020 + 0x021C = 0x0474
```

All sizes come from split boundaries, never aligned object sizes.

## Packed span and gates

```text
5370C: 8fb00010 = 8fb00010  preceding epilogue
53710: 27bd0030 = 27bd0030  preceding teardown
53714: 03e00008 = 03e00008  preceding return
53718: 00000000 = 00000000  preceding delay slot
5371C: 27bdffe8 = 27bdffe8  leaf 1
53720: afbf0010 = afbf0010  leaf 2
53724: 0c0189a7 = 0c0189a7  leaf 3
53728: 00000000 = 00000000  leaf 4
5372C: 8fbf0010 = 8fbf0010  leaf 5
53730: 27bd0018 = 27bd0018  leaf 6
53734: 03e00008 = 03e00008  leaf 7
53738: 00000000 = 00000000  leaf 8
5373C: 27bdffe8 = 27bdffe8  following function entry
53740: 00801821 = 00801821  following function word 2
53744: 8f8403e4 = 8f8403e4  following function word 3
53748: 24050001 = 24050001  following function word 4
PACKED_SPAN=EXACT
```

```text
Compare:  EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 347
```
