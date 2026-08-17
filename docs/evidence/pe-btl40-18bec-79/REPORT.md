# PE-BTL40 — type-2 0x79 / 18BEC actor flag 0x20

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80018BEC` — 9 words `0x80018BEC..0x80018C10`,
SHA-256 `d373eee2…000c`. `D_800910A0[0x79]`. Zero jal.
Always v0=1.

```text
*D_8009D2F0 + 0x98 |= 0x20
```

Live type-2 after `0x1E`. Next words are already-ported
`0x0B` pose writes (`0x2230000`, `0xFBA90000`,
`0xFC410000`), then more mailbox tests.

## Verify

```text
python3 pc_port/tools/pe_btl40_18bec_oracle.py
PE_TEST_FILTER=BTL40 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
