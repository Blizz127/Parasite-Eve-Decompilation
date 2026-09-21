# `func_800428D4` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_three_word`.

## Span
- File `[0x330D4,0x33110)`, VA `[0x800428D4,0x80042910)`, size `0x3C`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; profile adds
  `MASPSX_THREE_WORD_SYMBOL_STORE=1` for the indexed `sb` to `D_800A0ED5`.

## Source
Clear the state byte at `D_800A0ED5[(D_800A1860-1)*1048]`.
Triage: `python3 tools/analysis/try_leaf.py src/func_800428D4.c 0x330D4 0x3C --env MASPSX_THREE_WORD_SYMBOL_STORE=1`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
