# `func_8008A02C` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0`.

## Span
- File `[0x7A82C,0x7A868)`, VA `[0x8008A02C,0x8008A068)`, size `0x3C`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`.

## Source
Add `a1 - word0` to words 0 and 1 of each `0x40`-byte record for `a2` records.
Triage: `python3 tools/analysis/try_leaf.py src/func_8008A02C.c 0x7A82C 0x3C`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
