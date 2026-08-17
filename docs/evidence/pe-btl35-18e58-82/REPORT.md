# PE-BTL35 — type-0 0x82 / 18E58 → 66800 view apply

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80018E58` — 11 words `0x80018E58..0x80018E84`,
SHA-256 `99b34ecf…def3`. `D_800910A0[0x82]`.
jal `66800(*arg0)`; v0=1.

`func_80066800` — 99 words `0x80066800..0x8006698C`.
View = `*D_800B1624 + *(container+0x1C) + index*52`.
Publishes H through `*D_800BCFA8` and `79024`, copies
nine rotation halfwords and three translation words
through `*D_800BCFA4`, stores the byte index at
`D_800BCFFD`, and sets `D_800BCF88 |= 0x80`.

Live type-0 after `0x65`/`0x2E`: imm 1. `D_800B1624`
is the 6B4F8 `+0x94C` publish, not a hand-written
pointer. Do not invent the view record.

## Verify

```text
python3 pc_port/tools/pe_btl35_18e58_oracle.py
python3 pc_port/tools/pe_ch2_66800_oracle.py
PE_TEST_FILTER=BTL35 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
