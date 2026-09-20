# func_80059F08 — VRAM 0x80059F08 / file 0x4A708 / size 0xC8

**Landed.** Profile `era_o2_g8_aspsx_230` (new assignment for this leaf).

Mode-setup selector: arms the D_8009D048 record pointer and its accessor
table from either the live D_8009D04C context or `func_80052F70`, then returns
`D_8009D090[arg0]`.

## Levers
- `-G8` for the six gp-relative state words; `ERA_ASPSX_VER=2.30` because the
  indexed `lw %lo(D_8009D090)(at)` reads need the 3-word aspsx-2.30 form.
- D_8009D050 = `D_8009D04C + 4` and D_8009D064 = `D_8009D058 + 0xC` (no
  symbols of their own), reached through containing-symbol scalar offsets.
- D_8009D090 / D_8009D098 stay absolute indexed arrays (incomplete arrays).

## Evidence
`try_leaf src/func_80059F08.c 0x4A708 0xC8 --flags "-O2 -G8" --env ERA_ASPSX_VER=2.30`
-> `WORDS MATCH (+8 pad bytes)`. Fresh build (commit `dde81b15`): EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, 875 leaves, `VERIFY_US=PASS`.
