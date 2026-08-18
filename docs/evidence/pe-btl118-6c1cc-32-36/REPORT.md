# PE-BTL118 — 6C1CC 32–36; 24A3C case 0/2/5 3C5D8

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

`6C1CC` `+0xED` jtbl at `0x800113D0`. `6A674` stores `+0xED=32`.
Case 2 parks while `+0xED` is in `[32,40)`.

| ED | Target | a0=1 |
|----|--------|------|
| 32 | `6C218` | overlay`\|=0x20000`, CE2=14, save old to `+0x13`, ED=33, v0=1 |
| 33 | `6C270` | ED=34 |
| 34 | `6C278` | ED=35 |
| 35 | `6C280` | optional `0x200000` / `+0xE\|=4`, ED=36 |
| 36 | `6C304` | `6BECC`; a0!=0 → ED=39 |
| 39 | `6C390` | `1A680(21)` / `3D050` / `6698C` / `3D834` — not this cut |

Do not stub state 39 to return 0.

`24A3C` case 0 jals `6C1CC(1)` without checking v0, `CE48=0`,
`3C5D8(Aya+0x1B4, 30)`, `6F39C(0x6B)` → `D258`. Case 2 jals
`3C5D8(Aya+0x1B4, 30)` and `6F39C(0x6C)`. Case 5 shared tail
`25144` is `3C5D8(Aya+0x1B4, 15)`.

`6F39C` runs only when `D_800942E0` is non-null (host table).

## Verify

```text
python3 pc_port/tools/pe_btl114_ce54_1a_oracle.py
PE_TEST_FILTER=BTL118 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
