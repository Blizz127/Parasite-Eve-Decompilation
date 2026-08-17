# PE-BTL71 — 3EB04 Up via `BE9A2`, not planted `D26C`

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`3F3C4` @ `3F40C` jals `3EB04`. `3E974` registers
`3EAC8(0x8, 0x10)` so `A76F0[3] = 0x10` (Up) and
`3EAC8(0x10, 0x20)` so `A76F0[4] = 0x20` (Right).

Active-low `BE9A2 = 0xFFEF` is Up. `3EB04` sets
`D26C` bit 3. Row 21 then jalrs `710A4` and one
`35C84` tick adds `+0x50000` to `+0x30`.

Do not plant `D26C`. Idle raw stays `0xFFFF`.

## Verify

```text
python3 pc_port/tools/pe_btl71_3eb04_pad_oracle.py
PE_TEST_FILTER=BTL71 ./pc_port/build/pe-native-tests
```
