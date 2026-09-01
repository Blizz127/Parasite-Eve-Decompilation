# `func_80051060` — exact gp-backed state forwarding callback

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G8`, with no
maspsx behavior gate. Integrated as matching-C leaf **392**.

## Function hood first

- File `[0x41860,0x41884)`, VA `[0x80051060,0x80051084)`: `0x24`, nine
  words.
- `func_80050204` materializes the exact start at file `0x40A0C/0x40A10`
  (`lui a1,%hi(func_80051060)` / `addiu a1,a1,%lo(func_80051060)`) and
  passes it to `func_800638D8`. This is an executable callback registration,
  not a label-derived inference.
- The body has a canonical `jr ra` at file `0x4187C` and `nop` delay slot at
  `0x41880`.
- Preceding real `func_8005100C` ends at `0x41858/0x4185C` with `jr ra; nop`.
- Following real `func_80051084` starts at `0x41884`; its absolute-address
  load/store body is already exact C.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_REGISTRATION_AND_CANONICAL_RETURN`.

## Retail body and screens

```text
41860 80051060 8F8401E8  lw    a0,0x1E8(gp)
41864 80051064 27BDFFE8  addiu sp,sp,-0x18
41868 80051068 AFBF0010  sw    ra,0x10(sp)
4186C 8005106C 0C014D92  jal   func_80053648
41870 80051070 00000000  nop
41874 80051074 8FBF0010  lw    ra,0x10(sp)
41878 80051078 27BD0018  addiu sp,sp,0x18
4187C 8005107C 03E00008  jr    ra
41880 80051080 00000000  nop
```

| screen | result |
|---|---|
| Callee bucket | one direct unresolved callee, `func_80053648`; it consumes the forwarded state pointer and reads byte fields `+4`, `+5`, and `+6` before downstream calls |
| Stage-0 globals | gp base `0x8009CD70` plus `0x1E8` resolves to `D_8009CF58`; retail initializes it from pointer-returning `func_800532B4`, and users dereference byte fields, proving pointer state rather than a scalar |
| Coloring pressure | minimal; the gp load goes directly to argument register `$a0` and no value must survive the call |
| `$v0` liveness | the callee result is discarded; the void wrapper has no result canonicalization |
| Address retention | no computed address; the sole state address is a direct gp-relative pointer load |
| `-O` signal | compact 24-byte call frame and established era call scheduling select `-O2` |
| Loop/back-edge | none |

The retail `lw ...($gp)` requires `-G8`; `_gp=0x8009CD70` gives
`D_8009CF58 - _gp = 0x1E8`.

## Minimal C and single-leaf comparison

```c
/* gp base 0x8009CD70; gp+0x1E8 resolves to D_8009CF58. */
extern unsigned char *D_8009CF58;

void func_80053648(unsigned char *state);

void func_80051060(void) {
    func_80053648(D_8009CF58);
}
```

```text
00000000 <func_80051060>:
   0: 8f840000  lw    a0,0(gp)     R_MIPS_GPREL16 D_8009CF58
   4: 27bdffe8  addiu sp,sp,-24
   8: afbf0010  sw    ra,16(sp)
   c: 0c000000  jal   0            R_MIPS_26 func_80053648
  10: 00000000  nop
  14: 8fbf0010  lw    ra,16(sp)
  18: 27bd0018  addiu sp,sp,24
  1c: 03e00008  jr    ra
  20: 00000000  nop

ROM: 8f8401e8 27bdffe8 afbf0010 0c014d92 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   8f8401e8 27bdffe8 afbf0010 0c014d92 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_GPREL16 D_8009CF58; R_MIPS_26 func_80053648
BYTE_EXACT=9/9
PHRASINGS_USED=1/2
```

Compile flags: era GCC 2.7.2 `-O2 -G8`; no pins, inline/file-scope assembly,
fabricated nops, or maspsx gates.

## Carve geometry

The former active asm span was `[0x41520,0x41884)` = `0x364`:

```text
asm prefix: 0x41860 - 0x41520 = 0x340
C leaf:     0x41884 - 0x41860 = 0x024
closure:    0x340 + 0x024 = 0x364
```

The leaf closes directly against existing C `func_80051084`; no zero-size or
invented resume span is created. Sizes are retail boundary arithmetic, not
aligned object sizes.

## Packed span and gates

```text
file    VA        retail   candidate  role
41850 80051050  1000b08f 1000b08f   preceding s0 restore
41854 80051054  1800bd27 1800bd27   preceding frame teardown
41858 80051058  0800e003 0800e003   preceding return
4185C 8005105C  00000000 00000000   preceding delay slot
41860 80051060  e801848f e801848f   leaf 1
41864 80051064  e8ffbd27 e8ffbd27   leaf 2
41868 80051068  1000bfaf 1000bfaf   leaf 3
4186C 8005106C  924d010c 924d010c   leaf 4
41870 80051070  00000000 00000000   leaf 5
41874 80051074  1000bf8f 1000bf8f   leaf 6
41878 80051078  1800bd27 1800bd27   leaf 7
4187C 8005107C  0800e003 0800e003   leaf 8
41880 80051080  00000000 00000000   leaf 9
41884 80051084  0a80023c 0a80023c   following real entry
41888 80051088  a01a4224 a01a4224   following address low
4188C 8005108C  a40282af a40282af   following gp store
41890 80051090  0800e003 0800e003   following return
PACKED_SPAN=EXACT
```

```text
BUILD_RC=0
RESULT: EXACT MATCH
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

VERIFY_RC=0
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 392 leaves

grep matching-C count: 392
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
