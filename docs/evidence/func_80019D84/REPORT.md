# `func_80019D84` — matching C leaf
Outcome: **MATCHED**. `era_o2_g0`. File `[0xA584,0xA5B8)`, VA
`[0x80019D84,0x80019DB8)`, size `0x34`. `func_800375E0(**a0, 1, buf)` with
`short buf[8]` (same frame-size lever as func_80017410).
Triage: `python3 tools/analysis/try_leaf.py src/func_80019D84.c 0xA584 0x34`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1 `452fb033…`.
