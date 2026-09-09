# Additional station and park routes

Stage110 adds pinned decoded inventories for M0059I, M0060I, M0061I, M0230I,
M0239I and M0040I. These six scripts contain37 modules and8585 commands.
`pc_port/tools/pe_day2_extended_routes.py` records script/chunk hashes, static
transfers, story writes, opcode counts, immediate EA/ED keys and battle-command
addresses. Adjacency alone does not classify a room or every shared branch as
Day2 content. The decoded original text remains under ignored local/live.

The M0374I exit reaches59, which has a return selector and an onward60 exit.
Original59 return selector at8019FED0 writes previous-room59, then chooses374
when signed story<120, otherwise58. Another exit points to60. Script59 also contains nine battle-related
opcodes; these are inventoried rather than executed. Script60 points
to61 and59. Script61 points to63,65,358 and60 and contains a battle protocol
around801A2A5C..801A2B18. Battle initialization, scheduling and completion on
that route remain unexecuted. These are static edges plus one tested return
selector, not proof that the entire chain is playable or crosses a day boundary.

The park230 script initializes persist34 to16 unconditionally at8019DEA8.
Its actor gate uses `(counter==16 && signed story<D0)` for the first type2
creation; the alternative type2 placement requires `(counter==17 && signed
story>=D0)`. Other actors follow the selector. Its later message gate uses the
same two predicates for payload1 or payload3 to type2; otherwise it re-enables
control. All gates are tested independently of intervening scene behavior.
A scene-controlled230 exit goes to40; its previous-room value is191, despite
being emitted by230. Other230 exits lead to195 and239.

Script239 has a storyCE scene gate and later story170/shared content. The
CE scene's final closed block at801D41B0 writes counter17 and storyD0, then
prepares map flags. Its map exit writes previous-room191. Script40's storyD0
gate starts another scene; its final block writes storyD8 and prepares the
same mask. Its map exit writes previous-room999. For either tested final
block the result is prior flags OR1E000, plus800000 when persist0 mask1 is set.
Both music starts between mask preparation and final previous-room assignment
are explicit execution boundaries. Dialogue, motion, effects and preceding
waits are not bypassed and claimed executed.

The audit tests all stories0..300 plus7FFFFFFF/80000000/FFFFFFFF against
59/239/40 gates, and the230 selectors against six counter values. Separate
counter initialization and final-block checks cover counter writes and map
mask preservation. These original-handler executions use the pinned scripts
and existing signed ALU semantics; they do not replace native scene acceptance.
No native runtime changes are made in this stage. Validation results and
remaining scope are in ACTIVE_HANDOFF.md.

Next route frontiers include61→63/65/358, the real61 battle, and239's scene
producer/consumer chain. The source of storyCE and the full scene/day boundary
classification remain to be traced. FullDay1/Day2, live gameplay, packages and
release acceptance are unfinished.

Validation:11604 original closed paths pass; Python compilation and whitespace
checks pass. No native sources changed, so no new native build/test run was
needed. Stage109 remains the latest native regression verification.
