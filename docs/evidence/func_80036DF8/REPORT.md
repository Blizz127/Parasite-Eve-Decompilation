# `func_80036DF8` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0`.

## Span
- File `[0x275F8,0x27634)`, VA `[0x80036DF8,0x80036E34)`, size `0x3C`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`.

## Source
Init `D_800A76A8` / `D_800A76A4` / `D_800A76A0` with the retail two-store
flag protocol; the globals are `volatile` so the redundant zero-then-value
stores survive at `-O2`.
Triage: `python3 tools/analysis/try_leaf.py src/func_80036DF8.c 0x275F8 0x3C`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
