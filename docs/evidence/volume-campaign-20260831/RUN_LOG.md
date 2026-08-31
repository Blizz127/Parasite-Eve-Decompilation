# Tier-2 shape campaign run log — 2026-08-31

Baseline: clean `main` at `81ab3fe`, 380 exact matching-C leaves.

| rung | target | class | result | count | note |
|---:|---|---|---|---:|---|
| PHASE-0 | 175 active Tier-2 rows | partition | PASS | 380 | W=10, S=1, B=121, C=39, SKIP=4; work order W -> S -> B -> C |
| VOLUME-151 | `func_8007FC64` (`0x70464`, 9 words) | W | MATCHED phrasing 1 | 381 | direct-call hood; SDK `CD_sync(1, result)` wrapper; era `-O2 -G0`; 9/9 object, packed span, exact SHA, verifier all pass |
| VOLUME-152 | `func_8007FC88` (`0x70488`, 9 words) | W | MATCHED phrasing 1 | 382 | independently proven direct caller; SDK `CD_ready(1, result)` twin; exact 9/9; consumes final 0x24 asm span; exact SHA and verifier pass |
| VOLUME-153 | `func_800C7D00` (`0xB8500`, 11 words) | W | MATCHED phrasing 1 | 383 | exact-start callback-table hood at `0xD1074`; six-argument forward to matched `func_800C2AF0`, then zero; exact 11/11 object and packed span; carve `0xE18 + 0x2C + 0x98 = 0xEDC`; exact SHA and verifier pass |

The campaign is active. Each leaf must independently pass function hood,
Stage-0 typing, whole-object comparison, carve geometry, exact executable SHA,
packed-span comparison, verifier, one-leaf commit, and push. This log records
results; it does not promote an attempted or parked candidate to matching C.
