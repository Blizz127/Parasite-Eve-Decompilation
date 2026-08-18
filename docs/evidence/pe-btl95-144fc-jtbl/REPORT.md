# PE-BTL95 — 144FC unused states complete

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`144FC` `sltiu` `0x3C` then `jtbl_800100A0`.
`jtbl[0]=14544`, `[0x37..0x3B]` are the named
cuts, `[1..0x36]=14658` (`v0=1`). State
`>=0x3C` takes `14660` park.

BTL91 parked unknown states. That is wrong
for `1..0x36`. Live `F4` is still `0`/`0x37`.
Type-6 `0x55` remains behind `scratch[0]&4`.

## Verify

```text
python3 pc_port/tools/pe_btl95_144fc_jtbl_oracle.py
PE_TEST_FILTER=BTL95 ./pc_port/build/pe-native-tests
```
