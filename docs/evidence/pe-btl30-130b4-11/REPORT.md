# PE-BTL30 — type-5 0x11 / 130B4 flag-mask test

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_800130B4` — 77 words `0x800130B4..0x800131E8`,
SHA-256 `41ee2a16…5bbd`. `D_800910A0[0x11]`. Zero jal.
Always v0=1.

`*arg0` selects the flag word; `*arg1` is the mask;
`*arg2` receives 1 iff `(flags & mask) == mask`, else 0.

| code | word |
|---|---|
| 0 | `D_8009D26C` |
| 1 | `D_8009D1F4` |
| 2 | `D_8009D1E4` |
| 3 | COP2 FLAG + `D_800A7770`; not this cut |
| ≥4 | no store |

Live type-5 `+0x2FC`: code 1, mask `0x100`, dest
`local[4]`. `3E974` / `ResetTestState` leave
`D1F4=0`, so `local[4]=0`. Do not invent a pad press.

After `0x11` the stream is `0x09` `local[4]!=0` then
`0x05` rel `0x378`. Live 0 skips to `+0x6F0` (`0x20`
then `FFFFFFFF` sentinel at the type-5/type-6 join).
The `0x1C` / `0x0D` arm is the pad-hit path only.

## Verify

```text
python3 pc_port/tools/pe_btl30_130b4_oracle.py
PE_TEST_FILTER=BTL30 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
