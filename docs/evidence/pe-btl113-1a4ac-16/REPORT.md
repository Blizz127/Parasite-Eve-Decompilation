# PE-BTL113 — 1A4AC produces Aya `+0x16==10`

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

`2B0E8` phase 0 waits `lhu Aya+0x16 == 10` (or `D1A0&0x800`).
`1A680` `sw $0, 0x14(actor)` zeros that halfword. It is the
high half of the clip word at `+0x14`.

`35038` spawn stores `+0x1C = 0x10000` (`351CC`/`351D0`).
`1A4AC` (`1A578`) adds `+0x1C` to `+0x14`. Ten ticks with
`+0x0F >= 10` (from `1A680` `resource+2-1`) make `+0x16==10`.
`35558` jals `1A4AC` on Aya after `2B0E8`, so the 11th combat
tick sees 10 and phase 0 advances. Do not plant `+0x16`.

`D1A0&0x800` stays a second retail gate. Its writer is not
this cut.

## Verify

```text
python3 pc_port/tools/pe_btl113_1a4ac_16_oracle.py
PE_TEST_FILTER=BTL113 ./pc_port/build/pe-native-tests
```
