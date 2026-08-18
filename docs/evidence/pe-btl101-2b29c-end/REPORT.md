# PE-BTL101 — 2B29C phases; case 5 mode=-1 / 6A25C

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

This is **player-death** aftermath, not encounter victory.
`2B29C` case 5 stores mode `-1` and `6A25C` writes dest
`0xA9400048` (boot/title special), saving the old dest at
`D_800A77F4` and `D_800B0CD8 |= 0x100`.

## Phases (`D_8009CE74`)

| Phase | Behavior this cut |
|---|---|
| 0 | BTL100 death-clip wait / enemy walk |
| 1 | CE70!=60: remaining actors `+0x98\|=0x10`; CE70--; at 0 → phase 2, CE70=30 |
| 2 | CE70 drain → phase 3, CE70=80 |
| 3 | CE70 drain → phase 4, CE70=60 |
| 4 | Aya `+0x252==0` && `B0D8A==0` → phase 5; else CE70-- |
| 5 | `295E4` tail, mode=-1, `6A25C` |

Deferred: overlay geometry, `21D4C`, `51510`, `67B40`,
`86C5C`, `703F4`, `866A4`, sound/CD jals inside `6A25C`.

## Verify

```text
python3 pc_port/tools/pe_btl101_2b29c_end_oracle.py
python3 pc_port/tools/pe_btl100_mode3_oracle.py
PE_TEST_FILTER=BTL101 ./pc_port/build/pe-native-tests
PE_TEST_FILTER=BTL100 ./pc_port/build/pe-native-tests
```
