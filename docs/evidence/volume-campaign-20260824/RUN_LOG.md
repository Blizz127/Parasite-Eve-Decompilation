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

| 11 | `func_8007E6B0` @ `0x6EEB0` | TIER 1; canonical `jr ra`/delay slot; 4 exact-start callers; real/real boundaries | `PARKED-SYMBOLIC-ADDRESS-REGISTER-COLORING` | 21 | 2 | docs-only commit |

Function hood passed. Two source phrasings preserve the indexed ring-buffer
semantics but not retail's `$v1` base / `$a0` index coloring; both exceed the
0x54-byte span at 0x60 bytes. No integration or count change; exact count
remains 335 and the next Tier-1 pool is 20.

| 12 | `func_8005BCBC` @ `0x4C4BC` | TIER 1; canonical `jr ra`/delay slot; 2 exact-start callers; real/real boundaries | `PARKED-GP-STATUS-REGISTER-AND-DELAY-SCHEDULE` | 21 | 2 | docs-only commit |

Function hood passed. Two era `-O2 -G8` phrasings preserve the table-selection
semantics but produce a 20-word body: the status stays in `$a1` instead of
retail `$v0`, and the table base is scheduled later. No integration or count
change; exact count remains 335 and the next Tier-1 pool is 19.

| 13 | `func_80083D9C` @ `0x7459C` | TIER 1; canonical `jr ra`; exact-start callback-table refs; real/real boundaries | `PARKED-SWITCH-TAIL-BLOCK-LAYOUT` | 21 | 2 | docs-only commit |

Function hood passed. Two era `-O2 -G0` switch phrasings recovered the
semantics but emitted 0x60 bytes rather than retail's 0x54: cc1 reloads the
mode in the case-2 path and lays out the shared tail differently. No
integration or count change; exact count remains 335 and the next Tier-1 pool
is 18.

| 14 | `func_80079178` @ `0x69978` | TIER 1; canonical `jr ra`; 4 exact-start callers; real/real boundaries | `SKIP-SDK-LIBRARY-COP2` | 21 | 0 | docs-only commit |

Function hood passed. The body is a handwritten GTE/COP2 wrapper using
`cfc2`, `ctc2`, `lwc2`, a COP2 operation, and `swc2`; it was screened without
burning an ordinary-C attempt. Exact count remains 335 and the next Tier-1
pool is 17.

| 15 | `func_800791D0` @ `0x699D0` | TIER 1; canonical `jr ra`; one exact-start caller; real/real boundaries | `SKIP-SDK-LIBRARY-COP2` | 22 | 0 | docs-only commit |

The body is the exact structural sibling of `func_80079178`, differing only
in the COP2 operation word. It was screened without an ordinary-C attempt.
Exact count remains 335 and the next Tier-1 pool is 16.

## Terminal condition — post-policy continuation

`HARD_STOP=THREE_CONSECUTIVE_PARKS` applies to attempts 11–13
(`func_8007E6B0`, `func_8005BCBC`, `func_80083D9C`). Attempts 14–15 were
zero-attempt COP2 family screens and do not reset or evade that stop. No
additional ordinary-C phrasing attempt is authorized in this campaign until
human review or a refreshed campaign. Exact count remains 335; the 16-row
queue is preserved.
