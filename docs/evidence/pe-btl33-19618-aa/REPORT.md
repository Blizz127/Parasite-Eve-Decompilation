# PE-BTL33 — type-0 0xAA / 19618 overlay bit

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80019618` — 8 words `0x80019618..0x80019638`,
SHA-256 `5eb9bc4a…c537`. `D_800910A0[0xAA]`. Zero jal.
Always v0=1.

```text
D_800B0CD8 |= 0x2000
```

Reachable on the type-0 `0xFF` mailbox arm after
`0x04` / `0x01` (not on the persist==39 skip).
Next word is already-ported `0x40`, then `0x65`
(new). Do not invent other overlay bits.

## Verify

```text
python3 pc_port/tools/pe_btl33_19618_oracle.py
PE_TEST_FILTER=BTL33 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
