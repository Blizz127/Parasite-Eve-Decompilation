# Executed priority — pc_port-only functions the route reaches

Source: the **route** coverage run (`docs/evidence/exec-coverage/route_exec_coverage.txt`), classified by `tools/progress/exec_coverage.py` against the retail boundary map. Binary `pc_port/build-coverage/parasite-eve-port` (SHA-256 `337152bde25ecf6c7b1295972f96e6d9e420b6e00e2e0efc9eee62771cd5543d`).

The route entered **710** distinct guest functions: **264** backed by a decompiled C leaf (37.18%) and **446** backed by a pc_port transcription only. The table below ranks the top 25 pc_port-only functions by descending retail size — matching them is what moves the executed C-share, because the run already reaches them.

pc_port-only bytes (all 446 functions): **216,840**.

| # | function | retail bytes | running bytes |
| ---: | --- | ---: | ---: |
| 1 | `func_8001D340` | 8596 | 8596 |
| 2 | `func_8002BC90` | 5472 | 14068 |
| 3 | `func_8002DC58` | 5208 | 19276 |
| 4 | `func_80037870` | 4256 | 23532 |
| 5 | `func_80030894` | 3152 | 26684 |
| 6 | `func_8002D1F0` | 2664 | 29348 |
| 7 | `func_80063E0C` | 2500 | 31848 |
| 8 | `func_80036448` | 2432 | 34280 |
| 9 | `func_8007041C` | 2292 | 36572 |
| 10 | `func_80027D14` | 2144 | 38716 |
| 11 | `func_80032B0C` | 2128 | 40844 |
| 12 | `func_8001F9C4` | 2072 | 42916 |
| 13 | `func_8001AE40` | 1980 | 44896 |
| 14 | `func_800236E8` | 1836 | 46732 |
| 15 | `func_80028574` | 1748 | 48480 |
| 16 | `func_80033A40` | 1732 | 50212 |
| 17 | `func_80055760` | 1716 | 51928 |
| 18 | `func_80034104` | 1712 | 53640 |
| 19 | `func_8006C5BC` | 1708 | 55348 |
| 20 | `func_8004C608` | 1608 | 56956 |
| 21 | `func_8006AD40` | 1564 | 58520 |
| 22 | `func_80031760` | 1548 | 60068 |
| 23 | `func_8003A6A8` | 1512 | 61580 |
| 24 | `func_80026FF8` | 1492 | 63072 |
| 25 | `func_8003B144` | 1476 | 64548 |

Retail sizes come from the `nonmatching <name>, <size>` lines in `asm/disc1/*.s` (the same boundary map `exec_coverage.py` uses). Regenerate with the same command that writes `coverage.json`, adding `--priority-out docs/evidence/exec-coverage/EXECUTED_PRIORITY.md`.

## Derived reclassification at 902 leaves (NOT a re-measured run)

The executed guest graph does not change when a function becomes a matching C leaf, so the
C-share can be updated exactly by reclassifying the recorded hit set against the current
`configs/USA/disc1.yaml`. This is a DERIVED number; the binary sha256 and the raw hit logs
above still correspond to the 864-leaf snapshot. Re-run the coverage build to re-measure.

| run | executed | C at 864 | C at 902 | share at 902 |
|---|---:|---:|---:|---:|
| route | 710 | 264 (37.18%) | 300 | **42.25%** |

Newly matched and executed (route, 36): `func_80017588`, `func_800181CC`, `func_8001897C`, `func_8003D834`, `func_800409B4`, `func_8004620C`, `func_8004AE1C`, `func_8004BB80`, `func_8004FA10`, `func_80050878`, `func_800509E0`, `func_80051CC4`, `func_800527C8`, `func_80059F08`, `func_8005C498`, `func_8005E038`, `func_8005E788`, `func_8005F27C`, `func_8005FA3C`, `func_8005FB74`, `func_8005FDF0`, `func_8005FF28`, `func_80065260`, `func_80068D28`, `func_8006C4C4`, `func_8006E1C0`, `func_800701B4`, `func_800702DC`, `func_8007BAC0`, `func_800825C0`, `func_80085644`, `func_800C22F8`, `func_800C3238`, `func_800C6EF8`, `func_800C6F4C`, `func_800CEB8C`

| movie | 229 | 94 (41.05%) | 102 | **44.54%** |

Newly matched and executed (movie, 8): `func_800409B4`, `func_80051CC4`, `func_800527C8`, `func_80068D28`, `func_8006E1C0`, `func_8007BAC0`, `func_800825C0`, `func_80085644`
