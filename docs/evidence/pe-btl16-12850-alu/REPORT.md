# PE-BTL16 — 12850 ALU; 1731C skip-if-false

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_80012850

244 words `0x80012850..0x80012C20`, SHA-256 `cec98712…b37a`.
`D_800910A0[0x09]`. `sltiu *arg0, 24` then `jtbl_80010000`.
Always v0=1. OOR stores nothing.

Args `[subop, dst, a, b]`. 24 cases match BINDER.md and the
literal body: add/sub/or/and/xor, bool or/and, is-zero, not,
signed slt both orders, eq/ne, signed >=/<=, mul lo, signed
div/rem (retail `break` 6/7 on /0 and INT_MIN/-1), sllv/srav,
copy, neg, plus `3708C`/`370A8`.

`3708C`: 7 words, `mult` then bits `[47:16]` (16.16).
`370A8`: 5 words, `sra b,8` / `div` / `sll 8`.

Live type-6 `+0x138` is subop `0x09`: `cond[0] = (3 < actor+0xF0)`.
After the earlier `0xA`, that local is 0, so `cond[0]=0`.

## func_8001731C

16 words `0x8001731C..0x8001735C`, SHA-256 `20d208e0…0b1f`.
`D_800910A0[0x05]`. If `*arg0==0`, `gp+0x90 = *(D2F0)+0x9C +
(*arg1)<<1`. Always v0=1. Live skip imm `0xB8` → `+0x170`.

Next live word is `0xA` (already ported) then `0x14`/`17588`
(76w) — not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl16_12850_oracle.py
PE_TEST_FILTER=BTL16 ./pc_port/build/pe-native-tests
```
