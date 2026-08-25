# Matching-C volume campaign run log — 2026-08-25

Base: a648f45, 287 accepted leaves; pool refresh and subsequent screens reduced the active pool to 1136 candidates before this attempt (TIER 1 90, TIER 2 214, TIER 3 99, SKIP 733).

| attempt | target | pool row | outcome | words | iterations | commit or stash |
|---:|---|---|---|---:|---:|---|
| 1 | func_80072714 @ 0x62F14 | TIER 1; jr-ra; 26 direct callers; real boundaries | SKIP-SDK-LIBRARY-SYSCALL after function-hood/static screen; adjacent func_80072724 screened with same proof | 4 | 0 | labeled stash; committed c28edcc |
| 2 | func_80062A20 @ 0x53220 | TIER 1; jr-ra; 39 direct callers; real boundaries; argument-indexed load | MATCHED@era -O2 -G0; 5/5 exact | 5 | 2 | committed 655628e; target leaf 288 |
| 3 | func_800824C8 @ 0x72CC8 | TIER 1; jr-ra; 12 direct callers; scalar global read/write; address-retention | PARKED-ADDRESS-RETENTION after two allowed phrasings; no integration | 5 | 2 | labeled stash; docs committed f8703c0 |
| 4 | func_80085084 @ 0x75884 | TIER 1; jr-ra; 6 direct callers; constant-add getter; real boundaries | MATCHED@era -O2 -G0; 5/5 exact | 5 | 0 | committed 37ab013; target leaf 289 |
| 5 | func_80077AA4 @ 0x682A4 | TIER 1; jr-ra; 50 direct callers; packed coordinate helper; real boundaries | MATCHED@era -O2 -G0; 6/6 exact; full SHA exact; verify green at 290 | 6 | 0 | committed 3894b85 |

The adjacent func_800824DC @ 0x72CDC has the same scalar-global exchange
shape over D_800B8AB8 and was screened into the same park family without a
separate attempt.

The next Tier 1 span, func_80078120 @ 0x68920, was screened without a C
attempt as SKIP-SDK-LIBRARY-GTE-TAIL; its five-word tail entry branches into
func_80078134’s shared handwritten-COP2 continuation. No attempt was consumed.

The next Tier 1 span, func_80081E5C @ 0x7265C, was screened without a C
attempt as the same PARKED-ADDRESS-RETENTION-FAMILY established by 824C8/824DC.
No attempt was consumed.

## Stop / continuation

This is not a hard stop: the syscall family is a proven SDK/handwritten architecture class, so it is removed from scheduling. No C count increase, carve, build integration, or false progress is claimed. Continue with the next eligible Tier 1 candidate after the docs commit.
