# PE-BTL105 — 27D14 death gate → 28E94 → 292EC

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

`299CC` @ `0x8002A53C` walks non-Aya actors with a body
into `27D14` after `1D340`. `27D14` jals `28E94` when
`body+0x10<=0`, `!(D1A0&0x100)`, and `body+5` is not 1/3.

`28E94` phase 0 decrements `D2A0` and falls into phase 1.
`body+0xAF==0` stores phase 3. Phase 3: `+0x98|=0x410`,
`2F970(actor)` nulls the body, then `292EC`.

This is **victory arming**, not player death.

Deferred: `27A08`, `28574`, `6DCE4`, `866A4`, `3C5D8`,
`3CAEC`, kind 1/3 `28474` walk.

## Verify

```text
python3 pc_port/tools/pe_btl105_27d14_oracle.py
PE_TEST_FILTER=BTL105 ./pc_port/build/pe-native-tests
```
