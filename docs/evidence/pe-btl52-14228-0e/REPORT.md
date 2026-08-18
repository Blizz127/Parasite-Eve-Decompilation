# PE-BTL52 — type-2 fork 0x0E / 14228

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80014228` — 98 words `0x80014228..0x800143B0`,
SHA-256 `e26b9d26…bfab`. `D_800910A0[0x0E]`.
Zero jal. v0=1.

`*arg1==0` uses D254. Self type/id uses D2F0
without the `+0x98` bit-4 check. Else walks
D20C. Miss stores -1. Codes: 0=`+0x0E`,
1=`+0x98`, 2=`lh +0x16`, 3=`+0x0F`.

Live after `0x4B`: `(0,2,0,local[0xD])` reads
self `+0x0E`. The fork then waits until that
byte equals 7. Do not invent command 7.

## Verify

```text
python3 pc_port/tools/pe_btl52_14228_oracle.py
PE_TEST_FILTER=BTL52 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
