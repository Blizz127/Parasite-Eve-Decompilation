# PE-BTL25 — type-3 0x5E pose-copy

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80014694` — 147 words `0x80014694..0x800148E0`,
SHA-256 `97c3ff9a…059e`. `D_800910A0[0x5E]`. Zero jal.
Always v0=1.

`*arg1 == 0` uses `D_8009D254` and does not test `+0x98&0x10`.
`*arg1 != 0` walks `D_8009D20C` via `+4` for matching
`+0x0C/+0x0D` with `!(+0x98&0x10)`. Miss stores `*arg6 = -1`.
Hit stores `*arg6 = 1` then copies one of the seven 12C20
pose groups (`jtbl_80010190`) into `*arg3/*arg4/*arg5`.
Code 5 sign-extends `lh +0x38/+0x3A/+0x3C`.

Live type-3 after the first `0x02` yield:

```text
2400E05E  argc 7
code 0, type 0, idB 0
dests local[0..2], flag local[3]
```

Type 0 is `D254` from the earlier type-1 `0x08`. Live type-0
`+0x28/+0x2C/+0x30` were stored as 0 by that spawn.

Next type-3 word is `0x77` / `14DA0` (jal `1CAB0`). Not this
cut. Do not stub `1CAB0`.

Type-0 first opcode remains `0x9B`/`15240`. That body always
jals unrecovered `39B74`/`3A6A8`/`3E188` and needs the
unpublished `+0x1B4` model object (`6C118` / `362B8` /
`3D050`). Do not stub those. Do not force `3999C`.

## Verify

```text
python3 pc_port/tools/pe_btl25_14694_oracle.py
PE_TEST_FILTER=BTL25 ./pc_port/build/pe-native-tests
```
