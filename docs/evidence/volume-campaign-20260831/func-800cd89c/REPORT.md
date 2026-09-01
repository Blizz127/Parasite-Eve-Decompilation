# `func_800CD89C` — exact six-argument callback forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf **389**.

## Function hood first

- File `[0xBE09C,0xBE0C8)`, VA `[0x800CD89C,0x800CD8C8)`: `0x2C`, eleven
  words.
- Canonical return is `jr ra` at `0x800CD8C0`; frame teardown at
  `0x800CD8C4` is its live delay slot.
- Exact-start callback-table reference: file `0xD1758`, VA `0x800E0F58`, is
  `.word func_800CD89C` inside `D_800E0F48`.
- Preceding real `func_800CD728` ends at `0xBE094/0xBE098` with `jr ra` and
  its live 24-byte frame teardown.
- Following real `func_800CD8C8` starts at `0xBE0C8` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_TABLE_REFERENCE_AND_CANONICAL_RETURN`.
This span is callable code by independent retail evidence, not by its label or
similarity to earlier members.

## Retail body and screens

```text
BE09C 800CD89C 27BDFFE0  addiu sp,sp,-0x20
BE0A0 800CD8A0 8FA20030  lw    v0,0x30(sp)
BE0A4 800CD8A4 8FA30034  lw    v1,0x34(sp)
BE0A8 800CD8A8 AFBF0018  sw    ra,0x18(sp)
BE0AC 800CD8AC AFA20010  sw    v0,0x10(sp)
BE0B0 800CD8B0 0C030ABC  jal   func_800C2AF0
BE0B4 800CD8B4 AFA30014  sw    v1,0x14(sp)
BE0B8 800CD8B8 8FBF0018  lw    ra,0x18(sp)
BE0BC 800CD8BC 00001021  addu  v0,zero,zero
BE0C0 800CD8C0 03E00008  jr    ra
BE0C4 800CD8C4 27BD0020  addiu sp,sp,0x20
```

| screen | result |
|---|---|
| Callee bucket | one direct callee, matched `func_800C2AF0`; all six callback arguments are forwarded, then zero is returned |
| Stage-0 globals | no symbolic global in the wrapper; accepted callee evidence proves mutable `unsigned int *D_800E2248`, published and indexed by the callee |
| Coloring pressure | low; register arguments remain in `$a0`-`$a3`; stack arguments 5/6 use `$v0/$v1` |
| `$v0` liveness | carries incoming argument 5 before the call, is call-clobbered, then receives zero |
| Address retention | none in the wrapper; the callee owns base advancement and access |
| `-O` signal | compact 32-byte call frame, call-delay stack store, and return-delay teardown are established era `-O2` output |
| Loop/back-edge | none |

The paired stack copies prove a six-formal callback ABI even though the
callee's matched semantic body currently ignores those last two arguments.
No `$gp` access appears, selecting `-G0`.

## Minimal C and single-leaf comparison

```c
int func_800C2AF0(unsigned int *base, int unused, int index,
                  unsigned int value, int arg4, int arg5);

int func_800CD89C(unsigned int *base, int unused, int index,
                  unsigned int value, int arg4, int arg5) {
    func_800C2AF0(base, unused, index, value, arg4, arg5);
    return 0;
}
```

```text
00000000 <func_800CD89C>:
   0: 27bdffe0  addiu sp,sp,-32
   4: 8fa20030  lw    v0,48(sp)
   8: 8fa30034  lw    v1,52(sp)
   c: afbf0018  sw    ra,24(sp)
  10: afa20010  sw    v0,16(sp)
  14: 0c000000  jal   0                 R_MIPS_26 func_800C2AF0
  18: afa30014  sw    v1,20(sp)
  1c: 8fbf0018  lw    ra,24(sp)
  20: 00001021  move  v0,zero
  24: 03e00008  jr    ra
  28: 27bd0020  addiu sp,sp,32

ROM: 27bdffe0 8fa20030 8fa30034 afbf0018 afa20010 0c030abc afa30014 8fbf0018 00001021 03e00008 27bd0020
C:   27bdffe0 8fa20030 8fa30034 afbf0018 afa20010 0c030abc afa30014 8fbf0018 00001021 03e00008 27bd0020
RELOCS_NORMALIZED=R_MIPS_26 func_800C2AF0
BYTE_EXACT=11/11
PHRASINGS_USED=1/2
```

Compile flags: era GCC 2.7.2 `-O2 -G0`; no pins, inline/file-scope assembly,
fabricated nops, or maspsx gates.

## Carve geometry

The former active asm span was `[0xBDF28,0xBE160)` = `0x238`:

```text
asm prefix: 0xBE09C - 0xBDF28 = 0x174
C leaf:     0xBE0C8 - 0xBE09C = 0x02C
asm resume: 0xBE160 - 0xBE0C8 = 0x098
closure:    0x174 + 0x02C + 0x098 = 0x238
```

The carve creates `BE0C8.s`; every size is retail boundary arithmetic, not
aligned object size.

## Packed span and gates

```text
file    VA        retail   candidate  role
BE08C 800CD88C  8fbf0010 8fbf0010   preceding ra restore
BE090 800CD890  00001021 00001021   preceding zero result
BE094 800CD894  03e00008 03e00008   preceding return
BE098 800CD898  27bd0018 27bd0018   preceding frame teardown
BE09C 800CD89C  27bdffe0 27bdffe0   leaf 1
BE0A0 800CD8A0  8fa20030 8fa20030   leaf 2
BE0A4 800CD8A4  8fa30034 8fa30034   leaf 3
BE0A8 800CD8A8  afbf0018 afbf0018   leaf 4
BE0AC 800CD8AC  afa20010 afa20010   leaf 5
BE0B0 800CD8B0  0c030abc 0c030abc   leaf 6
BE0B4 800CD8B4  afa30014 afa30014   leaf 7
BE0B8 800CD8B8  8fbf0018 8fbf0018   leaf 8
BE0BC 800CD8BC  00001021 00001021   leaf 9
BE0C0 800CD8C0  03e00008 03e00008   leaf 10
BE0C4 800CD8C4  27bd0020 27bd0020   leaf 11
BE0C8 800CD8C8  27bdffe8 27bdffe8   following real entry
BE0CC 800CD8CC  afbf0010 afbf0010   following prologue save
BE0D0 800CD8D0  3c05800e 3c05800e   following global address
BE0D4 800CD8D4  24a50f28 24a50f28   following global address low
PACKED_SPAN=EXACT
```

```text
BUILD_RC=0
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

VERIFY_RC=0
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 389 leaves

grep matching-C count: 389
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
