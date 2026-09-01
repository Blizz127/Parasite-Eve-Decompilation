# `func_800C9A00` — exact signed-halfword callback twin

Leaf 326, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0xBA200,0xBA234)`, VRAM `[0x800C9A00,0x800C9A34)`, is
`0x34` bytes / thirteen words and ends in canonical `jr ra; nop`. Its exact
start occurs as a callback-table word at file `0xD11BC`, VRAM `0x800E09BC`:

```text
0xD11BC: 800C9A00
```

The preceding real function ends at `0x800C99F8/0x800C99FC` with
`jr ra; nop`. The following real `func_800C9A34` begins at `0x800C9A34`
with `lhu v0,4(a2)`. Both boundaries are executable function instructions.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_TABLE_ENTRY`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; callback leaf |
| written-state Stage 0 | argument-relative only: decrements signed halfword `values[2]`; below 20 clamps it to zero and writes byte 2 to `state[1]`; no global writer |
| coloring pressure | `$a2` retains `values`, `$a1` retains `state`; `$v0` carries update, sign extension, predicate, then constant 2 |
| `$v0` liveness | load → decrement → store → signed re-extension → predicate; reused in the branch delay slot |
| address retention | none beyond fixed argument offsets |
| optimization signal | load-delay nop, post-store sign extension, `slti`, and branch-delay constant select era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

## C and flags

```c
void func_800C9A00(void *unused, signed char *state, signed short *values) {
    values[2] -= 20;
    if (values[2] < 20) {
        values[2] = 0;
        state[1] = 2;
    }
}
```

Era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`; first phrasing, no pins,
inline assembly, or special maspsx switch. The signed type is proven by
retail's `sll 16; sra 16` before the comparison.

## Full thirteen-word comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `94C20004` | `94C20004` | `lhu v0,4(a2)` |
| 1 | `00000000` | `00000000` | `nop` |
| 2 | `2442FFEC` | `2442FFEC` | `addiu v0,v0,-20` |
| 3 | `A4C20004` | `A4C20004` | `sh v0,4(a2)` |
| 4 | `00021400` | `00021400` | `sll v0,v0,16` |
| 5 | `00021403` | `00021403` | `sra v0,v0,16` |
| 6 | `28420014` | `28420014` | `slti v0,v0,20` |
| 7 | `10400003` | `10400003` | `beqz v0,+3` |
| 8 | `24020002` | `24020002` | `li v0,2` |
| 9 | `A4C00004` | `A4C00004` | `sh zero,4(a2)` |
| 10 | `A0A20001` | `A0A20001` | `sb v0,1(a1)` |
| 11 | `03E00008` | `03E00008` | `jr ra` |
| 12 | `00000000` | `00000000` | `nop` |

Single-leaf object, with no relocations:

```text
00000000 <func_800C9A00>:
   0: 94c20004  lhu    v0,4(a2)
   4: 00000000  nop
   8: 2442ffec  addiu  v0,v0,-20
   c: a4c20004  sh     v0,4(a2)
  10: 00021400  sll    v0,v0,0x10
  14: 00021403  sra    v0,v0,0x10
  18: 28420014  slti   v0,v0,20
  1c: 10400003  beqz   v0,0x2c
  20: 24020002  li     v0,2
  24: a4c00004  sh     zero,4(a2)
  28: a0a20001  sb     v0,1(a1)
  2c: 03e00008  jr     ra
  30: 00000000  nop
```

Gas aligns `.text` to `0x40`; the trim guard proved the twelve bytes after
the boundary-derived `0x34` body are zero.

## Carve geometry

The prior asm span was `[0xBA174,0xBA400)`, size `0x28C`:

```text
prefix asm:  0xBA200 - 0xBA174 = 0x08C
C leaf:      0xBA234 - 0xBA200 = 0x034
resume asm:  0xBA400 - 0xBA234 = 0x1CC
closure:     0x08C + 0x034 + 0x1CC = 0x28C
```

All sizes come from span boundaries, not aligned object sizes.

## Packed-span proof

Retail and candidate match across the preceding return, all thirteen words,
and the following function entry:

```text
800C99F8: 03E00008  jr     ra
800C99FC: 00000000  nop
800C9A00: 94C20004  lhu    v0,4(a2)
800C9A04: 00000000  nop
800C9A08: 2442FFEC  addiu  v0,v0,-20
800C9A0C: A4C20004  sh     v0,4(a2)
800C9A10: 00021400  sll    v0,v0,16
800C9A14: 00021403  sra    v0,v0,16
800C9A18: 28420014  slti   v0,v0,20
800C9A1C: 10400003  beqz   v0,0x800C9A2C
800C9A20: 24020002  li     v0,2
800C9A24: A4C00004  sh     zero,4(a2)
800C9A28: A0A20001  sb     v0,1(a1)
800C9A2C: 03E00008  jr     ra
800C9A30: 00000000  nop
800C9A34: 94C20004  lhu    v0,4(a2)
800C9A38: 00000000  nop
800C9A3C: 2442FFF8  addiu  v0,v0,-8
800C9A40: A4C20004  sh     v0,4(a2)
```

```text
retail:    0800e003000000000400c29400000000ecff42240400c2a400140200031402001400422803004010020002240400c0a40100a2a00800e003000000000400c29400000000f8ff42240400c2a4
candidate: 0800e003000000000400c29400000000ecff42240400c2a400140200031402001400422803004010020002240400c0a40100a2a00800e003000000000400c29400000000f8ff42240400c2a4
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
326
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 326 leaves
```

Result: `MATCHED=13/13`, first phrasing; consecutive parks remain zero.
