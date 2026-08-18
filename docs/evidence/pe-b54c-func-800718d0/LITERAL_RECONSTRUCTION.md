# Literal reconstruction — `func_800718D0`

Executable SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
File offset `pc - 0x80010000 + 0x800`. 29 words,
`0x800718D0..0x80071944` exclusive.

```text
0x800718D0  27BDFFE0  addiu $sp, $sp, -32
0x800718D4  AFBF0018  sw    $ra, 24($sp)
0x800718D8  AFB10014  sw    $s1, 20($sp)
0x800718DC  AFB00010  sw    $s0, 16($sp)
0x800718E0  8C820004  lw    $v0, 4($a0)          # TIM flag
0x800718E4  00000000  nop
0x800718E8  30420008  andi  $v0, $v0, 8
0x800718EC  10400005  beq   $v0, $zero, 0x80071904
0x800718F0  00008021  addu  $s0, $zero, $zero    # delay: s0=0
0x800718F4  8C820008  lw    $v0, 8($a0)          # CLUT bnum
0x800718F8  24900008  addiu $s0, $a0, 8
0x800718FC  0801C642  j     0x80071908
0x80071900  02021021  addu  $v0, $s0, $v0        # image = clut + bnum
0x80071904  24820008  addiu $v0, $a0, 8          # no-CLUT image
0x80071908  24440004  addiu $a0, $v0, 4          # image RECT
0x8007190C  2451000C  addiu $s1, $v0, 12         # image pixels
0x80071910  0C01D41B  jal   func_8007506C        # IMAGE first
0x80071914  02202821  addu  $a1, $s1, $zero
0x80071918  12000003  beq   $s0, $zero, 0x80071928
0x8007191C  26040004  addiu $a0, $s0, 4          # CLUT RECT
0x80071920  0C01D41B  jal   func_8007506C        # CLUT second
0x80071924  2605000C  addiu $a1, $s0, 12
0x80071928  02201021  addu  $v0, $s1, $zero
0x8007192C  8FBF0018  lw    $ra, 24($sp)
0x80071930  8FB10014  lw    $s1, 20($sp)
0x80071934  8FB00010  lw    $s0, 16($sp)
0x80071938  27BD0020  addiu $sp, $sp, 32
0x8007193C  03E00008  jr    $ra
0x80071940  00000000  nop
```

Sole callee: `func_8007506C` twice (`0x0C01D41B`). No MMIO, no
`func_80030894`.

## Pack sites (in `func_8006AD40`, after `jal 718D0`)

```text
0x8006AFB0  a1 = 0
0x8006AFB4  loop:
              pack D_80091648+a1 {A,B,C,D} -> dests +8/+0xA
0x8006AFF8  sh packed1, 0x1650($at)     # tpage  D_80091650 when a1=0
0x8006B02C  sh packed2, 0x1652($at)     # CLUT   D_80091652 when a1=0
0x8006B030  a1 += 0x10
0x8006B034  sltiu a1, 0x20
0x8006B038  bne -> AFB4
```

`a1=0` writes record 0 (`0x0025` / `0x3F14`). `a1=0x10` writes
record 1 (`0x0026` / `0x3F15`). Same GetTPage/GetClut formula as
B54D records 2/3.
