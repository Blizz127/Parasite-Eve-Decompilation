# PE-BTL108 — opcode 0xB2 / 392EC persist scale

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

m0005i type-6 `+0x3A18` `0xB2` runs after the mode-9
`0x40` / `0xAD` / `0xAA` arm. `19798` jals `392EC` and
stores `v0&0xFF` to `*arg0`. `392EC` returns 1 when
`0x80091A1C==0`, else the byte at `0x80091A1D`.

This is the field-script reward/scale interface, not
dest `0xA9400048` and not a synthesized EXP table.

## Verify

```text
python3 pc_port/tools/pe_btl108_0xb2_oracle.py
PE_TEST_FILTER=BTL108 ./pc_port/build/pe-native-tests
```
