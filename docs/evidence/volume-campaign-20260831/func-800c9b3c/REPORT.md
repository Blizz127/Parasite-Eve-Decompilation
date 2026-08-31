# `func_800C9B3C` — exact six-argument callback forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, without a
maspsx behavior gate. Integrated as matching-C leaf **385**.

## Function hood first

- File `[0xBA33C,0xBA368)`, VA `[0x800C9B3C,0x800C9B68)`: `0x2C`, eleven
  words.
- Canonical return is `jr ra` at `0x800C9B60`, with frame teardown at
  `0x800C9B64` in its live delay slot.
- Exact-start callback-table reference: file `0xD12C4`, VA `0x800E0AC4`, is
  `.word func_800C9B3C` inside `D_800E0AB4`.
- Preceding real `func_800C9A70` ends at file `0xBA334/0xBA338` with
  `jr ra` and frame teardown.
- Following real `func_800C9B68` begins at `0xBA368` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_TABLE_REFERENCE_AND_CANONICAL_RETURN`.
This proof was performed independently of the earlier forwarding wrappers;
the span is not padding, data, or a label-only hypothesis.

## Retail body and screens

```text
BA33C 800C9B3C 27BDFFE0  addiu sp,sp,-0x20
BA340 800C9B40 8FA20030  lw    v0,0x30(sp)
BA344 800C9B44 8FA30034  lw    v1,0x34(sp)
BA348 800C9B48 AFBF0018  sw    ra,0x18(sp)
BA34C 800C9B4C AFA20010  sw    v0,0x10(sp)
BA350 800C9B50 0C030ABC  jal   func_800C2AF0
BA354 800C9B54 AFA30014  sw    v1,0x14(sp)
BA358 800C9B58 8FBF0018  lw    ra,0x18(sp)
BA35C 800C9B5C 00001021  addu  v0,zero,zero
BA360 800C9B60 03E00008  jr    ra
BA364 800C9B64 27BD0020  addiu sp,sp,0x20
```

| screen | result |
|---|---|
| Callee bucket | one direct callee, matched `func_800C2AF0`; forwards all six arguments, discards the callee result, returns zero |
| Stage-0 globals | no symbolic global in the wrapper; accepted callee evidence proves mutable `unsigned int *D_800E2248`, which the callee publishes and uses as its indexed base |
| Coloring pressure | low; register arguments remain in `$a0`-`$a3`, and stack arguments 5/6 pass through `$v0/$v1` |
| `$v0` liveness | carries incoming argument 5 before the call, is call-clobbered, then becomes the explicit zero result |
| Address retention | none in this wrapper; the callee owns base advancement and indexed access |
| `-O` signal | 32-byte one-call frame, useful call-delay store, and return-delay teardown are the established era `-O2` shape |
| Loop/back-edge | none |

Retail's copies at `0xBA340/0xBA344` prove that the callback interface has six
arguments even though the current matched callee semantics do not read the
last two. There is no `$gp` access, selecting `-G0`.

## Minimal C and single-leaf comparison

```c
int func_800C2AF0(unsigned int *base, int unused, int index,
                  unsigned int value, int arg4, int arg5);

int func_800C9B3C(unsigned int *base, int unused, int index,
                  unsigned int value, int arg4, int arg5) {
    func_800C2AF0(base, unused, index, value, arg4, arg5);
    return 0;
}
```

```text
00000000 <func_800C9B3C>:
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

The former active asm span was `[0xBA234,0xBA400)` = `0x1CC`:

```text
asm prefix: 0xBA33C - 0xBA234 = 0x108
C leaf:     0xBA368 - 0xBA33C = 0x02C
asm resume: 0xBA400 - 0xBA368 = 0x098
closure:    0x108 + 0x02C + 0x098 = 0x1CC
```

The carve creates `BA368.s`. Every size is boundary arithmetic, not aligned
object size.

## Packed span and gates

```text
file    VA        retail   candidate  role
BA32C 800C9B2C  8fbf0010 8fbf0010   preceding epilogue load
BA330 800C9B30  00001021 00001021   preceding zero result
BA334 800C9B34  03e00008 03e00008   preceding return
BA338 800C9B38  27bd0018 27bd0018   preceding return delay
BA33C 800C9B3C  27bdffe0 27bdffe0   leaf 1
BA340 800C9B40  8fa20030 8fa20030   leaf 2
BA344 800C9B44  8fa30034 8fa30034   leaf 3
BA348 800C9B48  afbf0018 afbf0018   leaf 4
BA34C 800C9B4C  afa20010 afa20010   leaf 5
BA350 800C9B50  0c030abc 0c030abc   leaf 6
BA354 800C9B54  afa30014 afa30014   leaf 7
BA358 800C9B58  8fbf0018 8fbf0018   leaf 8
BA35C 800C9B5C  00001021 00001021   leaf 9
BA360 800C9B60  03e00008 03e00008   leaf 10
BA364 800C9B64  27bd0020 27bd0020   leaf 11
BA368 800C9B68  27bdffe8 27bdffe8   following real entry
BA36C 800C9B6C  afbf0010 afbf0010   following prologue save
BA370 800C9B70  3c05800e 3c05800e   following global address
BA374 800C9B74  24a50a94 24a50a94   following global address low
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
C conversion: Phase 5HD-12850 — 385 leaves

grep matching-C count: 385
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
