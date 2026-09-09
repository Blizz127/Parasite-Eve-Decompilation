# Day 2 decompilation: initial route evidence

Current goal includes decompiling **all Day 1 and Day 2**, while implementing
and fixing Day 1. This is an initial static inventory, not complete Day 2
coverage or a native playthrough. Shared rooms contain later-story branches;
none of the untraced branches below is automatically classified as Day 2.

Reproduce with `python3 pc_port/tools/pe_day2_route_audit.py`. The tool reads
the original Disc 1 packages, verifies the original executable SHA1 and four
script SHA256 values, and checks 16 immediate transfers plus six immediate
story assignments. It parses 3,636 commands across 26 modules, but that count
measures decoded instruction boundaries, not semantic decompilation coverage.
Original script bytes/decoded inspection remain ignored under `local/live/`.
The generated `day2-route-audit.json` records chunk LBAs, lengths, hashes,
script relocation bases, module numbers and individual instruction addresses.

Stage103 expands the investigation to five adjacent shared scripts and verifies
closed routing predicates plus world-map mask preparation. See
[DAY2_STATION_ROUTES.md](DAY2_STATION_ROUTES.md). Its nine-script inventory and
5958 original-handler paths supplement this initial audit; neither inventory
establishes full Day2 membership or live traversal.

Stage104 follows the station's output through M0000I's original record writer
and exit selector into M0038I/M0191I; see
[DAY2_WORLD_MAP_ROUTES.md](DAY2_WORLD_MAP_ROUTES.md). The intermediate scene's
gate and final block are verified independently; its asynchronous middle and
the full Day2 terminal transition remain unresolved.

Stage105 verifies M0191I's secondary progression conditions and M0037I's
return-scene endpoints, and pins M0192I/M0195I. See
[DAY2_PARK_PROGRESSION.md](DAY2_PARK_PROGRESSION.md). The asynchronous scene
middle, park interior and terminal transition still need work.

Stage106 follows return-station storyD8->DA and separate DA/E0/E4 scene
endpoints, including original map selection intoM0039I andM0374I. See
[DAY2_STATION_RETURN.md](DAY2_STATION_RETURN.md). The Day2 ending remains
unproven; M0374I's progression and park interiors are still outstanding.

## Entry and shared branches

The existing Day 1 audit establishes that the M0000I exit selector with
story g74=0x80 selects M0351I (prefix execution only). M0351I module 1 writes
0x88 at80191114 before its M0042I request at80191124. Its alternative writes
0x140 at80191158 before M0092I at80191168. That alternative is shared later
content. DAY2_ENTRY_DECOMP.md now proves the final selector: signed g74<=80
selects M0042I after clearing persist1; otherwise g74==138 selects M0092I.
The earlier scene conditions and actual transition still need work.

Here `g74` means persist **decimal index74**, encoded as argument0x4A.
It does not mean persist index0x74.

| Source | Immediate destinations, in decoded order | Story assignments (hex) |
| --- | --- | --- |
| M0351I | M0042I, M0092I | 88, 140 |
| M0042I | M0043I, M0041I, M0041I | 90 |
| M0041I | M0042I, M0352I, M0042I, M0051I, M0047I | 218, A6 |
| M0043I | M0042I, M0046I, M0045I, M0051I, M0047I, M0056I | A4 |

M0042I script base is801C5224. Module0 compares g74 with0x88 at801C5324;
module2 repeats that comparison at801C6894 and801C69F0. The module2 sequence
also tests scratch22 and persist44 bit8, followed by persist0 bit2/bit4
branches. Its eventual story assignment writes0x90 at801C70E4 before the
M0041I transfer at801C7108. The closed predicates now have original-handler execution evidence in stage81
below; producer scheduling and asynchronous scene effects remain unresolved.

M0041I's 0x218 branch writes at801E0070 and transfers to M0352I at801E0090.
Its 0xA6 write is at801E1CDC, before M0047I at801E1D40. M0043I writes0xA4
at801C04F4. These shared-story values demonstrate why adjacency alone is
insufficient to define the Day 2 room set or ending.

## Required continuation

1. Finish the missing Day 1 transition update/render/outer graph and verify
   actual M0351I entry/exit behavior.
2. Trace predicates, events and called modules through the Day 2 entry scripts;
   include optional branches while distinguishing later-story content.
3. Expand into the resulting reachable rooms and identify Day 2's original
   terminal transition. Enumerate scene code, scripts, shared callees, enemies,
   items, menus and audiovisual paths needed by both days.
4. Decompile every in-scope script/native graph, recording unresolved behavior
   explicitly. Decoded command counts and transfer checks do not prove this.

## DAY1/DAY2-63: closed entry-path semantics

`pe_day2_entry_paths.py` verifies772 original-handler final-selector cases and96
Day1 calculation cases. A49-command branch graph proves the unconditional
persist311=800 write dominates the calculation exit. See DAY2_ENTRY_DECOMP.md
for the ordered pseudocode, source addresses and explicit fixture boundaries.
The four-room inventory was rerun unchanged:3636 commands,26 modules,16 static
transfers and six immediate story assignments. Counts remain inventory evidence.

DAY1/DAY2-64 restores missing VM D3 conversion and consolidates D2 timer reads
with2048 original-handler comparisons.256 additional entry-script chains now
execute original D2/D3 before the calculation. Native VM dispatch and sanitizer
checks pass; full CTest8/8 and1265 groups. Earlier asynchronous scene gates and
live timer scheduling remain unproven. See DAY2_ENTRY_DECOMP.md.

DAY1/DAY2-65 proves1,050 ordered input/persist1/persist8/story gate paths in
M0351I and256 connected input-to-timer/calculation chains. Opcode11 selector1
polls newly pressed bit100; it is not a mailbox. All six gate exits and signed
persist8 boundaries are checked against original handlers. See
DAY2_ENTRY_DECOMP.md. Async prologue, message/fade behavior and actual transfer
remain open; this does not expand the four-room inventory by itself.

## DAY1/DAY2-81: M0042I station predicates

`DAY2_STATION_DECOMP.md` and `pe_day2_station_paths.py` now establish the closed
module1 producer, module2 entry/placement/component gates, seven counter waits,
and completion assignments through original handlers. The scene gate consumes
scratch22 only atstory88 and when nonzero, before testing persist44 bit8; flag2
of persist0 selects completion, otherwise flag4 selects the component-producing
sequence. Completion sets persist44 bit8 andstory90 before the still-unexecuted
fade/wait/transfer. Module0/1 setup, module3/4 producer scheduling and module2
asynchronous commands remain required to prove actual scene progression.

Stage82 adds original mailbox receiver/placement checks for modules0/3/4 and
source continuations for eight counter producers. SeeDAY2_STATION_DECOMP for
payload/target distinctions and remaining delivery/animation boundaries. This
advances station semantics without expanding the proven full-Day2 room set.

Stage107 follows M0374I actual sender/delivery/receiver VM into the E6 scene
start; original/native comparison and explicit model boundary are recorded in
[DAY2_M0374I_DELIVERY.md](DAY2_M0374I_DELIVERY.md).

Stage108 extends that path through the real model/mesh animation wait and
placement into dialogue62 poll: [DAY2_M0374I_ANIMATION.md](DAY2_M0374I_ANIMATION.md).

Stage109 restores ED3100 readiness and EA217 music reset used by M0374I;
[DAY2_M0374I_READINESS.md](DAY2_M0374I_READINESS.md) records original/native scope.

Stage110 expands six shared scripts and verifies11604 closed route/progression
paths: [DAY2_EXTENDED_ROUTES.md](DAY2_EXTENDED_ROUTES.md).

Stage111 restores239 dialogue-background/scene flags and compares two real
first pages: [DAY2_PARK_DIALOGUE_FLAGS.md](DAY2_PARK_DIALOGUE_FLAGS.md).

Stage112 verifies the shared park transition component through fade, dialogue
confirmation and exit publication: [DAY2_SHARED_PARK_TRANSITION.md](DAY2_SHARED_PARK_TRANSITION.md).

Stage113 restores M0040I EA305/314 audio control:
[DAY2_PARK_AUDIO_CONTROL.md](DAY2_PARK_AUDIO_CONTROL.md).

Stage114 restores the effect color/allocation commands used by M0059I. See
[DAY2_PARK_EFFECT_ALLOCATION.md](DAY2_PARK_EFFECT_ALLOCATION.md). Stage115 restores the nonempty
effect update/draw graph; see [DAY2_PARK_EFFECT_DRAW.md](DAY2_PARK_EFFECT_DRAW.md).
Full scene/GPU acceptance remains open.

Stage116 verifies the actual M0059I repeating effect component through the
VM and allocation/update/draw graph over800 original/native frames. See
[DAY2_M0059I_EFFECT_LOOP.md](DAY2_M0059I_EFFECT_LOOP.md).

Stage117 connects both M0059I battle request/wait sequences to the original
mode6 readiness controller and subsequent task publication. See
[DAY2_M0059I_BATTLE_READINESS.md](DAY2_M0059I_BATTLE_READINESS.md).

Stage118 restores the shared controller status-message and field/menu flag
join paths. See [DAY2_CONTROLLER_STATUS_JOIN.md](DAY2_CONTROLLER_STATUS_JOIN.md).

Stage119 verifies escape-status message production through rendering and
expiry, and fixes skipped extended glyphs in the shared renderer. See
[DAY2_ESCAPE_MESSAGE_LIFETIME.md](DAY2_ESCAPE_MESSAGE_LIFETIME.md).

Stage120 restores icons and FC/FD extended message glyphs, verified across
all operand bytes. See [DAY2_MESSAGE_GLYPH_COMMANDS.md](DAY2_MESSAGE_GLYPH_COMMANDS.md).

Stage121 restores numeric insertion in the shared message renderer and
verifies full setup-to-render graphs. See
[DAY2_MESSAGE_NUMBER_INSERTION.md](DAY2_MESSAGE_NUMBER_INSERTION.md).

Stage122 corrects shared message wait/confirmation cursor rules, verified
across2880 original/native frames. See
[DAY2_MESSAGE_WAIT_CONTROLS.md](DAY2_MESSAGE_WAIT_CONTROLS.md).

Stage123 restores the shared choice-selection cursor packet, verified over
4480 original/native frames. See
[DAY2_MESSAGE_CHOICE_CURSOR.md](DAY2_MESSAGE_CHOICE_CURSOR.md).

Stage124 verifies actual M0040I dialogue1, rendered selection, confirmation,
VM result read and both branches through their next wait in eight runs
with44 checkpoints. See [DAY2_M0040I_CHOICE.md](DAY2_M0040I_CHOICE.md).
This component does not establish scene-wide or Day2 membership acceptance.

Stage125 extends selection1 through fade completion, story D8 and map
publication with cached music14/mode4, and fixes the omitted valid music
payload command. See [DAY2_M0040I_TRANSITION.md](DAY2_M0040I_TRANSITION.md).
CD/SPU mode changes and audible playback remain unfinished.

Stage129 expands the61→63/65/358 interior routes to seven pinned scripts and
verifies2316 original story-gate paths. See
[DAY2_PARK_INTERIOR_ROUTES.md](DAY2_PARK_INTERIOR_ROUTES.md). The next static
frontier is67→68; actual day-boundary and interior scene/battle acceptance
remain open. Stage128 already resolved the opening music-mode regression
mentioned in the historical125 notes above.

Stages130/131 extend eight park scripts and closed progression/music gates:
[DAY2_PARK_PASSAGE_ROUTES.md](DAY2_PARK_PASSAGE_ROUTES.md). The Day2
ending and connected asynchronous scenes remain unproved.

Stage132 extends late park adjacency through89 and inspects candidate ending
endpoints90/91 plus the blocking movie-loader contract:
[DAY2_LATE_PARK_AND_MOVIES.md](DAY2_LATE_PARK_AND_MOVIES.md). The connection
from story120 to these candidate ending scenes remains unproved.

Stage133 ports movie overlay buffer allocation, display backup/restoration and
completion flags with original/native comparisons:
[DAY2_MOVIE_INITIALIZATION.md](DAY2_MOVIE_INITIALIZATION.md). Playback and
14E30 loader integration remain unresolved.

Stage134 ports movie display setup and frame acquisition:
[DAY2_MOVIE_FRAME_ACQUISITION.md](DAY2_MOVIE_FRAME_ACQUISITION.md). The movie
player/updater and actual playback remain unresolved.

## Stage135 movie callback

The124-word1214D4 slice callback now has native buffer/rectangle/display
control and final-slice GPU uploads; stream7C564 and MDEC output10C01C
remain explicit stops.360 original provider graphs include84 native-complete
final-slice cases and276 prefixes ending at a missing callee. See
[callback evidence](DAY2_MOVIE_SLICE_CALLBACK.md). This does not complete a
movie, any scene, or Day1/Day2 acceptance. Player/updater integration remains open.

## Stage136 stream record assembly

7C564 now assembles RAM-backed sector headers/payloads and publishes completed
records, with7B290 status polling and movie callback integration. Physical
FIFO/DMA and MDEC output remain explicit boundaries. See
[stream evidence](DAY2_STREAM_RECORD_ASSEMBLY.md); this does not certify
a movie, scene, or completion of either day.

## Stage137 CD interrupt acknowledgment

7AAB4 now consumes response bytes and publishes original command/data status
through its guest jump table.505 original cases cover501 complete graphs and
4 unresolved-call prefixes. The stream path now runs idle acknowledgment.
See [interrupt evidence](DAY2_CD_INTERRUPT_ACKNOWLEDGMENT.md). Physical disc
and movie completion remain unverified.

## Stage138 data-ready callbacks

CPU IRQ dispatch now recognizes7C13C and executes the data-ready callback
chain into RAM-backed stream assembly.52 original graphs/prefixes compare
through copied sector payloads. Registration and IRQ assertion are explicit
test setup, not a live disc event producer. See
[data-ready evidence](DAY2_CD_DATA_READY_DISPATCH.md). Movie and full-day
acceptance remain open.

## Stage139 queued-command recovery

The data-ready error path now clears pending commands, publishes original
completion records and invokes saved callbacks with the original sequence
deduplication rules.360 recovery graphs/prefixes and18 completion-writer
cases match original execution. See [queue recovery evidence](DAY2_CD_QUEUE_RECOVERY.md).
Initialization, physical disc delivery, movie playback and full-day acceptance
remain open.

## Stage140 command completion

Registered80164 and its three completion state machines now preserve public
command responses, internal startup/retry phases and callback gates.3314
original graphs/prefixes provide comparison evidence; the interrupt test now
executes command completion. See [command completion](DAY2_CD_COMMAND_COMPLETION.md).
Queued completion7E964, initialization and live disc/movie delivery remain open.

## Stage141 queued completion and retries

7E964 and7E5C4 now handle completion, retry counts, grouped removal and
callback ordering.5184 original completion graphs/prefixes and30 removals
provide comparison evidence. See [queued completion](DAY2_CD_QUEUE_COMPLETION.md).
Initialization and live disc/movie delivery remain unfinished.

## Stage142 CD source registration

The CPU registration helper previously rejected source2. Original-worker
evidence now enables CD registration, with270 complete original comparisons.
The52 CD interrupt cases use real registration before delivery. See
[source registration](DAY2_CD_SOURCE_REGISTRATION.md). Automatic startup and
physical disc/movie delivery remain unfinished.

## Stage143 controller initialization

7BBFC's control flow and7FA2C's SDK state setup are translated. Original/native
comparisons cover rejected-command startup and complete state initialization;
successful device-driven startup and outer caller wiring remain unverified.
See [controller initialization](DAY2_CD_CONTROLLER_INITIALIZATION.md).

## Stage144 CD VBlank updater

Translated the startup/read/play state updater, timeout retry and known queue
callback.3360 original graphs/prefixes execute through asynchronous command
issue and compare with native VBlank-slot dispatch. See
[VBlank updater](DAY2_CD_VBLANK_UPDATER.md). Device response production and
automatic startup installation remain unfinished.

## Stage145 audio setup and startup helpers

Translated SPU/CD volume setup, mode setter and VBlank installation wrapper.
Original comparisons cover98 helper graphs; native tests also verify banked
audio-gain application and response isolation. See
[startup helpers](DAY2_CD_STARTUP_HELPERS.md). The outer initializer and live
disc/movie delivery remain incomplete.

## Stage146 low-level initializer

7F994 now connects translated helpers and automatically binds/installs its
VBlank updater. Original comparisons cover initialization and the following
tick through a rejected-command path. Public already-initialized return is
corrected to0. See [low-level initializer](DAY2_CD_LOWLEVEL_INITIALIZER.md).
Successful device responses and public cold-path wiring remain unfinished.

## Stage147 device-driven startup

An explicitly enabled CD command device now drives real response FIFO/CPU IRQ
delivery. Native7F994 and four installed updater ticks reach SDK ready state
using original command tables. See [device responses](DAY2_CD_DEVICE_RESPONSES.md).
Public activation, sector/movie delivery and timing fidelity remain incomplete.

## Stage148 public startup and VBlank delivery

[DAY2_CD_PUBLIC_STARTUP.md](DAY2_CD_PUBLIC_STARTUP.md) connects the public
initializer to the explicitly enabled command device and translates7F960.
Host timing generates VBlank interrupts; the mounted-disc test reaches ready
through the CPU scanner and verifies masking without manual callback dispatch.
Twenty original notification graphs verify null and unknown callback cases.
Default runtime activation, sector delivery and complete movie playback remain
outstanding; this is not opening-through-Day2 acceptance.

## Stage149 mounted-disc sector delivery

[DAY2_CD_SECTOR_DEVICE.md](DAY2_CD_SECTOR_DEVICE.md) adds device Setloc,
SeekL/SeekP, ReadN/ReadS, Pause and consuming sector FIFO. The raw image owner
supplies actual bytes; SDK data-ready notification uses the existing IRQ chain.
DMA3 and the physical stream transfer remain disconnected, so this does not
establish movie playback or complete Day1/Day2 acceptance.

## Stage150 physical stream DMA

[DAY2_CD_DMA_TRANSFER.md](DAY2_CD_DMA_TRANSFER.md) connects sector FIFO bytes
to stream records and payloads through DMA3, translates7CEAC's channel3 path
and dispatches7C214 through the shared DMA IRQ path. Original issuer graphs
cover the idle/request-ready register contract. MDEC output and full movie
player/updater/loader acceptance remain outstanding.

## Stage151 MDEC pixel decoding

[DAY2_MDEC_PIXELS.md](DAY2_MDEC_PIXELS.md) adds run-length expansion, programmable
IDCT and packed pixel output to the MDEC owner. Analytical vectors and original
reset-table fixtures test the numerical pipeline. Hardware-exact rounding,
DMA1/libpress output integration and complete movie playback remain unproven.

## Stage152 libpress and movie DMA output

[DAY2_MDEC_DMA_OUTPUT.md](DAY2_MDEC_DMA_OUTPUT.md) connects original input/output
calls, deferred MDEC DMA completion and the movie slice callback. A two-slice
integration verifies decoded RAM and final VRAM in15/24-bit modes with DMA/IRQ
mask checks. Complete player traversal and hardware-exact pixel fidelity
remain unproven.

## Stage153 complete opening STR frames

[DAY2_MOVIE_COMPLETE_FRAMES.md](DAY2_MOVIE_COMPLETE_FRAMES.md) verifies original
VLC instruction output for the first three complete320×240 opening frames,
then checks native table/input/RLE hashes and decodes all300 macroblocks/frame
through libpress DMA in15/24-bit formats. This closes a real compressed-frame
coverage gap; full movie playback and hardware pixel accuracy remain open.

## Stage154 movie updater dependencies

[DAY2_MOVIE_UPDATER.md](DAY2_MOVIE_UPDATER.md) records the full updater/abort
control flow and adds original retry-position and SDK teardown helpers to the
native port. Original graph checks cover signed conversion, alias behavior and
callback-layer selection. The trace identifies command queue/poll semantics
still needed for both Setloc retry and Pause teardown before updater wiring.

## Stage155 SDK command queue and response polling

[DAY2_CD_COMMAND_QUEUE.md](DAY2_CD_COMMAND_QUEUE.md) ports original queue issue,
completion-ring polling and blocking commands for the enabled CD device.
Setloc accepts the updater's response buffer and Pause uses real SDK completion.
The disabled-device accommodation and unwired movie updater remain explicit
limits; this does not establish full opening or Day1/Day2 acceptance.
