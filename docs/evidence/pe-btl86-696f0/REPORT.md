# PE-BTL86 — 696F0 live tail after D1A0&0x80 skip

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`696F0` — 121 words `0x800696F0..0x800698D4`,
SHA-256 `8ea0ad39…8089`. Sole live `3F3C4`
jal `3F6E4` on dest-change exit.

`D1A0&0x80` walks jalr slots then clears that
bit. Live `0x4000` skips that half. The
`6984C` tail still runs: null `*(*942E0+4*i)`
for `i=8..0x54`, then `*(*E10BC+4*j)` for
`j=0x1E..0x67`. Unpublished `942E0==0` skips
the first walk (host). Overlay `E10BC` zeros
are a no-op.

## Verify

```text
python3 pc_port/tools/pe_btl86_696f0_oracle.py
PE_TEST_FILTER=BTL86 ./pc_port/build/pe-native-tests
```
