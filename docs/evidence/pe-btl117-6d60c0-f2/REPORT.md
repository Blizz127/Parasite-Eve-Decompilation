# PE-BTL117 — 6D60C(0) completes F2 without planting 0x41

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

`6D60C` `sltiu` vs 65: `F2>=0x41` returns 0. That bound is
not a writer. TEXT `sb` sites are only inside `6D60C` plus
`6A868` (`F2=0`).

`jtbl` at `0x80011508`:

| F2 | Target | Notes |
|----|--------|-------|
| 0 | `6D658` | `sw 0` gp+0x418, `87024`; `a0==0` → `6D6EC` → `F2=45` |
| 45 | `6D8BC` | if `+0xEA+idx==0` (6A674), idx++, `F2=50` |
| 50 | `6D944` | `lh +0xE8==-1` skips `6CDA4`; `F2=64` |
| 64 | `6D9E8` | overlay bit 4 clear → `sb F2=0`, return 0 |

`2B0E8` phase 3 only jals `6D60C(0)`. Boot `6A674` stores
`+0xE8=-1` and `+0xEA/+0xEB=0`. That path completes in one
call. `+0xE8==0` jals `6CDA4(1,0)` and parks at 50.

Do not plant `F2=0x41`. `a0==1` still starts at `0x2C`.
State 64 with bit 4 set parks (`86FF8` / 62 deferred).

## Verify

```text
python3 pc_port/tools/pe_btl114_ce54_1a_oracle.py
PE_TEST_FILTER=BTL117 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
