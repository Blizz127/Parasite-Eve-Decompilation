# Shared M0367I park transition

Stage112 pins M0367I script SHA-256
`dc0d5b09ccc503f24ed981f17c6a1cb2329fdc4723e6b1b0a5a0ef4fe7a5676e`,
script base801AA2FC. The field chunk occupies78 sectors starting disc1LBA87312.
It is a shared transition script; its branches are not all classified asDay2.

The scene selector chooses26,5E,CD,11B and29E arms. The actor selector has
special entries for26,5E,CD and29E. Final assignments in those five scenes
write27,5F,CE,11C and29F respectively. `pe_day2_shared_transition_routes.py`
executes the selectors over all stories0..300 and signed extremes, and checks
the final assignments separately. Intervening non-CD scenes are not executed.

M0239I writes CD at801DA808 before transferring to367. Its separate
pre-transition gate at801DA03C checks story!=CE, not a signed threshold.
The first draft of the research expectation misread ALU0E; original execution
rejected it atCE. The corrected audit uses original inequality semantics.
The transfer producer is checked as a final assignment independently of the
preceding dialogue and effects. M0367I's CD arm later writesCE at801AA938 and
queues M0239I, whose CE gate was verified in stage110.

`pe_m0367i_transition_oracle.py` executes a larger original component from
801AA8AC through exit publication at801AA954. This begins after the initial
EA200 music load. It runs actual script RGB fade87/9C, delay30, real dialogue94,
37870 rendering and confirmation, the EA206 call, final fade, story write and
31 destination publication. Music channel34 is explicitly absent in the
fixture, so EA206 takes its no-channel path. Music playback/loading is not
proved. Nor does publishing a destination load or verify the next room.

Four fixtures vary the starting packet bank and confirmation delay0/3. At
every simulated frame they reset the ordering tables, supply confirmation
when the open message has reached state2 for the chosen delay, run the real
VM, draw text, and tick the fade. All622 original/native frame checkpoints
match: selected actor/task, script, text, fade, packet and destination state.
The test ends with storyCE and tokenM0239I. Actor/task allocation and update
order are explicit inputs, not a complete host game-loop/scene-construction
acceptance test. GPU submission and audible output remain outside its scope.

Native fixtures: retail_m0367i_transition_cases.h and test_m0367i_transition.h.
Original text stays disc-backed. Original component evidence is in
local/live/m0367i-transition-112.json; closed-route evidence is in
local/live/day2-shared-transition-routes-112.json. No runtime change was needed
for this component. FullDay1/Day2 coverage and release acceptance remain open.

Validation:2322 original closed-route cases pass. Four component runs match
622 original frame checkpoints in normal and ASan/UBSan tests; focused runs
pass1group with1303skipped. Full CTest passes8/8 in81.43s. Original fixture
regeneration, Python and whitespace checks pass; builds warning-free.
