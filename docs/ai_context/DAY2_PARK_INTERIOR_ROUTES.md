# Park interiors beyond M0061I

Stage129 follows the previously unexamined 61→63/65/358 branches through
seven original Disc1 scripts. Reproduce with
`python3 pc_port/tools/pe_day2_park_interior_routes.py`. Script SHA256 pins
are in the tool; the executable SHA1 is checked by the shared loader.
Ignored JSON records relocated addresses, script/chunk hashes, module counts,
transfers and original-handler traces for the closed gates.

| Script | Immediate destinations, in decoded order |
| --- | --- |
| M0062I | 63 |
| M0063I | 61, 61, 64, 62, 358 |
| M0064I | 63 |
| M0065I | 61, 358, 66, 61 |
| M0066I | 67, 65 |
| M0067I | 68, 68, 358, 66 |
| M0358I | 61, 67, 63, 65 |

These 21 static transfers expand the adjacency inventory. None of these
seven decoded scripts directly assigns persist decimal index74 (main story).
This does not prove that called scene/native code cannot change it. Shared
room membership in Day2 cannot be inferred from adjacency alone. The decoder
also traverses trailing data in some modules: decoded command-boundary counts
are inventory measurements, not executable-code or semantic coverage counts.

Original ALU12850 and branch1731C executions establish three closed gates:

- M0065I module1 at801A090C: signed story>=F0 reaches EA200 at801A0934,
  music directory ID29; story<F0 reaches801A0960, music ID9.
- M0358I module1 at801A973C: the same signed predicate chooses music ID29
  at801A9764 or ID9 at801A9790.
- M0358I module6 at801AD324: signed story>=110 reaches the jump801AD34C;
  story<110 reaches the jump801AD358. Execution stops before these jumps;
  the subsequent actor/battle setup has not been accepted.

The original branch jumps when its condition is zero; this was checked by
execution after an initial probe exposed the inverse assumption. Fixtures
cover all stories0..300 inclusive and7FFFFFFF/80000000/FFFFFFFF for each gate,
with original script pointer banks and preserved story values. Music loading,
scene actions and battle scheduling are explicit stops, not skipped operations
claimed as executed. Native ALU/branch implementations were already restored;
this stage makes no runtime edits or new runtime acceptance claim.

M0067I module0 contains two scene-controlled transfers to68 at801A1A44 and
801A1B00. The first follows movement, dialogue, fade/wait and previous-room67
at801A1A34. Those asynchronous predecessors remain unexecuted. Continue with
M0068I and its callees, tracing progression writes and the actual day boundary,
while retaining optional62/64 routes and the unverified interior battle paths.
Full Day1/Day2 decompilation and native port acceptance remain open.
