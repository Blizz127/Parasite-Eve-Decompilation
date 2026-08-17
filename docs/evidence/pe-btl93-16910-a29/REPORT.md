# PE-BTL93 — 0xED key 0xA29 actor+0x27D

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`16910` case `0x80016C78`: `lw *arg1`,
`lw D2F0`, delay-slot `sb` to `actor+0x27D`,
`v0=1`. Live type-0 / type-2 first `0xED`
is key `0xA29` imm `0x80`. `35038` already
plants `+0x27D=0x80`, so first visit is a
matching store. Not the scratch[0]&4 setter.

`E00CC` arms stay not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl93_16910_a29_oracle.py
PE_TEST_FILTER=BTL93 ./pc_port/build/pe-native-tests
```
