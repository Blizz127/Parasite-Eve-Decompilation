# PE-BTL36 — type-0 0x9C / 19410 fade-wait

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80019410` — 16 words `0x80019410..0x80019450`,
SHA-256 `ab0d665b…1ce8`. `D_800910A0[0x9C]`. Zero jal.

```text
if (CFEE & 3) < 2:
    v0 = 1
else:
    CE00 -= 8
    D300+0x10 = 1
    v0 = 0
```

Live type-0 after persist[0]&2 skip: `0x1C` payload
`0xFB`, `0x86` imm `0x3C` (CFEE=6, CFF6=60, CFF8=0),
then this wait. CFEE=6 ⇒ yield and rewind.

The authentic progressor is `func_80068E24` (202w
`0x80068E24..0x8006914C`). Field-tick `3F3C4` jals
it at `0x8003F588`. On CFEE&3==2 it increments CFF8
and, when CFF8>=CFF6 and CFEE&4!=0, `sb 0` to CFEE.
Do not invent the clear. Native `3F3C4` is still a
bootstrap stub; `68E24` is still a stub.

## Verify

```text
python3 pc_port/tools/pe_btl36_19410_oracle.py
PE_TEST_FILTER=BTL36 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
