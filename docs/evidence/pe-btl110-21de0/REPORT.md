# PE-BTL110 — Attack clip 21DE0 arms D294

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

`299CC` @ `0x8002A4B8` jals `21DE0` when `21054>0` and
(`rec+0x4C&0x4000==0` or `D1A0&0x100`). `21DE0` (84 words)
requires `CE3C!=0`, `D1D4<CE3C`, `Aya+0x0E>=4`, and
`slot+4<3`, then jals `21F38`. `21F38` jals `2312C`.
Kinds 6/8/10 with `+0x0F==+0x16` and `gp+0xC8/C9==0`
jal `23008`, which sets `D294`.

`236E8` still needs `gp+0xE4==1` to OR body `0x2000`.
That byte has one EXE `sb` (`24F94`). Do not invent it.

## Verify

```text
python3 pc_port/tools/pe_btl110_21de0_oracle.py
PE_TEST_FILTER=BTL110 ./pc_port/build/pe-native-tests
```
