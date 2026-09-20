# `func_80016FE0` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0`.

## Span
- File `[0x77E0,0x7818)`, VA `[0x80016FE0,0x80017018)`, size `0x38`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`.

## Source
Store 0/1 through `*a0` based on `D_8009D2E8` bit 0; return 1. Single-leaf
triage: `python3 tools/analysis/try_leaf.py src/func_80016FE0.c 0x77E0 0x38`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
