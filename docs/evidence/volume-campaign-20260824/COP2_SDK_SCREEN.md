# libGTE/COP2 setter-family screen

Date: 2026-08-24  
Scope: `asm/disc1/68478.s`, file span `0x69604` through `0x69830`  
Disposition: `SKIP-SDK-LIBRARY-COP2`

## Finding

The three volume-campaign parks are members of one contiguous handwritten
Psy-Q GTE support cluster. The cluster contains 23 tiny entry points whose
observable effects are loads into COP2 data registers (`lwc2`/`mtc2`) or
transfers into COP2 control registers (`ctc2`), followed by `jr ra; nop`.
They are library register/setup helpers, not PE1 game logic. The existing SDK
policy in `docs/ai_context/sdk_map.md` directs libGTE to PsyCross for the port
and says not to decompile SDK code as game code.

This screen therefore moves the family to the pool's SKIP disposition. It does
not claim matching-C leaves, change YAML, or make the three missing `ctc2`
instructions expressible in ordinary C. The campaign hard stop after attempts
7--9 remains a valid historical record.

## Enumerated family

| function | file span | retail operation | proven SDK/COP2 role |
|---|---:|---|---|
| `func_80078E04` | `0x69604-0x69634` | load 5 words; `ctc2` control 0--4 | matrix/control block setter |
| `func_80078E34` | `0x69634-0x69664` | load 5 words; `ctc2` control 8--12 | matrix/control block setter |
| `func_80078E64` | `0x69664-0x69694` | load 5 words; `ctc2` control 16--20 | matrix/control block setter |
| `func_80078E94` | `0x69694-0x696B4` | load 3 words; `ctc2` control 5--7 | projection/depth control setter |
| `func_80078EB4` | `0x696B4-0x696C4` | `lwc2` data 0--1 | GTE data loader |
| `func_80078EC4` | `0x696C4-0x696D4` | `lwc2` data 2--3 | GTE data loader |
| `func_80078ED4` | `0x696D4-0x696E4` | `lwc2` data 4--5 | GTE data loader |
| `func_80078EE4` | `0x696E4-0x69704` | `lwc2` data 0--5 from three pointers | GTE data loader |
| `func_80078F04` | `0x69704-0x69718` | `lwc2` data 20--22 | GTE data loader |
| `func_80078F18` | `0x69718-0x6972C` | `mtc2` data 9--11 | GTE data setter |
| `func_80078F2C` | `0x6972C-0x69738` | `mtc2` data 8 | GTE data setter |
| `func_80078F38` | `0x69738-0x6974C` | `mtc2` data 17--19 | GTE data setter |
| `func_80078F4C` | `0x6974C-0x69764` | `mtc2` data 16--19 | GTE data setter |
| `func_80078F64` | `0x69764-0x69778` | `mtc2` data 12--14 | GTE data setter |
| `func_80078F78` | `0x69778-0x6978C` | `ctc2` controls 0, 2, 4 | GTE control setter |
| `func_80078F8C` | `0x6978C-0x697A0` | `mtc2` data 25--27 | GTE data setter |
| `func_80078FA0` | `0x697A0-0x697AC` | `mtc2` data 30 | GTE data setter |
| `func_80078FAC` | `0x697AC-0x697B8` | `ctc2` control 27 | depth-cue/control setter |
| `func_80078FB8` | `0x697B8-0x697C4` | `ctc2` control 28 | depth-cue/control setter |
| `func_80078FC4` | `0x697C4-0x697E4` | shift 4; `ctc2` controls 13--15 | depth-cue/control setter |
| `func_80078FE4` | `0x697E4-0x69804` | shift 4; `ctc2` controls 21--23 | depth-cue/control setter |
| `func_80079004` | `0x69804-0x6981C` | shift 16; `ctc2` controls 24--25 | `SetGeomOffset`-family helper |
| `func_80079024` | `0x69824-0x69830` | `ctc2` control 26 | `SetGeomScreen` |

The exact names `SetGeomOffset` and `SetGeomScreen` are supported by the
existing projection-math evidence. The remaining names are intentionally not
invented here; their instruction-level COP2 roles are sufficient for the SDK
classification.

## Callable evidence and campaign disposition

The three attempted entries are real callable functions, not padding:

- `func_80078FAC`: direct `jal` at `0x80077F48`, canonical `ctc2; jr ra; nop`.
- `func_80078FB8`: direct `jal` at `0x80077F50`, canonical `ctc2; jr ra; nop`.
- `func_80079024`: five direct callers, including `0x8003E648`, `0x8003F1E8`,
  `0x8006685C`, `0x80068418`, and `0x80068BD0`, canonical `ctc2; jr ra; nop`.

The adjacent family geometry, handwritten annotations, and COP2 register
operations establish the broader library cluster. No source phrasing or
ordinary-C intrinsic is introduced. The three candidates remain preserved in
their named stashes as historical diagnostic evidence.

The refreshed pool later exposed two callable rows that had incorrectly
remained in Tier 1 despite this family-wide disposition:

- `func_80078E04`: three direct callers at `0x80031A14`, `0x80068400`, and
  `0x800C724C`; five loads followed by `ctc2` controls 0--4.
- `func_80078E94`: three direct callers at `0x80031A2C`, `0x80068408`, and
  `0x800C7240`; three loads followed by `ctc2` controls 5--7.

Both have canonical returns and real boundaries. They are functions, not
padding, but their proven handwritten COP2 bodies place them in this same
`SKIP-SDK-LIBRARY-COP2` family. The 2026-08-25 pool correction moved those two
rows from Tier 1 to SKIP without attempts or matching-C count changes. The
other 18 unattempted family members were already present in the base SKIP
section.

`MATCHING_C_COUNT=287`  
`INTEGRATION=NONE`  
`POOL_DISPOSITION=SKIP-SDK-LIBRARY-COP2`  
`HARD_STOP=VALID-HISTORICAL-THREE-CONSECUTIVE-PARKS`
