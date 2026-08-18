# PE-BTL106 — mode 9 join; m0005i 0x94 vs 9

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

`2A7F8` has no mode-9 arm. After `bne` vs 8 the join is
`2AA24`: `D1CE` / `4D4` / `lh gp+0x534`. None of those
stores dest. `2B0E8` phase 3 stores mode 9; dest stays
the encounter token.

m0005i type-6 `+0x38F4` compares the `0x94` local to 9
and continues `0x40` / `0xAD` / `0xAA` (already ported).
That is field-script continuation, not `0xA9400048`.

`gp+0x534==1000` (2B0E8 phase 1) is produced by
`5C498` → `514F8` (`D_8009D010`). `51504` zeros it;
`512AC(10)` writes 1000. Not this cut. Do not invent
534 or enemy `body+0x10`.

## Verify

```text
python3 pc_port/tools/pe_btl106_mode9_join_oracle.py
PE_TEST_FILTER=BTL106 ./pc_port/build/pe-native-tests
```
