# PE-BTL91 — 0x55 / 144FC park-rewind wrapper

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`D_800910A0[0x55] = 0x800144FC`. Park epilogue
`14660`: `v0=0`, `CE00 -= 0xC`, `task+0x10=1`.
Complete `3B` takes `14658` `v0=1`.

This cut dispatches the already-ported state
cuts (0 / 0x37 / 0x38 / 0x39 / 0x3A / 0x3B)
and applies that park rewind so 17018 retries
`0x55` instead of skipping the encounter start.

Type-6 `+0xFC8` imm 2 is still behind
`scratch[0]&4`. Do not force the bit.

## Verify

```text
python3 pc_port/tools/pe_btl91_144fc_55_oracle.py
PE_TEST_FILTER=BTL91 ./pc_port/build/pe-native-tests
```
