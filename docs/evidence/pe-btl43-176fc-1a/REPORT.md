# PE-BTL43 — type-2 0x1A / 176FC 70D6C/70DD0 write

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_800176FC` — 26 words `0x800176FC..0x80017764`,
SHA-256 `ae4816ac…2ab2`. `D_800910A0[0x1A]`.
Always v0=1.

```text
if (*arg1 == *arg2)
    *arg0 = func_80070D6C()
else
    *arg0 = func_80070DD0(*arg1, *arg2)
```

`70D6C` / `70DD0` are already ported (Phase 6E-B2).
Live type-2 after `0xDC`: dest `local[0x18]`,
`70DD0(0, 0x64)`.

## Verify

```text
python3 pc_port/tools/pe_btl43_176fc_oracle.py
PE_TEST_FILTER=BTL43 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
