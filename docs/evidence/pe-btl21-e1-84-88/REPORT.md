# PE-BTL21 — type-1 0xE1 / 0x84 / 0x88

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

| op | VA | words | stores | live |
|---|---|---:|---|---|
| 0xE1 | `1A374` | 7 | `sb *arg0 → D_800BCFFC` | 0x54 |
| 0x84 | `18E84` | 12 | `sh *arg0 → D_800CD020`, `sh *arg1 → D_800CD022` | 0x800, 0x800 |
| 0x88 | `18F54` | 8 | `D_800BCFEE &= ~0x40` | argc 0 |

All v0=1. Do not name these camera/battle without more
consumers.

Type-1 then runs already-ported `0x14` (actor+0x19C =
base+0x200) and `0x02` (yield). Next type-1 visit is
`0x08`/`1735C` (35038 spawn, including type 0) — not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl21_e1_84_88_oracle.py
PE_TEST_FILTER=BTL21 ./pc_port/build/pe-native-tests
```
