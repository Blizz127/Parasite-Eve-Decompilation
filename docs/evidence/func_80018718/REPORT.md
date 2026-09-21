# `func_80018718` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0`.

## Span
- File `[0x8F18,0x8F54)`, VA `[0x80018718,0x80018754)`, size `0x3C`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`.

## Source
Store 1/0 through `*a0` by `D_800A76C4` bit 2; return 1.
Triage: `python3 tools/analysis/try_leaf.py src/func_80018718.c 0x8F18 0x3C`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
