# PE-CH1 — func_8002FF78 opcode 0x5A Aya tagged setter

Native translation of the Aya-object `0x5A` store. Matching `src/` C
was not added: this worktree has no `asm/`, no era `cc1`, and no
extracted SLUS.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x8002FF78..0x80030108  (101 words, 0x194)
file        0x20778
yaml        [0x20210, asm]
table       D_800910A0[0x5A] @ 0x80091208 = wrapper 0x80018164
jal         0x80018194 (0x5A) and 0x80018200 (0xCE)
```

## Contract

Wrapper `func_80018164`: if `lbu(*(D_8009D2F0)+0x0C)==0` then
`func_8002FF78(tag, value)` with dest `*D_8009D254`. Else
`func_80030220(actor, tag, value)` — **that** is the m0005i
40/41/42/50–52 slot path after `0x6F`.

`func_8002FF78` switch (`tag & 0xFF`) onto `*D_8009D254`:

```text
0 sw+0  1 sh+4  2 sh+6  3 sw+8  4 sh+0xC  5 sh+0xE  6 sh+0x10
10 sh+0x1C  11 sh+0x1E  12 sh+0x20  14 sh+0x22  18 sh+0x26
30 sh+0x50  31 sb+0x56  32 sb+0x57  33 sh+0x58  34 sb+0x5E
255 sh D_800942EC
else no store
```

## Verify

```text
python3 pc_port/tools/pe_ch1_2ff78_oracle.py
# from pc_port/build: PE_TEST_FILTER=2FF78 ./pe-native-tests
```

Oracle: 101/101 ROM words + 0x5A wrapper + Aya tag map.
Native tests: 5 focused `2FF78_*` plus full suite **636/636**.