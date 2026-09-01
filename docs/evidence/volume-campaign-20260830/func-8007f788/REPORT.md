# `func_8007F788` — exact unsigned-byte forwarding wrapper

Outcome: **MATCHED** on the first natural C phrasing under era `-O2 -G0`,
with no maspsx behavior gate. Integrated as matching-C leaf 354 and Tier-2
continuation rung 13.

## Function hood and retail span

- File `[0x6FF88,0x6FFA8)`, VRAM `[0x8007F788,0x8007F7A8)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x8007F7A0`, with frame teardown in its
  live delay slot at `0x8007F7A4`.
- One executable direct call targets the exact start at `0x80069D88`
  (file `0x5A588`, word `0x0C01FDE2`). The caller immediately tests bit
  `0x10` of the returned byte.
- Preceding real `func_8007F778` ends immediately with `jr ra; nop` at
  file `0x6FF80/0x6FF84`.
- Following real `func_8007F7A8` begins immediately at file `0x6FFA8`
  with another 24-byte call frame.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL_AND_CANONICAL_RETURN`; this is a real
callable function, not padding, data, or a tail fragment.

## Retail body

```text
6FF88 8007F788 27BDFFE8  addiu sp,sp,-0x18
6FF8C 8007F78C AFBF0010  sw    ra,0x10(sp)
6FF90 8007F790 0C01FF15  jal   func_8007FC54
6FF94 8007F794 00000000  nop
6FF98 8007F798 8FBF0010  lw    ra,0x10(sp)
6FF9C 8007F79C 304200FF  andi  v0,v0,0xFF
6FFA0 8007F7A0 03E00008  jr    ra
6FFA4 8007F7A4 27BD0018  addiu sp,sp,0x18
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one already-C callee, `func_8007FC54`, the unsigned-byte getter for `D_8009B56C` |
| Stage-0 globals | no direct symbolic access; the sole state read belongs to the already-proven callee, and the direct caller independently treats the result as an 8-bit bitfield |
| Coloring pressure | only `$ra` is preserved; there are no arguments or locals |
| `$v0` liveness | callee result remains in `$v0` and is zero-extended in place with `andi 0xFF`; the caller then tests bit `0x10` |
| Address retention | none |
| `-O` signal | canonical 24-byte one-call frame, in-place narrow-return canonicalization, and useful teardown delay slot are era `-O2` |
| Loop/back-edge | none |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
unsigned char func_8007FC54(void);

unsigned char func_8007F788(void) {
    return func_8007FC54();
}
```

The unsigned-byte return type is the codegen lever: era cc1 emits the retail
post-call `andi v0,v0,0xFF` at this ABI boundary.

Compile: era GCC 2.7.2 `-O2 -G0`; no three-word-symbol, store-delay,
division, dispatch-fold, pin, or inline-assembly mechanism is enabled.

## Single-leaf object and ROM comparison

```text
00000000 <func_8007F788>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_8007FC54
   c: 00000000  nop
  10: 8fbf0010  lw    ra,16(sp)
  14: 304200ff  andi  v0,v0,0xff
  18: 03e00008  jr    ra
  1c: 27bd0018  addiu sp,sp,24

ROM: 27bdffe8 afbf0010 0c01ff15 00000000 8fbf0010 304200ff 03e00008 27bd0018
C:   27bdffe8 afbf0010 0c01ff15 00000000 8fbf0010 304200ff 03e00008 27bd0018
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active assembly span was exactly `[0x6FF88,0x6FFA8)` =
`0x0020`:

```text
asm prefix: 0x6FF88 - 0x6FF88 = 0x0000
C leaf:     0x6FFA8 - 0x6FF88 = 0x0020
asm resume: 0x6FFA8 - 0x6FFA8 = 0x0000
closure:    0x0000 + 0x0020 + 0x0000 = 0x0020
```

All sizes come from retail boundaries, never aligned object sizes.

## Full packed-span comparison and gates

Retail and rebuilt candidate words are equal over `[0x6FF78,0x6FFB8)`,
including four words on each boundary side:

```text
6FF78: 3c02800a = 3c02800a  preceding global address
6FF7C: 8c423608 = 8c423608  preceding global load
6FF80: 03e00008 = 03e00008  preceding return
6FF84: 00000000 = 00000000  preceding return delay
6FF88: 27bdffe8 = 27bdffe8  leaf 1
6FF8C: afbf0010 = afbf0010  leaf 2
6FF90: 0c01ff15 = 0c01ff15  leaf 3
6FF94: 00000000 = 00000000  leaf 4
6FF98: 8fbf0010 = 8fbf0010  leaf 5
6FF9C: 304200ff = 304200ff  leaf 6
6FFA0: 03e00008 = 03e00008  leaf 7
6FFA4: 27bd0018 = 27bd0018  leaf 8
6FFA8: 27bdffe8 = 27bdffe8  following function entry
6FFAC: afbf0010 = afbf0010  following saved return
6FFB0: 0c01ff2b = 0c01ff2b  following call
6FFB4: 00000000 = 00000000  following call delay
PACKED_SPAN=EXACT
```

```text
sha1sum build/disc1.candidate.exe
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

scripts/verify_us.sh: exit 0
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: 354 leaves

grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
354
```

No pins, inline assembly, file-scope assembly, fabricated padding, or
mismatch-hiding mechanism is present.
