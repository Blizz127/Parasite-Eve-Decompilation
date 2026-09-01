# `func_800CBEE0` — exact six-argument callback forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, without a
maspsx behavior gate. Integrated as matching-C leaf **387**.

## Function hood first

- File `[0xBC6E0,0xBC70C)`, VA `[0x800CBEE0,0x800CBF0C)`: `0x2C`, eleven
  words.
- Canonical return is `jr ra` at `0x800CBF04`; frame teardown at
  `0x800CBF08` is its live delay slot.
- Exact-start callback-table reference: file `0xD1558`, VA `0x800E0D58`, is
  `.word func_800CBEE0` inside `D_800E0D40`.
- Preceding real `func_800CBCA4` restores `$s0` and its frame, then ends at
  `0xBC6D8/0xBC6DC` with `jr ra; nop`.
- Following real `func_800CBF0C` begins at `0xBC70C` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_TABLE_REFERENCE_AND_CANONICAL_RETURN`.
The table and boundaries independently establish callable code; the symbol
name and family resemblance are not the proof.

## Retail body and screens

```text
BC6E0 800CBEE0 27BDFFE0  addiu sp,sp,-0x20
BC6E4 800CBEE4 8FA20030  lw    v0,0x30(sp)
BC6E8 800CBEE8 8FA30034  lw    v1,0x34(sp)
BC6EC 800CBEEC AFBF0018  sw    ra,0x18(sp)
BC6F0 800CBEF0 AFA20010  sw    v0,0x10(sp)
BC6F4 800CBEF4 0C030ABC  jal   func_800C2AF0
BC6F8 800CBEF8 AFA30014  sw    v1,0x14(sp)
BC6FC 800CBEFC 8FBF0018  lw    ra,0x18(sp)
BC700 800CBF00 00001021  addu  v0,zero,zero
BC704 800CBF04 03E00008  jr    ra
BC708 800CBF08 27BD0020  addiu sp,sp,0x20
```

| screen | result |
|---|---|
| Callee bucket | one direct callee, matched `func_800C2AF0`; forwards all six arguments and returns zero |
| Stage-0 globals | no symbolic global in the wrapper; the callee's accepted evidence proves mutable `unsigned int *D_800E2248`, which it publishes and indexes |
| Coloring pressure | low; `$a0`-`$a3` retain argument homes and stack arguments 5/6 use `$v0/$v1` |
| `$v0` liveness | holds incoming argument 5, is call-clobbered, then receives explicit zero |
| Address retention | none in the wrapper; the callee performs base advancement and indexed access |
| `-O` signal | 32-byte nonleaf frame, useful call-delay store, and return-delay teardown are established era `-O2` output |
| Loop/back-edge | none |

The two incoming-to-outgoing stack copies prove the six-formal callback
interface even though the callee's matched body does not currently read those
formals. There is no `$gp` access, selecting `-G0`.

## Minimal C and single-leaf comparison

```c
int func_800C2AF0(unsigned int *base, int unused, int index,
                  unsigned int value, int arg4, int arg5);

int func_800CBEE0(unsigned int *base, int unused, int index,
                  unsigned int value, int arg4, int arg5) {
    func_800C2AF0(base, unused, index, value, arg4, arg5);
    return 0;
}
```

```text
00000000 <func_800CBEE0>:
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

The former active asm span was `[0xBC3F0,0xBC7A4)` = `0x3B4`:

```text
asm prefix: 0xBC6E0 - 0xBC3F0 = 0x2F0
C leaf:     0xBC70C - 0xBC6E0 = 0x02C
asm resume: 0xBC7A4 - 0xBC70C = 0x098
closure:    0x2F0 + 0x02C + 0x098 = 0x3B4
```

The carve creates `BC70C.s`; sizes are retail boundary arithmetic rather than
aligned object sizes.

## Packed span and gates

```text
file    VA        retail   candidate  role
BC6D0 800CBED0  8fb00010 8fb00010   preceding saved-register restore
BC6D4 800CBED4  27bd0018 27bd0018   preceding frame teardown
BC6D8 800CBED8  03e00008 03e00008   preceding return
BC6DC 800CBEDC  00000000 00000000   preceding return delay
BC6E0 800CBEE0  27bdffe0 27bdffe0   leaf 1
BC6E4 800CBEE4  8fa20030 8fa20030   leaf 2
BC6E8 800CBEE8  8fa30034 8fa30034   leaf 3
BC6EC 800CBEEC  afbf0018 afbf0018   leaf 4
BC6F0 800CBEF0  afa20010 afa20010   leaf 5
BC6F4 800CBEF4  0c030abc 0c030abc   leaf 6
BC6F8 800CBEF8  afa30014 afa30014   leaf 7
BC6FC 800CBEFC  8fbf0018 8fbf0018   leaf 8
BC700 800CBF00  00001021 00001021   leaf 9
BC704 800CBF04  03e00008 03e00008   leaf 10
BC708 800CBF08  27bd0020 27bd0020   leaf 11
BC70C 800CBF0C  27bdffe8 27bdffe8   following real entry
BC710 800CBF10  afbf0010 afbf0010   following prologue save
BC714 800CBF14  3c05800e 3c05800e   following global address
BC718 800CBF18  24a50d08 24a50d08   following global address low
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
C conversion: Phase 5HD-12850 — 387 leaves

grep matching-C count: 387
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
