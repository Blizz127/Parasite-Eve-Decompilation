# `func_800C8E44` — exact six-argument callback forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf **384**.

## Function hood first

- File `[0xB9644,0xB9670)`, VA `[0x800C8E44,0x800C8E70)`: `0x2C`, eleven
  words.
- Canonical return is `jr ra` at `0x800C8E68`; frame teardown at
  `0x800C8E6C` is its live delay slot.
- Exact-start callback-table reference: file `0xD11DC`, VA `0x800E09DC`,
  contains `.word func_800C8E44` in `D_800E09C8`.
- Preceding real `func_800C8D34` ends at `0xB963C/0xB9640` with `jr ra` and
  frame teardown.
- Following real `func_800C8E70` begins at `0xB9670` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_TABLE_REFERENCE_AND_CANONICAL_RETURN`.
The proof is independent of the analogous callback at `0xB8500`; this is not
padding, data, or label-derived function hood.

## Retail body and screens

```text
B9644 800C8E44 27BDFFE0  addiu sp,sp,-0x20
B9648 800C8E48 8FA20030  lw    v0,0x30(sp)
B964C 800C8E4C 8FA30034  lw    v1,0x34(sp)
B9650 800C8E50 AFBF0018  sw    ra,0x18(sp)
B9654 800C8E54 AFA20010  sw    v0,0x10(sp)
B9658 800C8E58 0C030ABC  jal   func_800C2AF0
B965C 800C8E5C AFA30014  sw    v1,0x14(sp)
B9660 800C8E60 8FBF0018  lw    ra,0x18(sp)
B9664 800C8E64 00001021  addu  v0,zero,zero
B9668 800C8E68 03E00008  jr    ra
B966C 800C8E6C 27BD0020  addiu sp,sp,0x20
```

| screen | result |
|---|---|
| Callee bucket | one direct callee, matched `func_800C2AF0`; all six incoming arguments are forwarded, then the callback returns zero |
| Stage-0 globals | none in this wrapper; accepted callee evidence establishes `D_800E2248` as the mutable `unsigned int *` base it publishes and indexes |
| Coloring pressure | low; `$a0`-`$a3` stay in argument homes while stack arguments 5/6 pass through `$v0/$v1` |
| `$v0` liveness | carries incoming stack argument 5 before the call, is call-clobbered, then receives the explicit zero result |
| Address retention | none in the wrapper; the matched callee owns base advancement and indexed addressing |
| `-O` signal | compact 32-byte nonleaf frame, call-delay stack store, and return-delay teardown match established era `-O2` output |
| Loop/back-edge | none |

The six-formal declaration is structural evidence, not speculative naming:
retail copies both incoming stack arguments to the outgoing argument area.
No `$gp` access occurs, so the campaign rule selects `-G0`.

## Minimal C and single-leaf comparison

```c
int func_800C2AF0(unsigned int *base, int unused, int index,
                  unsigned int value, int arg4, int arg5);

int func_800C8E44(unsigned int *base, int unused, int index,
                  unsigned int value, int arg4, int arg5) {
    func_800C2AF0(base, unused, index, value, arg4, arg5);
    return 0;
}
```

```text
00000000 <func_800C8E44>:
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

The former active asm span was `[0xB9480,0xB9708)` = `0x288`:

```text
asm prefix: 0xB9644 - 0xB9480 = 0x1C4
C leaf:     0xB9670 - 0xB9644 = 0x02C
asm resume: 0xB9708 - 0xB9670 = 0x098
closure:    0x1C4 + 0x02C + 0x098 = 0x288
```

The carve creates `B9670.s`; all sizes derive from retail split boundaries,
not aligned object size.

## Packed span and gates

```text
file    VA        retail   candidate  role
B9634 800C8E34  8fbf0010 8fbf0010   preceding epilogue load
B9638 800C8E38  00001021 00001021   preceding zero result
B963C 800C8E3C  03e00008 03e00008   preceding return
B9640 800C8E40  27bd0018 27bd0018   preceding return delay
B9644 800C8E44  27bdffe0 27bdffe0   leaf 1
B9648 800C8E48  8fa20030 8fa20030   leaf 2
B964C 800C8E4C  8fa30034 8fa30034   leaf 3
B9650 800C8E50  afbf0018 afbf0018   leaf 4
B9654 800C8E54  afa20010 afa20010   leaf 5
B9658 800C8E58  0c030abc 0c030abc   leaf 6
B965C 800C8E5C  afa30014 afa30014   leaf 7
B9660 800C8E60  8fbf0018 8fbf0018   leaf 8
B9664 800C8E64  00001021 00001021   leaf 9
B9668 800C8E68  03e00008 03e00008   leaf 10
B966C 800C8E6C  27bd0020 27bd0020   leaf 11
B9670 800C8E70  27bdffe8 27bdffe8   following real entry
B9674 800C8E74  afbf0010 afbf0010   following prologue save
B9678 800C8E78  3c05800e 3c05800e   following global address
B967C 800C8E7C  24a509a0 24a509a0   following global address low
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
C conversion: Phase 5HD-12850 — 384 leaves

grep matching-C count: 384
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
