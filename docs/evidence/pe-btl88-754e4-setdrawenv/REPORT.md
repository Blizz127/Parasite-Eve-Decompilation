# PE-BTL88 — 754E4 DrawOTagEnv software path

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`754E4` — 54 words `0x800754E4..0x800755BC`,
SHA-256 `628eb654…9b55`. DrawOTagEnv:

- debug byte `9574E < 2` skips the printf jalr
- `75EE0(env+0x1C, env)` SetDrawEnv, 156w
- tag splice `(dr[0] & 0xFF000000) | (ot & 0x00FFFFFF)`
- jalr `76C34(76B98, dr, 0x40, 0)` is **not** this cut
- `71A34` is BIOS `A(0x2A)` memcpy of `0x5C` to `9575C`

`70E54` live args: `ot = B0E38[CDDC]+0x3FFC`,
`env = 0x800BCDC8 + 92*CDDC` (`lui 0x800C` +
signed `addiu 0xCDC8`). Live `3E754` DRAWENV
has `isbg=1`, so the aligned FILL path runs
(`0x02000000` at `dr+0x1C`, length 9).

## Verify

```text
python3 pc_port/tools/pe_btl88_754e4_setdrawenv_oracle.py
PE_TEST_FILTER=BTL88 ./pc_port/build/pe-native-tests
```
