# PE-BTL81 — 6EC08 two-byte status

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`6EC08` — 25 words `0x8006EC08..0x8006EC6C`,
SHA-256 `9169b21f…eeed`. Zero jal. `70E54` @
`70EEC` and `3F3C4` @ `3F50C`.

`lb B0DBA==0` → 0. Else `lh B0DBC<=0` → 0.
Else `lb B0DBB==0` → 1, else 2.

`3F3C4@3F50C` jals it after `B0CD8&0x100`.
Live 0 skips overlay `122040`/`121A00`/`6E60C`.

`70E54` `sll 24` / `bne`: nonzero goes to
`75424`. Live 0 plus `B0CD8&0x200==0` takes
`754E4`. That DrawOTagEnv tail is not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl81_6ec08_oracle.py
PE_TEST_FILTER=BTL81 ./pc_port/build/pe-native-tests
```
