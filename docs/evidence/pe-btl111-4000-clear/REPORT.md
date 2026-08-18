# PE-BTL111 — 27D14 0x4000 clears body hit bits

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

`28574` ends with `body = (body & ~0x6000) | 0x4000`.
The next `27D14` takes the `0x4000` arm and at
`0x80028088` stores `body & 0x9FFF` (clears `0x6000`).
`1A680` react stays deferred.

## Verify

```text
python3 pc_port/tools/pe_btl111_4000_clear_oracle.py
PE_TEST_FILTER=BTL111 ./pc_port/build/pe-native-tests
```
