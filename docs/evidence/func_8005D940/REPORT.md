# `func_8005D940` — matching C leaf

Outcome: **MATCHED**. Profile `era_o2_g0`. Span file `[0x4E140,0x4E170)`,
VA `[0x8005D940,0x8005D970)`, size `0x30`. Fill an 8-byte stack record from
`D_8009D280` and submit it (the buffer must be `char buf[8]` — a wider buffer
grows the frame to 0x28 and mismatches).
Triage: `python3 tools/analysis/try_leaf.py src/func_8005D940.c 0x4E140 0x30`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1 `452fb033…`.
