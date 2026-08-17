# PE-BTL6 — 144FC state 0 at 0x80014544 and 0x37 / 42EDC

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. After 0x3B stores `+0xF4=0`, the next
dispatch is JT[0] = `0x80014544`. This does not complete `0x55`.

## State 0 (`0x80014544`, 11 words)

```text
lbu +0xE; andi 3
bnez → skip sb
li 0x37; sb → +0xF4
lw *s1; ori 0x00800000; sw *s1
j 0x80014660   # v0=0, rewind gp+0x90 by 12
```

`$s0` is overlay `D_800B0CD8`. `$s1` is `a0` (actor). Native uses
overlay word 0 as the actor pointer, same as the 0x3B cut.

## State 0x37 (`0x80014570`, 11 words)

```text
lw overlay word 0; andi 0x400000
bnez → skip jal
jal 0x80042EDC
li 0x38; sb → +0xF4
j 0x80014518   # re-dispatch on +0xF4
```

Named cut stops after the `sb 0x38`. It does not enter state
`0x38` / `jal 6D60C(1)`.

## `func_80042EDC` (17 words, `0x80042EDC..0x80042F20`)

`lbu D_800BD024`; `sw 1` at gp+0x168 / gp+0x174; `sw 0` at
gp+0x178; clamp the byte into gp+0x16C (`<0` → 1, `>=33` → 32).
TEXT jals: `0x80014588` (this state) and `0x8005D5F8`.

## Next

`0x8001459C` state 0x38: `jal 0x8006D60C(1)` (387 words,
`0x8006D60C..0x8006DC18`). If v0==1, rewind/return 0. Else
`jal 0x80042F20` and sb 0x39. Do not stub `6D60C`/`6914C`,
jump to mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_14544_oracle.py
python3 pc_port/tools/pe_btl6_6cc2c_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
