# `func_80085DC4` — exact command-9 forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf **393**.

## Function hood first

- File `[0x765C4,0x765E8)`, VA `[0x80085DC4,0x80085DE8)`: `0x24`, nine
  words.
- Direct caller at file `0x765A4`, VA `0x80085DA4`, is
  `jal func_80085DC4` from real `func_80085D84`.
- The body has a canonical `jr ra` at file `0x765E0` and `nop` delay slot at
  `0x765E4`.
- Preceding real `func_80085D84` ends at `0x765B8/0x765BC` with `jr ra` and
  its live frame teardown. File `0x765C0` is one explicit alignment nop.
- Files `0x765E8`, `0x765EC`, and `0x765F0` are explicit alignment nops;
  following real `func_80085DF4` starts at `0x765F4` with a frame prologue.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLER_AND_CANONICAL_RETURN`.
`BOUNDARY_CLASSIFICATION=REAL_FUNCTIONS_WITH_EXPLICIT_ALIGNMENT_NOPS`.
The padding remains asm on both sides and is not counted as part of C.

## Retail body and screens

```text
765C4 80085DC4 27BDFFE8  addiu sp,sp,-0x18
765C8 80085DC8 AFBF0010  sw    ra,0x10(sp)
765CC 80085DCC 00802821  addu  a1,a0,zero
765D0 80085DD0 0C01CF31  jal   func_80073CC4
765D4 80085DD4 24040009  addiu a0,zero,9
765D8 80085DD8 8FBF0010  lw    ra,0x10(sp)
765DC 80085DDC 27BD0018  addiu sp,sp,0x18
765E0 80085DE0 03E00008  jr    ra
765E4 80085DE4 00000000  nop
```

| screen | result |
|---|---|
| Callee bucket | one direct unresolved callee, `func_80073CC4`; it dispatches through the function pointer at `D_8009566C->+8`, preserving the wrapper's `(9, value)` argument contract |
| Stage-0 globals | none in this wrapper; the callee owns `D_8009566C` and its indirect dispatch table |
| Coloring pressure | minimal; incoming `$a0` is copied to `$a1`, then command 9 is placed in `$a0` in the call delay slot |
| `$v0` liveness | callee result is discarded; wrapper is void |
| Address retention | none |
| `-O` signal | compact call frame and useful immediate in the `jal` delay slot are established era `-O2` output |
| Loop/back-edge | none |

No gp-relative access appears in the wrapper, selecting `-G0`.

## Minimal C and single-leaf comparison

```c
void func_80073CC4(int command, int value);

void func_80085DC4(int value) {
    func_80073CC4(9, value);
}
```

```text
00000000 <func_80085DC4>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 00802821  move  a1,a0
   c: 0c000000  jal   0             R_MIPS_26 func_80073CC4
  10: 24040009  li    a0,9
  14: 8fbf0010  lw    ra,16(sp)
  18: 27bd0018  addiu sp,sp,24
  1c: 03e00008  jr    ra
  20: 00000000  nop

ROM: 27bdffe8 afbf0010 00802821 0c01cf31 24040009 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 00802821 0c01cf31 24040009 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_26 func_80073CC4
BYTE_EXACT=9/9
PHRASINGS_USED=1/2
```

Compile flags: era GCC 2.7.2 `-O2 -G0`; no pins, inline/file-scope assembly,
fabricated nops, or maspsx gates.

## Carve geometry

The former active asm span was `[0x75F44,0x778E0)` = `0x199C`:

```text
asm prefix: 0x765C4 - 0x75F44 = 0x0680
C leaf:     0x765E8 - 0x765C4 = 0x0024
asm resume: 0x778E0 - 0x765E8 = 0x12F8
closure:    0x0680 + 0x0024 + 0x12F8 = 0x199C
```

The carve creates `765E8.s`, beginning with the three retail alignment nops.
All sizes come from retail boundaries, not aligned object sizes.

## Packed span and gates

```text
file    VA        retail   candidate  role
765B4 80085DB4  1000b08f 1000b08f   preceding s0 restore
765B8 80085DB8  0800e003 0800e003   preceding return
765BC 80085DBC  1800bd27 1800bd27   preceding live delay slot
765C0 80085DC0  00000000 00000000   preceding alignment
765C4 80085DC4  e8ffbd27 e8ffbd27   leaf 1
765C8 80085DC8  1000bfaf 1000bfaf   leaf 2
765CC 80085DCC  21288000 21288000   leaf 3
765D0 80085DD0  31cf010c 31cf010c   leaf 4
765D4 80085DD4  09000424 09000424   leaf 5
765D8 80085DD8  1000bf8f 1000bf8f   leaf 6
765DC 80085DDC  1800bd27 1800bd27   leaf 7
765E0 80085DE0  0800e003 0800e003   leaf 8
765E4 80085DE4  00000000 00000000   leaf 9
765E8 80085DE8  00000000 00000000   following alignment 1
765EC 80085DEC  00000000 00000000   following alignment 2
765F0 80085DF0  00000000 00000000   following alignment 3
765F4 80085DF4  e8ffbd27 e8ffbd27   following real entry
765F8 80085DF8  1000b0af 1000b0af   following prologue save
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
C conversion: Phase 5HD-12850 — 393 leaves

grep matching-C count: 393
```

`MATCHED_C=YES`
`INTEGRATED_EXACT=YES`
