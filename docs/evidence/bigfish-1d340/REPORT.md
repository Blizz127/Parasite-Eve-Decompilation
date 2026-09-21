# `func_8001D340` — player battle tick (8,596 B) — structural report

**Branch `agent/bigfish-1d340`, worktree `/tmp/pe-agent-bigfish`. Outcome: NOT
MATCHED. Deliverable: complete structural map + closest compiling candidate +
levers tried + recommendation.**

## 1. Baseline gate — PASS

Fresh, on this worktree:

```
scripts/split_us.sh         -> disc1 plan: 1268 spans (870 c, 396 asm, 2 rodata)
scripts/build_us.sh         -> EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh        -> Matching claim: YES (870 registered C leaves)
                               VERIFY_US=PASS
try_leaf sanity             -> src/func_8006DBE0.c 0x5E3E0 0x38 => WORDS MATCH
```

Baseline log: `/tmp/bigfish_baseline.log`.

## 2. The single most important finding: this function CANNOT be carved

`func_8001D340` has, in the whole 0x2194-byte body:

* exactly **one** stack adjustment into the frame — `addiu $sp,$sp,-0x328` at
  0x8001D340 (file 0xDB40);
* exactly **one** frame teardown — `addiu $sp,$sp,0x328` at 0x8001F4C8;
* exactly **one** `jr $ra` — at 0x8001F4CC (the epilogue);
* **one** entry and **one** exit; no nested prologue/epilogue pair anywhere.

The repo's partial-span technique ("prefix asm, C leaf, resume asm", e.g.
`configs/USA/disc1.yaml` lines 56-90, 181, 263-303) only works when the asm blob
contains one or more **self-contained functions** with their own
prologue/epilogue. Here there is nothing to carve: a C span covering a prefix
would make cc1 emit its own C epilogue at the cut point, which retail does not
have; a C span covering a suffix would make cc1 emit a C prologue that retail
does not have. A mid-function `goto` to a symbol outside the C function is not
valid C. **Conclusion: do not spend time trying to carve this leaf — it must be
matched whole or left as asm.**

## 3. Block map

168 basic blocks (delay slots folded into the branching block), 2,149
instructions, file 0xDB40-0xFCD4 (VRAM 0x8001D340-0x8001F4D4).

| region | blocks | instrs | bytes | file range | purpose |
|---|---|---:|---:|---|---|
| R1 | 0-34 | 176 | 0x2C0 | 0xDB40-0xDE00 | entry, `mode&0xFF` gate, `D_8009D1A0&0x100` gate, approach-speed (`rec+0x10 += rec+0x24`, flags&0xC0 /0x100), approach-rate ramp (`rec+0x8/0x28/0x2C/0x30/0x34`, `func_8006DF50(0x455)`), `mode==1` command/move-flag -> `D_8009D2E8` bit0 |
| R2 | 35-52 | 547 | 0x88C | 0xDE00-0xE68C | clamp `rec+0x8`; `flags&0x2000` PE-mode: phase (`D_8009D1E8&3`) quad colours to `D_800B0158..` (4 unrolled cases), `cmd 18` when `actor[0xE]!=0x12`, `D_8009CE30` (gp+0xC0) `0x5A` countdown -> reset colours + `rec+0x8=0x10000`, else player_stop_motion |
| R3a | 53-68 | 525 | 0x834 | 0xE68C-0xEEC0 | `!(D_8009D1A0&0x100)`: `rec+0x34` PE timer decay + phase colours to `D_800B0134..` / reset to `D_800B0134/0x17C` |
| R3b | 69-123 | 375 | 0x5DC | 0xEEC0-0xF49C | flag timers (`0x800000` -> `D_8009D234`, `0x180000`), enemy-list walk `D_8009D20C` (hit acknowledgement -> `func_8006DCE4`/`func_8001F4D4`, contact damage `node+0x98&0x2000000`, damage numbers `rec+0x50..0x57`, `rec+0x48/0x49/0x4A` contact-recovery motion via `func_80077CF4`/`func_80077DC4`), `func_8001F9C4` |
| R4 | 124-144 | 124 | 0x1F0 | 0xF49C-0xF68C | damage-number timers `rec+0x50`/`rec+0x58` (`func_80032B0C`), `rec+0xE=rec+0xC`, `rec+8<=0` -> set PE mode + cancel attacks (`func_80021054`, `func_8006F6D4(D_8009D200/D_8009D2FC)`, `func_80021D4C`) |
| R5 | 145-148 | 305 | 0x4C4 | 0xF68C-0xFB50 | `D_8009D28C==1` -> reset AT colours (`D_800B00EC`/`D_800B0110`) + PE colours, both fully unrolled |
| R6 | 149-167 | 97 | 0x184 | 0xFB50-0xFCD4 | exit: if `rec+0xC<=0` stop motion + reset all colours, `func_80020CE4` (if flags&0x10000), `func_800374E8`, cancel attacks, `D_8009D1AC&=~0x300`, `D_8009D28C=3`, `cmd 19`, `func_8006DE80(0x46B)`, `D_8009D244=0`, `func_80062F9C`, `func_80067CBC`, `D_8009D1A0&=~4`, `D_8009D2E8|=1`, `rec+0xC=0`, epilogue |

Full 168-row table (index, file offset, VRAM, size, instruction count, region,
local labels, first instruction, terminator):

| # | file | VRAM | size | n | region | labels | first | terminator |
|---|---|---|---|---|---|---|---|---|
|   0 | 0x0DB40 | 0x8001D340 | 0x30 |  12 | R1 | - | addiu $sp, $sp, -0x328 | addiu $s2, $a0, 0x4C |
|   1 | 0x0DB70 | 0x8001D370 | 0x18 |   6 | R1 | - | lui $v0, %hi(D_8009D1A0) | andi $v1, $s0, 0xFF |
|   2 | 0x0DB88 | 0x8001D388 | 0x20 |   8 | R1 | - | lhu $v1, 0x10($a0) | sh $a1, 0x10($a0) |
|   3 | 0x0DBA8 | 0x8001D3A8 | 0xC |   3 | R1 | - | addiu $v0, $zero, 0x80 | nop  |
|   4 | 0x0DBB4 | 0x8001D3B4 | 0x2C |  11 | R1 | .L8001D3B4 | lui $v0, (0x66666667 >> 16) | subu $v0, $a1, $v0 |
|   5 | 0x0DBE0 | 0x8001D3E0 | 0xC |   3 | R1 | .L8001D3E0 | andi $v0, $a2, 0x100 | nop  |
|   6 | 0x0DBEC | 0x8001D3EC | 0x10 |   4 | R1 | - | lhu $v0, 0x24($a0) | addu $v0, $a1, $v0 |
|   7 | 0x0DBFC | 0x8001D3FC | 0x4 |   1 | R1 | .L8001D3FC | sh $v0, 0x10($a0) | sh $v0, 0x10($a0) |
|   8 | 0x0DC00 | 0x8001D400 | 0x24 |   9 | R1 | .L8001D400 | lui $a1, %hi(D_8009D278) | nop  |
|   9 | 0x0DC24 | 0x8001D424 | 0x14 |   5 | R1 | - | lw $v0, 0x0($s2) | nop  |
|  10 | 0x0DC38 | 0x8001D438 | 0x14 |   5 | R1 | - | lw $v0, 0x30($a1) | sw $v0, 0x30($a1) |
|  11 | 0x0DC4C | 0x8001D44C | 0x8 |   2 | R1 | - | addiu $v0, $zero, 0x1 | sw $v0, 0x30($a1) |
|  12 | 0x0DC54 | 0x8001D454 | 0x2C |  11 | R1 | .L8001D454 | lw $v1, 0x30($a1) | nop  |
|  13 | 0x0DC80 | 0x8001D480 | 0x4 |   1 | R1 | - | break 7 | break 7 |
|  14 | 0x0DC84 | 0x8001D484 | 0xC |   3 | R1 | .L8001D484 | addiu $at, $zero, -0x1 | lui $at, (0x80000000 >> 16) |
|  15 | 0x0DC90 | 0x8001D490 | 0x8 |   2 | R1 | - | bne $v1, $at, .L8001D49C | nop  |
|  16 | 0x0DC98 | 0x8001D498 | 0x4 |   1 | R1 | - | break 6 | break 6 |
|  17 | 0x0DC9C | 0x8001D49C | 0x20 |   8 | R1 | .L8001D49C | mflo $v1 | addiu $v0, $zero, 0x1999 |
|  18 | 0x0DCBC | 0x8001D4BC | 0x4 |   1 | R1 | - | sw $v0, 0x2C($a1) | sw $v0, 0x2C($a1) |
|  19 | 0x0DCC0 | 0x8001D4C0 | 0x30 |  12 | R1 | .L8001D4C0 | lui $a0, %hi(D_8009D278) | addiu $v0, $zero, 0xF0 |
|  20 | 0x0DCF0 | 0x8001D4F0 | 0x1C |   7 | R1 | - | lui $a0, %hi(D_800B0E08) | addiu $a1, $zero, 0x455 |
|  21 | 0x0DD0C | 0x8001D50C | 0x18 |   6 | R1 | - | addu $a2, $zero, $zero | sw $v0, 0x10($sp) |
|  22 | 0x0DD24 | 0x8001D524 | 0x4 |   1 | R1 | .L8001D524 | andi $v1, $s0, 0xFF | andi $v1, $s0, 0xFF |
|  23 | 0x0DD28 | 0x8001D528 | 0xC |   3 | R1 | .L8001D528 | addiu $v0, $zero, 0x1 | nop  |
|  24 | 0x0DD34 | 0x8001D534 | 0x24 |   9 | R1 | - | lui $v0, %hi(D_8009D254) | addiu $v0, $zero, 0x5 |
|  25 | 0x0DD58 | 0x8001D558 | 0x8 |   2 | R1 | - | bne $a0, $v0, .L8001D5B8 | nop  |
|  26 | 0x0DD60 | 0x8001D560 | 0x1C |   7 | R1 | .L8001D560 | lw $v0, 0x6C($v1) | nop  |
|  27 | 0x0DD7C | 0x8001D57C | 0x14 |   5 | R1 | - | lw $v0, 0x4C($v1) | nop  |
|  28 | 0x0DD90 | 0x8001D590 | 0x8 |   2 | R1 | - | jal func_80021054 | nop  |
|  29 | 0x0DD98 | 0x8001D598 | 0xC |   3 | R1 | - | sll $v0, $v0, 24 | nop  |
|  30 | 0x0DDA4 | 0x8001D5A4 | 0x14 |   5 | R1 | .L8001D5A4 | lui $v0, %hi(D_8009D2E8) | and $v0, $v0, $v1 |
|  31 | 0x0DDB8 | 0x8001D5B8 | 0x20 |   8 | R1 | .L8001D5B8 | lui $v0, %hi(D_8009D254) | addiu $v1, $zero, -0x2 |
|  32 | 0x0DDD8 | 0x8001D5D8 | 0x10 |   4 | R1 | - | lui $v0, %hi(D_8009D2E8) | and $v0, $v0, $v1 |
|  33 | 0x0DDE8 | 0x8001D5E8 | 0x10 |   4 | R1 | .L8001D5E8 | lui $v0, %hi(D_8009D2E8) | ori $v0, $v0, 0x1 |
|  34 | 0x0DDF8 | 0x8001D5F8 | 0x8 |   2 | R1 | .L8001D5F8 | lui $at, %hi(D_8009D2E8) | sw $v0, %lo(D_8009D2E8)($at) |
|  35 | 0x0DE00 | 0x8001D600 | 0x1C |   7 | R2 | .L8001D600 | lui $v1, %hi(D_8009D278) | nop  |
|  36 | 0x0DE1C | 0x8001D61C | 0x4 |   1 | R2 | - | sw $zero, 0x8($v1) | sw $zero, 0x8($v1) |
|  37 | 0x0DE20 | 0x8001D620 | 0x14 |   5 | R2 | .L8001D620 | lw $v0, 0x0($s2) | nop  |
|  38 | 0x0DE34 | 0x8001D634 | 0x18 |   6 | R2 | - | lui $v0, %hi(D_8009D1E8) | addiu $v0, $zero, 0x1 |
|  39 | 0x0DE4C | 0x8001D64C | 0x1B0 | 108 | R2 | - | addiu $t1, $zero, 0xFF | sll $v0, $v1, 3 |
|  40 | 0x0DFFC | 0x8001D7FC | 0x8 |   2 | R2 | .L8001D7FC | bne $v1, $v0, .L8001D9A8 | addiu $v0, $zero, 0x2 |
|  41 | 0x0E004 | 0x8001D804 | 0x1A4 | 105 | R2 | - | addiu $a2, $zero, 0xC1 | sll $v0, $v1, 3 |
|  42 | 0x0E1A8 | 0x8001D9A8 | 0x8 |   2 | R2 | .L8001D9A8 | bne $v1, $v0, .L8001DB60 | addiu $v0, $zero, 0x3 |
|  43 | 0x0E1B0 | 0x8001D9B0 | 0x1B0 | 108 | R2 | - | addiu $t1, $zero, 0x83 | sll $v0, $v1, 3 |
|  44 | 0x0E360 | 0x8001DB60 | 0x8 |   2 | R2 | .L8001DB60 | bne $v1, $v0, .L8001DD1C | addiu $a2, $zero, 0xC1 |
|  45 | 0x0E368 | 0x8001DB68 | 0x1A0 | 104 | R2 | - | addiu $a1, $zero, 0x28 | sll $v0, $v1, 3 |
|  46 | 0x0E508 | 0x8001DD08 | 0x14 |   5 | R2 | .L8001DD08 | addu $v0, $v0, $v1 | sb $a0, %lo(D_800B0172)($at) |
|  47 | 0x0E51C | 0x8001DD1C | 0x1C |   7 | R2 | .L8001DD1C | lui $a0, %hi(D_8009D254) | nop  |
|  48 | 0x0E538 | 0x8001DD38 | 0x8 |   2 | R2 | - | jal func_8001A680 | addiu $a1, $zero, 0x12 |
|  49 | 0x0E540 | 0x8001DD40 | 0x10 |   4 | R2 | .L8001DD40 | lb $v1, 0xC0($gp) | addu $a0, $v1, $zero |
|  50 | 0x0E550 | 0x8001DD50 | 0x1C |   7 | R2 | - | lui $v0, %hi(D_8009D278) | nop  |
|  51 | 0x0E56C | 0x8001DD6C | 0x104 |  65 | R2 | - | addiu $v1, $zero, -0x2001 | nop  |
|  52 | 0x0E670 | 0x8001DE70 | 0x1C |   7 | R2 | .L8001DE70 | lui $v1, %hi(D_8009D254) | sw $zero, 0x70($v1) |
|  53 | 0x0E68C | 0x8001DE8C | 0x18 |   6 | R3a | .L8001DE8C | lui $v0, %hi(D_8009D1A0) | nop  |
|  54 | 0x0E6A4 | 0x8001DEA4 | 0x1C |   7 | R3a | - | lui $v1, %hi(D_8009D278) | addiu $v0, $v0, -0x1 |
|  55 | 0x0E6C0 | 0x8001DEC0 | 0x8 |   2 | R3a | - | beqz $v0, .L8001E5EC | sw $v0, 0x34($v1) |
|  56 | 0x0E6C8 | 0x8001DEC8 | 0x18 |   6 | R3a | - | lui $v0, %hi(D_8009D1E8) | addiu $v0, $zero, 0x1 |
|  57 | 0x0E6E0 | 0x8001DEE0 | 0x1AC | 107 | R3a | - | addiu $t0, $zero, 0x82 | sll $v0, $v1, 3 |
|  58 | 0x0E88C | 0x8001E08C | 0x8 |   2 | R3a | .L8001E08C | bne $v1, $v0, .L8001E238 | addiu $v0, $zero, 0x2 |
|  59 | 0x0E894 | 0x8001E094 | 0x1A4 | 105 | R3a | - | addiu $a2, $zero, 0x25 | sll $v0, $v1, 3 |
|  60 | 0x0EA38 | 0x8001E238 | 0x8 |   2 | R3a | .L8001E238 | bne $v1, $v0, .L8001E3EC | addiu $v0, $zero, 0x3 |
|  61 | 0x0EA40 | 0x8001E240 | 0x1AC | 107 | R3a | - | addiu $t0, $zero, 0x4A | sll $v0, $v1, 3 |
|  62 | 0x0EBEC | 0x8001E3EC | 0x8 |   2 | R3a | .L8001E3EC | bne $v1, $v0, .L8001E5A8 | addiu $a2, $zero, 0x25 |
|  63 | 0x0EBF4 | 0x8001E3F4 | 0x1A0 | 104 | R3a | - | addiu $a1, $zero, 0xC1 | sll $v0, $v1, 3 |
|  64 | 0x0ED94 | 0x8001E594 | 0x14 |   5 | R3a | .L8001E594 | addu $v0, $v0, $v1 | sb $a0, %lo(D_800B014E)($at) |
|  65 | 0x0EDA8 | 0x8001E5A8 | 0x24 |   9 | R3a | .L8001E5A8 | lui $a0, %hi(D_8009D278) | nop  |
|  66 | 0x0EDCC | 0x8001E5CC | 0x4 |   1 | R3a | - | sw $zero, 0x34($a0) | sw $zero, 0x34($a0) |
|  67 | 0x0EDD0 | 0x8001E5D0 | 0x1C |   7 | R3a | .L8001E5D0 | lui $v0, %hi(D_8009D278) | nop  |
|  68 | 0x0EDEC | 0x8001E5EC | 0xD4 |  53 | R3a | .L8001E5EC | addiu $a2, $zero, 0x82 | sb $v0, %lo(D_800B0196)($at) |
|  69 | 0x0EEC0 | 0x8001E6C0 | 0x14 |   5 | R3b | .L8001E6C0 | lw $v1, 0x0($s2) | lui $v0, (0x180000 >> 16) |
|  70 | 0x0EED4 | 0x8001E6D4 | 0x24 |   9 | R3b | - | lui $v0, %hi(D_8009D234) | lui $v1, (0xFF7FFFFF >> 16) |
|  71 | 0x0EEF8 | 0x8001E6F8 | 0x14 |   5 | R3b | - | lw $v0, 0x0($s2) | sw $v0, 0x0($s2) |
|  72 | 0x0EF0C | 0x8001E70C | 0xC |   3 | R3b | .L8001E70C | and $v0, $v1, $v0 | nop  |
|  73 | 0x0EF18 | 0x8001E718 | 0x14 |   5 | R3b | - | lui $s0, %hi(D_8009D20C) | nop  |
|  74 | 0x0EF2C | 0x8001E72C | 0x4 |   1 | R3b | - | lui $s3, (0x1000000 >> 16) | lui $s3, (0x1000000 >> 16) |
|  75 | 0x0EF30 | 0x8001E730 | 0x14 |   5 | R3b | .L8001E730 | lui $v0, %hi(D_8009D254) | nop  |
|  76 | 0x0EF44 | 0x8001E744 | 0x10 |   4 | R3b | - | lw $v1, 0x0($s0) | nop  |
|  77 | 0x0EF54 | 0x8001E754 | 0x14 |   5 | R3b | - | lw $v0, 0x98($s0) | addu $s1, $v1, $zero |
|  78 | 0x0EF68 | 0x8001E768 | 0x10 |   4 | R3b | - | lw $a0, 0x18($s1) | nop  |
|  79 | 0x0EF78 | 0x8001E778 | 0x14 |   5 | R3b | - | lw $v0, 0x0($s2) | nop  |
|  80 | 0x0EF8C | 0x8001E78C | 0x10 |   4 | R3b | - | lw $v1, 0x0($s1) | nop  |
|  81 | 0x0EF9C | 0x8001E79C | 0x18 |   6 | R3b | - | lbu $v0, 0x0($a0) | srl $v0, $v1, 21 |
|  82 | 0x0EFB4 | 0x8001E7B4 | 0x10 |   4 | R3b | - | andi $v1, $v0, 0x7 | sll $v0, $v1, 1 |
|  83 | 0x0EFC4 | 0x8001E7C4 | 0x20 |   8 | R3b | - | addu $v0, $v0, $s1 | sw $v0, 0x10($sp) |
|  84 | 0x0EFE4 | 0x8001E7E4 | 0x18 |   6 | R3b | .L8001E7E4 | addu $a0, $s0, $zero | sw $v0, 0x0($s2) |
|  85 | 0x0EFFC | 0x8001E7FC | 0x48 |  18 | R3b | - | lui $a0, (0x7FFFFFFF >> 16) | sw $v1, 0x0($s2) |
|  86 | 0x0F044 | 0x8001E844 | 0x24 |   9 | R3b | .L8001E844 | lui $a0, %hi(D_8009D278) | addu $a1, $v1, $zero |
|  87 | 0x0F068 | 0x8001E868 | 0x48 |  18 | R3b | - | lhu $v0, 0xE($a0) | nop  |
|  88 | 0x0F0B0 | 0x8001E8B0 | 0x8 |   2 | R3b | - | j .L8001E8CC | sb $zero, 0x57($a1) |
|  89 | 0x0F0B8 | 0x8001E8B8 | 0x14 |   5 | R3b | .L8001E8B8 | lw $v0, 0x4C($a1) | sb $v0, 0x57($a1) |
|  90 | 0x0F0CC | 0x8001E8CC | 0x1C |   7 | R3b | .L8001E8CC | lui $a0, %hi(D_8009D278) | sw $v0, 0x4C($a0) |
|  91 | 0x0F0E8 | 0x8001E8E8 | 0x14 |   5 | R3b | .L8001E8E8 | lw $v0, 0x98($s0) | nop  |
|  92 | 0x0F0FC | 0x8001E8FC | 0x14 |   5 | R3b | - | lw $v1, 0x0($s2) | nop  |
|  93 | 0x0F110 | 0x8001E910 | 0x10 |   4 | R3b | - | lw $v0, 0x10($s1) | andi $v0, $v1, 0x200 |
|  94 | 0x0F120 | 0x8001E920 | 0x8 |   2 | R3b | - | bnez $v0, .L8001E948 | addiu $v0, $zero, 0x5A |
|  95 | 0x0F128 | 0x8001E928 | 0x20 |   8 | R3b | - | lui $a0, %hi(D_8009D278) | addiu $v0, $zero, 0x5A |
|  96 | 0x0F148 | 0x8001E948 | 0x30 |  12 | R3b | .L8001E948 | sb $v0, 0xC4($gp) | addu $a1, $v1, $zero |
|  97 | 0x0F178 | 0x8001E978 | 0x48 |  18 | R3b | - | lhu $v0, 0xE($a0) | nop  |
|  98 | 0x0F1C0 | 0x8001E9C0 | 0x8 |   2 | R3b | - | j .L8001E9DC | sb $zero, 0x57($a1) |
|  99 | 0x0F1C8 | 0x8001E9C8 | 0x14 |   5 | R3b | .L8001E9C8 | lw $v0, 0x4C($a1) | sb $v0, 0x57($a1) |
| 100 | 0x0F1DC | 0x8001E9DC | 0x1C |   7 | R3b | .L8001E9DC | lui $a0, %hi(D_8009D278) | sw $v0, 0x4C($a0) |
| 101 | 0x0F1F8 | 0x8001E9F8 | 0x30 |  12 | R3b | .L8001E9F8 | lui $a1, (0xFFF3FFFF >> 16) | sw $v1, 0x98($v0) |
| 102 | 0x0F228 | 0x8001EA28 | 0x2C |  11 | R3b | - | lui $v1, %hi(D_8009D278) | sb $v0, 0x48($v1) |
| 103 | 0x0F254 | 0x8001EA54 | 0x10 |   4 | R3b | .L8001EA54 | lw $s0, 0x4($s0) | nop  |
| 104 | 0x0F264 | 0x8001EA64 | 0x28 |  10 | R3b | .L8001EA64 | lui $a0, %hi(D_8009D278) | addu $a2, $v1, $zero |
| 105 | 0x0F28C | 0x8001EA8C | 0x14 |   5 | R3b | - | lh $v0, 0x1C($a0) | subu $v0, $a1, $a2 |
| 106 | 0x0F2A0 | 0x8001EAA0 | 0x40 |  16 | R3b | - | lui $v1, %hi(D_8009D254) | nop  |
| 107 | 0x0F2E0 | 0x8001EAE0 | 0x8 |   2 | R3b | - | j .L8001EAFC | sb $zero, 0x57($a1) |
| 108 | 0x0F2E8 | 0x8001EAE8 | 0x14 |   5 | R3b | .L8001EAE8 | lw $v0, 0x4C($a1) | sb $v0, 0x57($a1) |
| 109 | 0x0F2FC | 0x8001EAFC | 0x1C |   7 | R3b | .L8001EAFC | lui $a0, %hi(D_8009D278) | sw $v0, 0x4C($a0) |
| 110 | 0x0F318 | 0x8001EB18 | 0x18 |   6 | R3b | .L8001EB18 | lui $v1, %hi(D_8009D278) | sh $v0, 0xE($v1) |
| 111 | 0x0F330 | 0x8001EB30 | 0x2C |  11 | R3b | .L8001EB30 | lw $v0, 0x0($s2) | lui $v0, (0xC0000 >> 16) |
| 112 | 0x0F35C | 0x8001EB5C | 0x20 |   8 | R3b | - | lui $a1, %hi(D_8009D254) | lui $v0, (0xFFF3FFFF >> 16) |
| 113 | 0x0F37C | 0x8001EB7C | 0x10 |   4 | R3b | - | lbu $v0, 0x49($v1) | lui $v0, (0xFFF3FFFF >> 16) |
| 114 | 0x0F38C | 0x8001EB8C | 0x20 |   8 | R3b | .L8001EB8C | ori $v0, $v0, (0xFFF3FFFF & 0xFFFF) | sb $zero, 0x49($v0) |
| 115 | 0x0F3AC | 0x8001EBAC | 0xC |   3 | R3b | .L8001EBAC | lh $a0, 0x4A($v1) | nop  |
| 116 | 0x0F3B8 | 0x8001EBB8 | 0x44 |  17 | R3b | - | lui $a1, %hi(D_8009D278) | nop  |
| 117 | 0x0F3FC | 0x8001EBFC | 0x50 |  20 | R3b | - | lui $a0, %hi(D_8009D278) | nop  |
| 118 | 0x0F44C | 0x8001EC4C | 0x4 |   1 | R3b | - | break 7 | break 7 |
| 119 | 0x0F450 | 0x8001EC50 | 0xC |   3 | R3b | .L8001EC50 | addiu $at, $zero, -0x1 | lui $at, (0x80000000 >> 16) |
| 120 | 0x0F45C | 0x8001EC5C | 0x8 |   2 | R3b | - | bne $v1, $at, .L8001EC68 | nop  |
| 121 | 0x0F464 | 0x8001EC64 | 0x4 |   1 | R3b | - | break 6 | break 6 |
| 122 | 0x0F468 | 0x8001EC68 | 0x2C |  11 | R3b | .L8001EC68 | mflo $v0 | sb $v0, 0x48($v1) |
| 123 | 0x0F494 | 0x8001EC94 | 0x8 |   2 | R3b | .L8001EC94 | jal func_8001F9C4 | nop  |
| 124 | 0x0F49C | 0x8001EC9C | 0x1C |   7 | R4 | .L8001EC9C | lui $a1, %hi(D_8009D278) | addu $a0, $zero, $zero |
| 125 | 0x0F4B8 | 0x8001ECB8 | 0x8 |   2 | R4 | - | jal func_80032B0C | addiu $a1, $a1, 0x50 |
| 126 | 0x0F4C0 | 0x8001ECC0 | 0x1C |   7 | R4 | - | lui $v1, %hi(D_8009D278) | sb $v0, 0x56($v1) |
| 127 | 0x0F4DC | 0x8001ECDC | 0x24 |   9 | R4 | .L8001ECDC | lui $a0, %hi(D_8009D278) | addu $a1, $v1, $zero |
| 128 | 0x0F500 | 0x8001ED00 | 0x58 |  22 | R4 | - | lui $v1, %hi(D_8009D254) | sh $v0, 0xE($v1) |
| 129 | 0x0F558 | 0x8001ED58 | 0x1C |   7 | R4 | .L8001ED58 | lui $a1, %hi(D_8009D278) | addu $a0, $zero, $zero |
| 130 | 0x0F574 | 0x8001ED74 | 0x8 |   2 | R4 | - | jal func_80032B0C | addiu $a1, $a1, 0x58 |
| 131 | 0x0F57C | 0x8001ED7C | 0x24 |   9 | R4 | - | lui $v1, %hi(D_8009D278) | lw $a1, %lo(D_8009D278)($a1) |
| 132 | 0x0F5A0 | 0x8001EDA0 | 0x14 |   5 | R4 | .L8001EDA0 | nop  | addu $a0, $zero, $zero |
| 133 | 0x0F5B4 | 0x8001EDB4 | 0x8 |   2 | R4 | - | jal func_80032B0C | addiu $a1, $a1, 0x60 |
| 134 | 0x0F5BC | 0x8001EDBC | 0x1C |   7 | R4 | - | lui $v1, %hi(D_8009D278) | sb $v0, 0x66($v1) |
| 135 | 0x0F5D8 | 0x8001EDD8 | 0x20 |   8 | R4 | .L8001EDD8 | lui $v0, %hi(D_8009D278) | sh $v1, 0xE($v0) |
| 136 | 0x0F5F8 | 0x8001EDF8 | 0x14 |   5 | R4 | - | lw $v0, 0x0($s2) | sw $v0, 0x0($s2) |
| 137 | 0x0F60C | 0x8001EE0C | 0x18 |   6 | R4 | - | sll $v0, $v0, 24 | addiu $s0, $zero, -0x1 |
| 138 | 0x0F624 | 0x8001EE24 | 0x14 |   5 | R4 | - | lui $a0, %hi(D_8009D200) | addu $a1, $zero, $zero |
| 139 | 0x0F638 | 0x8001EE38 | 0x14 |   5 | R4 | - | addu $a2, $zero, $zero | sw $zero, 0x14($sp) |
| 140 | 0x0F64C | 0x8001EE4C | 0x8 |   2 | R4 | - | lui $at, %hi(D_8009D200) | sw $s0, %lo(D_8009D200)($at) |
| 141 | 0x0F654 | 0x8001EE54 | 0x14 |   5 | R4 | .L8001EE54 | lui $a0, %hi(D_8009D2FC) | addu $a1, $zero, $zero |
| 142 | 0x0F668 | 0x8001EE68 | 0x14 |   5 | R4 | - | addu $a2, $zero, $zero | sw $zero, 0x14($sp) |
| 143 | 0x0F67C | 0x8001EE7C | 0x8 |   2 | R4 | - | lui $at, %hi(D_8009D2FC) | sw $s0, %lo(D_8009D2FC)($at) |
| 144 | 0x0F684 | 0x8001EE84 | 0x8 |   2 | R4 | .L8001EE84 | jal func_80021D4C | nop  |
| 145 | 0x0F68C | 0x8001EE8C | 0x14 |   5 | R5 | .L8001EE8C | lui $v1, %hi(D_8009D28C) | addiu $a2, $zero, 0x46 |
| 146 | 0x0F6A0 | 0x8001EEA0 | 0x1CC | 115 | R5 | - | addiu $a1, $zero, 0x82 | sb $v1, %lo(D_800B0196)($at) |
| 147 | 0x0F86C | 0x8001F06C | 0x1C |   7 | R5 | .L8001F06C | lui $v0, %hi(D_8009D278) | addiu $a2, $zero, 0x46 |
| 148 | 0x0F888 | 0x8001F088 | 0x2C8 | 178 | R5 | - | addiu $a1, $zero, 0x82 | nop  |
| 149 | 0x0FB50 | 0x8001F350 | 0x8 |   2 | R6 | - | jal func_80020CE4 | nop  |
| 150 | 0x0FB58 | 0x8001F358 | 0x8 |   2 | R6 | .L8001F358 | jal func_800374E8 | nop  |
| 151 | 0x0FB60 | 0x8001F360 | 0x8 |   2 | R6 | - | jal func_80021054 | nop  |
| 152 | 0x0FB68 | 0x8001F368 | 0x18 |   6 | R6 | - | sll $v0, $v0, 24 | addiu $s0, $zero, -0x1 |
| 153 | 0x0FB80 | 0x8001F380 | 0x14 |   5 | R6 | - | lui $a0, %hi(D_8009D200) | addu $a1, $zero, $zero |
| 154 | 0x0FB94 | 0x8001F394 | 0x14 |   5 | R6 | - | addu $a2, $zero, $zero | sw $zero, 0x14($sp) |
| 155 | 0x0FBA8 | 0x8001F3A8 | 0x8 |   2 | R6 | - | lui $at, %hi(D_8009D200) | sw $s0, %lo(D_8009D200)($at) |
| 156 | 0x0FBB0 | 0x8001F3B0 | 0x14 |   5 | R6 | .L8001F3B0 | lui $a0, %hi(D_8009D2FC) | addu $a1, $zero, $zero |
| 157 | 0x0FBC4 | 0x8001F3C4 | 0x14 |   5 | R6 | - | addu $a2, $zero, $zero | sw $zero, 0x14($sp) |
| 158 | 0x0FBD8 | 0x8001F3D8 | 0x8 |   2 | R6 | - | lui $at, %hi(D_8009D2FC) | sw $s0, %lo(D_8009D2FC)($at) |
| 159 | 0x0FBE0 | 0x8001F3E0 | 0x8 |   2 | R6 | .L8001F3E0 | jal func_80021D4C | nop  |
| 160 | 0x0FBE8 | 0x8001F3E8 | 0x50 |  20 | R6 | - | addiu $a0, $zero, 0x46B | sw $v0, 0x10($sp) |
| 161 | 0x0FC38 | 0x8001F438 | 0x10 |   4 | R6 | - | lui $at, %hi(D_8009D244) | nop  |
| 162 | 0x0FC48 | 0x8001F448 | 0x18 |   6 | R6 | - | lui $v0, %hi(D_800BCF88) | addiu $a2, $zero, -0x5 |
| 163 | 0x0FC60 | 0x8001F460 | 0x8 |   2 | R6 | - | jal func_80067CBC | nop  |
| 164 | 0x0FC68 | 0x8001F468 | 0x4 |   1 | R6 | - | addiu $a2, $zero, -0x5 | addiu $a2, $zero, -0x5 |
| 165 | 0x0FC6C | 0x8001F46C | 0x38 |  14 | R6 | .L8001F46C | lui $a0, %hi(D_8009D254) | addiu $a1, $zero, 0x13 |
| 166 | 0x0FCA4 | 0x8001F4A4 | 0x10 |   4 | R6 | - | lui $v0, %hi(D_8009D278) | sh $zero, 0xC($v0) |
| 167 | 0x0FCB4 | 0x8001F4B4 | 0x20 |   8 | R6 | .L8001F4B4 | lw $ra, 0x320($sp) | nop  |

Note on `rec`/`actor`: `rec = *(u8**)0x8009D278`, `actor = *(u8**)0x8009D254`.
`gp+0xC0` is `0x8009CE30`, `gp+0xC4` is `0x8009CE34` (gp = 0x8009CD70).
Every store into the colour tables reloads `D_8009CDDC` and re-materialises
`lui $at,%hi(D_800B0xxx); addu $at,$at,$v0; sb ...` — i.e. the source did **not**
hoist the `D_8009CDDC*0x48` base; cc1 must re-load the index after each store
(the colour tables alias the index in cc1's conservative view).

## 4. Closest C so far and its exact diff

Three candidates are saved under this directory:

| file | source of structure | status | best diff |
|---|---|---|---|
| `func_8001D340_m2c_typed_frame.c` | m2c CFG + width-typed fields + phantom frame | **compiles; frame 0x328 matched** | **1032 words** (`-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`) |
| `func_8001D340_m2c_typed.c` | m2c CFG + width-typed fields | compiles, no phantom frame | 1146 words (`-O2 -G0`) |
| `func_8001D340_candidate.c` | port SPEC | compiles, wrong structure | 1928 words (`-O2 -G0`) |

### 4a. Best candidate — `func_8001D340_m2c_typed_frame.c`

The m2c skeleton with each `BASE->unkNN` rewritten to a width-correct absolute
access (`RB/RH/RW/RS/RSW/RP` macros; widths read from the retail load/store
opcodes), plus a **phantom local `u8 phantom[0x2B8]; (void)phantom;`**. Retail's
0x328 frame is 0x310 bytes of *never-accessed* local space (the only stack
accesses in the whole function are the five saved registers at 0x310-0x324 and
one outgoing argument at `0x10($sp)`), so the frame size has to be manufactured.
`phantom[0x2B8]` makes cc1 report `vars=760, regs=5/0, args=24`, i.e. exactly
`frame=0x328`. This is the same cc1 "phantom frame" lever already used on
`func_800C6EF8`.

```
try_leaf.py docs/evidence/bigfish-1d340/func_8001D340_m2c_typed_frame.c 0xDB40 0x2194
  -O2 -G0 : retail 8596 B, candidate 8736 B, 1140 differing words
  -O1 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 : 1032 differing words  <-- best
  -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 : 1032 differing words
```

With the frame aligned, **words 0x0000-0x0010 now match byte-for-byte** (prologue
+ `s2=rec+0x4C` delay slot). The first divergence is word 0x0014: retail
`320200FF` (`andi $v0,$s0,0xFF`) vs candidate `320300FF` (`andi $v1,$s0,0xFF`)
— m2c materialises `mode&0xFF` in `$v1` where retail keeps it in `$v0`. The next
differences are the same class of allocation shifts (`0x28: 1040009F` vs
`10600092`, `beqz $v0` vs `beqz $v1`) plus a shorter early straight-line run
(branch displacement `0x69` vs `0x61` at word 0x40).

### 4b. `func_8001D340_m2c_typed.c`

Same transform without the phantom local (frame 0x70). Diff 1146 words under
`-O2 -G0`; useful when isolating frame effects from allocation effects.

### 4c. Port-derived candidate — `func_8001D340_candidate.c`

Derived from the behavioural SPEC `pc_port/game/boot/func_8001D340_port.c`
with the `PE_Load*/PE_Store*` accessors expanded to direct typed accesses so it
compiles under the era toolchain. It is **not** a registered match.

```
try_leaf.py docs/evidence/bigfish-1d340/func_8001D340_candidate.c 0xDB40 0x2194
  -O2 -G0 : retail 8596 B, candidate 3152 B, 1928 differing words
  -O2 -G8 : 1928 differing words
  -O1 -G0 : 1930 differing words
  -O2 -G0 + MASPSX_EXPAND_DIV=1                  : 1931
  -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1     : 1927
  -O2 -G8 + MASPSX_THREE_WORD_SYMBOL_STORE=1     : 1927
```

First divergence is at **word 0x0000** (the frame size itself: retail
`27BDFCD8` = `addiu $sp,$sp,-0x328`; candidate `3C02800A` = `lui $v0,%hi(...)`).
Why: the port computes `phase = D_8009D1E8 & 3` in its initialiser list, so cc1
hoists that load to the top; retail computes `phase` lazily, only inside the
`flags & 0x2000` block, and starts with the `D_8009D278` load / `s2 = rec+0x4C`
/ `s0 = a0`. The candidate then also uses a 0xB0 frame (three helper functions
colour the register allocation), so the entire function is mis-framed.

Structural reasons the port candidate cannot be iterated into a match:

1. **Helpers vs loops.** The port factors the colour writes into
   `player_quad_colors`/`player_reset_pe_colors`/`player_reset_at_colors`,
   whose bodies are `for (vertex=0..3) for (channel=0..2)` loops. Retail has
   these **fully unrolled** (12 `sb` per quad, 24/30 per reset) and re-loads
   `D_8009CDDC` before every `sb`. cc1 2.7.2 does not unroll loops at `-O2`, so
   the source must have been hand-unrolled or macro-driven; the port's loop
   shape is behaviourally identical but structurally wrong.
2. **Declared-before-use order.** The port loads `rec` and `actor` eagerly at
   the top of the function; retail loads `rec` first, derives `s2 = rec+0x4C`
   in the branch delay slot, and only later loads `actor`.
3. **`volatile` / alias reloads.** The per-store reload of `D_8009CDDC` is the
   conservative-aliasing signature the repo already documented as a
   CSE/`volatile` lever; the port's helper-loop structure hides it.

### 4d. Raw m2c output — `func_8001D340_m2c_draft.c`

The raw `python3 tools/analysis/m2c_leaf.py --print func_8001D340` output (647
lines). It recovers the true control flow, all 168 blocks, every field offset
and the unrolled colour stores, so it is the source of the typed skeletons in
4a/4b. It does **not** compile as emitted because m2c models `D_8009D278` /
`D_8009D254` / `D_8009D20C` as struct pointers but declares the locals
(`void *var_s0`, `void *temp_v1_6`) without the matching struct types, and it
writes `D_8009D278 + 0x4C` where it means a byte offset. `func_8001D340_m2c_typed*.c`
is the mechanical fix-up; to regenerate it the substitutions are:

* declare `extern u8 *D_8009D278, *D_8009D254, *D_8009D20C;`
* define three structs (`Rec`, `Actor`, `Node`+`Body`) with the field widths in
  the table below, casting the globals to those pointers;
* rewrite `D_8009D278 + 0x4C` as `(s32*)((u8*)D_8009D278 + 0x4C)` and
  `D_8009D278->unk6C->unk4` as `*(u32*)(D_8009D278->unk6C + 4)`;
* `saved_reg_gp->unkC0/unkC4` -> `*(s8*)0x8009CE30` / `*(s8*)0x8009CE34`.

Field widths recovered directly from the retail loads/stores:

```
Rec  (D_8009D278): 0x08 sw/lw 0x0C h 0x0E h 0x10 h 0x12 b 0x1C h 0x24 h
                   0x28 w 0x2C w 0x30 w 0x34 w 0x48 b 0x49 b 0x4A h 0x4C w
                   0x50 h 0x52 h 0x54 h 0x56 b 0x57 b 0x58 h 0x5A h 0x5C h
                   0x5E b 0x5F b 0x66 b 0x6C w(ptr)
Actor(D_8009D254): 0x0E b 0x28 w 0x2A h 0x2E h 0x30 w 0x32 h 0x40 w 0x48 w
                   0x68 w 0x6C w 0x70 w 0x98 w 0x210 h 0x212 h
```

(0x28 `w` and 0x2A `h` overlap on `Actor`, and 0x30 `w` / 0x32 `h` overlap; a
union or explicit byte-offset macros are required.)

## 5. Levers tried

| lever | result |
|---|---|
| `scripts/split_us.sh` + `build_us.sh` + `verify_us.sh` baseline | PASS, EXACT SHA-1, 870 leaves |
| `m2c_leaf.py --print func_8001D340` | 647-line draft; correct CFG, not compilable as emitted (untyped struct locals) |
| `try_leaf.py` on port-derived typed candidate, 5 profiles (-O2/-O1, -G0/-G8, expand-div, three-word) | 1927-1931 differing words in every profile; no profile moves the frame/entry divergence |
| width-typed m2c skeleton (`m2c_typed.c`) | compiles; 1146 differing words (-O2 -G0), 1038 with three-word |
| **phantom frame `u8 phantom[0x2B8]`** on the typed skeleton | **frame becomes exactly 0x328; words 0x0000-0x0010 match; best diff 1032** |
| `MASPSX_THREE_WORD_SYMBOL_STORE=1` on the typed skeleton | -114 words (1146 -> 1032) |
| `MASPSX_EXPAND_DIV=1` on the typed skeleton | 1141 (no gain at this stage) |
| `-O2 -G8` on the typed skeleton | frame 7280 B / 1498 diffs — retail uses absolute (`lui`) addressing for the colour tables, so `-G0` is correct |
| partial-span carve analysis | ruled out by construction: one prologue / one epilogue / one `jr $ra` |
| delay-slot-aware block split | 168 blocks (vs 196 naive) — delay slots folded into the branching block |

## 6. Recommendation for the next attempt

Do **not** try to carve. Attack it whole, starting from
`func_8001D340_m2c_typed_frame.c` (compiles, frame already 0x328, words
0x0000-0x0010 matching, 1032 differing words) and iterate:

1. The remaining work is **register allocation only** — the CFG, the field
   widths, the unrolled colour stores and the frame are all in place. Do not
   re-derive control flow.
2. Fix the **entry block first** and freeze it. With the phantom frame, words
   0x00-0x10 already match: `addiu sp,-0x328`; `sw s0,0x310`; `addu s0,a0`;
   `lui a0,%hi(D_8009D278)`; `lw a0,%lo(...)`; `andi v0,s0,0xFF`; `sw ra,0x320`;
   `sw s3,0x31C`; `sw s2,0x318`; `sw s1,0x314`; `beqz v0,0x8001D5E8`; delay
   `addiu s2,a0,0x4C`. The first real divergence is word 0x14: m2c puts
   `mode&0xFF` in `$v1` (`andi $v1,$s0,0xFF`), retail keeps it in `$v0`. Make
   `rec->unk4C`/`rec->unk8` reads use the value already in `$v0`, or reorder the
   `D_8009D1A0` load so the `andi` lands in `$v0`. Note the branch at word 0x28
   is already 8 words shorter in the candidate — closing an early straight-line
   gap will re-align all later branch displacements.
3. Then proceed region by region (R1 -> R6). Region boundaries are branch
   targets, so each region can be diffed independently by an `asm` splice:
   temporarily emit the matched prefix as C and leave the rest as `.s`, using
   `try_leaf` on the prefix *only for triage* — remember no prefix span can be
   registered, this is purely to localise regressions.
4. `MASPSX_THREE_WORD_SYMBOL_STORE=1` is **on** for this leaf (it removes ~114
   words, i.e. the colour-table `sb` sequences want the three-word store form).
   Profile it as `era_o2_g0_three_word` when it is eventually registered.
5. The per-store `D_8009CDDC` reload in the colour blocks (R2/R3a/R5) will
   need either a `volatile` index or an explicit recomputed base per store —
   the `volatile`-on-structure-pointer lever already proven in this repo
   (ACTIVE_HANDOFF, `func_800762A0` note) is the candidate mechanism. The typed
   skeleton already reproduces the reload shape, so diff those blocks to
   confirm.
6. Keep the phantom frame: retail's 0x328 frame is 0x310 bytes of never-accessed
   local space. If a future edit shrinks the frame, re-tune `phantom[]` so cc1
   reports `vars + 0x2C == 0x328`.

Realistic assessment: this is a multi-session leaf. A whole match needs the
entry + all 6 regions byte-exact simultaneously because cc1's allocator is
global; a single early register difference perturbs all 2,149 instructions.
The map above is intended to make that incremental attack possible.

## 7. Provenance

* retail `SLUS_006.62` sha1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
* function file range 0xDB40-0xFCD4, size 0x2194 (the prompt's `AB74.s` name is
  stale; this split emits the body in `asm/disc1/DB40.s`)
* `pc_port/game/boot/func_8001D340_port.c` is the behavioural SPEC
