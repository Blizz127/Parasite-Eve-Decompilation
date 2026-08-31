# `func_80080AC4` — exact fixed-success command-wrapper twin

Outcome: **MATCHED** on the first natural C phrasing under era `-O2 -G0`,
with no maspsx behavior gate. Integrated as matching-C leaf 355 and Tier-2
continuation rung 14.

## Function hood and retail span

- File `[0x712C4,0x712E4)`, VRAM `[0x80080AC4,0x80080AE4)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x80080ADC`, with frame teardown in its
  live delay slot at `0x80080AE0`.
- One executable direct call targets the exact start at `0x80016894`
  (file `0x7094`, word `0x0C0202B1`). The caller builds a four-byte
  command buffer at `sp + 0x20`, passes that exact address, and continues
  with an independently established result of 1.
- Preceding real `func_800809E0` ends with `jr ra` and live teardown at
  file `0x712B4/0x712B8`; the two intervening zero words at
  `0x712BC/0x712C0` are explicit alignment, not part of either function.
- Following real `func_80080AE4` begins immediately at file `0x712E4`
  with a 24-byte call frame.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL_AND_CANONICAL_RETURN`. The preceding
alignment is called out rather than misrepresented as executable function
body; the exact-start call independently excludes padding or data.

## Retail body

```text
712C4 80080AC4 27BDFFE8  addiu sp,sp,-0x18
712C8 80080AC8 AFBF0010  sw    ra,0x10(sp)
712CC 80080ACC 0C01EE59  jal   func_8007B964
712D0 80080AD0 00000000  nop
712D4 80080AD4 8FBF0010  lw    ra,0x10(sp)
712D8 80080AD8 24020001  addiu v0,zero,1
712DC 80080ADC 03E00008  jr    ra
712E0 80080AE0 27BD0018  addiu sp,sp,0x18
```

The body is word-identical to independently hood-proven
`func_8007A88C`; this rung still carries its own caller and boundary proof.

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_8007B964`; it copies four caller bytes into the CD command-register path and returns zero |
| Stage-0 globals | none accessed directly; the exact caller supplies a four-byte stack buffer and all hardware/global effects belong to the callee |
| Coloring pressure | only `$ra` is preserved; the sole pointer forwards unchanged and no local survives the call |
| `$v0` liveness | callee zero is dead; retail replaces it with constant 1 before returning |
| Address retention | none |
| `-O` signal | canonical 24-byte call frame, late fixed result, and useful teardown delay slot are era `-O2` |
| Loop/back-edge | none |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
int func_8007B964(void *buffer);

int func_80080AC4(void *buffer) {
    func_8007B964(buffer);
    return 1;
}
```

Compile: era GCC 2.7.2 `-O2 -G0`; no three-word-symbol, store-delay,
division, dispatch-fold, pin, or inline-assembly mechanism is enabled.

## Single-leaf object and ROM comparison

```text
00000000 <func_80080AC4>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_8007B964
   c: 00000000  nop
  10: 8fbf0010  lw    ra,16(sp)
  14: 24020001  addiu v0,zero,1
  18: 03e00008  jr    ra
  1c: 27bd0018  addiu sp,sp,24

ROM: 27bdffe8 afbf0010 0c01ee59 00000000 8fbf0010 24020001 03e00008 27bd0018
C:   27bdffe8 afbf0010 0c01ee59 00000000 8fbf0010 24020001 03e00008 27bd0018
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The current active assembly span before this carve was
`[0x71150,0x712E4)` = `0x0194`:

```text
asm prefix: 0x712C4 - 0x71150 = 0x0174
C leaf:     0x712E4 - 0x712C4 = 0x0020
asm resume: 0x712E4 - 0x712E4 = 0x0000
closure:    0x0174 + 0x0020 + 0x0000 = 0x0194
```

All sizes come from retail boundaries, never aligned object sizes.

## Full packed-span comparison and gates

Retail and rebuilt candidate words are equal over `[0x712B4,0x712F4)`,
including the real predecessor epilogue, explicit alignment, and following
function words:

```text
712B4: 03e00008 = 03e00008  preceding return
712B8: 27bd0020 = 27bd0020  preceding teardown delay
712BC: 00000000 = 00000000  explicit alignment
712C0: 00000000 = 00000000  explicit alignment
712C4: 27bdffe8 = 27bdffe8  leaf 1
712C8: afbf0010 = afbf0010  leaf 2
712CC: 0c01ee59 = 0c01ee59  leaf 3
712D0: 00000000 = 00000000  leaf 4
712D4: 8fbf0010 = 8fbf0010  leaf 5
712D8: 24020001 = 24020001  leaf 6
712DC: 03e00008 = 03e00008  leaf 7
712E0: 27bd0018 = 27bd0018  leaf 8
712E4: 27bdffe8 = 27bdffe8  following function entry
712E8: afbf0010 = afbf0010  following saved return
712EC: 0c01efd1 = 0c01efd1  following call
712F0: 00000000 = 00000000  following call delay
PACKED_SPAN=EXACT
```

```text
sha1sum build/disc1.candidate.exe
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

scripts/verify_us.sh: exit 0
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: 355 leaves

grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
355
```

No pins, inline assembly, file-scope assembly, fabricated padding, or
mismatch-hiding mechanism is present.
