# PE-BTL15 — 15DAC nop keys; ops 0xA and 0x1D

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_80015DAC

727 words `0x80015DAC..0x80016908`, SHA-256 `bd05f480…a978`.
`D_800910A0[0xEA]`. Jump table on `*arg0-100`, `sltiu 311`,
table `0x800101B0`. Out-of-range and target `0x800168F4` are
`v0=1` with no stores.

Live type-6 first `0xEA` key `0x194` and type-1 first key
`0x193` are that nop. Key `0x190` → `0x80016658` is not this
cut.

## func_800173F4 / func_80017E20

`173F4`: 7 words, `*arg0=*arg1`, v0=1. Live type-6 kinds
1 then 2: `actor+0xF0 = D_800A77F0[8]` (0 after 34F10).

`17E20`: 18 words. If `*arg0 != *arg1`, `gp+0x90 =
*(D2F0)+0x9C + (*arg2)<<1`. Live compare is 0==0, so no
jump. Next is another `0xA` then op 0 goto `base+0x138`
(`0x9`/`12850`, 244w) — not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl15_15dac_nop_oracle.py
PE_TEST_FILTER=BTL15 ./pc_port/build/pe-native-tests
```
