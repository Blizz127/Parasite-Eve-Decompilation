# Exit-menu card records and cleanup frontier

DAY1/DAY2-70 restores428C4,42910,4298C,42A10,42AD8,42B28 and62CD0 under
pc_port/game/boot/func_80015AF0_port.c. All matching src files remain unchanged.
The original functions are verified directly from the pinned Disc1 executable.

- 428C4 returns wrapped32-bit A1860-1;42910 clears A1860 then A1868.
- 42B28 returns A1838 unchanged.
- 42AD8 reads record state at A0ED5+index*418hex and returns1 for3,8,10.
- 62CD0 saves current selection9D15C, publishes its argument there, then writes
  the saved selection to9D160.
- 4298C accepts a record only when its state is0 or12. In order it writes
  state1, byte+0B=2, halfword+16=10decimal, calls62CD0(0), and publishes its
  mode argument to A186C. Other states leave all state unchanged.
- 42A10 runs the same two-record cleanup as42798, then clears record1's state,
  record0's state and A1838, in that order. These final resets require normal
  return from every called routine.

Addresses above have800 prefix; record offsets are hexadecimal. Index strides
and selection subtraction wrap to32 bits. No additional record-index bound
is imposed. This state mutation does not itself implement card I/O.

Found and fixed an existing stop violation in42798: after reporting its
unresolved72774 call, it invalidated the handle, advanced state to12 and
continued to later records. It now returns at that boundary, preserving the
handle and state.42A10 propagates the stop and omits its reset suffix. A future
implementation of72774 must restore the original post-return stores before
claiming complete cleanup. Existing EV1 tests now require preservation at the
boundary instead of requiring fabricated post-call success.

`pe_card_record_oracle.py` executes256 original chains covering every record
state byte, alternating records, mode values, selection wrap and cleanup.
Execution stops at72774 when reached, otherwise runs to the original return.
Native tests compare the exact corresponding RAM frontier and stop status;
no BIOS result is synthesized for the unresolved call. Returning and stopped
paths are distinct evidence. The first stopped call also prevents a later
record from being processed. The generated header stores state hashes/results.
Source spans428C4..428D4,42910..42928,4298C..42B38 and62CD0..62CE4 are pinned
individually by SHA256 in the tool.

## Input callback decompilation frontier

Original4D2DC always obtains selection via62A20(window,0), then6346C.
Confirm bit10000 takes priority over cancel bit40:

- Confirm with negative selection or selection>2 calls error sound526C4.
- Confirm selection0/1 first calls42848(selection); false returns1. Next,
  absent42770(selection) returns1; nonzero42B28() also returns1. Otherwise
  call42A10(),4298C(selection,1),525EC(), then return1.
- Confirm selection2 finds/closes windows37,38,36,19 in that order, calls
  5C1EC(0),512AC(9,0), clears9CFF8, calls525EC and returns1.
- Without confirm, cancel performs the same closes/reset/clear but ends with
  52634. With neither input, return1.

The input callback remains unported while its dependent graph is restored.
42848 returns whether record flag4 is clear; when set and A1860 is zero, it
writes index+1 then calls4DAA4. That routine can construct a modal42 window,
resolve/concatenate strings, measure text and install callback50580, or use
window40 and callback42910 in another mode. Its complete execution and the
remaining card/event/BIOS paths still need verification. These decoded facts
are not a claim of native confirmation/cancellation or full Day1/Day2 coverage.

DAY1/DAY2-70 final validation:256 original/native chains pass normal and
sanitizer checks; three EV1 cleanup groups pass under sanitizers. FullCTest8/8
passes in68.46s,1270 native groups. Header regeneration, Python compilation
and scoped whitespace checks pass; final builds have no warnings/errors.
Changes remain local.

## DAY1/DAY2-71: state-flag gate and modal constructor

Restored42848..428C4 (31 words) and4DAA4..4DC84 (120 words),151 words total.
42848 reads record flag4. If clear, return1. If set, return0; before returning,
when A1860 was zero, publish index+1 and call4DAA4. A preexisting nonzero
selection suppresses that call. Record indexing and index+1 retain32-bit wrap.

4DAA4 checks9CF50. In nonzero mode it reuses an existing window42, otherwise
finds list36, resolves first text at resource(428C4()+47hex), and allocates a
modal42 window/list. It installs window drawing44E14, input44E98 and list
callback4F950, writes9CF14=6C, changes selection via62CD0 and sets list+44=1.
After selecting the resource bank9CF10, it copies first text into A19C0 or
writes FF for missing text, then appends resource49hex. It writes9CFA4=4A,
measures the text and uses a minimum width120; widths at/above120 cause the
original second measurement. Window width is measured width+20, height66,
x=(300-width)>>1; list x=(window width-128)>>1 and y=height-20. It publishes
callback50580 at9CFA8.

In zero mode an existing window40 suppresses construction. Otherwise it calls
the existing4CC50 with resources(428C4()+47hex,49hex) and installs42910 as the
notice callback at9CFFC. Installed callbacks are not executed during this
constructor. The current notice dispatcher still needs a42910 arm, and the
50580/input4D2DC graphs remain separate work. Stop guards prevent caller suffix
writes after resource/text callees newly request a stop.

`pe_card_modal_oracle.py` compares96 original gate/modal graphs and16 direct
missing-first-text cases. Fixtures vary record flag, pending selection, mode,
correct/wrong existing window, short/long synthetic text and record index.
Visited-PC checks verify new modal construction, the missing-text branch, the
second width measurement and the shared message-window path. Expected RAM and
return values come from complete original graph execution with the existing
allocator/text/resource helpers; no modal callee is silently skipped. The
native DAY1_card_modal test compares all112 results.

Source SHA256 values:

- 42848..428C4: c671b8b029ab6145ec837dfda77f6c39d239a87dfd8d2bec18feb1c6b3f7048b
- 4DAA4..4DC84: 59f33cebaf98ea30ad979b913d7b96049459d44d034844811572827b3817af33

These are synthetic text/resource and initial-menu fixtures, not proof of
retail text presentation, interactive dismissal or successful card I/O.

The first native modal comparison exposed a shared resource-bank state split:
52E30 changed host D_8009D048/D050/D058/D064 while guest RAM retained the old
values. Replaced the six pointer/count owners D048,D04C,D050,D054,D058,D064
with canonical PE_GUEST_U32 lvalues and removed their duplicate definitions
from pc_port/src/pe_globals.c. This also makes reuse-path inputs come from the
same state as guest pointer reads. D018 and D03C remain outside this change.
The oracle's complete guest-state comparison detects the original split;
it is not papered over by synchronizing expected results in the test.

The ownership change also required updating13 older regression tests that
explicitly treated these guest addresses as untouched aliases or expected the
fields to survive PE_RamReset. Their checks now require cleared guest state,
verify reinitialization, and include the exact original resource-field writes
in whole-RAM footprints. The unrelated low-address guards and record canaries
remain checked. The51CC4 canary fixture now supplies a valid resource-bank
selection before its snapshot; the canary capacity byte165 yields the original
capped count50. These expectation changes follow the canonical guest-state
contract rather than restoring duplicate host storage.

DAY1/DAY2-71 final validation:112 original/native modal cases pass; full normal
CTest8/8 passes in70.12s, and the full sanitizer suite passes1271/1271 groups.
Original64 drawing/1024 predicate regression and generated header check pass.
Python compilation and scoped whitespace checks pass. All functional changes
were tested; subsequent ownership-comment cleanup changed no executable code.
Changes remain local, with full Day1/Day2 and interactive acceptance unfinished.

## DAY1/DAY2-73: confirmation, delayed state13 and progress display

Native implementation in func_80015AF0_port.c now contains247 original words
across these spans:

| Span | Words | SHA256 |
| --- | ---: | --- |
|42464..424B4|20|2f36c1a49622eb717df9384857344051e1c1b62805c202e711453fefe94aa65f|
|428D4..42910|15|be4a2386c8f02ce7763cdcd7d6c2faf671ec204ccb2fed237ae189dc1fa4add9|
|42B50..42B6C|7|f1d4cafb97144b261c444d5a29e5f2457e89a61cec555bd5cb8a150539b0b274|
|4CE28..4D024|127|c3de5ab0a5128340d5c650c7d8f672daf4a87de276b3ce882af1c3bdc9347a6a|
|4DA04..4DAA4|40|78064bafdecda69b22e864f09347c4da45b4c10720cda0f8e1af50f69ce9c46c|
|50580..50618|38|388ea5e89ff1d52fc4be1e4562ce1d86b9c46aca4eb145eaa300797a4ee18e81|

50580 nonzero-confirm constructs window39 under current selection, installs
4DA04 draw and4DA9C input(always1), selects it, writes text id40 to9D000,
and schedules428D4 via42B50. The setter stores guest callback identityA1870
and counterA1874=1. Existing42B6C increments this counter and invokes at4;
its new428D4 arm writes state13 to A0ED5+(A1860-1)*418 before clearing the
callback/counter. It does not perform card I/O or invent completion of state13.

50580 zero-confirm constructs two-line window40 via4CE28(firstindex+47,4B)
and sets notice callback42910. 4CE28 copies the two independently resolved
texts toA1A20/A1A60, reproduces repeated signed width comparisons/measurements,
uses minimum100, centers width+20, increases window height and list y by14,
and places the list at width-68. Draw callback4CFD4 draws at(0,10), then(0,14)
relative increments. This differs from the earlier concatenated-text4CC50.

4DA04 draws9D000 at(0,10); text40 skips progress. Otherwise42464 returns0
whenA1854 is null, or min(8, signed_wrapped(A1858-sign_extend_half(record+14)
+400)>>10), with no lower clamp. After(16,20), eight glyphs83/82 indicate
signed i<count and each advances(16,0). All arithmetic is original32-bit wrap.
New draw/input callbacks are registered in menu dispatchers;50580 is registered
in44E98 confirmation dispatch with a stop-epoch guard before its sound suffix.

Next original frontier:5C498 still stops at425DC when9D030>1. 425DC first
calls405A4(1),405A4(0), then handlesA1864 countdown/status dialogs and calls
41108(1),41108(0), with selected-record removal cleanup between. Original
41108 jump table10F6C maps state13 to411E4 and state14 to41270 (not the
nearby sequential labels). State13 requires flags exactly5; otherwiseA1864=-1
and state0. With flags5 it waits for own status+8=4 and other status0/4, then
sets busyA1838=1 and state14. State14 formats a device string through71A84,
calls72784, storesA1864=12 on nonzero return or-1 on zero, clears flag4,
busy and state. This callee's behavior remains unresolved. Full405A4 and41108
state machines and their BIOS dependencies must be restored, not bypassed to
pretend card-operation success.

DAY1/DAY2-73 validation:128 original confirmation graphs and regenerated header
PASS;64 include delayed callback and drawing. Normal/sanitizer focused tests
PASS1 group each,1272 skipped. Full normal CTest8/8 PASS74.44s, including1273
native groups. No build warnings/errors; Python compilation/scoped whitespace
PASS. Tests use synthetic text and zero sound package, so do not prove audible
output, card I/O, live scheduling/presentation or whole-day acceptance.

## DAY1/DAY2-74: card status machine and frame driver

New func_800405A4_port.c translates408 original words across405A4..409B4,
425DC..42770,42928..42964,4CDAC..4CDD4,4D4A0..4D4C4,4D9D8..4DA04,
and4DC84..4DC8C. Full executable SHA1 and individual spans are pinned in
pe_card_status_oracle.py and pe_card_driver_oracle.py. No matching-source edits.
The status routine preserves original branch bodies, with explicit stops at
unimplemented BIOS callees; those stops do not establish returning BIOS effects.

405A4 indexes recordA0ED4+index*418 and switches on byte+8:

- 0 clears flags then attempts scheduled polling;4 sets flag1 then polls.
- 1 consumesA1820 first: existing flag1 goes to status4/alternate; otherwise
  reaches TestEvent726F4(BCDB8 handle). If no1820,1824 clears itself/status and
  flag4 then alternates. Otherwise1828 clears itself before the same BIOS edge.
- 2 consumes182C first before TestEvent726F4(BCDA8 handle). Otherwise either
  1830/1834 clears both/status and alternates; no event means no change.
- 3 consumes1820 first, sets status4 and checks for menu36 via4D4A0. When absent,
  sets flag1 and calls4298C(index,0), preserving its operation-state gate.
  Otherwise1824 clears status; otherwise1828 sets status4 and flag4. These
  returning event paths alternate the selected polling slot.
- Other status bytes return without changes.

Alternation clearsA1840 and stores boolean(oldA183C==0) toA183C. Polling is
blocked byA1838 or a different slot. Otherwise it storesA1840-1 (wrap) and
uses the OLD signed counter to decide whether to return. At zero/negative old
counter it reaches726F4(BCDA8 handle). Four event calls and subsequent card
calls7DD44/7DD54/7DD74 are represented in original order, but the first unresolved
726F4 stops execution before later clears/status publication.726F4 is BIOS
B0(0B);7DD44/7DD54 are A0(AB/AC), and7DD74 calls7DDC4 then7DDB4. None has a
fabricated response here.

425DC calls405A4(1), then405A4(0), preserving every newly raised stop. With
nonzeroA1864, status outside1/4 forces-2; positive countdown decrements, then
nonpositive status closes progress39. Zero opens text52 with callback42928;
-1 opens3C with42910; both setA1868=1. Other negatives reset selection/pending.
It clearsA1864 after that handling. Then it processes slots1,0 in order: a
selected record lacking flag1 removes42/40/61 and resets selection/pending,
then reaches41108(index). This operation processor is still an explicit stop,
so no second-slot processing is fabricated after its first invocation.

5C498 now calls this driver for9D030>1 instead of stopping before it. Its
original branch skips incrementing9D030 after a returning driver; the native
condition now preserves that distinction. 42928 enqueues selected-1 with mode1
then clears selection/pending and is registered in4D030 notice dispatch.

Follow-up evidence: original41108..42020 is966 words, SHA256
 a9033ae5110fc414ffe13fa229fc7ffe9bc17a9e563eae68fc8e49b712fd3c6a.
Direct-call inventory and state entries are saved locally in
local/live/card-operation-calls.json. Dependencies include40F80,42264,
4D27C/4D298/4D4C4 and BIOS72734..727B4,71A04,71A84, in addition to restored
menu helpers. Existing platform/pe_libcard.c409B4 skips card hardware bring-up;
PE_Event_Open/Enable inpe_libetc.c retain only the audio event, not the card
events, and never invoke their handlers. Deterministic allocated handles do
not prove card-event state or completion. This is an implementation gap to
resolve before replacing the explicit726F4 boundary with returning behavior.

DAY1/DAY2-74 validation:4096 original status cases and96 driver/16 callback
cases match native RAM and return/stop frontiers. Generated-header checks PASS.
Normal and sanitizer focused DAY1_card_ suites PASS5 groups each (1270 skipped),
including prior card regressions. Full normal CTest8/8 PASS76.54s, including1275
native groups. No build warnings/errors; Python/scoped whitespace PASS. Changes
remain local. Unresolved BIOS and operation processor prevent full card/menu,
Day1/Day2, live presentation or release acceptance.
