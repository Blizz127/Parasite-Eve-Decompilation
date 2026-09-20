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
