# func_8002FE78 — opcode 0x59 tagged field reader (landed)

- VRAM `0x8002FE78`, file `0x20678`, size `0x100` (64 words).
- Unit `202F8.s` carve: prefix `202F8.s` 0x380, C leaf,
  resume `20778.s` 0x194 to `2090C`.
- Profile `era_o2_g0_dispatch_80010ac8`: era `-O2 -G0` +
  `MASPSX_THREE_WORD_SYMBOL_STORE=1` + `MASPSX_DISPATCH_FOLD=jtbl_80010AC8`.
- Count transition: 769 -> 770 registered C leaves.
- Fresh-build authority: `scripts/build_us.sh` prints
  `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`;
  `scripts/verify_us.sh` prints `VERIFY_US=PASS`.

## Structure

```
andi $a0,$a0,0xFF        t = tag & 0xFF
lui  $v0,%hi(D_8009D254)
lw   $v0,%lo(D_8009D254)($v0)
sltiu $v1,$a0,0x17       t < 23
lw   $a1,0($v0)          base = *(*D_8009D254)
beqz $v1, ret
 addiu $a2,$zero,-0x3E8  ret = -1000 (delay slot)
sll  $v0,$a0,2
lui  $at,%hi(jtbl_80010AC8)
addu $at,$at,$v0
lw   $v0,%lo(jtbl_80010AC8)($at)
nop
jr   $v0
nop
```

The sentinel lives in `$a2`; the epilogue is `jr $ra; addu $v0,$a2,$zero`.
Every case assigns `$a2` and jumps to it, so the source keeps a `ret`
variable (not direct returns).

## Jump table (retail bytes, `jtbl_80010AC8` = 23 words, 0x5C)

| idx | body VRAM | field |
|----:|-----------|-------|
| 0  | 0x8002FEB0 | `lw` base+0x00 |
| 1  | 0x8002FEBC | `lh` base+0x04 |
| 2  | 0x8002FEC8 | `lhu` base+0x06 |
| 3  | 0x8002FED4 | `lw` base+0x08 |
| 4  | 0x8002FEE0 | `lh` base+0x0C |
| 6  | 0x8002FEEC | `lhu` base+0x10 |
| 7  | 0x8002FEF8 | `lbu` base+0x12 |
| 10 | 0x8002FF04 | `lh` base+0x1C |
| 11 | 0x8002FF10 | `lhu` base+0x1E |
| 12 | 0x8002FF1C | `lhu` base+0x20 |
| 13 | 0x8002FF28 | `lw` base+0x28 |
| 14 | 0x8002FF34 | `lhu` base+0x22 |
| 20 | 0x8002FF40 | `(w4C >> 9) & 1` |
| 21 | 0x8002FF4C | `(w4C >> 6) & 3` |
| 22 | 0x8002FF60 | `(w4C >> 29) & 1` |

Indices 5, 8, 9 and 15..19 target the sentinel block.

## Divergences resolved

1. **Struct layout.** `f0C` is a halfword at 0x0C, so the next field is at
   0x0E unless padded. A 2-byte pad before `f10` is required (retail reads
   `lhu 0x10`), otherwise every later case is two bytes low.
2. **Shift destination register (cases 20/21/22).** The one-expression form
   `ret = (x >> n) & m` makes cc1 shift in place in `$v0` and tail-and into
   `$a2`; retail shifts directly into `$a2` and shares the `& 1` tail
   between cases 20 and 22. Writing `ret = x >> n; ret &= m;` reproduces
   retail's register allocation exactly (5 words).
3. **`-G0`.** Retail loads `D_8009D254` absolutely
   (`lui $v0,%hi / lw $v0,%lo($v0)`), not gp-relative; under `-G8` the load
   is `lw $v0,0($gp)` and the whole function diverges.

## Odd-word dispatch table

`jtbl_80010AC8` has 23 words (0x5C), so cc1's `.align 3` pads the object
`.rodata` to 0x60. The build's dispatch strip now accepts exactly four
trailing zero bytes for odd-sized tables (`dispatch_rodata_pad_ok`,
parent commit `7dffce11`); the relocation walk and whole-image SHA-1 remain
the proofs of table identity.

## Evidence

- `try_leaf.py src/func_8002FE78.c 0x20678 0x100 --flags "-O2 -G0"
  --env MASPSX_THREE_WORD_SYMBOL_STORE=1
  --env MASPSX_DISPATCH_FOLD=jtbl_80010AC8` -> `WORDS MATCH`.
- Fresh `scripts/build_us.sh` -> `EXACT SHA-1 452fb033...`,
  `Matching claim: YES (770 registered C leaves)`.
- Fresh `scripts/verify_us.sh` -> `VERIFY_US=PASS`.
