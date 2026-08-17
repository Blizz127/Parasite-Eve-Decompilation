# PE-BTL29 — type-5 0x24 / 1784C task-word copy

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_8001784C` — 12 words `0x8001784C..0x8001787C`,
SHA-256 `0a67c25d…17e9`. `D_800910A0[0x24]`. Zero jal.
Always v0=1.

`lw 0x590($gp)` is `D_8009D300`. Then:

```text
*arg0 = *(task + 0x18)
*arg1 = *(task + 0x1C)
```

Live type-5 after the `0xD9` ALU: both the
`cond[2]!=0` arm (`0x20` then `0x24`) and the
`0x05` rel `0x162` miss (`+0x2C4`) execute the same
`0x24` with kinds k1,k1 → `local[2]`, `local[3]`.
Zero-pose ratan2 wrap makes `cond[2]==1`, so the
live arm is `0x20` (yield) then `0x24`.

`12700` does not write `+0x18`/`+0x1C`. `124F8`
zeros the 72-slot pool. Live copies are 0 unless a
later writer fills those words. Do not invent that
writer. Do not name the fields.

Next live word is `0x09` `local[2]==0`, then `0x05`
rel `0x378`. Live 0 takes `0x11` (`0x800130B4`, 77w,
0 jals).

## Verify

```text
python3 pc_port/tools/pe_btl29_1784c_oracle.py
PE_TEST_FILTER=BTL29 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
