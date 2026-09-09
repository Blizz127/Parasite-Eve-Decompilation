# Park passage and side-branch progression

Stages130/131 extend the pinned original-script inventory beyond M0067I.
Reproduce with `python3 pc_port/tools/pe_day2_park_passage_routes.py` and
`python3 pc_port/tools/pe_day2_park_branch_routes.py`. Both tools verify the
executable SHA1 and script SHA256 before original-handler execution. Their
JSON evidence is under `local/live/day2-park-{passage,branch}-routes-*.json`.

| Script | Immediate destinations in decoded order |
| --- | --- |
| M0068I | 72, 67, 74 |
| M0072I | 69, 57, 69, 57, 70, 68 |
| M0074I | 68, 76, 73, 75 |
| M0069I | 71, 72, 72 |
| M0070I | 72, 72 |
| M0073I | 68, 74, 76 |
| M0075I | 74 |
| M0076I | 74, 77, 78, 79 |

These are26 static transfers across eight scripts, not proof of Day2
membership or live traversal. Decoder output can include trailing data;
command counts are not decompilation completion percentages.

Stage130 executes closed predicates and writes in10819 cases. Story means
persist decimal index74; flag word means persist decimal index54.
M0068I's music gate uses signed story>=F0. Its module2 scene gates use the
strict interval F0<story<F8. Its module1 follow-up requires story>=F8 and
flag40000 clear; module3 at801A5F88 sets that flag without clearing others.
M0072I module1 requires F0<=story<F8 and flag100 clear before publishing a
component request at801A0250. Module2 at801A042C sets flag100 and writesF4
before the first scene operation801A0464; another write at801A06A4 also setsF4.
Its module3 destination selectors write previous-room72, then fork on signed
story<120. The music/scene gate at801A0CBC uses F0<=story<F8.

Stage131 passes6176 original-handler cases across five additional scripts
and their closed story/music paths.
M0069I module0 writesF0 at801C66F8 after fade/wait and command82; the check
starts at the write and stops before placement801C6708. M0070I module3 writes
F8 at801C798C and801C7AC0 before music/fade/wait operations and eventual
return to72. These unconditional writes are checked with varied incoming
story values, but the asynchronous predecessors have not been executed.

M0069I module1 music gates, in source order, are:

- 801C6BE0: signed story>=F8 selects music29 at801C6C08.
- 801C6C28: F0<signed story<F8 selects music18 at801C6C80.
- 801C6CA0: signed story<F0 selects music9 at801C6CC8.

ExactlyF0 selects none of these three music calls. An initial oracle run
caught an inclusive-F0 assumption; original operation9 is strict greater-than.
M0070I selects music18 for signed story>=F0, otherwise9. M0075I selects29
for signed story>=F0, otherwise9. Calls are explicit stop points: these tests
verify the predicates, not audio loading or playback. Story fixtures cover
0..300 hex inclusive and7FFFFFFF/80000000/FFFFFFFF.

The F0/F4/F8 writes are local progression facts, not yet a connected scene
proof. M0069I's transfer71 and M0076I's77/78/79 branches remain the next
unexamined destinations. Scene scheduling, movement, dialogue, battles,
optional interactions and the real Day2 terminal transition remain open.
No native runtime behavior changes in these two audits; release128 remains
the latest documented full native regression result.
