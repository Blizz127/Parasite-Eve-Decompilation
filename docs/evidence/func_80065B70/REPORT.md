# `func_80065B70` — matching C leaf
Outcome: **MATCHED**. `era_o2_g0`. File `[0x56370,0x56438)`, VA
`[0x80065B70,0x80065C38)`, size `0xC8`. Block init of the
`D_800BCF88..D_800BD028` scratch struct (mixed int/char/short fields), return 0.
This leaf previously had no `src/` authority (a `game/decomp` orphan); the new
source removes it from the orphan allowlist.
Triage: `python3 tools/analysis/try_leaf.py src/func_80065B70.c 0x56370 0xC8`.
Cumulative authority `scripts/build_us.sh` EXACT SHA-1 `452fb033…`.
