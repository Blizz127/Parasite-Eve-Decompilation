# PE-BTL6 — func_80087198 + 6CDA4 state 0 a0==0

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. 87198 / 6914C / 6E6D4 are not stubbed
to succeed.

## Window

`0x80087198..0x800871AC` exclusive, 5 words, SHA-256
`cbcb73a8789f68035fa750d437ec12b385a5e999cd9d74973ce05f9bdb45b2de`.

```
addiu $v0, $zero, 1
lui   $at, 0x800a
sw    $v0, -0x2d90($at)   ; D_8009D270 = 1
jr    $ra
addu  $v0, $zero, $zero   ; return 0
```

Twin of `func_80087414` (`D_8009D270=2`, return 0). Matching
`src/func_80087198.c` already exists (READY-FROM-BITWISE). This
cut is the native port only.

`func_800871AC` reads the same word and `andi 1` (bit 0).

## 6CDA4 state 0

Table fill first (`D_8009317C` + `D_800B0DD8` → gp+0x400/404/408),
then `bne $s4,$zero` skips 87198 when a0!=0. a0==0 jals 87198;
a0==3 jals 87414. Both return 0 != s6(-1), so sb F0=7 and
return 1. State 7 is not entered on that tick.

## Live 6D60C F2=0x2F

`jal 6CDA4(0, lb +0xE1, 0, lw +0x194, 0x21, 0)`. Boot `6A674`
stores `+0xE1=0x0D`. Table a1=0x0D halves `0x1F8`/`0x268` →
remain `0x70`. After 87198, F0=7 and 0x2F parks (v0==1). Next
0x2F tick is state 7 `6E6D4` with that remain. Host issue is
not invented; 6E6D4 -1 sb F0=0 and 0x2F then sb F2=0x30.

144FC 0x38 still returns 0. No `0x39` / `6914C`.

## Next

`0x8006E6D4` from 0x2F F0=7 (already ported; host CD may -1),
then F2=0x30 `jal 0x80086464` / `0x80086C1C`. Do not stub
those, jump to mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_87198_oracle.py
python3 pc_port/tools/pe_btl6_6cda4_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
