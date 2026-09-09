# Controller status-message and field/menu join

Stage118 restores the missing conditional calls in the shared battle/field
controller join, original2AA24..2AA80. Native2A7F8_join_cut now invokes the
timed status-message path when D1CE is nonzero and modeD28C is zero, retains
the existing D244 HUD call, and invokes67CBC when signed-half D2A4 is nonzero.
The last leaf updates the original BCF88 field/menu flags.

The new status helper translates complete34DE0..34F10. State1 clears message
records, selects the original position from5BCB0 and CE80, opens message0
in mode2, installs D1F8 as its text pointer and sets its original flag bit.
It starts timer75, advances state to2, then decrements the timer. State2 with
timer0 clears messages and state; the unconditional decrement wraps timer to
255 on that call. Other nonzero state bytes also take the original timer and
border drawing path. The helper translates5E894's two drawing-origin stores
and calls existing61C34(320,20,0,0) for the complete border packet graph.

The original stack-local -1 list passed to375E0 uses native temporary guest
address80122388. This temporary is excluded from persistent-memory comparison;
the oracle does not claim whole-RAM equality with the original stack layout.

`pe_controller_join_oracle.py` pins executable SHA1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. It executes original2AA24 until
2AA80, excluding only the parent epilogue that restores the caller's stack.
All called status-message and border routines execute fully. The192 cases
cover state0/1/2/255, timer0/1/75, both CE80 location branches and5BCB0 results,
mode0/7, and zero/positive/negative-half field/menu guards. D244 is explicitly
zero; the already existing HUD branch is outside this oracle's coverage.

The shared menu-window fixture supplies packet arenas and drawing tables.
Compared ranges include message records, controller globals, drawing state,
field/menu flags and generated packets. These are individual join calls,
not an entire scene or live rendered-message acceptance run. Numeric fixture
headers contain seeds/fingerprints; original assets remain local.

The restoration applies to the shared runtime used by both days. It does not
close the remaining full dispatcher, scene, combat, audio, BIOS or release
requirements.

Existing escape-failure, enemy-item and steal-item producers already set
D1CE/D1F8 in native code. The restored consumer is reached from both the
mode-switch and idle damage-entry paths. Producer-to-render integration and
the full timed lifetime remain to be verified.

Validation:192 original comparisons pass in normal and ASan/UBSan tests
(one group passed,1309 skipped focused). Full CTest passes8/8 in84.51s.
Native app rebuilt without warnings; fixture regeneration, Python and
whitespace checks pass.
