# PE-BTL2 — ROM writer of D_8009D28C = 7

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
cut         0x8002CF24..0x8002CF2C exclusive, 2/2 words
store       sw $v0, 0x51C($gp) after addiu $v0, 7
symbol      D_8009D28C
jal_to_cut  none (inlined in the battle dispatcher)
```

Matching leaves store 0/3/4/5/6/8 only. The script-visible done
value 7 is this gp-relative store, not `func_80017FF0` / `0x89`.

The next word (`addiu $v0, 70` then several `sb` to `0x800B00Ex`)
is outside the cut. HP layout is now proven on the `0x55` →
`144FC` → `29810` → `293F4(0)` arm (`docs/evidence/pe-btl2-hp-layout/`).
First-command stores are still absent on that path. This rung does
not invent them.

NYPD / Eve-intro arm (m0005i module 6) parks on `0x55(2)` at
`+0x4140` with mode still 0. `0x89` is the *next* opcode
(`+0x414C`). Do not assume a prior 6→0 consume on that arm.

```text
python3 pc_port/tools/pe_ch1_2cf24_oracle.py
# PASS: 2/2 words + gp+0x51C D_8009D28C=7
PE_TEST_FILTER=2CF24 ./pc_port/build/pe-native-tests
# 2 passed, 0 failed, 651 skipped (653 run)
```
