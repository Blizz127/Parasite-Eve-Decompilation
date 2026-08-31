# `func_8007A88C` — exact fixed-success command wrapper

Outcome: **MATCHED** on the first natural C phrasing under era `-O2 -G0`,
with no maspsx behavior gate. Integrated as matching-C leaf 353 and Tier-2
continuation rung 12.

## Function hood and retail span

- File `[0x6B08C,0x6B0AC)`, VRAM `[0x8007A88C,0x8007A8AC)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x8007A8A4`, with frame teardown in its
  live delay slot at `0x8007A8A8`.
- One executable direct call targets the exact start at `0x80087180`
  (file `0x77980`, word `0x0C01EA23`). That caller passes the address of
  the four-byte command buffer `D_8009D1C8`, then immediately returns the
  wrapper's result.
- Preceding real `func_8007A740` ends immediately with `jr ra` and live
  frame teardown at file `0x6B084/0x6B088`.
- Following real `func_8007A8AC` begins immediately at file `0x6B0AC`
  with a 24-byte call frame.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL_AND_CANONICAL_RETURN`; this is a real
callable function, not padding, data, or a tail fragment.

## Retail body

```text
6B08C 8007A88C 27BDFFE8  addiu sp,sp,-0x18
6B090 8007A890 AFBF0010  sw    ra,0x10(sp)
6B094 8007A894 0C01EE59  jal   func_8007B964
6B098 8007A898 00000000  nop
6B09C 8007A89C 8FBF0010  lw    ra,0x10(sp)
6B0A0 8007A8A0 24020001  addiu v0,zero,1
6B0A4 8007A8A4 03E00008  jr    ra
6B0A8 8007A8A8 27BD0018  addiu sp,sp,0x18
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_8007B964`; it copies four caller bytes into the CD command-register path and returns zero, while this wrapper deliberately canonicalizes success to 1 |
| Stage-0 globals | none accessed directly; the exact caller supplies `D_8009D1C8`, and all hardware/global effects are inside the callee |
| Coloring pressure | only `$ra` is preserved; the sole argument forwards unchanged and the callee result is intentionally discarded |
| `$v0` liveness | callee zero is dead; retail writes constant 1 into `$v0` after restoring `$ra`, and the caller returns that 1 |
| Address retention | none in the wrapper; the argument pointer is not needed after the call |
| `-O` signal | canonical 24-byte one-call frame, dead callee result, late constant result, and useful return teardown delay slot are era `-O2` |
| Loop/back-edge | none in the wrapper |
| Relocations | one normalized `R_MIPS_26` call relocation |

## Minimal C and flags

```c
int func_8007B964(void *buffer);

int func_8007A88C(void *buffer) {
    func_8007B964(buffer);
    return 1;
}
```

Compile: era GCC 2.7.2 `-O2 -G0`; no three-word-symbol, store-delay,
division, dispatch-fold, pin, or inline-assembly mechanism is enabled.

## Single-leaf object and ROM comparison

```text
00000000 <func_8007A88C>:
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

The former active assembly span was `[0x6ACD0,0x6C930)` = `0x1C60`:

```text
asm prefix: 0x6B08C - 0x6ACD0 = 0x03BC
C leaf:     0x6B0AC - 0x6B08C = 0x0020
asm resume: 0x6C930 - 0x6B0AC = 0x1884
closure:    0x03BC + 0x0020 + 0x1884 = 0x1C60
```

All sizes come from retail boundaries, never aligned object sizes.

## Full packed-span comparison and gates

Retail and rebuilt candidate words are equal over `[0x6B07C,0x6B0BC)`,
including four words on each boundary side:

```text
6B07C: 8fb10014 = 8fb10014  preceding saved-register restore
6B080: 8fb00010 = 8fb00010  preceding saved-register restore
6B084: 03e00008 = 03e00008  preceding return
6B088: 27bd0038 = 27bd0038  preceding teardown delay
6B08C: 27bdffe8 = 27bdffe8  leaf 1
6B090: afbf0010 = afbf0010  leaf 2
6B094: 0c01ee59 = 0c01ee59  leaf 3
6B098: 00000000 = 00000000  leaf 4
6B09C: 8fbf0010 = 8fbf0010  leaf 5
6B0A0: 24020001 = 24020001  leaf 6
6B0A4: 03e00008 = 03e00008  leaf 7
6B0A8: 27bd0018 = 27bd0018  leaf 8
6B0AC: 27bdffe8 = 27bdffe8  following function entry
6B0B0: afbf0010 = afbf0010  following saved return
6B0B4: 0c01efd1 = 0c01efd1  following call
6B0B8: 00000000 = 00000000  following call delay
PACKED_SPAN=EXACT
```

```text
sha1sum build/disc1.candidate.exe
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

scripts/verify_us.sh: exit 0
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: 353 leaves

grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
353
```

No pins, inline assembly, file-scope assembly, fabricated padding, or
mismatch-hiding mechanism is present.
