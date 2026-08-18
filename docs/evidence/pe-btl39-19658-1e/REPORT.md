# PE-BTL39 — type-2 0x1E / 19658 actor flag 0x80

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80019658` — 9 words `0x80019658..0x8001967C`,
SHA-256 `cadffdfc…6fcd`. `D_800910A0[0x1E]`. Zero jal.
Always v0=1.

```text
*D_8009D2F0 + 0x98 |= 0x80
```

Live type-2 after type-0 `0x1C` payload `0xB`
(`+0x308` eq). Next word is `0x79`.

## Verify

```text
python3 pc_port/tools/pe_btl39_19658_oracle.py
PE_TEST_FILTER=BTL39 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
