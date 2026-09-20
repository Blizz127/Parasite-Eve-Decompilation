# `func_80017410` — matching C leaf
Outcome: **MATCHED**. `era_o2_g0`. File `[0x7C10,0x7C44)`, VA
`[0x80017410,0x80017444)`, size `0x34`. `func_800375E0(**a0, 0, buf)` with
`short buf[8]` (a single `short` shrinks the frame to 0x20 and mismatches).
Triage: `python3 tools/analysis/try_leaf.py src/func_80017410.c 0x7C10 0x34`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1 `452fb033…`.
