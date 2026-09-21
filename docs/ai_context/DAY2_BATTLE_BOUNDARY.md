# Battle entry and script polygon boundary (2026-09-12)

Two native omissions were identified while extending the connected route
through m0005i's encounter. Matching `src/` and manifest inputs are unchanged.

- `80029810` called only the HP prefix of `800293F4`. Original `80029910`
  calls the complete function with argument zero, resetting battle commands,
  statuses and input flags. The native initializer now calls `func_800293F4(0)`.
- Original `8001A9F8` tests `8009D2E8 & 4` and calls `8001D170` before the
  ordinary actor/mesh loop. The native loop omitted that call. The port now
  implements `1D170 -> 1CE88/1CBA0`, including scaled actor radius, closed
  polygon contact, endpoint checks, tangent projection, strict clearance
  search and the second-contact rollback. It reuses the existing integer
  normalizer through a host-vector interface; no floating-point substitute.

## Failure and connected-route evidence

The prior null write was not a missing Aya at battle entry. A temporary guest
RAM tracer identified `8003F074_dest_ready_cut -> 80034FC4` clearing Aya while
loading **m0009i**, token `A80004C8`. Battle bit 2 and the active command queue
survived the transfer. m0009i's initial type-1 task waits one frame before
spawning Aya, so `800299CC -> 80021DE0` wrote through actor zero on that frame.

Before transfer, the script had published six boundary points at `801B3F04`
through `8009D2F8`, count 6 at `8009D264`, and enabled input flag 4. Omitting
the polygon pass let the held direction carry Aya through the battle edge
into the door trigger. The initialization correction alone did not fix this.
With polygon collision, the same connected input reaches frame 11000 without
the transfer or crash, still in m0005i with Aya present and battle active.

Input-only continuation: `8990:FFDF,9140:FFEF,9600:FF7F`, plus the existing
periodic Cross pulse. m0005i is entered at frame **9300**, and its central
rectangle starts story `28` at frame **9766**. At frame **11000**, module 6
is at **801B284C**, token **A80002C8**, persist[1]=4. Captured Aya position is
`04C0AB80/FBA90000/0038AC80`; input=4, battle flags=48C2, Aya=800BF210.

The route harness now defaults to 11000 frames and that continuation, checks
16 observed milestones, and requires the module-6 PC, Aya, battle bit and
six-point boundary. This remains **Day 1, battle in progress**. Movies and
the opening menu still use the four documented HOST_ADAPTED skips.

## Reproducible verification

```
python3 pc_port/tools/pe_battle_entry_oracle.py --check
python3 pc_port/tools/pe_polygon_boundary_oracle.py --check
source /tmp/pe-tools/env.sh
cmake --build pc_port/build -j6
ctest --test-dir pc_port/build --output-on-failure
cmp build/extracted/disc1/SLUS_006.62 build/disc1.candidate.exe
sha1sum build/extracted/disc1/SLUS_006.62 build/disc1.candidate.exe
```

The battle-entry oracle executes **72 complete original graphs**, with
encounter, inherited flags, active-state and weapon-category variation.
Native `BENTRY_retail_initialization` compares guest RAM effects. An additional
local probe on naturally captured pre-entry RAM matched original/native
guest words outside original code and stack after the initialization fix.

The polygon oracle executes **960 complete original graphs**, with synthetic
axis-aligned, diagonal and triangular polygons, an empty list, different
positions, radii, scales and enable states. Native `POLY_retail_script_boundary`
compares RAM effects. The normalizer and square-root table values are generated
independently with integer square root and checked against the local EXE.
No original instruction bytes, room data or RAM snapshots are emitted in the
golden headers. These checks do not claim final GTE/stack byte equivalence.
For an empty polygon, the native contact function omits the original's two
unused endpoint reads before returning -1.

Both original-oracle checks and focused native tests pass. The existing retail
candidate is still exact: SHA-1 **452fb033f2eaa4b18aa20a5bca60b8125af3a37b**.
No new matching-C or whole-game fidelity claim is made.

Full CTest: **10/10 PASS**, native **1378/1378 PASS**, zero skips (223.73s
with concurrent diagnostic work). That run used the preceding 9600-frame
route assertion. After extending the harness, its replacement 11000-frame
route CTest separately passed in **96.98s**. No production changes followed
these checks.

Local logs: `/tmp/pe-polygon-{oracle,focused,full-build,ctest,route-ctest}.log`,
`/tmp/pe-entry-focused.log`. The exploratory release log
`/tmp/pe-polygon-route.log` completed 11000 frames but exited 1 because it
still asserted the older module-4 frontier; it motivated the updated check.
Local RAM diagnostics remain under `/tmp`, outside the repository.

## Next work

Continue the encounter with real controller inputs and compare newly reached
battle behavior with original execution. The module-6 loop at `801B284C`
reads the type-2 actor and its stat via opcodes 5E/8B; its local[2] at frame
11000 is `000F4265`. Battle completion and the next room remain unproved.
The separately identified `800355C8` overlay-bit-200 omission was subsequently
restored and checked in [DAY2_FRAME_GATE.md](DAY2_FRAME_GATE.md).
