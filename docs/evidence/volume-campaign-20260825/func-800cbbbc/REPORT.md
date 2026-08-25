# `func_800CBBBC` — exact signed-halfword callback twin

Leaf 328, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0xBC3BC,0xBC3F0)`, VRAM `[0x800CBBBC,0x800CBBF0)`, is
thirteen words and ends in canonical `jr ra; nop`. Exact-start callback-table
word `0x800E0BA8` at file `0xD13A8` targets it.

The preceding real function ends at `0x800CBBB4/0x800CBBB8` with
`jr ra; nop`; following real `func_800CBBF0` begins with `lhu v0,4(a2)`.
Both boundaries are executable instructions.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_TABLE_ENTRY`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; callback leaf |
| written-state Stage 0 | argument-relative only: signed halfword decrement/clamp and byte state transition; no global writer |
| coloring pressure | `$a2` retains values, `$a1` retains state, `$v0` carries update/predicate/constant |
| `$v0` liveness | load → decrement/store → sign extension → predicate → constant 2 |
| address retention | fixed argument offsets only |
| optimization signal | load-delay nop, post-store sign extension, `slti`, and branch-delay constant select era `-O2 -G0` |
| loop/back-edge owner | none |

## C and flags

```c
void func_800CBBBC(void *unused, signed char *state, signed short *values) {
    values[2] -= 20;
    if (values[2] < 20) {
        values[2] = 0;
        state[1] = 2;
    }
}
```

Era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`; first phrasing, no pins,
inline assembly, or special switch. Signed halfword is proven by retail's
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

Single-leaf object, with no relocations:

```text
00000000 <func_800CBBBC>:
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

The trim guard proves all gas-alignment bytes beyond the `0x34` body zero.

## Carve and packed-span proof

The prior asm span was `[0xBC330,0xBC7A4)`, size `0x474`:

```text
prefix asm:  0xBC3BC - 0xBC330 = 0x08C
C leaf:      0xBC3F0 - 0xBC3BC = 0x034
resume asm:  0xBC7A4 - 0xBC3F0 = 0x3B4
closure:     0x08C + 0x034 + 0x3B4 = 0x474
```

Retail and candidate match across both real boundaries:

```text
800CBBB4: 03E00008  jr     ra
800CBBB8: 00000000  nop
800CBBBC: 94C20004  lhu    v0,4(a2)
800CBBC0: 00000000  nop
800CBBC4: 2442FFEC  addiu  v0,v0,-20
800CBBC8: A4C20004  sh     v0,4(a2)
800CBBCC: 00021400  sll    v0,v0,16
800CBBD0: 00021403  sra    v0,v0,16
800CBBD4: 28420014  slti   v0,v0,20
800CBBD8: 10400003  beqz   v0,0x800CBBE8
800CBBDC: 24020002  li     v0,2
800CBBE0: A4C00004  sh     zero,4(a2)
800CBBE4: A0A20001  sb     v0,1(a1)
800CBBE8: 03E00008  jr     ra
800CBBEC: 00000000  nop
800CBBF0: 94C20004  lhu    v0,4(a2)
800CBBF4: 00000000  nop
800CBBF8: 2442FFF8  addiu  v0,v0,-8
800CBBFC: A4C20004  sh     v0,4(a2)
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
328
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 328 leaves
```

Result: `MATCHED=13/13`, first phrasing; consecutive parks remain zero.
