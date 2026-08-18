# PE-BTL37 — type-0 0xAB / 19638 overlay bit clear

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80019638` — 8 words `0x80019638..0x80019658`,
SHA-256 `03ea9775…7afc`. `D_800910A0[0xAB]`. Zero jal.
Always v0=1.

```text
D_800B0CD8 &= ~0x2000
```

Inverse of `0xAA`. Live type-0 after `0x9C` continues
`0x09`/`0x0A` (persist[0]&=~2, persist[0x4A]=40),
`0x2E`/`0x3F`, then this clear. Next words are the
already-ported mailbox poll at `+0x608`. Do not
invent the persist=40 store; it is the script.

## Verify

```text
python3 pc_port/tools/pe_btl37_19638_oracle.py
PE_TEST_FILTER=BTL37 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
