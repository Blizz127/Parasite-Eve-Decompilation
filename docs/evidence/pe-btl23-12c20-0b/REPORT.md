# PE-BTL23 — type-5 0x0B / 12C20 and 0x41 / 17D9C

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_80012C20

151 words `0x80012C20..0x80012E7C`, SHA-256 `18e047de…22a8`.
`D_800910A0[0x0B]`. `sltiu *arg0, 7` then `jtbl_80010060`.
Always v0=1. OOR stores nothing.

| code | stores | extra |
|---:|---|---|
| 0 | +0x28/2C/30 then jal `1AA78`; snapshot +0x40/44/48 | if actor==D254: `D_800BCF88 \|= 0x80` |
| 1 | +0x40/44/48 | |
| 2 | +0x68/6C/70 | |
| 3 | +0x78/7C/80 | |
| 4 | +0x88/8C/90 | |
| 5 | sh +0x38/3A/3C | |
| 6 | +0x58/5C/60 | |

Live type-5 first visit: code 0 with
`0x08C90000 / 0xF8790000 / 0xF80A0000`, then code 5 with
`0 / 0x400 / 0`. Type 5 is not D254, so BCF88 is not ORed.

## func_80017D9C

9 words `0x80017D9C..0x80017DC0`, SHA-256 `77759305…9549`.
`D_800910A0[0x41]`. Zero jal. `*(D2F0)+0x98 |= 0x40`. v0=1.

## Type-5 prefix

```text
0x14 code 1 rel 0x7C → actor+0x1A0 = base+0xF8
0x0B code 0 → pose + 1AA78 + snapshot
0x0B code 5 → +0x3A = 0x400
0x41 → +0x98 |= 0x40
0x0A / 0x09 / 0x05 already ported
next new: 0x2E / 17AE8 (jal 1A680)
```

Do not name type 5. Do not force 3999C.

## Verify

```text
python3 pc_port/tools/pe_btl23_12c20_oracle.py
PE_TEST_FILTER=BTL23 ./pc_port/build/pe-native-tests
```
