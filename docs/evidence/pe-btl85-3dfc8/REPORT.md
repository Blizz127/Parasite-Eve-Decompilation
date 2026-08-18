# PE-BTL85 — 3DFC8 nop on dest-change exit

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`3DFC8` — 2 words `0x8003DFC8..0x8003DFD0`,
SHA-256 `6d64edf9…e2f1`. `jr ra` / nop. Callers
`3F074@3F35C` and `3F3C4@3F6DC`.

`3F684` `D1C4==D280` loops to `3EB04` in retail.
`1220C` stores `D1C4=D280` before the jal, so
the exit is “D280 changed this tick”. This cut
snapshots `D280` on entry and jals `74DC0` /
`87024` / `3DFC8(1)` only then. `696F0` and the
`B0CD8`/`D1A0` stores are not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl85_3dfc8_oracle.py
PE_TEST_FILTER=BTL85 ./pc_port/build/pe-native-tests
```
