# PE-BTL104 — remaining-enemy tail → 2F300 mode 2

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

This is **ENCOUNTER VICTORY** arming, not player death.

`292EC` (tail of `28E94`) walks `D20C`:
- `D2A0!=0` → out
- any non-Aya actor with a body → out (enemies remain)
- else `record+0x0C>0` → `2F300` (mode 2)
- `record+0x0C<=0` → out (Aya dead; death is mode 3)

`2F300` named cut: `293F4(0)` then mode 2. HUD sb storm,
`1A680`, `6DE80` deferred. Parent `27D14`/`28E94` not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl104_victory_ready_oracle.py
PE_TEST_FILTER=BTL104 ./pc_port/build/pe-native-tests
```
