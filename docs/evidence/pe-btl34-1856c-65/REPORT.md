# PE-BTL34 — type-0 0x65 / 1856C pose-group clear

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_8001856C` — 11 words `0x8001856C..0x80018598`,
SHA-256 `309d956f…c25f`. `D_800910A0[0x65]`. Zero jal.
Always v0=1. argc 0.

```text
actor = *D_8009D2F0
actor+0x68 = 0
actor+0x6C = 0
actor+0x70 = 0
actor+0x78 = 0
actor+0x7C = 0
actor+0x80 = 0
```

Those are 12C20/12E7C codes 2 and 3. +0x74 is not
touched.

Live type-0 persist[0x4A]!=39 takes `+0x31C` 0xAA,
already-ported `0x40`, then this `0x65`. Next word
is already-ported `0x2E` command `0x15`, then `0x82`.
The mailbox `+0x68C` 0xAA arm is a second site; after
`0x65`/`0x2E` it `0x00` gotos `+0x2F4` and re-enters
the persist check. Do not invent persist==39.

## Verify

```text
python3 pc_port/tools/pe_btl34_1856c_oracle.py
PE_TEST_FILTER=BTL34 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
