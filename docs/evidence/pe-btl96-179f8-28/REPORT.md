# PE-BTL96 — opcode 0x28 bit-clear

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`179F8` — 11 words `0x800179F8..0x80017A24`,
SHA-256 `cb5775eb…1aeb`. `D_800910A0[0x28]`.
`*arg0 &= ~(1 << *arg1)`; `v0=1`. Clear twin
of `0x2A`.

Type-6 `+0x1A38` imms `[0,2]` clears
`scratch[0]` bit 2 after `+0x1850`. Not a
first-entry unlock. Do not force the bit.

## Verify

```text
python3 pc_port/tools/pe_btl96_179f8_28_oracle.py
PE_TEST_FILTER=BTL96 ./pc_port/build/pe-native-tests
```
