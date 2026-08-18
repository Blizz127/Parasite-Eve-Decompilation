# PE-BTL78 — 70E54 live prefix on 3F3C4

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`70E54` — 86 words `0x80070E54..0x80070FAC`,
SHA-256 `64c882e8…0537`. `3F3C4` @ `3F590`.
Also `6EB54`. The symbol stub in `psx_compat.h`
is not replaced this cut (shared dirty header).

Live 3F3C4 already rejected `B0CD8&0x200`, so
the `70E7C` taken branch is the bit-clear arm:

1. `74DC0(0)` DrawSync
2. `42FE8` — 20 words. `bne gp+0x168, 6`.
   Live `0x8009CED8` is 0/1/5, not 6, so
   `7506C` is skipped.
3. `73A44(2)` VSync

`74A44` / `755F0` / `6EC08` / `75424` / `754E4`
are not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl78_70e54_oracle.py
PE_TEST_FILTER=BTL78 ./pc_port/build/pe-native-tests
```
