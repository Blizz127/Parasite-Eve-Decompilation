# PE-BTL46 — type-2 0x59 / 18004 through 2FE78 and 3010C

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80018004` — 31 words `0x80018004..0x80018080`,
SHA-256 `2993bcdd…f4ac`. `D_800910A0[0x59]`.

```text
if (D2F0+0x0C == 0)
    v0 = 2FE78(lbu *arg0)       # Aya / *(*D254)
else
    v0 = 3010C(D2F0, lbu *arg0) # slot tags
*arg1 = v0
v0 = 1
```

`func_8002FE78` — 64 words `0x8002FE78..0x8002FF78`,
SHA-256 `10946440…c3aa`. Base is `*(*D_8009D254)`.
`sltiu 0x17`; `jr D_80010AC8[tag]`. Nops / OOB
return `-1000`.

`func_8003010C` — 69 words `0x8003010C..0x80030220`,
SHA-256 `10bca057…8787`. `slot=*actor`;
`idx=(tag&0xFF)-41`; `idx>=90` → `-1000`;
`jr D_80010B28[idx]`. Live tag 44 loads
`slot+0x10` and clamps negatives to 0.

Live type-2 `+0xBA4`: argc=2 k=`[0,1]` imms
`0x2C, 0xC` → tag 44 → `local[0xC]`. Type 2
takes 3010C. After ALU / `0x5A` writes, live
`scratch[0]&4==0` makes the following `0x05`
skip to `0x12`. Do not force that bit.

## Verify

```text
python3 pc_port/tools/pe_btl46_18004_oracle.py
PE_TEST_FILTER=BTL46 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
