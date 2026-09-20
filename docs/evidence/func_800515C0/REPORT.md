# `func_800515C0` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0`.

## Span
- File `[0x41DC0,0x41DF8)`, VA `[0x800515C0,0x800515F8)`, size `0x38`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`.

## Source
Latch `a0` into `*(*D_8009D254)+0xC` when present, and into `D_800C0E08`.
Single-leaf triage: `python3 tools/analysis/try_leaf.py src/func_800515C0.c 0x41DC0 0x38`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
