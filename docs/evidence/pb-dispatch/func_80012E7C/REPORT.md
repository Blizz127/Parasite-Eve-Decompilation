# func_80012E7C — selector-driven triple copy (landed)

- VRAM `0x80012E7C`, file `0x367C`, size `0x238` (142 words).
- Unit `3420.s` carve: prefix `3420.s` 0x25C, C leaf,
  resume `38B4.s` 0x2A04 to `62B8`.
- Profile `era_o2_g0_dispatch_80010080`: era `-O2 -G0` +
  `MASPSX_THREE_WORD_SYMBOL_STORE=1` + `MASPSX_DISPATCH_FOLD=jtbl_80010080`.
- Count transition: 770 -> 771 registered C leaves.
- Fresh-build authority: `scripts/build_us.sh` prints
  `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`;
  `scripts/verify_us.sh` prints `VERIFY_US=PASS`.

## Structure

```
lw   $v0,0($a0)          v0 = *ctx
lw   $v1,0($v0)          sel = **ctx
sltiu $v0,$v1,7
beqz $v0, end
sll  $v0,$v1,2
lui  $at,%hi(jtbl_80010080)
addu $at,$at,$v0
lw   $v0,%lo(jtbl_80010080)($at)
nop
jr   $v0
end: addiu $v0,$zero,1
```

## Cases (all copy three fields into `*ctx[1]`, `*ctx[2]`, `*ctx[3]`)

| sel | source offsets | width |
|----:|----------------|-------|
| 0 | `+0x28` `+0x2C` `+0x30` | word |
| 1 | `+0x40` `+0x44` `+0x48` | word |
| 2 | `+0x68` `+0x6C` `+0x70` | word |
| 3 | `+0x78` `+0x7C` `+0x80` | word |
| 4 | `+0x88` `+0x8C` `+0x90` | word |
| 5 | `+0x38` `+0x3A` `+0x3C` | signed halfword (`lh`), sign-extended by the `sw` |
| 6 | `+0x58` `+0x5C` `+0x60` | word |

All sources are fields of `*D_8009D2F0`. Every store reloads the pointer
value (`lw $v0,D_8009D2F0` then `lw $v0,off($v0)`): the writes go through
`ctx[1..3]`, which may alias the global, so cc1 must not CSE the value.

## Divergence resolved

`D_8009D2F0` must be declared as a **pointer** (`extern unsigned char
*D_8009D2F0;`), not as an incomplete array. With the array form cc1 hoists
the global's *address* into a `la $4` register and reloads through it
(`lui/addiu` + `lw $2,0($4)`), which maspsx renders as a 3-word
materialisation. Retail instead materialises the loaded value in the
destination register for each use (`lui $v0,%hi / lw $v0,%lo($v0)`), which
the pointer declaration reproduces exactly. This is the opposite of the
`D_8009D254`-style arrays used by the other dispatch leaves.

`jtbl_80010080` has 7 words (0x1C), an odd count, so the object `.rodata`
is 8-byte padded; the central `dispatch_rodata_pad_ok` fix (parent commit
`7dffce11`) accepts that trailing zero pad.

Candidate `.text` is `0x240` (8 pad bytes) before the YAML-derived
retail-size trim.

## Evidence

- `try_leaf.py src/func_80012E7C.c 0x367C 0x238 --flags "-O2 -G0"
  --env MASPSX_THREE_WORD_SYMBOL_STORE=1
  --env MASPSX_DISPATCH_FOLD=jtbl_80010080` -> `WORDS MATCH (+8 pad bytes)`.
- Fresh `scripts/build_us.sh` -> `EXACT SHA-1 452fb033...`,
  `Matching claim: YES (771 registered C leaves)`.
- Fresh `scripts/verify_us.sh` -> `VERIFY_US=PASS`.
