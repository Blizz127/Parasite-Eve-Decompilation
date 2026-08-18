# PE-BTL20 — 16910 key 2900 overlay bit

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_80016910

314 words `0x80016910..0x80016DF8`, SHA-256 `273744b7…8c37`.
`D_800910A0[0xED]`. Switch on `*arg0`. Always v0=1 on the
live arm.

Live type-1 `+0x048` is `0xED` argc 8, first imm `0xB54` =
2900. That case at `0x80016D00` is:

```text
D_800B0CD8 |= 0x00400000
```

Other keys (35038 spawn, E00CC, 339A0, 6914C) are not this
cut. Next live type-1 word is `0xE1`/`1A374`.

## Verify

```text
python3 pc_port/tools/pe_btl20_16910_oracle.py
PE_TEST_FILTER=BTL20 ./pc_port/build/pe-native-tests
```
