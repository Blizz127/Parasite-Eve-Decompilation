# `func_8007FC64` — exact SDK `CD_sync` mode-1 wrapper

Outcome: **MATCHED** on the first natural C phrasing under era `-O2 -G0`,
with no maspsx behavior gate. Integrated as matching-C leaf **381** and the
first W-class rung from the 2026-08-31 Tier-2 shape partition.

## Function hood first

- File `[0x70464,0x70488)`, VA `[0x8007FC64,0x8007FC88)`: `0x24`
  bytes, nine words.
- The body ends in canonical `jr ra; nop` at `0x8007FC80/0x8007FC84`.
- Exact direct caller `0x8007F434` (`jal` word `0x0C01FF19`) targets the
  function start. It passes zero in `$a0`; the wrapper forwards that value as
  `$a1` while supplying mode 1 in the call delay slot.
- The preceding real `func_8007FC54` ends at `0x7045C/0x70460` with
  `jr ra; nop`.
- The following real `func_8007FC88` begins immediately at `0x70488` with a
  24-byte one-call frame prologue.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL_AND_CANONICAL_RETURN`. This is a real
callable wrapper, not padding, data, or a mislabeled tail.

## Retail body

```text
70464 8007FC64 27BDFFE8  addiu sp,sp,-0x18
70468 8007FC68 AFBF0010  sw    ra,0x10(sp)
7046C 8007FC6C 00802821  addu  a1,a0,zero
70470 8007FC70 0C01EC04  jal   func_8007B010
70474 8007FC74 24040001  addiu a0,zero,1
70478 8007FC78 8FBF0010  lw    ra,0x10(sp)
7047C 8007FC7C 27BD0018  addiu sp,sp,0x18
70480 8007FC80 03E00008  jr    ra
70484 8007FC84 00000000  nop
```

## Screens

| screen | result |
|---|---|
| Callee bucket | one direct callee, `func_8007B010`; `sdk_map.md` identifies it from retail strings as PsyQ `CD_sync` |
| Stage-0 globals | none accessed or written by the wrapper; CD state belongs to the SDK callee |
| Coloring pressure | only `$ra` survives the call; the sole input moves directly from `$a0` to `$a1` |
| `$v0` liveness | the callee result remains in `$v0` through the wrapper return; no result copy or local result home |
| Address retention | none; the caller-owned result pointer/value is forwarded unchanged |
| `-O` signal | canonical 24-byte era one-call frame, argument move before `jal`, fixed mode in the delay slot; established `-O2` wrapper shape |
| Loop/back-edge | none |
| Relocations | one `R_MIPS_26` relocation to `func_8007B010` |

The function has no `$gp` access, so `-G0` is required by the campaign rule.
No pin, inline assembly, file-scope assembly, fabricated nop, or maspsx gate
is used.

## Minimal C

```c
int func_8007B010(int mode, unsigned char *result);

int func_8007FC64(unsigned char *result) {
    return func_8007B010(1, result);
}
```

Compile: era GCC 2.7.2 `-O2 -G0`.

## Single-leaf object and full nine-word comparison

```text
00000000 <func_8007FC64>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 00802821  addu  a1,a0,zero
   c: 0c000000  jal   0                 R_MIPS_26 func_8007B010
  10: 24040001  addiu a0,zero,1
  14: 8fbf0010  lw    ra,16(sp)
  18: 27bd0018  addiu sp,sp,24
  1c: 03e00008  jr    ra
  20: 00000000  nop

ROM: 27bdffe8 afbf0010 00802821 0c01ec04 24040001 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 00802821 0c01ec04 24040001 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=9/9
PHRASINGS_USED=1/2
```

## Carve geometry

The former active assembly span was `[0x70464,0x704AC)` = `0x48`:

```text
asm prefix: 0x70464 - 0x70464 = 0x00
C leaf:     0x70488 - 0x70464 = 0x24
asm resume: 0x704AC - 0x70488 = 0x24
closure:    0x00 + 0x24 + 0x24 = 0x48
```

The resumed split is `asm/disc1/70488.s`. Every size comes from retail
boundary arithmetic, never from an aligned object size.

## Packed-span comparison

Retail and rebuilt candidate words are equal over `[0x70454,0x7049C)`, with
the complete nine-word leaf and real instructions on both sides:

```text
file    VA        retail   candidate  role
70454 8007FC54  3c02800a 3c02800a   preceding global-address load
70458 8007FC58  9042b56c 9042b56c   preceding byte load
7045C 8007FC5C  03e00008 03e00008   preceding return
70460 8007FC60  00000000 00000000   preceding return delay
70464 8007FC64  27bdffe8 27bdffe8   leaf 1
70468 8007FC68  afbf0010 afbf0010   leaf 2
7046C 8007FC6C  00802821 00802821   leaf 3
70470 8007FC70  0c01ec04 0c01ec04   leaf 4
70474 8007FC74  24040001 24040001   leaf 5
70478 8007FC78  8fbf0010 8fbf0010   leaf 6
7047C 8007FC7C  27bd0018 27bd0018   leaf 7
70480 8007FC80  03e00008 03e00008   leaf 8
70484 8007FC84  00000000 00000000   leaf 9
70488 8007FC88  27bdffe8 27bdffe8   following real prologue
7048C 8007FC8C  afbf0010 afbf0010   following ra save
70490 8007FC90  00802821 00802821   following argument move
70494 8007FC94  0c01eca4 0c01eca4   following call
70498 8007FC98  24040001 24040001   following call delay
PACKED_SPAN=EXACT
```

## Gates

Full build raw summary:

```text
=== Summary ===
Assemble: OK (asm units + 35 gp carves)
Compile:  OK
Pad trim: OK
Link:     OK
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
```

```text
$ sha1sum build/disc1.candidate.exe
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

$ scripts/verify_us.sh
VERIFY_RC=0
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 381 leaves

$ grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
381
```

The verifier was run on the host, as documented. A discarded diagnostic run
inside the build container exited 127 at the host-bound `.venv` splat launcher;
it was not treated as a gate. The valid host invocation above exited zero.

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
