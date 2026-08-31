# Tier-2 shape campaign run log — 2026-08-31

Baseline: clean `main` at `81ab3fe`, 380 exact matching-C leaves.

| rung | target | class | result | count | note |
|---:|---|---|---|---:|---|
| PHASE-0 | 175 active Tier-2 rows | partition | PASS | 380 | W=10, S=1, B=121, C=39, SKIP=4; work order W -> S -> B -> C |

The campaign is active. Each leaf must independently pass function hood,
Stage-0 typing, whole-object comparison, carve geometry, exact executable SHA,
packed-span comparison, verifier, one-leaf commit, and push. This log records
results; it does not promote an attempted or parked candidate to matching C.
