# Matching-C volume campaign run log

Base: `811f8f0`, 281 accepted leaves.

| attempt | target | pool row | outcome | words | iterations | commit or stash |
|---:|---|---|---|---:|---:|---|
| 1 | `func_800CBFB4` @ `0xBC7B4` | TIER 1; jr-ra; exact-start table ref; 0 jal/gp/index/loop | `MATCHED@era -O2 -G0` | 2 | 0 | commit with leaf 282 |
| 2 | `func_800CBFBC` @ `0xBC7BC` | TIER 1; jr-ra; exact-start table ref; 0 jal/gp/index/loop | `MATCHED@era -O2 -G0` | 2 | 0 | commit with leaf 283 |
| 3 | `func_800CCF90` @ `0xBD790` | TIER 1; jr-ra; exact-start table ref; 0 jal/gp/index/loop | `MATCHED@era -O2 -G0` | 2 | 0 | commit with leaf 284 |
| 4 | `func_800CCF98` @ `0xBD798` | TIER 1; jr-ra; exact-start table ref; 0 jal/gp/index/loop | `MATCHED@era -O2 -G0` | 2 | 0 | commit with leaf 285 |
| 5 | `func_800CE1EC` @ `0xBE9EC` | TIER 1; jr-ra; exact-start table ref; 0 jal/gp/index/loop | `MATCHED@era -O2 -G0` | 2 | 0 | commit with leaf 286 |
| 6 | `func_800CE1F4` @ `0xBE9F4` | TIER 1; jr-ra; exact-start table ref; 0 jal/gp/index/loop | `MATCHED@era -O2 -G0` | 2 | 0 | commit with leaf 287 |
| 7 | `func_80079024` @ `0x69824` | TIER 1; proven callable handwritten GTE helper; `ctc2 a0,$26` | `PARKED-HANDWRITTEN-COP2` | 3 | 0 | stash ordinary-C boundary candidate; docs-only commit |
| 8 | `func_80078FAC` @ `0x697AC` | TIER 1; proven callable handwritten GTE helper; `ctc2 a0,$27` | `PARKED-HANDWRITTEN-COP2` | 3 | 0 | stash ordinary-C boundary candidate; docs-only commit |
| 9 | `func_80078FB8` @ `0x697B8` | TIER 1; proven callable handwritten GTE helper; `ctc2 a0,$28` | `PARKED-HANDWRITTEN-COP2` | 3 | 0 | stash ordinary-C boundary candidate; docs-only commit |

Final count: **9 attempts; 6 matched; 3 historically parked; 3 reclassified
to `SKIP-SDK-LIBRARY-COP2` after the family screen.**

## Terminal condition

`HARD_STOP=THREE_CONSECUTIVE_PARKS` after attempts 7–9. All three are proven
callable handwritten GTE helpers whose sole semantic instruction is `ctc2`;
ordinary C plus the sanctioned toolchain has no intrinsic for that side effect,
while inline assembly/macros are forbidden. The queue remains intact after
`func_80078FB8`; no Tier 1 candidate was skipped to evade the stop.

## Post-policy Tier-1 continuation

| attempt | target | pool row | outcome | words | iterations | commit or stash |
|---:|---|---|---|---:|---:|---|
| 10 | `func_8006DB48` @ `0x5E348` | TIER 1; canonical `jr ra`/delay slot; 4 exact-start callers; real/real boundaries | `MATCHED@era -O2 -G0` | 21 | 0 | commit `func_8006DB48` |

Function hood and the complete 21-word object comparison passed. A union over the
word-backed `D_800B0CD8` overlay preserves one symbolic base for the byte stores
and flag RMWs; the final C is byte-exact after ordinary relocations. Count:
334→335; Tier 1: 22→21. Evidence:
`docs/evidence/volume-campaign-20260825/func-8006db48/REPORT.md`.
