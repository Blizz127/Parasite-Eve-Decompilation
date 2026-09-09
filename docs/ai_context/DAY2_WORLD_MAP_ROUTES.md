# Day 2 world-map routing evidence

Stage104 follows the station's persist3 preparation into original M0000I
record initialization and exit selection. Reproduce with
`python3 pc_port/tools/pe_day2_world_map_routes.py`. Detailed output is ignored
at `local/live/day2-world-map-routes-104.json`; original assets stay local.

The tool pins the executable, transition overlay, nine station-neighbor
scripts and two newly followed scripts. It executes original M0046I's closed
mask block, passes its output directly into96620..96EAC (the constructor's
record writer), then executes92030 up to the first SDK call at74D28 for each
selection. This verifies data composition across code windows, not loading,
keyboard navigation, rendering or live scene traversal. The constructor's
preceding resource setup and exit's later SDK/media calls are not executed.

## Record flags and destinations

The constructor writes the following persist3 bits to byte36 of each52-byte
map record. The ten selection numbers correspond directly to exit92030.

| Selection | Persist3 bit | Mask |
| ---: | ---: | --- |
| 0 | 22 | 400000 |
| 1 | 20 | 100000 |
| 2 | 18 | 40000 |
| 3 | 21 | 200000 |
| 4 | 19 | 80000 |
| 5 | 23 | 800000 |
| 6 | 14 | 4000 |
| 7 | 15 | 8000 |
| 8 | 17 | 20000 |
| 9 | 16 | 10000 |

Starting with persist3=0 and persist0 mask1 clear, the composed original
station preparation and enabled-record exit results are:

| Story | Enabled selection -> destination |
| --- | --- |
| 90,98,A4,A6,B7 | 6 -> M0024I; 7 -> M0046I |
| B8 | 6 -> M0024I; 7 -> M0046I; 9 -> M0038I |
| B9,C0,C8,CF | 6 -> M0024I; 7 -> M0046I; 9 -> M0191I |
| D0 | 6 -> M0024I; 7 -> M0037I; 9 -> M0192I |

These are sampled story values with explicit prior-state assumptions. The
script ORs many availability bits, so previously set bits can retain additional
options. The tool also tests prior maskFFFFFFFF and persist0 mask1 set. That
mask enables selection5/M0290I; its availability must not be discarded from
optional-content scope without proving the flag's story provenance.

Mask2000 is not one of these ten record-enable bits. It affects presentation
and alternate destinations. AtD0 the station sets it, changing slot9 from
M0191I toM0192I, while slot7 has a story-specific M0037I override. This does
not by itself establish Day2's terminal scene.

## M0038I intermediate scene

Script SHA256845e6c3cf89ef3d77b9a3664cb201699b3d5d5e392b96a738f332e59ece9d77d,
base801C8ECC:5 modules/651 decoded commands,2 immediate transfers toM0000I,
2 immediate story writes(C0 and160). Shared later content remains separate.

Original module2 gate801C9538 selects the first delay at801C9560 only for
storyB8; all other tested stories reach801CA444, the later scene gate.
The scene's intervening asynchronous commands remain unverified.

The closed final block801C9A40..801CA438 writes storyC0 and persist1=999.
Its persist3 result is `(prior | 1C000 | (persist0 & 1 ? 800000 : 0)) & ~2000`.
The stop instruction at801CA438 is the immediate M0000I transfer. Passing
that final story/mask to original92030 with selection9 selectsM0191I. Thus
this is a verified endpoint connection, not a claim that the entire scene
between entry and exit has run.

## M0191I next frontier

Script SHA2562af377c4e4ab143122bd4f20998a3ccc894be387376adf95ecd8287da7a1c499,
base801B4BD0:10 modules/1634 decoded commands,9 immediate transfers and5
immediate story writes. Static destinations areM0000I(seven sites),M0239I,
andM0195I. Static story writes areC8,168,170,178,D0.

Inspection locates a C0/persist34 gate before the C8 write at801B560C and a
shared168/other selector before178/D0 at801B5A58. Those conditions, their
producers and preceding scene execution remain to be audited; none of the
later values is automatically assigned to Day2. Follow M0195I and the
M0037I/M0192I branches to identify the original Day2 terminal transition.

## Verification

PASS108 station/constructor compositions with1080 exit-selector prefixes,
772 M0038I story gates and8 scene-final-block-to-selector compositions.
Python compilation and whitespace checks pass. Stage104 changes research
and documentation only. Stage102 remains the latest native build/regression
result; no runtime regression rerun was needed for these additions. Full
Day1/Day2 decompilation and live acceptance remain unfinished.
