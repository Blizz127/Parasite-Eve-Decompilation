# PE-BTL83 — 70E54 flips guest CDDC on the 754E4 path

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`754E4` — 54 words `0x800754E4..0x800755BC`,
SHA-256 `628eb654…9b55`. DrawOTagEnv:
`75EE0(env+0x1C, ot)`, tag splice, jalr
`76B98`, `71A34` memcpy 0x5C to `9575C`.

`70E54` live: `6EC08==0` and `B0CD8&0x200==0`
takes `754E4(B0E38[CDDC]+0x3FFC, CDC8+92*CDDC)`
then `CDDC = (CDDC == 0)`. The GPU submit is
not this cut. The epilogue flip is.

`3F5DC` `66C7C(0x0F)` needs white RGB and
`CFEE&0x40`. Live `0x86` leaves `CFEE=6` and
zero RGB, so that jal is skipped. `6A25C`
needs `D26C&0x0F000006`. Do not invent pad.
Next executed `3F3C4` jal is `6A0E8`, which
early-outs while `D1A0&0x10==0` (live `0x4000`).

## Verify

```text
python3 pc_port/tools/pe_btl83_754e4_cddc_oracle.py
PE_TEST_FILTER=BTL83 ./pc_port/build/pe-native-tests
```
