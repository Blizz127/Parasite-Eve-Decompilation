# `func_800CA540` — exact signed-halfword callback twin

Leaf 327, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0xBAD40,0xBAD74)`, VRAM `[0x800CA540,0x800CA574)`, is
thirteen words and ends in canonical `jr ra; nop`. Exact-start callback-table
word `0x800E0AAC` at file `0xD12AC` targets it.

The preceding real function ends at `0x800CA538/0x800CA53C` with
`jr ra; nop`. The following real function starts at `0x800CA574` with
`addiu sp,sp,-24`. Both boundary words are executable instructions.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_TABLE_ENTRY`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; callback leaf |
| written-state Stage 0 | argument-relative only: decrement signed `values[2]`, then clamp and set `state[1]` on the threshold; no global writer |
| coloring pressure | `$a2` retains values, `$a1` retains state, `$v0` carries update/predicate/constant |
| `$v0` liveness | load → decrement/store → signed re-extension → predicate → constant 2 |
| address retention | fixed argument offsets only |
| optimization signal | load-delay nop, post-store sign extension, `slti`, and branch-delay constant select era `-O2 -G0` |
| loop/back-edge owner | none |

## C and flags

```c
void func_800CA540(void *unused, signed char *state, signed short *values) {
    values[2] -= 20;
    if (values[2] < 20) {
        values[2] = 0;
        state[1] = 2;
    }
}
```

Era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`; first phrasing, no pins,
inline assembly, or special switch. Signed halfword is proven by the
post-store `sll 16; sra 16`.

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

Single-leaf object (no relocations):

```text
00000000 <func_800CA540>:
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

Gas alignment grows the object to `0x40`; the trim guard verifies all bytes
beyond the boundary-derived `0x34` are zero.

## Carve and packed-span proof

The prior asm span was `[0xBACB4,0xBAF98)`, size `0x2E4`:

```text
prefix asm:  0xBAD40 - 0xBACB4 = 0x08C
C leaf:      0xBAD74 - 0xBAD40 = 0x034
resume asm:  0xBAF98 - 0xBAD74 = 0x224
closure:     0x08C + 0x034 + 0x224 = 0x2E4
```

Retail and candidate match across both neighboring real functions:

```text
800CA538: 03E00008  jr     ra
800CA53C: 00000000  nop
800CA540: 94C20004  lhu    v0,4(a2)
800CA544: 00000000  nop
800CA548: 2442FFEC  addiu  v0,v0,-20
800CA54C: A4C20004  sh     v0,4(a2)
800CA550: 00021400  sll    v0,v0,16
800CA554: 00021403  sra    v0,v0,16
800CA558: 28420014  slti   v0,v0,20
800CA55C: 10400003  beqz   v0,0x800CA56C
800CA560: 24020002  li     v0,2
800CA564: A4C00004  sh     zero,4(a2)
800CA568: A0A20001  sb     v0,1(a1)
800CA56C: 03E00008  jr     ra
800CA570: 00000000  nop
800CA574: 27BDFFE8  addiu  sp,sp,-24
800CA578: AFBF0010  sw     ra,16(sp)
800CA57C: 0C0308BE  jal    0x800C22F8
800CA580: 00000000  nop
```

```text
retail:    0800e003000000000400c29400000000ecff42240400c2a400140200031402001400422803004010020002240400c0a40100a2a00800e00300000000e8ffbd271000bfafbe08030c00000000
candidate: 0800e003000000000400c29400000000ecff42240400c2a400140200031402001400422803004010020002240400c0a40100a2a00800e00300000000e8ffbd271000bfafbe08030c00000000
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
327
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 327 leaves
```

Result: `MATCHED=13/13`, first phrasing; consecutive parks remain zero.
