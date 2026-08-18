# PE-BTL119 — 6C1CC state 39 returns 0; 3D834

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

`6C1CC` jtbl[7] is `6C390`. With Aya present it copies
overlay `+0x198/+0x214` onto `Aya+0x1AC/+0x1B0`, may
`1A680(21)`, jals `3D050` cuts when obj!=0, `6698C`,
`3D834`, clears overlay `0x60000`, a0!=0 sets `0x80000`,
`+0xED=32`, returns 0. Aya==0 stays fail-closed (v0=1).

`3D834` is 68 words. a1==0 skips the clip bind. Both
arms jal `3A088` / `3DFD8(B1638, dest+52)` / `3B97C` /
two `3BCE0` with `D_8009CDDC` xori 1. `39B74` is a
no-op when clip is 0.

Case 2 can now leave `+0xED` in `[32,40)` after six
`6C1CC(1)` ticks when Aya is live. `+0x252` clearer
is still unfound.

## Verify

```text
python3 pc_port/tools/pe_btl114_ce54_1a_oracle.py
PE_TEST_FILTER=BTL119 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
