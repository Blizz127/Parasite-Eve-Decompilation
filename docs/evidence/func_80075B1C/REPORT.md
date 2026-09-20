# `func_80075B1C` — matching C leaf

Outcome: **MATCHED**. Profile `era_o2_g0`. Span file `[0x6631C,0x6634C)`,
VA `[0x80075B1C,0x80075B4C)`, size `0x30`. Call the `+0x38` method of
`*D_80095744` and return `(unsigned)result >> 31`.
Triage: `python3 tools/analysis/try_leaf.py src/func_80075B1C.c 0x6631C 0x30`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1 `452fb033…`.
