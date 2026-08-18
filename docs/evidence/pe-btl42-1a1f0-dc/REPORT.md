# PE-BTL42 — type-2 0xDC / 1A1F0 actor flag 0x01000000

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_8001A1F0` — 9 words `0x8001A1F0..0x8001A214`,
SHA-256 `65cffdf8…d872`. `D_800910A0[0xDC]`. Zero jal.
Always v0=1.

```text
*D_8009D2F0 + 0x98 |= 0x01000000
```

Live type-2 after type-0 mailbox `0xFB`:
eq `0xFB` skips `0x04`/`0x01`, miss `0x7F`
`0x00` gotos `+0x70C`, already-ported `0x0B`
pose pair, then `0xDC`. Next words are
already-ported `0x0A`/`0x09`/`0x05`, then
`0x1A`.

## Verify

```text
python3 pc_port/tools/pe_btl42_1a1f0_oracle.py
PE_TEST_FILTER=BTL42 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
