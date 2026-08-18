# PE-BTL17 — 17588 label / resume pointer

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_80017588

76 words `0x80017588..0x800176B8`, SHA-256 `f4bc3706…5e69`.
`D_800910A0[0x14]`. Always v0=1.

`*arg0` selects the destination:

| code | dest |
|---:|---|
| 1 | actor+0x1A0 |
| 2 | actor+0x19C |
| 3 | `*(gp+0x590)+4` (task+4) |
| other | no store |

`*arg1 < 0` stores 0. Otherwise
`*(D2F0)+0x9C + (*arg1)<<1`.

Live type-6 `+0x180` is code 2, rel `0x684` →
`actor+0x19C = stream_base+0xD08`. Confidence: PROVEN
offset/width/writer; meaning TENTATIVE (script-relative
pointer, not named).

After this, type-6 `+0x190` is already-ported ALU
`scratch[0] & 4`, then `0x2` yield. First type-6 17018
visit ends there. Type-1 then hits `0xEA` key `0x190`
(`16658`) — not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl17_17588_oracle.py
PE_TEST_FILTER=BTL17 ./pc_port/build/pe-native-tests
```
