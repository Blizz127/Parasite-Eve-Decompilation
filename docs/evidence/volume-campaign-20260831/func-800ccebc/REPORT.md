# `func_800CCEBC` — exact six-argument callback forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf **388**.

## Function hood first

- File `[0xBD6BC,0xBD6E8)`, VA `[0x800CCEBC,0x800CCEE8)`: `0x2C`, eleven
  words.
- Canonical return is `jr ra` at `0x800CCEE0`; frame teardown at
  `0x800CCEE4` is its live delay slot.
- Exact-start callback-table reference: file `0xD16A4`, VA `0x800E0EA4`, is
  `.word func_800CCEBC` inside `D_800E0E90`.
- Preceding real `func_800CCBA8` restores `$s0` and a 72-byte frame, then ends
  at `0xBD6B4/0xBD6B8` with `jr ra; nop`.
- Following real `func_800CCEE8` starts at `0xBD6E8` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_TABLE_REFERENCE_AND_CANONICAL_RETURN`.
This span is callable code by independent retail evidence, not by its label or
similarity to earlier members.

## Retail body and screens

```text
BD6BC 800CCEBC 27BDFFE0  addiu sp,sp,-0x20
BD6C0 800CCEC0 8FA20030  lw    v0,0x30(sp)
BD6C4 800CCEC4 8FA30034  lw    v1,0x34(sp)
BD6C8 800CCEC8 AFBF0018  sw    ra,0x18(sp)
BD6CC 800CCECC AFA20010  sw    v0,0x10(sp)
BD6D0 800CCED0 0C030ABC  jal   func_800C2AF0
BD6D4 800CCED4 AFA30014  sw    v1,0x14(sp)
BD6D8 800CCED8 8FBF0018  lw    ra,0x18(sp)
BD6DC 800CCEDC 00001021  addu  v0,zero,zero
BD6E0 800CCEE0 03E00008  jr    ra
BD6E4 800CCEE4 27BD0020  addiu sp,sp,0x20
```

| screen | result |
|---|---|
| Callee bucket | one direct callee, matched `func_800C2AF0`; all six callback arguments are forwarded, then zero is returned |
| Stage-0 globals | no symbolic global in the wrapper; accepted callee evidence proves mutable `unsigned int *D_800E2248`, published and indexed by the callee |
| Coloring pressure | low; register arguments remain in `$a0`-`$a3`; stack arguments 5/6 use `$v0/$v1` |
| `$v0` liveness | carries incoming argument 5 before call, is call-clobbered, then receives zero |
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

int func_800CCEBC(unsigned int *base, int unused, int index,
                  unsigned int value, int arg4, int arg5) {
    func_800C2AF0(base, unused, index, value, arg4, arg5);
    return 0;
}
```

```text
00000000 <func_800CCEBC>:
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

The former active asm span was `[0xBC7C4,0xBD780)` = `0xFBC`:

```text
asm prefix: 0xBD6BC - 0xBC7C4 = 0xEF8
C leaf:     0xBD6E8 - 0xBD6BC = 0x02C
asm resume: 0xBD780 - 0xBD6E8 = 0x098
closure:    0xEF8 + 0x02C + 0x098 = 0xFBC
```

The carve creates `BD6E8.s`; every size is retail boundary arithmetic, not
aligned object size.

## Packed span and gates

```text
file    VA        retail   candidate  role
BD6AC 800CCEAC  8fb00040 8fb00040   preceding saved-register restore
BD6B0 800CCEB0  27bd0048 27bd0048   preceding frame teardown
BD6B4 800CCEB4  03e00008 03e00008   preceding return
BD6B8 800CCEB8  00000000 00000000   preceding return delay
BD6BC 800CCEBC  27bdffe0 27bdffe0   leaf 1
BD6C0 800CCEC0  8fa20030 8fa20030   leaf 2
BD6C4 800CCEC4  8fa30034 8fa30034   leaf 3
BD6C8 800CCEC8  afbf0018 afbf0018   leaf 4
BD6CC 800CCECC  afa20010 afa20010   leaf 5
BD6D0 800CCED0  0c030abc 0c030abc   leaf 6
BD6D4 800CCED4  afa30014 afa30014   leaf 7
BD6D8 800CCED8  8fbf0018 8fbf0018   leaf 8
BD6DC 800CCEDC  00001021 00001021   leaf 9
BD6E0 800CCEE0  03e00008 03e00008   leaf 10
BD6E4 800CCEE4  27bd0020 27bd0020   leaf 11
BD6E8 800CCEE8  27bdffe8 27bdffe8   following real entry
BD6EC 800CCEEC  afbf0010 afbf0010   following prologue save
BD6F0 800CCEF0  3c05800e 3c05800e   following global address
BD6F4 800CCEF4  24a50e60 24a50e60   following global address low
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
C conversion: Phase 5HD-12850 — 388 leaves

grep matching-C count: 388
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
