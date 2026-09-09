# Day 2 station script: verified closed predicates

DAY1/DAY2-81. M0042I script SHA256
70aefef84bb2c9b4ba1e8ba50726aac06fc7b4d81e2e9214034dedfbd4e1e4fd,
relocated base801C5224. Module1 begins801C5FEC; module2 begins801C67B0.
`pe_day2_station_paths.py` extracts the original disc package afresh, pins the
executable and script, and executes closed regions through original handlers
17294/1731C/12850/173F4. Unknown or asynchronous commands are boundaries, never
skipped. Argument pointer banks are an explicit fixture; mode4 uses original
scratch bankB6A80, matching171DC..17200. This is script decompilation evidence,
not full actor/task/scheduler binding or a native scene playthrough.

All persist/scratch indices below are decimal. `story` is persist74, `flags` is
persist0, and `completed` is bit8 of persist44. ALU comparisonA is signed less
than, B is equality, operation1 is wrapping subtraction,2 OR,3 AND,7 logical NOT.
Opcode5 branches when its predicate is zero. Branch destinations are relative
to the module base in halfwords, as executed by the original handler.

## Producer and initial placement

Module1 at625C tests signed story<A6h. Its true path stops at6284 before six
opcode08 actor commands; false proceeds to the later-story block6370. After
those actor commands, the closed632C..6364 region sets scratch22=1 only if
story==88h, then jumps6654. On other stories it preserves scratch22. This
establishes a producer of module2's gate counter; execution of the intervening
actor commands and scheduling remains required to prove the live handshake.

Module2's6894 placement selector has four outcomes:

| Condition, in priority order | Next original command |
| --- | --- |
|story==88h|68BC (opcode0B placement sequence)|
|signed story<A6h|692C (different opcode0B placement sequence)|
|story==B8h|699C (third opcode0B placement sequence)|
|otherwise|69E4 (opcode02 wait)|

These branches are not Day2 membership rules. The shared room also contains
later-story content. No unexecuted placement/animation effect is inferred.

## Module2 entry gate

The closed region beginning69F0 has this behavior:

```text
if story != 0x88: go to 0x801C7114
if scratch[22] == 0: stop before wait at 0x801C6A40
scratch[22] = scratch[22] - 1       // wraps in 32 bits
if persist[44] & 8: go to 0x801C7114
if persist[0] & 2: go to finalize at 0x801C70BC
if persist[0] & 4: stop before sequence at 0x801C6B1C
otherwise: stop before scene sequence at 0x801C6BB4
```

The counter is tested for nonzero, not positivity. Negative bit patterns are
also decremented. A completed scene still consumes the nonzero counter before
leaving this gate. The gate does not itself modify either persist flag word.

After the6B1C sequence, opcode43 at6B34 supplies component5. The closed predicate
at6B40 sends component5==0 to6BB4. Any nonzero component5 sets persist0 bit2
at6B98 and jumps70BC. The original asynchronous sequence that obtains component5
is not executed by this audit; it does not fabricate an interaction response.

## Scene synchronization counters

Each following closed region waits when its scratch word is zero. For every
nonzero word it decrements once modulo32 bits and reaches the continuation.
The wait command and continuation's scene command are both explicit boundaries.

| Predicate address | Scratch index | Wait address | Continuation |
| --- | --- | --- | --- |
|801C6C50|20|801C6C78|801C6CB8|
|801C6CF4|19|801C6D1C|801C6D5C|
|801C6D88|19|801C6DB0|801C6DF0|
|801C6E30|18|801C6E58|801C6E98|
|801C6EC4|18|801C6EEC|801C6F2C|
|801C6F40|20|801C6F68|801C6FA8|
|801C7004|18|801C702C|801C706C|

Decoded producer writes elsewhere in this same script include scratch20 in
module0 at55B4/5684, scratch19 in module3 at7600/7670, and scratch18 in module4
at7B04/7B98/7BFC/7C60. These addresses locate the next cross-module tracing work;
their scheduling and preceding effects are not proved by the closed-counter
checks. They must not be replaced by unconditional ready signals.

## Completion and limits

The final closed block70BC..70E4 performs persist44 |=8 and story=90h, preserving
other persist44 bits. It then reaches opcode85(30) at70F4, opcode9C at7100,
and the source-declared M0041I transfer at7108. This audit stops before85: it
proves the assignments and decoded downstream request, not fade completion or
an actual map transition. The branches into70BC prove a route can omit the
6BB4 scene sequence after the appropriate flags/component value; they do not
prove which player interactions establish those inputs.

The audit covers story boundaries, zero/positive/negative counters, all low
three bits of persist0, completion-bit preservation, component5 outcomes and
all seven synchronization gates. Raw traces/inputs are ignored in
`local/live/day2-station-paths.json`. Full module0 initialization, asynchronous
module1 setup, module2 scene commands, module3/4 producers, other station modules,
room expansion and Day2's ending remain to be decompiled and verified. Existing
Day1 runtime/card/BIOS/live/release work also remains unfinished.

Validation:2696 original-handler paths passed, including signed story extrema
and neighboring equality boundaries. A second complete run reproduced the JSON
trace artifact exactly. Python compilation and whitespace checks passed. No
runtime C changed, so no new native-build or full-suite pass is claimed here.

## DAY1/DAY2-82: mailbox receivers and signal-producing sequences

`pe_day2_station_mailboxes.py` extracts the pinned script and executes original
opcode1F/ALU/branch handlers for receiver selection. Opcode1F177AC copies the
current task's+14h payload word into component4. It does not acknowledge a queue
entry. Receiver predicates compare that full word, not a locally truncated byte.
The separate original opcode1C send audit verifies byte payload/id, halfword
destination type, halfword actor serial promoted into the sender word, zero extra,
and byte queue-count increment/wrap atA3180+12*count.

Module0 poll5494 selects: payload5→54C8;6→548C task-end command;
7→5588;8→55EC;2→5658;3→56BC;254→5708;255→5768;0→57C0;
all other tested values reach58D0, where additional source code remains outside
this closed gate. Module3 poll7594 selects6→75C8,2→7638,10→76A8,
otherwise7830. Module4 poll7A84 selects6→7AB8,7→7B3C,3→7BD0,
4→7C34,otherwise7C70. These endpoints are the first unexecuted scene/task command.

The following straight-line source continuations end in a constant-ready store.
The audit verifies every intervening opcode and its arguments, then executes the
assignment separately. It does not simulate completion of intervening commands.
Addresses in this table share prefix801C.

| Receiver module/payload | Sequence start | Intervening opcode sequence | Ready store |
| --- | --- | --- | --- |
|0 /7|5588|2E,4E,2F,30|55B4: scratch20=1|
|0 /2|5658|2E,4E,2F,30|5684: scratch20=1|
|3 /6|75C8|02,2E,4B,2E|7600: scratch19=1|
|3 /2|7638|2E,4E,2F,30,2E|7670: scratch19=1|
|4 /6|7AB8|2E,4B,2E,4E,2F,30|7B04: scratch18=1|
|4 /7|7B3C|5D,2E,B8,44,76,2E|7B98: scratch18=1|
|4 /3|7BD0|2E,4E,2F,30|7BFC: scratch18=1|
|4 /4|7C34|2E,4E,2F,30|7C60: scratch18=1|

Module2's source sends include `(type,id,payload)`:
6BB4=(0,0,5),6C04=(3,0,6),6C24=(0,0,2),6D5C=(3,0,2),
6DF0=(4,0,6),6E04=(0,0,3),6E98=(4,0,3),6F2C=(0,0,7),
6FC0=(4,0,4),706C=(0,0,8). Their queue records are verified through original
17764→653B8 execution. These sends align with the observed receiver/counter
structure, but actual actor type/module initialization, queue delivery, task
scheduling and sequence completion still require execution evidence.

In particular,6F2C sends payload7 to type0. Module0's7 branch produces scratch20;
module4's7 branch would produce scratch18. The freshly extracted script contains
no immediate opcode1C(4,0,7). That is a scoped direct-send fact, not a proof that
module4's branch is globally unreachable. Do not connect it to6F2C or fabricate
its signal based only on matching payload numbers.

Shared placement selectors also have original-handler evidence: both modules
select a special first placement for story88, otherwise a second for signed
story<A6. Module3's third placement requires storyDA; module4's requiresB8.
Other values reach their wait commands. Later-story dialogue and interaction
branches remain outside this closed audit.

Validation covers815 receiver/placement paths,8 signal assignments and160
original sends. Payload tests include all256 byte values and separate32-bit
edge fixtures; the latter test the poll handler, not a claim that normal queue
delivery supplies wide payloads. The previous2696 station paths are rerun because
the shared fixture gained an explicit task/payload binding. Raw evidence remains
ignored in `local/live/day2-station-mailboxes.json`.

Stage82 validation: prior2696 paths passed; the new trace/sequence/packet JSON
reproduced exactly on a second run. Python compilation and whitespace checks
passed. No runtime C changed and no new native-suite pass is claimed.

## DAY1/DAY2-83: installed entries and original queue delivery

`pe_day2_station_delivery.py` executes all ten opcode14 commands from the freshly
extracted station script through17588. Code2 writes actor+19Ch (mailbox entry),
code1 writes actor+1A0h, both as actor's module base plus twice the relative word.
The installed pairs are:

| Module | Mailbox entry | Other installed entry |
| --- | --- | --- |
|0|801C5494|801C5D88|
|2|801C711C|801C7348|
|3|801C7594|801C7838|
|4|801C7A84|801C7C78|
|5|801C7F78|801C8034|

Modules1/6/7 contain no opcode14 commands. In the audit their initial handler
fields are zero; this does not prove that arbitrary live actor setup leaves
them zero. Module2's mailbox entry711C is distinct from its initial67B0 scene
script. The receiver addresses audited in stage82 are thus actual installed
entries, rather than guessed points within those modules.

The audit supplies an explicit actor list with type equal to module index and
id0, plus a linked free-task pool. It executes original17764 sends using the
actual script argument addresses, then65400 queue drain and177AC payload poll.
All27 immediate station sends are included in full-batch cases; other cases use
only module2's sends. Variants change recipient IDs, disable module3's handler,
or supply a second type4 recipient with module4's entry. These are controlled
actor-list fixtures, not execution of actor construction or the scene scheduler.

For each matching recipient, the original code allocates a free task with the
installed entry PC, flags4, sender serial, payload, delay1, and a wrapping task
serial. Tasks prepend to actor+A8h, preserving forward/backward links; multiple
matching recipients each receive a task. Type/id mismatch or a zero handler
receives none. Drain clears the queue count while retaining records. Poll
returns the delivered payload. Task+18h/+1Ch remain the seeded pool values;
this graph does not invent contents for later opcode24 interaction reads.

Validation:32 original install/send/drain/poll graphs produced376 verified tasks.
Every task's entry, links, flags, serial, sender, delay and polled payload are
checked, including serial wrap fromFFFEh. Python compilation, whitespace checks
and exact JSON artifact reproduction passed. Raw evidence is ignored in
`local/live/day2-station-delivery.json`. No runtime C changed.

This establishes queue delivery once the stated actor/module/list binding is
present. Actual35038 actor construction, module initialization before14, task
scheduling, and asynchronous scene-command completion remain required to prove
station progression. It does not establish a native Day2 scene playthrough or
complete either day's inventory, implementation, live or release acceptance.

## DAY1/DAY2-84: original relocation and actor-to-module binding

`pe_day2_station_constructors.py` executes12574 on the freshly extracted M0042I
script. All eight relocated entry words equal the original module starts.
It then supplies that table throughB161C, executes original1266C task-pool setup,
and runs35038 for actor types1..7. This replaces the assumed type-to-module
entry mapping with original constructor execution for those types. Type chooses
the table word at+8+4*type; the descriptor's ID byte is copied independently.

The112 cases vary ID, parent/no-parent, an existing next actor, packet-building
mode, and model-resource presence. They verify the free-actor pop, list links,
actor serial wrapping/storage, active-count wrapping, module pointer+9Ch,
initial12700 task entry/delay/serial, and initial zero mailbox/other handler slots.
For56 model-free cases, the constructor returns with flags100000E0h. Only after
that return does the audit execute the module's opcode14 installs in isolation.
Modules1/6/7 have no such installs in their script; modules2..5 install the
entries listed in stage83. Pre-install initialization commands are not skipped
and claimed executed: they remain outside this isolated installation check.

For56 model-bearing cases, the audit supplies a small object header and zero
command pointers, and stops at the original362B8 allocation call. With three
faces, two entries counted by byte2 and one by byte3, the original requests180
bytes when building packets or76 otherwise. The actor's model pointer, initial
task and module binding already exist at that boundary. Allocation, geometry,
model commands and later constructor effects are not synthesized or declared
complete. This distinguishes a proved constructor prefix from the returning
model-free path, rather than treating real actors as permanently model-free.

Validation:112 original relocation/constructor/initial-task paths passed. The
JSON artifact reproduced exactly; Python compilation and whitespace checks
passed. Raw evidence is ignored in `local/live/day2-station-constructors.json`.
No runtime C changed, so no new native-suite result is claimed.

Remaining: actual publication of the table by the scene loader, type0's2F76C
resource path, real model/command resources, complete pre-install initialization,
and scheduled execution of the station scripts. The stage83 delivery graph is
still an explicit fixture; it has not yet been joined to an original/native
end-to-end station scene. Both days' broader decompilation and Day1 implementation,
live/freeze/BIOS/release requirements remain unfinished.

## DAY1/DAY2-85: type0 initialization and native relative-offset fix

The original type0 constructor also returns with model resources absent, even
when resource offsetsA8038/A803C are zero. The previous native2F76C guard treated
these relative offsets as null pointers and omitted5218C/51980/51E64. Source2F76C
contains no such conditional. Removed that guard so all three original stat,
weapon and armor initialization calls run.

Expanded original/native equipment comparisons exposed a second issue: when
capacity/selection yields a zero weapon lookup, original51980 still dereferences
low guest RAM. The canonical-only port API aborted at address7. Weapon record
access now maps physical low RAM to canonical RAM, preserving the original
access instead of skipping initialization. The change is local to51980; armor's
original null-result branch is preserved. Low-RAM contents in this audit are
zero fixtures, not a claim about an actual BIOS exception-vector image.

`pe_equipment_offsets_oracle.py` executes132 original2F76C graphs (33 equipment
cases across all four zero/nonzero relative-offset combinations). The existing
EQP1 native comparison now uses generated expectations for those132 cases.
Normal and sanitizer focused checks pass1group,1279skipped. The constructor
oracle now includes eight model-free type0 paths, bringing its total to120;
56 model-bearing type1..7 paths still stop at allocation. Exact generated-header
verification and constructor artifact reproduction passed, as did Python
compilation and scoped whitespace checks. Both final builds were warning-free.

This corrects the earlier assumption that a zero offset means these calls lack
resource authority. Actual scene resource publication and model continuation
remain separate work. Type0's model-free initial task/module binding is now
original-code evidence; its real model setup, pre14 commands, scheduling and
complete native station passage remain unfinished.

Stage85 full CTest passed8/8 in74.62s. All audit/build/test sessions finished.

## DAY1/DAY2-86: original VM animation waits and signal timing

`pe_day2_station_waits.py` runs original17018 itself, including argument binding,
handler dispatch, task delay decrement, saved script PC and task-list advancement.
It uses the actual2F/30/0A commands at five station sequence tails, rather than
calling a rewritten scheduler or binding each opcode manually.

Opcode2F/17B34 clamps its unsigned-halfword argument to actor+0Fh (one byte),
writes the target halfword atactor+12h and sets actor flags200h. Opcode30/17B74
always writes task delay1 and returns0. It compares actor+16h withactor+12h:
if unequal it rewinds the script PC by8; if equal it leaves the PC advanced.
It does not return1 on equality. Consequently, even an already-satisfied wait
ends the current VM pass before the following scratch assignment. A current
frame above the target still waits: the condition is equality, not >=.
The low16 fractional bits of actor+14h do not affect this comparison.

| Module | Target setter | Wait | Subsequent signal |
| --- | --- | --- | --- |
|0|801C55A0|801C55AC|801C55B4: scratch20=1|
|0|801C5670|801C567C|801C5684: scratch20=1|
|4|801C7AF0|801C7AFC|801C7B04: scratch18=1|
|4|801C7BE8|801C7BF4|801C7BFC: scratch18=1|
|4|801C7C4C|801C7C58|801C7C60: scratch18=1|

Each graph starts with task delay1 at the original target-setter PC. The first
VM pass executes the setter and wait, and leaves the scratch signal zero.
When initially unequal, the fixture supplies an equal frame for a second pass;
that pass advances to the signal address but still leaves the signal zero.
Only the later pass performs the assignment through original mode4 binding.
For the first four tails the audit then stops before the following ALU handler;
it does not fabricate that predicate or subsequent asynchronous commands. The
last tail executes opcode20, sets task flag10h and saves PC801C7C78.

Validation:300 original VM graphs cover five sites, five frame limits, six
current integer frames and both zero/FFFFh fractional parts. They verify the
clamped target, flag200h, PC rewind/advance, delay1, deferred signal write, mode4
binding and the task-end case. Exact artifact reproduction, Python compilation
and whitespace checks passed. Raw snapshots are ignored in
`local/live/day2-station-waits.json`.

The frame change between passes is an explicit input, not execution of animation
advancement. These graphs establish VM-pass semantics, not elapsed real frames,
rendering, camera/movement completion, outer scheduling or a full station scene.
The native17B34/17B74 implementations already express these conditions; no C
change or new native-suite pass is claimed for this checkpoint.

## DAY1/DAY2-87: actual animation advancement and arithmetic repair

Original `1A4AC` (`asm/disc1/AB74.s`, 117 words) exposes four differences in
native ticking: a missing strict target-crossing clamp before wrap; quotient
instead of the original HI/remainder on forward wrap; missing `cur=cap<<16`
on reverse wrap; and strict rather than inclusive comparisons of the old
frame against the target in the final clamp. Restored these in
`pc_port/game/boot/func_80029810_port.c`. For example, an unlatched nine-frame
clip advancing from frame8 by one frame now wraps to0, matching the original,
instead of1. Arithmetic retains unsigned 32-bit addition and signed comparisons.
The original really does guard null actor at entry.

New `pe_animation_tick_oracle.py` executes the complete original ticker with
no attachment, a non-Aya actor and zero additional sound records:3360 cases,
covering five limits, four targets, fractional/extreme current values, seven
speeds and three flag combinations. Generated native expectations compare
current frame, previous frame and flags. The new native group fails against
the old implementation and passes after repair; normal and sanitizer focused
runs each pass1 group with1280 skipped. Original sound routine6A318 actually
executes in these fixtures, but has no eligible sound records.

`pe_day2_station_waits.py --advance` now interleaves the original1A4AC ticker
and original17018 VM at all five previously audited station tails.300 cases
pass using speed0x30000; frame state is no longer assigned to the target in
this mode. Each unequal wait repeats until the ticker reaches the requested
frame, then the ready counter remains0 until the following VM pass. Both the
older supplied-frame artifact and new advancement artifact reproduce exactly.
This proves explicit ticker/VM interleaving, not real outer-scheduler ordering,
render timing, resource loading or live scene completion.

Correction to the old BTL66 native comment: original6A318 is UNCONDITIONAL,
not restricted to actors with a nonnull18C attachment. It processes sound
records before previous/current-frame stores. That native call remains omitted;
linked animation flag200000 also still omits1A784 plus copying parent14.
These are concrete next frontiers, not covered by the arithmetic comparisons.
Artifacts/logs: `local/live/day2-station-animation-waits.json`,
`oracle-day1-87-animation*.log`, `oracle-day2-87-animation-waits.log`,
`build*-day1-87*.log`, `test*-day1-87*.log`, `ctest-day1-87.log`.

Final verification: full CTest PASS8/8; Total Test time (real) =  75.52 sec.
Original header reproduction, Python compilation and whitespace checks PASS.
All stage87 processes finished.

## DAY1/DAY2-88: animation sound events restored

Restored original `6A318..6A5BC` in `func_80029810_port.c` and its unconditional
call from1A4AC, before flags and previous-frame stores. The scan emits positional
sounds through existing6DCE4/6DED4 projection and FIFO code. Four initial records
apply only to Aya outside battle bit2 and without B0CD8 bit800000; following
records also match actor type and ID. All records match animation command.
Forward intervals include the previous frame and exclude the current frame;
reverse intervals include previous and exclude current with reversed comparison.
The endpoint is adjusted by cap+1 when frame order indicates wrap. Zero speed
emits nothing. The live B0CEA byte selects a sound halfword, zero IDs are skipped,
and actor coordinates are signed high halves of the position words. The scan
runs even when100/200 would make the following ticker return early.

New `pe_animation_sound_oracle.py` executes512 complete original graphs through
sound projection and FIFO publication, half direct6A318 and half integrated
1A4AC. Cases cover both directions, wrap, zero speed, extreme frame values,
Aya/non-Aya and both global gates, actor/command mismatches, two sound variants,
zero sound IDs, additional-record counts0/1/8/255, queue seeds and paused/latched
flags. Tests compare all selected actor, table, camera, package and FIFO memory;
normal and sanitizer each PASS1group with1281 skipped (total1282 groups).
Original header reproduction and Python/whitespace checks PASS; final builds
have no warnings. This proves queue production, not audible synthesis or live
scene/audio timing. The fixture uses a synthetic valid sound package/camera.

Remaining adjacent frontier:1A4AC bit200000 still omits1A784 and copying parent14.
Then join actual actor resources, scheduler and station script paths. Both full
Day1 and Day2 scope, live/random-freeze/BIOS/card/release acceptance remain open.
Logs: `local/live/oracle-day1-88-animation-sound*.log`, `build*-day1-88.log`,
`test*-day1-88-sound.log`, `ctest-day1-88.log`.

Final full CTest PASS8/8; Total Test time (real) =  84.68 sec. All stage88 processes finished.

## DAY1/DAY2-89: linked animation synchronization and restart repair

Restored1A4AC's flag200000 branch: select the parent's command through1A784,
reload the18C parent pointer and copy its14 frame. Recursive parent ticking
still precedes the local sound scan and frame stores; an already-ticked parent
is skipped via800000. The100 pause and200 target-equality returns still precede
linked synchronization, as in the original.

Fixed1A784's command-change path to perform only its local reset/resource bind.
Calling1A680 there was incorrect because that routine recursively restarts all
linked descendants, including those already using the requested command.
Original1A784 lets each descendant check its own command before resetting.
The existing1A680 unconditional restart semantics remain intact.

New `pe_linked_animation_oracle.py`:576 complete original graphs,192 each for
1A680,1A784 and1A4AC. Six-actor fixtures cover a three-level chain, linked sibling,
unrelated actor and attached actor without animation-sharing flags; matching/
changed commands, paused/latched states, pre-ticked root, fractional frame/speed,
zero resource frame count (byte underflow to255) and 16-bit command masking.
All selected actor/list/command-table/resource bytes are compared. Old native
code fails case196 (changed parent, already-matching descendant); repaired normal
and sanitizer builds each PASS1group,1282 skipped (total1283). Original header
reproduction, Python compilation and whitespace checks PASS; builds warning-free.
These are synthetic valid resource/list fixtures, not live attachment creation,
model rendering or full scene acceptance.

Original dispatch table910A0 maps C5->19170, CC->19260, D4->19F04,
D5->19FE0, D6->1A064. These attachment/detachment commands are still unported.
An original-script inventory of the four initial Day2/shared packages41/42/43/351
finds zero occurrences of those five opcodes (`local/live/day2-linked-command-inventory.json`).
This does not establish absence across all Day2 or Day1 scripts. Other attachment
writers include15200, which needs its own caller/command trace. Next audit actual
attachment creation and extend scene/room coverage; complete Day1/Day2 and all
Day1 live/BIOS/card/freeze/release acceptance remain unfinished.

Logs: `local/live/oracle-day1-89-linked*.log`, `build*-day1-89*.log`,
`test*-day1-89*.log`, `ctest-day1-89.log`.

Final full CTest PASS8/8; Total Test time (real) =  85.97 sec. All stage89 processes finished.

## DAY1/DAY2-90: script attachment commands and source-site inventory

Native VM now dispatches C5/19170, CC/19260, D4/19F04, D5/19FE0 and D6/1A064.
C5 finds Aya or a live type/ID match, applies original3E0A4 model link/kind/joint
stores, then sets actor18C; CC applies3E0D0 model unlink/kind stores and clears18C.
D4 sets parent18C and animation-sharing flags. D6 clears the parent's100000 and
all matching children's18C/600000. D5 preserves its unusual original order:
clear current18C first, scan against that now-zero pointer, and if no other
parentless actor exists clear100000 at physical-RAM98. Physical descriptor/null
reads and that tail are mapped locally to canonical RAM, not globally enabled.
Missing parent returns1 without modifying the actor; type/ID requests compare
full words against actor bytes rather than truncating the requests.

`pe_attachment_vm_oracle.py` runs640 complete original17018 VM graphs: all five
commands followed by20/task termination, with real mode0 binding. Covers absent
Aya/list/target, type257 vs byte1, retired candidates, two model kinds, null
model descriptors, signed joint bits, parentless peers and the low-RAM D5 tail.
Visited-PC assertions prove all handlers, both model helpers and original1A04C
low-RAM access execute. Native and sanitizer memory comparisons each PASS1group,
1283 skipped (total1284). Header reproduction, Python and whitespace PASS;
warning-free builds. This is synthetic model/list VM execution, not live rendering.

New `pe_attachment_script_inventory.py` extracts the21 inspected Day1-route and
initial Day2/shared packages directly from disc (EXE SHA1 pinned; per-script
hashes recorded). One attachment opcode site: M0034I module4, raw script offset
40CC, opcodeA4. None of C5/CC/D4/D5/D6 occur in that inspected set. The inventory
is reproducible but is not a complete day-membership/reachability proof.
A4 maps to15108 and remains unported: source calls3DF50(model,parentmodel,joint),
3A6A8(model,B89F8), sets18C, copies signed actor254/256/258 into fixed-point28/2C/30,
and sets2000. That actual M0034I site is the next attachment frontier.

Artifacts/logs: `local/live/attachment-script-inventory.json`,
`oracle-day1-90-attachment*.log`, `build*-day1-90.log`, `test*-day1-90.log`,
`ctest-day1-90.log`. Complete Day1/Day2, live/random-freeze/BIOS/card/resize/
packages/release acceptance remain unfinished.

Final full CTest PASS8/8; Total Test time (real) =  82.89 sec. All stage90 processes finished.

## DAY1/DAY2-91: M0034I anchor attachment and transform repair

Restored opcodeA4/15108 in the native VM, using original3DF50's anchor binding
and3A6A8->3E188 transform. M0034I module4 starts with exactlyA4(3,0,49) at raw
script40CC; package script SHA256dc224a516c3d0eaaf16d87b03bc1dc622ebca19ef419a6a2d51c8dfe8daa3603.
The native command finds the requested parent, clears the local object pointer,
stores parent model/joint/record offsets, transforms, sets18C and copies signed
world coordinates to fixed-point position, then sets2000. Missing parent yields1.

This exposed a larger native3E188 omission: world coordinates must use the
parent's selected16-byte point record, not the local offsets. Original also
composes view and joint matrices and projects two anchors (64 and5C), temporarily
adding70 to local2C for the second projection. Restored those operations through
a view-aware internal helper. Existing one-argument3E188 wrapper usesB89F8;
3A6A8 passes its actual view. Scratch matrices remain native temporary storage.

New pe_anchor_attachment_oracle640 original fullVM graphs include real M0034I
command bytes, Aya/live/self/missing targets, type259 mismatch, joint-1/0/1/49,
rotated/translated matrices, signed/overflowing point offsets and nonzero camera
projection. Native and sanitizer each PASS1group,1284 skipped (total1285).
Continuation is synthetic20, not M0034I's following logic. Parent records and
joint matrices are synthetic; actual resource publication/rendering remains open.

Initial comparison failed. gdb isolated an apparent low-RAM difference caused by
the oracle's old scratchpad aliasing: it masked1F800000 to0. Added optional
`scratchpad=bytearray(0x400)` to the original runner's instruction load/store
path; this audit uses separate scratch memory. Default behavior is unchanged
for earlier audits; BIOS helpers and instruction fetch are not extended by this
option. The revised tests retain low-RAM canaries and use nonzero view state.
Native transform omissions above were independently identified from original
3E188 source. Header reproduction and the existing640 attachment-VM regression
PASS, Python/whitespace PASS; warning-free final builds. Old91 logs record the
initial failure; final logs use `-fixed`.

Next: actual M0034I model/resource construction for parent type3, joint49, and
module4's following predicates/actions. Module3 begins9B(35,130,11), CA(3),
9D(58982), ED(2601,96,0...), then battle/animation setup. Module4 next copies
persistent slot80 to scratch10, tests it through ALU/branch, then random/action
selection. No complete scene/day acceptance claim. All Day1/Day2 and Day1 live/
freeze/BIOS/card/resize/package/release requirements remain open.
Logs `local/live/oracle-day1-91-*.log`, `build*-day1-91-fixed.log`,
`test*-day1-91-fixed.log`, `gdb-day1-91.log`, `ctest-day1-91.log`.

Final full CTest PASS8/8; Total Test time (real) =  83.61 sec. All stage91 processes finished.

## DAY1/DAY2-92: real M0034I resources through attachment

New `pe_m0034i_anchor_resources.py` executes original resource publication
6B7B0..6B898 over M0034I's actual chunk, complete3D050 model initialization,
1A680 command selection,3D834 pose prefix through the boundary before3B97C
lighting, and the real module4 VM prefix through the boundary before its first
12850 ALU. No replacement model, clip, point or attachment-command bytes.
Memory placement, actor list/task publication and view are supplied fixtures;
this does not execute the outer scene loader or actor constructor.

Chunk SHA2560eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a,
204800 bytes, header31D0C. At supplied base8018EFE8, model3 is80197250
(relative8268), script base801B4B00. The model declares52 joints; joint49's
actual point is[0,-10,-174,245]. Original model initialization resolves points
to8019B20C and matrices to80160084 without packets or8016FC24 with packets
(storage supplied at80160000). Published clips are2,4,5,6,7,8,9,11,12,13,15,16,17,18.
The original initializes from the first available clip2, not nonexistent clip0.

84 slices (14clips x start/middle/final x two packet modes) PASS. There are35
distinct joint49 poses; paired packet/no-packet slices agree on pose, world
position and projected anchors. Actual module4 starts at801B8BCC with A4(3,0,49),
then executes the following assignment from persistent80 to scratch10. The audit
stops before ALU12850, with CE00 atscript+4108; no later branch/random-action
execution claimed. World positions and projected anchors are recorded in ignored
`local/live/m0034i-anchor-resources.json`; original bytes stay ignored.

Packet-mode continuation beyond pose reaches GTE NCCT command0118043F, unsupported
by the original runner. The audit therefore explicitly stops before lighting;
no primitive rendering or audible/live scene proof. Initial probes and one
incorrect expected CE00 offset failed; final assertions correct that offset and
retain the packet-lighting limitation. No native C changes this stage; no new
native-suite claim. Next compare native real-resource model/pose/attachment,
resolve lighting oracle coverage and trace remaining module4 actions/scene entry.
Both full days and Day1 live/freeze/BIOS/card/resize/package/release remain open.

Exact artifact reproduction and final strengthened assertions PASS; Python and
whitespace checks PASS. All stage92 processes finished. Native code unchanged.

## DAY1/DAY2-93: NCCT arithmetic and complete model packet updates

Added original-runner NCCT0118043F support: three normal vectors, light-matrix
stage, background/color-matrix stage, RGBC multiplication, intermediate signed
44-bit wrapping, signed32-bit post-shift MAC, nonnegative IR saturation, color
FIFO and accumulated command flags. Arithmetic authority is
[psx-spx GTE color commands](https://psx-spx.consoledev.net/geometrytransformationenginegte/#gte-color-calculation-commands)
and its matrix-operation/flag definitions. This is a documented hardware model,
not captured console output; original instruction at3BBDC is pinned by EXE SHA1.

New pe_ncct_oracle512cases cover ordinary and extreme matrices/vectors/background
values. Native comparison checks three RGB FIFO words plus final MAC/IR components.
Old native fails case256: background accumulation did not wrap before saturation,
producing a different color. Fixed pe_gte_mul3 (used only byNCCT) to wrap44 after
each sum and saturate IR from signed32-bit MAC. Normal/sanitizer each PASS1group,
1285skipped(total1286). Native NCCT still does not expose/update the original
FLAG register; the comparison claims only the nine tested data outputs.

`pe_m0034i_anchor_resources.py --lighting` now runs complete original3D834,
including3B97C and both3BCE0 packet-buffer updates, before the real module4 VM
prefix. All84cases PASS with39 distinct nonempty packet hashes. Original actual
model/clip/point bytes are retained; light matrix/color matrix/primary color are
explicit fixtures. Packet/no-packet pose/world/attachment results still agree.
Artifact `local/live/m0034i-anchor-lighting.json`. No GPU submission, rendered
image, real lighting configuration, outer constructor or complete scene proof.
This supersedes stage92's NCCT runner boundary, not its other scope limitations.

Next compare native complete real-resource graph to these packet expectations,
then trace remaining M0034I module4 predicates/random actions and real scene
entry. Full Day1/Day2 and Day1 live/freeze/BIOS/card/resize/packages/release remain
unfinished. Logs `oracle-day1-93-*.log`, `build*-day1-93*.log`,
`test*-day1-93*.log`, `ctest-day1-93.log` underlocal/live.

Final full CTest PASS8/8; Total Test time (real) = 118.11 sec. Both resource artifacts
reproduce exactly; NCCT header reproduction, Python and whitespace checks PASS.
Final builds warning-free. All stage93 processes finished.

## DAY1/DAY2-94: native real-resource model and packet comparison

Added a disc-backed native comparison `test_DAY1_m0034i_model`, using the actual
EXE and100-sector M0034I chunk directly from the configured original disc.
`pe_m0034i_anchor_resources.py --native-header` generates84 pairs of original
initialization/pose hashes; `--check-native-header` verifies reproduction without
rewriting the header. No original model/clip data is checked into the test header.

Native comparison composes the existing3D050 initialization pieces and calls
complete3D834 for all14 clips at start/middle/final frames, with/without packet
allocation. Hashes cover the full actor/model record, storage/packet/joint region,
original resource chunk (including original object header mutations), saved
matrix and packet-buffer selector. The initial test passes all84cases; no new
runtime fix was required. Command-table publication is supplied from the actual
directory, not passed through a broader loader/relocation shortcut.

The test does not run the full actor constructor, texture adjustment, real scene
lighting setup, attachment VM (separately covered in91-93), GPU submission or
actual screen rendering. Native composition matches the original model setup
with texture adjustment disabled. The configured-disc test skips when no disc
fixture is available; it executed here (1group PASS,1286skipped,total1287).
The log name `test-day1-94-before.log` records the first successful comparison,
not a failure or a pre-fix state. No native game/platform C changed this stage.

Next trace remaining M0034I module4 persistent80/scratch10 predicates and random
selection/actions, plus real actor construction/scene entry. Whole Day1/Day2 and
Day1 live/freeze/BIOS/card/resize/packages/release acceptance remain unfinished.
Logs `oracle-day1-94-model*.log`, `build-day1-94-before.log`,
`build-asan-day1-94.log`, `test-day1-94-before.log`, `test-asan-day1-94.log`,
`ctest-day1-94.log` inlocal/live.

Final verification: normal and ASan/UBSan focused comparison PASS1group each
(all84cases,1286skipped groups). Full CTest PASS8/8 in80.06s. Original header
reproduction, Python compilation and whitespace checks PASS; builds warning-free.
All stage94 processes finished.

## DAY1/DAY2-95: M0034I module4 battle-record setup

The actual script begins with the joint49 attachment audited in91-94. The new
`pe_m0034i_setup_oracle.py` executes original full17018 from script40E0 through
its first one-frame yield at4510. It supplies an already-attached type4 actor,
module base, task, battle-record occupancy and explicit two-draw RNG state.
Original script SHA remains dc224a516c3d0eaaf16d87b03bc1dc622ebca19ef419a6a2d51c8dfe8daa3603.
No original script bytes are checked into the generated expectation header.

Source behavior (offsets relative to original script base801B4B00):

- 40E0 copies persist80 into scratch10; zero becomes65536 at4118.
- 4128 draws integer[0,100) into local4. Values0..9 select50;10..99 select34.
  The branch at4174 compares the already-overwritten34 against39, so the
  assignment7 at419C is unreachable from this entry. It is not a third outcome.
- 41C8 draws integer[0,100) into local5;0..49 select49,50..99 select33.
- 4230 allocates the first free one of seven battle records. Attached flag2000
  prevents restarting animation. A full table leaves the previous actor record
  pointer unchanged; subsequent script setters still operate on that record.
- Tag42=1; tags44/45/60 receive signed arithmetic-shift16 of the low32-bit
  product scratch10*80; tag61 receives the corresponding product with6,
  truncated to a halfword. Tags92/93 receive local4/local5.
- Tags69=3,100..106=1,107=2,108=1,110=10; tags50..55 receive
  1240,1239,1241,1386,1386,1387 respectively.
- At4478/4488 the getters for tags44/60 clamp negative values to zero. The
  script adds1000000, then writes tags44/45 from the first result and60 from
  the second. It yields one frame and leaves the task PC at451C, before the
  type5/id0/payload112 send and task end.

Cases cover both sides of random thresholds, zero/default and signed/wrapping
persistent inputs, first/middle/last free records, full-table behavior and serial
byte wrap. The original VM executes RNG, allocation, setters/getters and delay;
none are replaced by expected-value shortcuts. All672cases match native results.
The first UBSan run failed on signed multiplication305419896*80 in ALU subop0F.
Changed func_80012850_port.c to unsigned multiplication, preserving original
low32-bit output without C signed-overflow undefined behavior. Fixed normal and
ASan/UBSan comparisons PASS1group each,1287skipped(total1288). Original header
reproduction and Python/whitespace checks PASS; builds warning-free. Full CTest
PASS8/8 in131.41s; all stage95 processes finished. This does not prove real scene
construction, next-frame message
delivery, GPU presentation or whole-Day1/Day2 acceptance.

Logs oracle-day1-95*.log, build*-day1-95*.log, test*-day1-95*.log and
ctest-day1-95.log underlocal/live; audit artifact m0034i-setup.json. Next trace
451C message delivery to type5 and its original recipient path, then remaining
M0034I actor construction and scene entry.

Next-path source inventory: module5 starts453C and opcode14 at459C/45AC installs
mailbox4DEC (relative1112 halfwords). Payload112 matches the comparison at4EB0;
4ED8..4F24 waits while scratch0 bit16 is clear, then reaches opcodeAC at4F30
when set (polarity corrected by original execution in96).
These recipient commands are decoded source inventory only, not executed by
stage95. The intervening message/task delivery and that recipient continuation
remain the next original/native execution target.


## DAY1/DAY2-96: native M0034I message delivery and battle exit

`pe_m0034i_delivery_oracle.py` executes the actual451C send and task end through
original17018, then full65400/12700 delivery into module5's actual installed
mailbox4DEC. Full recipient17018 consumes payload112, handles zero or two waits,
resumes when the supplied scratch0 bit16 becomes set, and executes through56D0
end. The VM advances its current-task pointer, so each fixture frame republishes
the mailbox task before execution. The old95 decoded-only wait polarity was
incorrect and has been corrected above.

The recipient installs AC/E3 lists at script5724/5744 with four entries each,
sets D2E8 bit4, reads Aya tag6 and skips its calculation when that value is9000.
Other cases run the actual level/stat comparisons, scratch averages, fixed-point
arithmetic and ranged RNG before storing the result in scratch3 and Aya tag6.
It sets scratch0 bit2, clears B0CD8 bit2000 and D2E8 bit1, calls33A2C to set
D244=1, requests mode0 and ends the task at nextPC56D8. These are script requests,
not proof that the battle controller completes the requested transition.

352 cases vary all11 outcomes of the script's [0,11) draw, sixteen stat/scratch
profiles (including signed level boundaries and the9000 bypass), and immediate
or two-frame waiting. They compare sender, delivery, each wait and final state:
actor/stat records, mailbox linked-list canaries, free pool and serial wrap,
queued packet, globals, scratch/conditions, RNG and task PC/flags.
Actor creation, list publication, task pool, Aya stats and external flag change
are supplied fixtures; no real scene/scheduler/playthrough claim.

Native comparison initially fails case0 at final stage. Original setter2FF78
loads D254 into v0 at8002FF80, then loads a2 from0(v0) at8002FF88. The native
setter omitted that second read, writing Aya's actor record rather than her
stats record. Restored the dereference for every tag and local physical RAM
aliases, including the original unconditional low-RAM read for absent Aya on
global-tag commands. Existing direct-leaf and wrapper fixtures were corrected
to publish an actor-to-stats pointer instead of the old incorrect layout.
An additional1024 original cases cover every byte tag and four value patterns,
checking actor/stat canaries and the tag255 global. Fixed normal and ASan/UBSan
PASS1group each (1288skipped,total1289). Header reproduction, Python and
whitespace checks PASS. Test builds and native parasite-eve-port app rebuild
warning-free. Full CTest PASS8/8 in106.04s; all96 processes finished.

Logs oracle-day1-96*.log, build*-day1-96*.log, test*-day1-96*.log and
ctest-day1-96.log underlocal/live; m0034i-delivery.json records original results.
Whole-Day1/Day2 acceptance and live/freeze/BIOS/card/resize/packages/release remain
unfinished. Next trace the actual battle-controller/scratch0 bit16 producer and
its M0034I scene entry, so these script requests connect to native scene behavior.


Next-path source inventory: module5 sends(type0,id0,payload101) at46E4. Module0
matches that payload at418 and executes4D(65536), two0B placement commands and
2E(4), then2A(scratch0,bit4) at488. That is the located script producer of the
bit16 that releases the mailbox wait. These commands are decoded only here;
execute the sender/recipient placement and animation graph next, rather than
assuming the native battle controller sets the flag directly.


## DAY1/DAY2-97: real walkmesh placement produces readiness

`pe_m0034i_ready_oracle.py` executes the actual payload101 sender command at46E4,
full65400 delivery into module0's installed214 mailbox, and the complete Aya
recipient VM. It starts module5's112 mailbox first and verifies that it waits.
Aya's actual placement/animation-selection script then sets bit16 itself;
module5 resumes and runs its9000 bypass through the exit requests/end. There is
no fixture write of the readiness bit between these VM executions.

The original M0034I204800-byte resource chunk supplies both script and walkmesh.
The package directory identifies mesh801BA270; original1A918 rebases/publishes it.
The actual0B placement tests original floor polygon801BA31A. With ordinary flags,
position becomes (-873,1200,11) in16.16 coordinates: the script requestsY1197,
and the mesh snaps it to1200. Actor flag2 preserves the suppliedY; flag80 bypasses
floor lookup. Both behaviors are compared with original execution, alongside
animation-pause100 and target-latch200 flag combinations.

The script sets speed65536, places/rotates Aya, selects animation4 via full1A680,
clears pause100 and sets scratch0 bit16 in the same VM pass. It does NOT wait for
animation playback completion before signaling. The common Aya animation4 is
absent from this room's clip directory; the test supplies its frame-count header
with counts0/1/8/255 to verify command selection and byte-underflow behavior.
This does not assert real common-clip loading, pose production or visible motion.

64 original graphs compare five checkpoints: mesh/setup, controller waiting,
message delivery, Aya completion and controller resumption. Full resource chunk,
actor records, floor links, task queue, readiness/exit globals and mailbox data
are covered. Actor construction/list publication, task scheduling, common clip
header and the controller's previously delivered112 task remain fixtures. The
actual101 send handler is executed, but its surrounding sender VM/subtask-spawn
continuation is outside this comparison. Native normal and ASan/UBSan PASS
1group each,1289skipped(total1290), all64cases. No runtime fix needed. Header
reproduction/Python/whitespace PASS; builds warning-free. Full CTest PASS8/8
in105.91s; all97 processes finished.

Logs oracle-day1-97*.log, build*-day1-97*.log, test*-day1-97*.log and
ctest-day1-97.log underlocal/live; original artifact m0034i-ready.json. Next execute
module5's full46D4 sender VM, including46F8 subtask creation and470C wait, then
connect the remaining native scene construction/entry. Whole Day1/Day2 and
live/freeze/BIOS/card/resize/packages/release acceptance remain unfinished.


## DAY1/DAY2-98: full sender VM and subtask ordering

`pe_m0034i_fork_oracle.py` replaces97's direct101 send-handler call with the
actual complete sender VM beginning46D4. Original40/AA flags,101 send,12 subtask
creation at46F8, and20 end all execute. The child at470C waits for scratch0 bit2;
Aya's101 handler produces bit16, then the112 handler releases its own wait and
sets bit2. The child subsequently advances through its extra one-frame delay
at4764 to4770. Neither shared bit is supplied by a fixture write.

64 cases include sender task flags0/1/2/3, empty or occupied next-task links,
Aya flags and four supplied common-clip frame counts. With (task.flags&3)==0,
131E8 inserts after the current task; original17018 visits the child immediately,
and it yields at4758 in that same VM invocation. Otherwise the child is prepended
to actor+A8, preserving the prior mailbox task and its back-link; it remains at
470C until explicitly traversed later. Spawn and mailbox allocation consume two
separate free nodes and wrap the task serial. The native path matches all six
state checkpoints, including full actor/task/resource/global hashes.

Normal and ASan/UBSan PASS1group each,1290skipped(total1291), all64cases. Header
reproduction and Python compilation PASS. No native runtime fix was needed.
Full CTest PASS8/8 in96.11s; whitespace PASS and builds warning-free. All98
processes finished. Logs oracle-day1-98*.log, build*-day1-98*.log,
test*-day1-98*.log andctest-day1-98.log underlocal/live. Original artifact is
m0034i-fork.json. The actual room mesh and script are retained; actor/root-task
publication, VM traversal calls and common animation frame-count header remain
fixtures. This is not scene construction, actual animation playback or live
battle acceptance.

Next source boundary4770 reads the battle mode via94. Mode10 reaches47B4,
mode9 reaches4880; other modes loop through4DD0 back to4764's one-frame delay.
Those continuations require original/native execution next, along with the
controller that produces modes9/10. Entire Day1/Day2 and live/freeze/BIOS/card/
resize/packages/release acceptance remain unfinished.


## DAY1/DAY2-99: mode continuations and missing native mode4

`pe_m0034i_modes_oracle.py` executes two complete original VM passes from4770
in112 cases: exact modes0/8/9/10/11/10009h/FFFFFFFFh, scratch bits1000h/2000h,
scene-index values0/1/10/255, Aya positions and persistent patterns. Native
normal and ASan/UBSan comparisons pass. Unselected modes remain in the4770 loop.
Mode10 requests its30-frame fade and waits at4818. These mode9 fixtures end at
nextPC4DD0 after queuing117/114; repeated traversal does not execute an ended task.
The comparison includes full script bytes, actor/task state, persistent words,
shared scratch/conditions, queued packets, flags and fade-control state.

The research decoder had a separate bug: parse_script_command read every mode
from the first32-bit header. Original17018 uses that word for arguments0..4 and
switches to the second word for arguments5 onward. Corrected pe_pst0_scan.py;
pe_script_argument_modes_oracle.py verifies20 original multiword-command pointer
bindings in M0034I. Native VM decoding was already correct. An apparent immediate
write by opcode77 was therefore a research-decoder error, not original behavior:
its output is actor local6. Older decoded metadata for commands with more than
five arguments must be regenerated when used. The Day2 static route audit was
rerun successfully; complete Day2 scene coverage remains unproven.

Original mode9 publication is at2B27C (2B0E8). Mode10 is published at2BC78 in
2B94C, selected by mode4 at2A9BC. That entire mode4 branch was missing from the
native dispatcher. Added full2B94C in func_8002AA98_port.c and mode4 dispatch in
func_800299CC_port.c, using original19DE4.s instructions:

- Phase0 clears D244, clears the effect pool, visits all actors, selects record
  animation commands for non-Aya actors, clears velocity, applies original
  retirement rules, starts30-frame model fades and advances the phase.
- Phase1 waits for Aya and auxiliary fade counters, calls293F4(0), sets other
  eligible model flags and advances.
- Phase2 retires finished or flag40 actors, waits for remaining fades, performs
  the original audio/effect cleanup and advances when ready.
- Phase3 waits while6D60C(0)==1, clears battle slots, selects Aya command21,
  executes295E4 cleanup and publishes mode10. Other phase values do nothing.

pe_mode4_cleanup_oracle.py supplies shared sparse fixtures and executes the full
original routine in41 cases, including empty lists, attached actors, retirement
flags, fade gating, music-busy gating and out-of-range phase. Native tests invoke
the actual mode dispatcher with the join inactive (D244 zero) and compare all
original fixture ranges. No callee is replaced by a successful fake return.
The first run aborted because command selection read resource0+2 as a host-invalid
address. Original reads physical RAM there. Added local alias mapping to the
frame-count byte read shared by1A680/1A784, retaining the original published
resource pointer and count-minus-one wrap. Native final41case normal/ASan PASS
1group each (1292skipped,total1293). Busy case remains phase3/mode4. Header
reproduction, decoder20case check, Python and whitespace PASS; builds warning-free.
Full CTest PASS8/8 in80.28s; all99 processes finished.

The native application was rebuilt with these runtime changes. No actual scene
entry, rendered fades, audible cleanup, live mode4 completion or freeze fix is
claimed. Remaining original dispatcher modes5(2F0B0) and7(2D1F0) are also absent
from this native mode switch and need audit/porting. Next connect mode4 entry and
real model/music progression with the script continuations, then restore those
remaining branches. Entire Day1/Day2 and live/BIOS/card/resize/packages/release
acceptance remain unfinished. Logs build*-day1-99*.log, test*-day1-99*.log,
oracle-day1-99*.log, gdb-day1-99-mode4.log andctest-day1-99.log underlocal/live;
M0034I artifact m0034i-modes.json, Day2 audit day2-route-audit-day1-99.json.


## DAY1/DAY2-100: native mode5 cleanup restored

The original dispatcher calls2F0B0 for mode5 at2A9D4. The native mode switch
previously omitted that branch. Restored full2F0B0..2F300 from19DE4.s in
func_8002AA98_port.c and connected it in func_800299CC_port.c.

Phase0 clears D244 and visits actors other than Aya. Eligible actors have a
record or attachment. Record-bearing actors select the signed-byte record+6
command (promoted to unsigned halfword), clear velocity and set flag1000. The
original40000000/recordAF retirement rule is retained. Eligible models start a
30-frame fade and receive model flag2. Phase advances even with an empty list.
Phase1 retires eligible actors whose fade counter is zero or whose actor flag40
is set; any other active fade prevents advancing. When ready it calls703F4 and
advances. Phase2 waits while6D60C(0)==1; otherwise it clears slots, selects Aya21,
runs295E4, publishes mode11 and finally executes293F4(0). Other phases do nothing.
The ordering differs from mode4 and is preserved independently.

pe_mode5_cleanup_oracle.py generates41 full original executions over populated/
empty lists, record/attachment combinations, retirement flags, unfinished fades,
music-busy gating and out-of-range phases. Sparse inputs and expected state
come from the existing original-execution fixture family; native tests call the
actual mode dispatcher with its join inactive. The comparisons cover actors,
records, effects, globals and cleanup state. Native normal and ASan/UBSan PASS
1group each (1293skipped,total1294), all41cases. Original header reproduction,
Python and whitespace PASS; builds warning-free. Native app rebuilt. Full CTest
PASS8/8 in82.25s; all100 processes finished.

No actor construction, actual rendered fade advancement, audible music cleanup,
live battle completion or scene playthrough is claimed. Next restore remaining
mode7/2D1F0 dispatch and connect modes4/5 entry and real progression with scene
scripts. Whole Day1/Day2 and live/freeze/BIOS/card/resize/packages/release acceptance
remain unfinished. Logs build*-day1-100.log, test*-day1-100.log,
oracle-day1-100*.log andctest-day1-100.log underlocal/live.

## DAY1/DAY2-101: mode7 HUD and hit-flash dispatcher restored

Original2D1F0..2DC58 is a666-word handler with four call sites (three32B0C,
one27A08). Restored native implementation in func_8002DC58_port.c and mode7
branch in func_800299CC_mode_switch_cut. AT pulse uses current bank and
D250&3, gated by unsigned16 record10>=9000. Aya record50/58 and enemy recordD0
amounts capture actor210/212 only at timer30. Aya58 sets color1 on that tick.
Each active amount goes through full32B0C and then decrements its byte timer.
Enemy records with6000 bits call27A08; remaining2000 becomes4000. Aya and
recordless actors are skipped. No mode publication occurs inside this handler.

pe_mode7_hud_oracle.py pins the original executable and executes full2D1F0
with existing modeled BIOS/GTE support and complete native callees.240 sparse
fixtures cover2 banks x4 pulse phases x3 AT thresholds x10 amount/list/hit
profiles. Output hashes include actors, stats, effect pools, battle globals,
AT/gauge packets, floating-digit packets and ordering tables. Native helper
calls the actual dispatcher with join inactive and requires no stop/stub.
Normal and ASan/UBSan PASS240 comparisons (1group,1294skipped,total1295).
Native app rebuilt; header reproduction/Python/whitespace PASS, warning-free.
Full CTest result is recorded in ACTIVE_HANDOFF and local/live/ctest-day1-101.log.
Logs use day1-101; no live scene, display, resource-loading or full-day claim.

Original2CF28 in mode6 publishes7; native pe_battle_ready_tail has this store,
but native2BC90 remains a partial handler. Its complete body and real entry
progression are the next frontier, followed by Day2 route expansion. The full
Day1/Day2 objective remains active.

## DAY1/DAY2-102: full mode6 readiness and gauge counter fix

Original2BC90..2D1F0 is restored natively; old named-cut entry delegates to it.
The handler cancels queued commands, clears four transient text records, locks
directional input, zeros Aya velocity, clears battle flag4, pulses AT/PE colors,
and draws/decrements Aya record50/58/60 amounts. Any active timer makes this
tick unready, including timer1. Unlike mode7, mode6 never snapshots amount
coordinates on timer30.

Aya command mismatch clears flag100 and prevents readiness. At frame cap,
commands<4 with D298!=0 resume queued D29A: D298>=2 restores flag100, clears
D298, selects the clip and sets current D29C/previous D29C-65536. Otherwise it
selects record12. Eligible enemies outside commands2/3 clear flag1000, release
tasks with36254 and delay readiness; cap selects signed-byte record6, otherwise
speed becomes65536.27D14 runs for eligible enemies including commands2/3.
6914C runs after the actor loop regardless of readiness; only all-ready and
its zero result execute the original mode7/color-reset/input-release tail.

pe_mode6_ready_oracle.py:384 cases (24 bank/pulse/AT combinations x16 profiles)
execute the complete original call graph. Sparse fixtures and full range hashes
cover actors/stats, task chains, effect pools, battle globals, text, gauge and
floating-number packets and ordering tables. Includes three amount slots,
queued Aya states0/1/2/65535, clip cap/wait, enemy command/task settling,
recordless/kind1/empty-list arms. Full native dispatcher with inactive join
matches all384, normal and ASan/UBSan (1group,1295skipped,total1296).

First failurecase33 showed hostD250=0 versus guestD250=1 in shared exit_pulse;
GDB evidence local/live/gdb-day1-102-mode6.log. Fixed its phase read to guest
memory, benefiting both6 and8. Original expectations unchanged. The old
mode6-ready unit fixture gains a required stats pointer now that it executes
the whole handler. Final -fix builds and tests pass, warning-free; native app
rebuilt. Header reproduction/Python/whitespace pass. Full CTest result recorded
in ACTIVE_HANDOFF/local/live/ctest-day1-102.log. No real scene/media/render/full
Day1 or Day2 acceptance claim. Next actual entry/progression and Day2 expansion.
