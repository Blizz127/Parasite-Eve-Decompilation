# func_800527C8 — VRAM 0x800527C8 / file 0x42FC8 / size 0xC4

**Landed.** Profile `era_o2_g8` (new assignment for this leaf).

Phase-transition init: clears the three-word state block at D_8009D02C, runs
the 15-call reset chain, sets D_800B0CD8 bit 0x40000000 and tail-calls
func_800371A4(1).

## Levers
- The block clear is gp-relative in retail: `D_8009D02C` and `D_8009D034` are
  scalars under `-G8`; the interior word D_8009D030 has no symbol, so it is
  reached as `*(int *)((char *)&D_8009D02C + 4)`.
- `D_800B0CD8` must stay **absolute** in this -G8 unit, so it is declared as an
  incomplete array (`extern unsigned int D_800B0CD8[];`) and accessed as
  `D_800B0CD8[0]`.
- The outer function is declared to return `func_800371A4`'s int value so cc1
  leaves `$v0` untouched and the epilogue delay slot is `nop`, matching retail.

## Evidence
`try_leaf src/func_800527C8.c 0x42FC8 0xC4 --flags "-O2 -G8"` -> `WORDS MATCH`.
Fresh build (commit `dde81b15`): EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, 875 leaves, `VERIFY_US=PASS`.
