# Day 2 station-neighbor routing evidence

Stage103 expands the initial four-script station inventory to nine shared
scripts. These are investigation candidates, not nine proven Day2 rooms.
`pc_port/tools/pe_day2_station_routes.py` reads the original disc, pins the
executable SHA1 and all nine script SHA256 values, and executes closed routing
regions through original17294/1731C/12850/173F4. Argument pointer banks, story,
flags and incoming selector values are explicit fixtures. Every asynchronous
or scene command is a stop boundary. No preceding movement, conversation,
trigger geometry, mailbox delivery or native transition is claimed executed.

Original script data and detailed results stay ignored in
`local/live/day2-station-routes-103.json`. Addresses below are the extracted
script's existing relocation convention. Persist indices are decimal; story
is persist74. Numeric story values and masks below are hexadecimal.

## Expanded script inventory

The original four scripts remain M0351I,
M0041I, M0042I and M0043I. Added neighbors are:

| Script | Modules | Commands decoded | Immediate destination sequence | Immediate story writes |
| --- | ---: | ---: | --- | --- |
| M0045I | 10 | 379 | M0043I | none |
| M0046I | 6 | 533 | M0043I, M0000I, M0039I | DA, 216 |
| M0047I | 10 | 1142 | M0043I, M0041I, M0043I | B8, E0 |
| M0051I | 8 | 593 | M0043I, M0055I, M0106I, M0106I, M0052I, M0048I, M0055I | none |
| M0056I | 5 | 188 | M0043I, M0113I, M0115I | none |

Decoded counts measure instruction boundaries, not completed decompilation.
Nine scripts total65 modules,6471 commands,33 immediate transfers and10
immediate story writes. The four new story assignments are unproven as Day2
progression; their presence is why the shared branches require classification.

## Closed routing conditions

M0043I module0 at801BE9B4 dispatches its already-delivered local4 value:

| local4 | First scene boundary | Eventual immediate destination in that arm |
| ---: | --- | --- |
| 1 | 801BE9DC | M0042I at801BEADC |
| 0 | 801BEB10 | M0046I at801BEC14 |
| 4 | 801BEC48 | M0045I at801BED4C |
| 5 | 801BED80 | M0051I at801BEDBC |
| 6 | 801BEDF0 | M0047I at801BEF28 |
| 7 | 801BEF5C | M0056I at801BEF98 |

Other tested values reach801BEFA4, where additional interactions begin.
The destination column is static inspection beyond each boundary, not
execution of those intervening scene commands. Within the M0047I arm,
801BEEBC selects the task fork at801BEEE4 only when story==DA; otherwise
it reaches the delay at801BEEF0. Neither task fork nor delay is executed here.

M0041I module8 at801E3488 permits its first geometry query only when signed
story<=A4. After geometry has separately reported entry,801E356C selects the
reminder send at801E3604 exactly when98<=signed story<A4 and persist45 bit10
is clear. Otherwise it reaches801E3640. The A4-specific send at801E3558 is
outside this latter region, as is the subsequent persist45 update.

M0051I module0 at801BE6E8 writes persist1=51. The following selector chooses
M0052I at801BE720 when signed story<188, else M0048I at801BE738. Both transfer
instructions are checked against their packed names, but not executed.

## M0046I world-map mask preparation

The closed block801AAB20..801AB4F8 updates persist3 before the later M0000I
transfer at801AB548. It always ORs C000. Increasing signed story thresholds
also OR the following cumulative masks; no unrelated bits are cleared:

| Threshold | Mask ORed |
| --- | --- |
| B8 | 1C000 |
| E0 | 3C000 |
| 148 | 7C000 |
| 1C0 | FC000 |
| 218 | 3FC000 |
| 258 | 7FC000 |

Persist0 mask1 additionally ORs800000. Mask2000 has
ordered set/clear rules: set at78, clear at90, set atD0, clear at148, set at178,
clear at1B8, set at1C0, clear at218, set at258. Below78 its prior value remains.
Thus story90 throughCF clears2000; the later threshold rules must not be
mistaken for the initial station's behavior. Which map option each mask
controls still requires the original M0000I consumer trace.

## Verification and next work

The tool checks all story values0..300 plus signed extremes for the41/43/51
predicates, all selector bytes plus100/FFFFFFFF for the43 door dispatch, and
all threshold neighbors plus signed extremes for46's mask with both all-zero
and all-one masks and four persist0 profiles. All5958 original-handler cases passed (258 door dispatch,772 stair selector,
772 exit fork,772 trigger gate,3088 reminder,296 world-map mask). Python
compilation and whitespace checks passed; all103 jobs finished. The assertions compare explicit decompiled conditions with
original instruction execution, not a Python replacement for the handlers.

Next follow M0046I's prepared persist3 into M0000I's original selector and
classify the resulting Day2 destinations. Also bind the station trigger
geometry and message delivery to these predicates, and trace the asynchronous
scene arms. Full Day1/Day2 inventory, terminal transition and live acceptance
remain unfinished. Stage103 changes research tooling/documentation only;
stage102's native implementation and build remain the latest runtime work.
