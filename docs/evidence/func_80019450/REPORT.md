# `func_80019450` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0`.

## Span
- File `[0x9C50,0x9C84)`, VA `[0x80019450,0x80019484)`, size `0x34`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`.

## Source
`(2 * (short)(*D_8009D2F0)[0x224]) * **a0 >> 16` stored to the short at
`(*(int*)(p+0x1B4))+0x14`; return 1.
Triage: `python3 tools/analysis/try_leaf.py src/func_80019450.c 0x9C50 0x34`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
