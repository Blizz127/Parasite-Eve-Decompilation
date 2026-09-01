# `func_8007A488` — exact `CD_ready` forwarding wrapper

Outcome: **MATCHED** on the first natural C phrasing under era `-O2 -G0`,
with no maspsx behavior gate. Integrated as matching-C leaf 352 and the first
Tier-2 continuation rung after the bounded ten-leaf probe.

## Function hood and retail span

- File `[0x6AC88,0x6ACA8)`, VRAM `[0x8007A488,0x8007A4A8)`: `0x20`
  bytes, eight words.
- The body ends in canonical `jr ra; nop` at `0x8007A4A0/0x8007A4A4`.
- One executable direct call targets the exact start, at `0x8007C5E8`
  (file `0x6CDE8`, word `0x0C01E922`). The caller passes a result buffer in
  `$a1 = sp + 0x30`, tests the returned `$v0` against 5, and then consumes
  bytes 0 and 1 of the result buffer.
- Preceding real `func_8007A468` ends immediately with `jr ra; nop` at file
  `0x6AC80/0x6AC84`.
- Following real `func_8007A4A8` begins immediately at file `0x6ACA8` with
  its symbolic callback-global load.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL_AND_CANONICAL_RETURN`; this is a real
callable function, not padding, data, or a mislabeled tail.

## Retail body

```text
6AC88 8007A488 27BDFFE8  addiu sp,sp,-0x18
6AC8C 8007A48C AFBF0010  sw    ra,0x10(sp)
6AC90 8007A490 0C01ECA4  jal   func_8007B290
6AC94 8007A494 00000000  nop
6AC98 8007A498 8FBF0010  lw    ra,0x10(sp)
6AC9C 8007A49C 27BD0018  addiu sp,sp,0x18
6ACA0 8007A4A0 03E00008  jr    ra
6ACA4 8007A4A4 00000000  nop
```

## Screens

| Screen | Result |
|---|---|
| Callee bucket | one unresolved retail/asm callee, `func_8007B290`; the adjacent SDK string map identifies it as PsyQ `CD_ready` |
| Stage-0 globals | none accessed directly by this wrapper; result-buffer and CD state effects belong to the callee |
| Coloring pressure | only `$ra` is preserved; `$a0/$a1` forward unchanged and no local value spans the call |
| `$v0` liveness | the callee result remains in `$v0` through the wrapper return; the direct caller immediately compares it with 5 |
| Address retention | none; the caller-owned result pointer forwards unchanged in `$a1` |
| `-O` signal | canonical 24-byte one-call frame with unchanged argument forwarding and no redundant result copy is era `-O2` |
| Loop/back-edge | none in the wrapper |
| Relocations | one normalized `R_MIPS_26` call relocation |

The SDK name is independently supported by the embedded `"CD_ready"` string
and the established mapping in `docs/ai_context/sdk_map.md`; the C keeps the
retail symbol because this rung does not rename existing interfaces.

## Minimal C and flags

```c
int func_8007B290(int mode, unsigned char *result);

int func_8007A488(int mode, unsigned char *result) {
    return func_8007B290(mode, result);
}
```

Compile: era GCC 2.7.2 `-O2 -G0`; no three-word-symbol, store-delay, division,
dispatch-fold, pin, or inline-assembly mechanism is enabled.

## Single-leaf object and ROM comparison

```text
00000000 <func_8007A488>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_8007B290
   c: 00000000  nop
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c01eca4 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c01eca4 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

The former active assembly span was `[0x6AC00,0x6ACA8)` = `0x00A8`:

```text
asm prefix: 0x6AC88 - 0x6AC00 = 0x0088
C leaf:     0x6ACA8 - 0x6AC88 = 0x0020
asm resume: 0x6ACA8 - 0x6ACA8 = 0x0000
closure:    0x0088 + 0x0020 + 0x0000 = 0x00A8
```

All sizes come from retail split boundaries, never from aligned object sizes.
Regenerating the split reduced `6AC00.s` to the proven `0x88`-byte prefix;
the trim guard had correctly refused the stale pre-regeneration object rather
than silently discarding nonzero leaf bytes.

## Full packed-span comparison and gates

Retail and rebuilt candidate words are equal over `[0x6AC78,0x6ACB8)`,
including four words on each boundary side:

```text
6AC78: 8fbf0010 = 8fbf0010  preceding epilogue load
6AC7C: 27bd0018 = 27bd0018  preceding frame teardown
6AC80: 03e00008 = 03e00008  preceding return
6AC84: 00000000 = 00000000  preceding return delay
6AC88: 27bdffe8 = 27bdffe8  leaf 1
6AC8C: afbf0010 = afbf0010  leaf 2
6AC90: 0c01eca4 = 0c01eca4  leaf 3
6AC94: 00000000 = 00000000  leaf 4
6AC98: 8fbf0010 = 8fbf0010  leaf 5
6AC9C: 27bd0018 = 27bd0018  leaf 6
6ACA0: 03e00008 = 03e00008  leaf 7
6ACA4: 00000000 = 00000000  leaf 8
6ACA8: 3c02800a = 3c02800a  following function entry
6ACAC: 8c42afb4 = 8c42afb4  following global load
6ACB0: 3c01800a = 3c01800a  following store address
6ACB4: 03e00008 = 03e00008  following return
PACKED_SPAN=EXACT
```

```text
sha1sum build/disc1.candidate.exe
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

scripts/verify_us.sh: exit 0
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: 352 leaves

grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
352
```

No pins, inline assembly, file-scope assembly, fabricated padding, or
mismatch-hiding mechanism is present.
