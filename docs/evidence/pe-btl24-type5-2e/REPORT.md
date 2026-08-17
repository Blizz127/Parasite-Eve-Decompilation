# PE-BTL24 — type-5 0x2E / 0x4E / 0x2F / 0x30

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

Live type-5 `0x05` skip-if-false (persist[0x54]&0x2000==0)
jumps to `+0xB8`:

| op | VA | words | effect | v0 |
|---|---|---:|---|---|
| 0x2E | `17AE8` | 19 | `1A680(actor, lhu *arg0)`; `+0x98 &= ~0x100` | 1 |
| 0x4E | `17EC4` | 14 | `+0x14 = min(+0x0F, *arg0) << 16` | 1 |
| 0x2F | `17B34` | 16 | `+0x12 = min(+0x0F, *arg0)`; `+0x98 \|= 0x200` | 1 |
| 0x30 | `17B74` | 16 | `task+0x10=1`; if `+0x16 != +0x12` then `CE00-=8` | 0 |

Live immediates are 0. Host persist zeros take the skip.
`0x30` yields. Equal `+0x12/+0x16` (both 0 after this
prefix) does not rewind, so the next visit is `0x02`.

`1A680` is the already-ported command cut. Type-5 command 0
needs a published `D_800B0E98[5*48]` resource; EXE BSS is 0.

Do not name type 5. Do not force 3999C.

## Verify

```text
python3 pc_port/tools/pe_btl24_type5_2e_oracle.py
PE_TEST_FILTER=BTL24 ./pc_port/build/pe-native-tests
```
