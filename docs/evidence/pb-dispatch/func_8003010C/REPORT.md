# func_8003010C — opcode 0x59 slot tagged reader (landed)

- VRAM `0x8003010C`, file `0x2090C`, size `0x114` (69 words).
- Unit `202F8.s` (now carved: prefix `202F8.s` 0x614, C leaf, resume `20A20.s` 0x314 to `20D34`).
- Profile `era_o2_g0_dispatch_80010b28`: era `-O2 -G0` +
  `MASPSX_THREE_WORD_SYMBOL_STORE=1` + `MASPSX_DISPATCH_FOLD=jtbl_80010B28`.
- Count transition: 768 -> 769 registered C leaves.
- Fresh-build authority: `scripts/build_us.sh` prints
  `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`;
  `scripts/verify_us.sh` prints `VERIFY_US=PASS`.

## Structure

```
lw   $a0,0($a0)          slot = *actor
andi $a1,$a1,0xFF        t = tag & 0xFF
addiu $a1,$a1,-0x29      idx = t - 41
sltiu $v0,$a1,0x5A       idx < 90
beqz $v0, ret
 addiu $v1,$zero,-0x3E8  ret = -1000 (delay slot)
sll  $v0,$a1,2
lui  $at,%hi(jtbl_80010B28)
addu $at,$at,$v0
lw   $v0,%lo(jtbl_80010B28)($at)
nop
jr   $v0
nop
```

The `ret` sentinel lives in `$v1` and is moved to `$v0` by the shared
epilogue (`addu $v0,$v1,$zero`), so each case assigns `$v1` and jumps to
`ret` — the same shape as `func_8002FE78`.

## Jump table (retail bytes, `jtbl_80010B28` = 90 words, 0x168)

Only nine indices have real bodies; every other in-range index targets the
sentinel. Index = tag - 0x29 (file offset `0x1328`):

| idx | tag | body VRAM | body |
|----:|----:|-----------|------|
| 0  | 41  | 0x80030140 | `lb` slot+0x04 |
| 2  | 43  | 0x8003014C | `lhu` slot+0x0C |
| 3  | 44  | 0x80030158 | `lw` slot+0x10, clamp negative to 0 |
| 7  | 48  | 0x80030170 | `lbu` slot[(w00>>17 & 0x70) + 0x1C] |
| 19 | 60  | 0x80030190 | `lw` slot+0x88, clamp negative to 0 |
| 20 | 61  | 0x800301A8 | `lhu` slot+0x8C |
| 36 | 77  | 0x800301B4 | `(w00 >> 13) & 3` |
| 41 | 82  | 0x800301C8 | `byte[3] & 0x3F` |
| 89 | 130 | 0x800301D4 | destructive get, see below |

Tag 130 body:

```
lw   $a1,0xCC($a0)
lui  $v0,0x01000000
and  $v0,$a1,$v0
beqz $v0, zero
 lui $v1,0x000C0000
lw   $v0,0($a0)
and  $v0,$v0,$v1
bne  $v0,$v1, ret
 addu $v1,$zero,$zero
lui  $v0,0xFEFF
ori  $v0,$v0,0xFFFF
and  $v0,$a1,$v0
sw   $v0,0xCC($a0)
j    ret
 addiu $v1,$zero,1
zero: addu $v1,$zero,$zero
```

i.e. the destructive read only fires when bit 24 of slot+0xCC is set *and*
the low `0x000C0000` field of `*slot` is fully set.

## Divergences resolved

1. **`b03` overlaps `w00`.** Retail reads `lbu 3($a0)` and `lw 0($a0)` from
   the same word. A struct field at 0x03 cannot coexist with an `int` at
   0x00; the byte is accessed via `((unsigned char *)slot)[3]` instead, and
   all following fields are laid out from 0x04.
2. **Shift destination register (idx 36) and mask destination (idx 41).**
   The single-expression forms `(x >> 13) & 3` and `byte[3] & 0x3F` make
   cc1 compute into `$v0` and mask into `$v1`; retail computes directly
   into `$v1` (ret). Writing them as `ret = x >> 13; ret &= 3;` (and
   likewise for the byte mask) reproduces retail.
3. **`addu` operand order (idx 7).** Grouping the constant with the index
   (`slot + (X + 0x1C)`) makes cc1 emit `addu v0,v0,a0` and an additional
   word; the left-associative `slot + X + 0x1C` yields retail's
   `addu v0,a0,v0` with `lbu 0x1C(v0)`.

Candidate `.text` is `0x120` (12 pad bytes) before the YAML-derived
retail-size trim; trimmed build is byte-exact.

## Evidence

- `try_leaf.py src/func_8003010C.c 0x2090C 0x114 --flags "-O2 -G0"
  --env MASPSX_THREE_WORD_SYMBOL_STORE=1
  --env MASPSX_DISPATCH_FOLD=jtbl_80010B28` -> `WORDS MATCH (+12 pad bytes)`.
- Fresh `scripts/build_us.sh` -> `EXACT SHA-1 452fb033...`,
  `Matching claim: YES (769 registered C leaves)`.
- Fresh `scripts/verify_us.sh` -> `VERIFY_US=PASS`.
