# PE-BTL51 — type-2 0x64 / 184EC through 2FAF8

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_800184EC` — 32 words `0x800184EC..0x8001856C`,
SHA-256 `9b345fcb…631d`. `D_800910A0[0x64]`.
`jal 2FAF8(D2F0, lbu *arg0)`; `*arg1` is the
sign-extended byte. v0==0 rewinds `CE00` by
`0x10` and sets delay 1. Live `(3, local[6])`.

`func_8002FAF8` — 224 words `0x8002FAF8..0x8002FE78`,
SHA-256 `67f58732…4673`. Early-out 1 if `*actor==0`
or `(D1A0&2)==0`. JT `D_80010A88` on `+0x0E`.
Retail `1A680(actor,2)` leaves `+0x0E=2` →
`JT[2]=2FD74` activate of `slot+code*16+0x1C`.
Returns 0 until that record byte is 4.

`1A680` sites use the existing bootstrap cut.
`6DCE4` is not this cut and is not live.
Do not invent the byte-4 producer.

## Verify

```text
python3 pc_port/tools/pe_btl51_184ec_oracle.py
PE_TEST_FILTER=BTL51 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
