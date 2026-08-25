# Matching-C volume campaign run log — 2026-08-25

Base: a648f45, 287 accepted leaves; pool refreshed from 2026-08-24 state to 1137 candidates (TIER 1 96, TIER 2 214, TIER 3 99, SKIP 728).

| attempt | target | pool row | outcome | words | iterations | commit or stash |
|---:|---|---|---|---:|---:|---|
| 1 | func_80072714 @ 0x62F14 | TIER 1; jr-ra; 26 direct callers; real boundaries | SKIP-SDK-LIBRARY-SYSCALL after function-hood/static screen; adjacent func_80072724 screened with same proof | 4 | 0 | labeled stash; committed c28edcc |
| 2 | func_80062A20 @ 0x53220 | TIER 1; jr-ra; 39 direct callers; real boundaries; argument-indexed load | MATCHED@era -O2 -G0; 5/5 exact | 5 | 2 | committed 655628e; target leaf 288 |
| 3 | func_800824C8 @ 0x72CC8 | TIER 1; jr-ra; 12 direct callers; scalar global read/write; address-retention | PARKED-ADDRESS-RETENTION after two allowed phrasings; no integration | 5 | 2 | labeled stash; docs commit pending |
| 4 | func_80085084 @ 0x75884 | TIER 1; jr-ra; 6 direct callers; constant-add getter; real boundaries | MATCHED@era -O2 -G0; 5/5 exact | 5 | 0 | commit pending; target leaf 289 |

The adjacent func_800824DC @ 0x72CDC has the same scalar-global exchange
shape over D_800B8AB8 and was screened into the same park family without a
separate attempt.

## Stop / continuation

This is not a hard stop: the syscall family is a proven SDK/handwritten architecture class, so it is removed from scheduling. No C count increase, carve, build integration, or false progress is claimed. Continue with the next eligible Tier 1 candidate after the docs commit.
