# Remaining M0000I transition rendering graph

Original Disc1 overlay atLBA2805 is the authority. Inspection after DAY1-53
finds this remaining graph. These are source boundaries/calls, not native
rendering acceptance. Decompiled disassembly is ignored in `local/live/`.

| Original routine/span | Words | Direct dependencies relevant to restoration |
| --- | ---: | --- |
| 80192800..80193478 | 798 | 90E04,91114,90D3C,SDK rand71A54 |
| 80190D3C..80190E04 | 50 | RESTORED DAY1-53; full wrapper comparisons |
| 80190E04..80191114 | 196 | RESTORED DAY1-53; full wrapper comparisons |
| 80191114..80191580 | 283 | RESTORED DAY1-53; full wrapper comparisons |
| 80197BA0..8019959C | 1663 | RESTORED DAY1-52;2240 original comparisons |
| 801995BC..8019A318 | 855 | RESTORED DAY1-50;1280 comparisons |
| 8019A318..8019B1D0 | 942 | RESTORED DAY1-51;1304 original comparisons |
| 8019B1D0..8019BD78 | 746 | RESTORED DAY1-49;1280 comparisons |

The91580..91854 object helpers and9959C..995BC are already translated in
func_80190998_port.c; do not include them in the missing91114/model spans.
DAY1-47 restores all8FFF4..90998 visibility routines, including90254/904B0.
Matrix helpers above are already present. All four packet emitters and three object wrappers are restored. The remaining
graph is the main renderer and outer transition loop; wrapper comparisons alone
do not prove their live integration.

DAY1-48 restores SDK79384/79414, with3304 original/native comparisons and
normal plus ASan/UBSan targeted passes.21 ISA-level GTE tests pass, including
the newly supported AVSZ4 oracle opcode. DAY1-49 additionally restores9B1D0; DAY1-50 restores995BC.
DAY1-51 restores9A318; DAY1-52 restores97BA0. DAY1-53 restores the object wrappers. Next:92800.
The shared SDK source boundary is retained below. The contiguous80-word slice
79384..794C4 includes8 padding bytes between the two helpers; SHA256:
418c0e10f86f636b682b2353689a5ce9a3f0719d39d63a9e371c6714f35e3028.
Both execute RTPT then NCLIP, store projection FLAG even when winding rejects,
and return the clip result. The positive-winding triangle path stores three
screen coordinates and IR0, then AVSZ3 depth. The positive-winding quad path
stores the first three coordinates, loads/projects the fourth vertex, merges
RTPT/RTPS flags, stores IR0 and AVSZ4 depth. Preserve original store/load order
when inputs and outputs overlap. The native GTE now retains projection FLAG and SXY/SZ FIFOs; the SDK wrappers
preserve early rejection and RAM alias ordering. Cases compare all output RAM
and18 relevant hardware registers. Hardware authority:
[psx-spx GTE](https://psx-spx.consoledev.net/geometrytransformationenginegte/).

Packet emitter source SHA256 values:

- 97BA0: 8a823f37071d3fc46dba908ad18da507e51145bbd576efe69d59af85fce3f4a3
- 995BC: 9e05144086c6f9bfa77edeb3dbf5bb41ae8d10d5f82629af07bfaad6d9c8af69
- 9A318: cb887e9575fa14d4044f7d590846e5f0d5c29d1580889af040cb5262a584a2d4
- 9B1D0: 2c946ccd523fdfd5dbcc32a802e0545f54e8f1e8f18d0a656a47f04b3f11f658

Rendering closure must be followed by the9234C outer loop and its retained
menu-list history (TRANSITION_MESSAGE_STACK.md), original/live transition
verification, and continued full Day1/Day2 coverage. This graph is not the
complete remaining game scope.

## Restored emitter: 8019B1D0 source findings

The746-word routine selects a submodel through `a1 + load(a1 + a2*4)`.
That submodel has eight halfword counts at+0..+0E and eight relative stream
pointers at+10..+2C. Initial packet cursor is `**8019C9C0`; OT base is
`(*8019C9C0)+4` dereferenced. Fixed OT index from801EA5E4 goes into
scratchpad+18. Final packet cursor is written through8019C9C0 at8019BD40.

Source strides for streams0..7 are1C,24,24,30,28,30,30,3C bytes;
accepted packet sizes are14,18,1C,24,20,28,28,34 bytes. Each stream re-reads
its count and each accepted packet links through the current descriptor/OT.
Preserve read/write ordering, including prior tag high bytes.

Two inline-GTE streams are not equivalent to SDK79384/79414:

- Stream4 (count+8) runs RTPT/NCLIP but copies coordinates from existing
  scratchpad+0,+4,+8 (8019B7DC..7FC), without refreshing those locations.
  Tests must seed scratch and exercise predecessor acceptance/rejection;
  substituting newly projected coordinates would change original behavior.
- Stream5 (count+A) runs AVSZ3 before projecting the fourth vertex (8019B93C),
  writes current SXY directly into packet coordinates, then RTPS for vertex4.
  Do not replace that ordering with the quad SDK helper's AVSZ4 contract.

DAY1-49 implements these original behaviors and passes1280 original/native
comparisons, sanitizer checks and all8 regression tests. This is source-level
emitter verification, not live acceptance of the surrounding renderer.

## Restored emitter: 801995BC source differences

DAY1-50 translates the855-word source and source hash above;1280 original/native
comparisons, ASan/UBSan targeted tests and full CTest8/8 pass. Submodel/stream layout
resembles9B1D0, but depth and projection paths differ:

- Streams0/1 call SDK triangle/quad with depth destination scratch+18. They
  add801EA5E4 even after winding rejection, writing the sum back. On positive
  winding, unsigned `(sum-1)<0xFFF` accepts sums1..4095, then arithmetic shift
  right2 selects the OT index. Preserve rejected-path scratch history.
- Stream2 shifts the wrapped depth+bias sum before the same unsigned bounds
  check (801999B8 vs801999DC), unlike streams0/1.
- Streams3..7 use inline RTPT/NCLIP and AVSZ3. Only after positive winding do
  they store OTZ and add bias; signed positive sum is the depth condition, then
  arithmetic shift right2. No matching upper-bound check appears in these
  paths. Quad streams project vertex4 after that AVSZ3 and coordinate stores.

The source also publishes its cursor immediately after stream3, even if empty.
Sixteen comparison cases place the descriptor in later-stream color data so this
intermediate write affects emitted bytes. Shared input layout does not prove
interchangeable rejection, scratch lifetime or packet construction.

## Restored emitter: 8019A318 color and depth differences

DAY1-51 restores the942-word source with1304 original/native comparisons,
ASan/UBSan targeted pass and full CTest8/8 pass. Its relative model/stream layout matches the
other two emitters, but preserve these source differences:

- Streams0/1/2 call SDK projection, add801EA5E4 to retained/generated depth,
  arithmetic-shift2 and store before checking winding and unsigned
  `(index-1)<0xFFF`. Stream3 calls the quad SDK but bounds-checks the unshifted
  sum first (8019A8C0), then shifts/stores only on acceptance.
- Streams2/3 copy only the first original color/command word, then write each
  RGB byte shifted right2. Secondary color fourth bytes are retained from the
  packet buffer. They do not copy full secondary source color words.
- Streams4..7 use inline RTPT/NCLIP and AVSZ3; they add bias, shift2 and store
  before testing signed-positive index. Quad paths then load/project vertex4
  after first-three coordinate writes. Rejection scratch/GTE history differs
  from995BC even where packet sizes coincide.
- Streams4/5 copy source color/command then replace its RGB bytes with bytes
  from801EA264. Streams6/7 do the same first word, then load the full uniform
  color word once and repeat it into remaining color slots. Preserve read/write
  order when source, global color or packet bytes alias.
- Only final cursor publication appears (8019B198);995BC's mid-function
  publication is not present here.

Source evidence: ignored local/live/original-8019A318-whole.txt. DAY1-52 restores the1663-word97BA0 emitter described below.

## Restored final emitter: 80197BA0 source findings

DAY1-52 restores1663 words in explicit static native C. Takes the model header directly in a1, with no
submodel-index lookup. Eight counts+0..+E and eight relative stream pointers
+10..+2C. Source strides are1C,24,24,30,40,48,3C,4C; vertex starts are
4,4,C,10,28,28,24,2C. The last four layouts differ from the restored emitters.

Bias comes from801EA5E0 (not5E4). Entry stores a bias snapshot into scratch+24;
textured streams4/5 use this cached value, while SDK paths load the global.
For streams4/5: RTPT/NCLIP, AVSZ3, then index=arithmetic-shift2 of wrapped
OTZ+bias. Store index; reject if signed `((index<<2)-cached_bias)<=0` with
wrapped32-bit operations. Preserve this post-quantization test.

FT3 (stream4) subdivides at signed index<500; FT4 (stream5) subdivides at
index<501. Near paths compute signed vertex midpoints and byte UV midpoints,
project the subgeometry and emit four packets. Do not reduce them to one
triangle/quad. Source has three UV layouts selected around index510 and701
for far paths. Required comparisons should explicitly straddle499/500/501,
509/510 and700/701, in addition to rejection, alias and scratch-history cases.
FT3 near path writes scratch pointers+2C..34, midpoint coordinates+48..5C,
projected midpoint coordinates+68..70 and UV bytes+78..7D; FT4 uses additional
scratch positions. Inspect its complete ordering before implementation.

GT3/GT4 (streams6/7) call the SDK leaves, add bias and shift/store even on
winding rejection, then test winding and unsigned `(index-1)<0xFFF`. Main
cursor publication is at80199564. All source is in ignored
local/live/original-80197BA0-whole.txt; exact source hash is listed above.

The97BA0 transcription uses named scalar locals and37 explicit branch labels,
with original source addresses on statements. pe_subdiv_model_translate.py is
an offline, source-pinned transcription/reproducibility tool; the runtime has
no instruction fetch, opcode dispatch or generic register array. Original
nonescaping spill-frame values live in native locals; ABI saves are collapsed.
This preserves complete subdivision ordering rather than merging it into the
simpler emitters.2240 original/native cases compare persistent tested RAM and16
GTE data registers, including randomized incoming caller registers, all8 streams,
near/far texture paths, UV-band thresholds, rejection and aliases.

Original diagnostic local/live/original-day1-52-subdiv-cursors.log confirms:
near FT3 emits/links four32-byte packets and advances cursor128 bytes; near FT4
emits/links four40-byte packets and advances160 bytes. Far paths emit/link one.
Payload and OT cursor updates occur at different source instruction positions,
but the final links form the expected four-packet chain; no cursor bug is claimed.

Restored90D3C is50 words: rotate object+28 into object+8, copy the matrix to
*8019BFF0 in two16-byte load-before-store blocks, concatenate8019CC30 with that
matrix, reload its pointer for SetTransMatrix/SetRotMatrix, then call97BA0 with
load(object+4). Preserve matrix alias ordering and SDK translation-overflow stops.
Its source is local/live/original-80190D3C-whole.txt. All three wrappers are now restored;92800 and9234C/menu-stack context remain
before transition completion can be claimed.

## Object wrapper verification (DAY1-53)

Combined529-word90D3C..91580 source SHA256:
d430a28dfa21a41f473027f8bc3c6247511b48a0440d938af53abe89cef742d7.
453 original/native cases reach all four real emitters through original SDK and
visibility graphs, including five signed-add/divide trap prefixes. Checks cover
matrix/packet/scratch/global RAM and28 GTE data/control components. Native fixture
loads the original sin/cos table (DAY1_view_tables) required by RotMatrix.

90E04 culls before rotating/submitting, snapshots its point and template first,
and still calls visibility when forced. Color pass1 left-multiplies object
rotation by template and transforms its translation with signed16 vector inputs.
91114 rotates/submits before culling. Mode2 right-multiplies by the saved template,
executes its translation transform but restores the saved translation words.
Its signed depth bands select indices at object+6,+4,+2, reject below385 in the
first band, and omit mode2 in the middle band. Preserve those differences.

Next92800 is798 words; all direct dependencies now have native implementations.
It mutates objects between render calls, writes ordered depth biases and color
passes, and conditionally calls rand71A54 twice for animation. Source
local/live/original-80192800-whole.txt. Do not collapse it into a static draw list.
The outer9234C loop and retained menu-stack provenance remain separate work.

## DAY1-54: main transition renderer restored (2026-09-08)

Implemented all798 words of80192800..80193478 in func_80192800_port.c;
source SHA256926ba5886cd51536f747a8514c1ae8cd268b44b75aeaae8492f72112eef84690.
Preserves ordered depth passes, four post-draw advances followed by separate
wrap checks,24/52 model toggling, conditional two-call BIOS random animation,
grouped matrix/angle copies, dynamic signed half-count object traversal, color
bytes, conditional LOD passes and final subdivision depth500. Calls restored
native wrappers and propagates their stop epochs. No runtime instruction decoder.

pe_transition_render_oracle.py executes the complete original graph with real
wrappers, SDK matrices, visibility and all packet emitters.128 cases cover all51
original call sites, mode0/1/2, effect gates, signed/odd object counts, wrap/overflow
values, object aliases, visibility bands, model masks and RNG advancement.
Compare packet/OT/scratch/source/object/global RAM and28 GTE components. Source,
EXE and overlay authority are pinned. Normal targeted group passes all128 cases.
Initial oracle coverage assertion expected53 sites; full source contains51 and
all51 were executed. Corrected to compare against the source-derived site set.

Normal and ASan/UBSan targeted checks pass1 actual group each (128 cases).
Full CTest8/8 PASS;1256/1256 native groups. Oracle71024, build56624,
sanitizer21788 and CTest89786 ended0. Evidence in local/live/
{oracle-day1-54-render,native-day1-54-render,native-asan-day1-54-render,
ctest-day1-54}.log. Build logs have no warnings; py_compile and scoped diff checks
pass. LOCAL; launcher remains41.
Next: restore9234C outer frame loop and trace retained menu-stack provenance
through constructor/SDK/frame calls before binding update menu context. Complete
renderer comparison does not establish live transition or full-day acceptance.
Full Day1/Day2 scope and unlocated random-battle freeze remain unfinished.


## DAY1-55: outer-loop decompilation and OT compaction (2026-09-08)

Previous goal turn was progress: main renderer restored with128 full-graph cases.
Decompiled253-word9234C..92740 into TRANSITION_OUTER_LOOP.md, preserving frame
order, reset-versus-normal exit, retained effect handle and empty-run state,
double-buffer descriptor switching, and SDK call/result requirements.
Implemented its35-word925A0..9262C OT loop as PE_TransitionCompactOT in
transition_ordering_table_port.c, with compat/CMake linkage. Source SHA256
 e3bbb8893991be52590560e3222bffa28dcdec3c44eaf5ff1be22fa387a1464a.
pe_transition_ot_oracle.py executes original code for512 cases across empty,
occupied, alternating, long-run, mixed and descriptor/bucket-alias tables.
Compares persistent table RAM and returned retained empty-run index.

Normal and ASan/UBSan targeted checks pass1 actual group each (512 cases).
Full CTest8/8 passes,1257/1257 native groups. Oracle56748, native58311,
sanitizer32400, stack audit65537 and CTest11002 all ended0. Logs:
local/live/{oracle-day1-55-ot,native-day1-55-ot,native-asan-day1-55-ot,
ctest-day1-55}.log. Build logs have no warnings; py_compile and scoped whitespace
checks pass. The native outer loop remains unintegrated pending the dependencies
above; this verification covers its compaction block and constructor observation.

pe_transition_outer_stack_audit.py runs the real constructor at outer SP801FEFD0
under its existing explicit provider contracts. Eight mode/fill cases confirm
menu-list bytes4..7 have no observed constructor writer and retain initial00/A5.
Do not bind a zero menu tail from this partial history. Evidence:
local/live/transition-outer-stack-audit.json and TRANSITION_MESSAGE_STACK.md.

Two additional live-loop dependencies surfaced: existing3EB04 is only a digital
edge cut (missing controller init/hold counters/priority/analog paths and SDK
825C0/82974/82680/8292C/828F4), and HostFB_VSync returns void although the outer
loop consumes its vblank/scanline query results. No input/timing shortcut was
introduced. Next restore those original input/SDK/timing paths, trace their
stack writes, then integrate9234C with proven menu context. Bootstrap9234C still
stops explicitly. Full Day1/Day2 scope, release/live acceptance, and unlocated
random-battle freeze remain unfinished. LOCAL; launcher41.


## DAY1-56: original controller query/configuration SDK (2026-09-08)

Previous turn was progress: full outer-loop decompilation, native OT compaction,
and constructor stack evidence. Restored ten controller functions (246 source
words) in pe_controller_sdk_port.c:825C0,82680,828F4,8292C,82974 and their real
84B20,84F8C,835A4,83BB8,83D04 helpers. Uses original guest controller slots and
the callbacks already installed by native844E4 in pe_save.c. State queries retain
conditional2/3-to1 and6-to4 translation; info queries preserve signed index
bounds. Configuration calls preserve busy checks, byte truncation, queue writes
and callback IDs. Unrecognized installed callbacks stop explicitly. No host pad
state fallback or invented controller success. Source SHA256
fa58951f31f09efabde626872321aeacd180bc41e70fdcafcec1eea1cc446918.

pe_controller_sdk_oracle.py runs1280 complete original cases without replacing
callbacks, comparing results and guest record/table/buffer RAM. Native target
passes all1280 plus two unknown-callback stop-prefix checks. Configuration queued
serial callbacks are retained as guest IDs; their later execution remains separate
work. Complete game input3EB04 is still only its previous digital cut.

Normal and ASan/UBSan targeted checks pass1 actual group each (1280 original
cases plus2 unknown-callback prefixes). Full CTest8/8 PASS71.33s;1258/1258 native
groups. Oracle99506, native63442, sanitizer60595, extended audit11179 and CTest
57345 all ended0. Logs local/live/{oracle-day1-56-controller,
native-day1-56-controller,native-asan-day1-56-controller,ctest-day1-56}.log.
Build logs have no warnings; py_compile and scoped whitespace checks pass.

Extended pe_transition_outer_stack_audit.py through twelve complete original
3EB04 digital-controller executions after constructor, varying disconnect,
state1/2/6, flags4000/C000, busy/ready and stack fill00/A5. No input/SDK providers.
When state6/C000 chooses828F4, original83BD0 saves return address8008291C into
menu-list bytes0..3 (even when busy). Otherwise their last writer remains96F8C.
Bytes4..7 still retain the initial fill; constructor provider-stack gaps remain.
This proves caller history varies and must not be replaced with a universal tail.

Next restore complete3EB04 game input behavior and reconcile D_8009D1A0 host/
guest ownership, then VSync query/timing and remaining menu-stack provenance.
After that integrate9234C with live acceptance. Full Day1/Day2 scope, random-battle
freeze, platform/release acceptance remain unfinished. LOCAL; launcher41.


## DAY1-57: complete game input and shared flags (2026-09-08)

Previous turn was progress: controller SDK restored and original stack-history
paths observed. Replaced the digital-edge-only3EB04 cut with all348 original
words through3F074. Source SHA256
c6a27aa96c180ee441866ac86dfc995683fb48c2366dfd864f0bbd03aeb4660b.
Now preserves disconnect initialization, controller state/configuration calls,
32 hold counters, the special nine-step input sequence, all priority masks,
analog menu-versus-game thresholds and ordered press/release edges. Calls real
controller SDK and menu lookup; unknown SDK callbacks propagate stop epochs.

D_8009D1A0 and D_8009D280 now use PE_GUEST_U32 aliases in psx_compat.h. Removed
host scalar definitions and stale extern declarations, so named and address-based
readers/writers share original storage. This fixes the discovered split between
input/initialization and battle/frame state. No duplicated synchronization copy.

pe_game_input_oracle.py executes2816 complete original inputs with real SDK and
menu calls. Cases cover pad identity/state, configuration/busy status, flags,
held counters, analog boundary values and both menu/game behavior, plus special
sequence progress/completion. Native comparisons check all fixture RAM including
shared flags. The first oracle coverage check exposed correlated fixture bits
that suppressed mode setup; varying mode availability independently covers it.
Normal target passes all2816. Wider initial native run1251/1259: seven old digital
fixtures lacked controller reply context, and3E974's footprint expected a separate
host flag. Updated explicit test controller setup and original38-word footprint.
Production input behavior was not weakened to retain the old partial fixtures.

Normal and ASan/UBSan targeted checks pass1 actual group each (2816 cases).
Final CTest8/8 PASS65.14s;1259/1259 native groups. Final57290 and sanitizer57082
ended0; earlier75430 exposed two stale stable-destination expectations: original
connected state2 clears setup bit4000 before frame flags are processed. Updated
their exact flags expectations, retaining destination assertions. Logs:
local/live/{oracle-day1-57-input,native-day1-57-input,native-asan-day1-57-input,
ctest-day1-57-final}.log. Builds have no warnings; scoped diff/py_compile pass.

Bootstrap source80012320..12344 explicitly switches to SP1F8003F8 before9234C.
The audit now supports --scratchpad: constructor/input SP1F8003C8, menu1F800390.
All20 actual-scratchpad constructor/input observations pass the same writer
assertions under existing constructor provider contracts. Log
local/live/transition-outer-scratchpad-audit.json (8364 ended0). Earlier relocated
RAM-stack observations are not live address/value proof; future whole-frame
stack auditing must use the scratchpad location and account for shared scratch
writes. No final menu-tail binding was made.

Next restore VSync return/timing semantics and remaining original menu-stack
provenance before9234C integration. Controller queued serial callbacks retain
original IDs; their later scheduling/execution and live host-input acceptance
remain separate work. Full Day1/Day2 scope, random-battle freeze and release
acceptance remain unfinished. LOCAL; launcher41.


## DAY1-58: full VSync SDK semantics and device traces (2026-09-08)

Previous turn was progress: complete game input, unified flags and actual-stack
audit. Implemented132-word73A44..73C54 as PE_RetailVSync with explicit clock/BIOS
operations in platform/pe_vsync.c/.h, linked by CMake. Source SHA256
 e356692b0a159f0f9e07da321a2ea515c4789379094ff39d6d77f42858169af4.
Preserves stable timer reads, entry-time return delta, negative absolute-counter
queries, signed relative waits, GPU field synchronization, both baseline writes,
and exact watchdog/timeout BIOS order. No runtime instruction decoding.

pe_vsync_oracle.py executes original VSync and wait instructions using device
read sequences and BIOS contracts; it does not replace either callee.264 cases,
4846 RLE events, compare every global/device read/write, timeout service and
return value. Native targeted group passes all264 including eight stalled-device
cases. This establishes the SDK algorithm against explicit devices, not a live
host timer. VSYNC_CONTRACT.md records the source and integration requirements.

Normal and ASan/UBSan targeted checks pass1 actual group each (264 traces).
Full CTest8/8 PASS66.36s;1260/1260 native groups. Oracle47188, native79172,
sanitizer78888, CTest29625 and scratchpad audit65540 all ended0. Logs:
local/live/{oracle-day1-58-vsync,native-day1-58-vsync,native-asan-day1-58-vsync,
ctest-day1-58,oracle-day1-58-vsync-scratchpad}.log. No build warnings; py_compile
and scoped whitespace checks pass. Initial oracle-only run failed on an observer
variable name (simm versus si), corrected before any comparison results.

Scratchpad audit local/live/vsync-stack-audit.json confirms264 original cases at
SP1F8003C8. Query modes leave menu1F800390..399 untouched. Waits write watchdog
bytes0..3 through73BC4/73BF0 and saved-return low halfword3B20 atbytes8..9 through
73BDC. Bytes4..7 retain initialA5. Timeout can leaveFFFF in the second halfword;
do not infer a universal tail or live timing from the device-sequence fixtures.

HostFB_VSync is still the old void host provider. The platform lacks timer1 MMIO,
and CPU IRQ delivery currently recognizes the DMA handler only; source0/7440C
still reaches an indirect boundary. Next restore timer1 and source0 delivery with
existing IRQ masks/generation/BIOS policy, implement the clock adapter, and bind
public VSync. Then finish actual-scratchpad frame history and9234C integration.
Do not equate the shim's invocation count or GPU frame counter with956AC/timer1.
Full Day1/Day2, live/release acceptance and random-battle freeze remain unfinished.
LOCAL; launcher41.


DAY1-59 restores the original VBlank RNG/four-timer callback and connects CPU
source0 to checked7440C dispatch.512 original comparisons plus four IRQ scenarios
pass normally and under ASan/UBSan; full suite1261 groups, CTest8/8. Unknown or
stopped callbacks prevent subsequent slot execution. Timer1 MMIO, host VBlank
production and VSync adapter remain outstanding; no new live-frame/menu-tail
or full-day acceptance is established. See VSYNC_CONTRACT.md.

DAY1-61 restores all318 PutDispEnv words and GP1(05..08) display-register
state/readback.2048 original software comparisons and ASan/UBSan checks pass;
full CTest8/8,1263 native groups. This supplies original mode/range inputs for
the remaining GPU scheduler. It does not yet establish scanline timing, public
VSync integration, final menu-stack history or live transition acceptance.
See DISPENV_CONTRACT.md.
