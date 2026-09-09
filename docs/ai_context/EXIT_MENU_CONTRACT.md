# M0351I E7 menu-entry command

DAY1/DAY2-68 restores opcodeE7/15AF0 and menu constructor4D18C under
pc_port/game/boot/func_80015AF0_port.c, with a native VM dispatch arm. Original
source spans15AF0..15BAC (47 words) and4D18C..4D27C (60 words) have SHA256:

- 2d4a817590127635949524c2af473ebc41aa77d79c7b3681fada37ee2657a1e7
- 80a6ad99960fd8029c6bbe703a9dcdfd67d0406957b7478983ae03e73feeca2a

The pinned M0351I script calls E7 with no arguments at801910C0. The preceding
choice-zero branch writes persist1=998, and the following command85 starts the
exit fade. E7 itself has three ordered paths:

1. If scene flags800B0CD8 contain1000, return1 without changing state.
2. Otherwise, if current task+8 lacks20, set that bit, load next-PC, write task
   delay1 at+10, rewind next-PC by8 and return0. The VM retries the same command.
3. On retry with task bit20 set, call67CBC then4D18C. Reload scene flags and the
   current task pointer, set scene bits9000, set input flags8009D1A0 bit4, clear
   task bit20 and return1. A newly raised native stop prevents the suffix.

67CBC sets display flags800BCF88 bit1000, toggles2000, clearsC000 then sets4000.
4D18C constructs window/list identifier36 (24hex), owned by the current menu
selection. It installs callbacks4D2DC at window+2C,4FDE8 at list+30 and4FDA4 at
list+8C. A restored list selection2 is reset to0; other selections are retained.
It publishes the list as the current selection, creates help window19 only if
missing (callback4C608), and writes36 to help+38. It then writes1 to9CF50,
calls5C1EC(1), resets event records via42538, and initializes resource records
via5DE88. All addresses in this paragraph use800 prefix where omitted.

The constructor's existing native callees are executed, including actual menu
allocation, cursor restoration and list lookup. The installed callbacks are
not executed by the constructor. At stage68 all three remained native
boundaries; stage69 below restores drawing and the predicate. Input callback
4D2DC remains an explicit boundary. Entry alone does not make the complete
menu usable.

`pe_exit_menu_oracle.py` executes128 original E7 cases, retrying those that yield.
Cases vary existing menu/help nodes, scene flags, task flags, saved selection,
renderer-enabled state and display flags. Visited-PC checks establish execution
of the initial-yield path, constructor path, selection2 reset and help creation.
RAM hashes cover menu nodes and globals, task/PC, display flags, event reset and
resource records. The test-only runner supplies the established BIOS A(28h)
bzero memory contract for42538; this is not emulation of BIOS implementation.
Fixtures derive from the existing inventory menu audit, with explicit initial
menu/static-data state. Script opcode/argument count and executable hashes are
checked. The generated header contains fixture words and expected state hashes.

Native `DAY1_exit_menu` compares each initial/retry state and runs E7 through the
VM, checking the first yield/rewind and subsequent scene-flag bypass into the
next opcode. Complete menu callback execution, rendering, input interaction,
full M0351I scene scheduling and Day2 map loading remain separate work.

DAY1/DAY2-68 final validation:128 original-state cases pass normal and sanitizer
checks; VM yield/continuation passes. FullCTest8/8 passes in68.01s,1268 native
groups. Original entry-path checks, Python compilation and scoped whitespace
checks pass. Final builds have no compiler warnings/errors. Changes remain local.

## DAY1/DAY2-69: option predicate and drawing graph

Restored42770 (10 words),4FDA4 (17),4FDE8 (28) and50C08 (18),73 words in
total.42770 reads bit0 at800A0ED4+index*418hex with wrapped32-bit addressing;
it does not impose a new two-record bound.4FDA4 returns1 immediately for
index2; otherwise it returns that record bit. This enables menu options0/1
according to card-record state while keeping option2 enabled.

50C08 moves the drawing origin by(-2,-2), then draws resource(index+84hex) for
signed index<2, otherwise resource62hex.4FDE8 publishes its list at9CEF4, calls
the existing638D8 renderer with row callback50C08, sets draw style1, then emits
resource68hex and advances(0,16) once per unsigned list+38 count. It reloads
the count after list rendering. Native stop propagation prevents suffix work
when the drawing graph cannot return normally.

Registered4FDE8 and50C08 in the menu draw dispatcher and4FDA4 in the existing
cell-predicate dispatcher. Thus638D8 now resolves the exit menu's predicate and
row callbacks. The original input callback4D2DC remains unported; rendering
this menu does not establish working confirmation/cancellation.

`pe_exit_menu_draw_oracle.py` checks64 complete original4FDE8 graphs with both
buffers, alternate blink phase, all combinations of two-bit record flags,
three selections and packet-buffer exhaustion fixtures. It compares menu
state, enabled-bit masks, clipping/draw packets, text/glyph state and packet
allocation/link effects. A further1024 predicate cases cover every byte value
for indices0/1 and sampled unchecked indices, including index2 and wrapping
high-bit indices. Executable and all73 source words are checksum-pinned.

Fixtures reuse the inventory drawing audit's synthetic text/resource data,
then execute the original4D18C constructor before supplying record flags and
selection. They prove instruction/packet equivalence for those inputs; they
do not prove retail text assets, GPU presentation, host timing or interactive
menu completion. Checked-in expected-state data is generated from original
instruction execution. Native DAY1_exit_menu_draw compares all cases and
checks that drawing does not advance the supplied VBlank count.

DAY1/DAY2-69 final regression:8/8 CTest targets pass in68.34s,1269 native
groups. Original128-case E7 constructor regression and generated drawing
header check pass. Normal/sanitizer focused groups, Python compilation and
scoped whitespace checks pass. Final builds have no compiler warnings/errors.

DAY1/DAY2-70 restores seven card-record/selection helpers required by the input
path and fixes42798 continuing past an unresolved72774 call.256 original
chains match normal/sanitizer native states, distinguishing completion from
stopping at the call. Handles/states now remain unchanged on that stop, and
42A10 omits subsequent resets. See CARD_RECORD_CONTRACT.md for ordered input
callback decompilation and remaining modal/card/BIOS dependencies. Native input
callback4D2DC remains unported; full menu interaction is not yet established.

DAY1/DAY2-71 restores42848/4DAA4 (151 original words): flag-gated modal
creation, existing-window reuse, both modes, missing first text and measured
layout.112 original/native cases match normal and sanitizer tests. The audit
also exposed resource-bank scalar copies diverging from guest RAM; D048/D04C/
D050/D054/D058/D064 now use canonical guest lvalues. See CARD_RECORD_CONTRACT.md
for details and synthetic-text limits. Input4D2DC, modal callback50580 and the
notice42910 dispatch arm remain outstanding; no full interactive acceptance.

## DAY1/DAY2-72: confirmation, cancellation and notice dismissal

Restored full4D2DC..4D4A0 (113 words), SHA256
 d06876052f322de0fa143cfa7b8e3755822ccf3209d89af7ae8006bb8e83374d,
and registered it in the real menu input dispatcher. The ordered callback
semantics previously decompiled in CARD_RECORD_CONTRACT.md are now native:
confirm10000 has priority over cancel40; selection0/1 follows the card gates,
selection2 and cancel close37/38/36/19, and invalid selections use526C4.
Confirm-close uses525EC, cancel uses52634. All newly raised stops from modal,
cleanup and close/reset callees prevent subsequent effects. The host return
on a stopped path is not asserted as an original return value.

Added42910 to4D030's notice callback dispatch. Dismissing that shared notice
now calls the existing original selection/pending reset and clears its callback
slot before the sound call. This closes the mode-zero modal's previously
unresolved dismissal edge. The separate mode-one50580 callback is still missing.

`pe_exit_menu_input_oracle.py` executes320 original4D2DC graphs across four
button masks, five selections, four record-flag combinations, busy/not-busy,
and normal/live-handle cleanup state. It also executes eight4D030 notice cases,
including confirm, cancel, simultaneous input and unrelated buttons. The oracle
stops at72774 only if reached; it does not synthesize an I/O return. Native
DAY1_exit_menu_input compares corresponding RAM and return/stop outcomes and
checks that a stopped path records exactly the72774 boundary. A native queued
event through5E30C checks the new input-dispatch arm is connected.

Visited original PCs prove execution of the enqueue/modal/confirm-close/
cancel-close/error paths and all three sound routines. These fixtures set the
sound-package pointer to zero: they verify sound-path selection, not audio
queue effects or audible output. Menu/text fixtures remain synthetic. Card I/O,
mode-one modal confirmation50580, full scene scheduling, GPU presentation and
whole Day1/Day2 acceptance remain unfinished.

The first native comparison failed cancel case80: onlyB0CD8 differed, with
native retainingC000. Restored the omitted5C1EC(0) suffix from4C974.s:
clear9D030, call42798, then clearB0CD8 bitsC000 only after cleanup returns.
An epoch guard preserves the unresolved72774 stop before that suffix. The old
B54KQ quiet-cleanup test incorrectly required retained flags; it now requires
the original cleared value. All328 original/native cases now agree in normal
and sanitizer builds (one test group each,1271 unrelated groups skipped).

Final DAY1/DAY2-72 normal CTest passes8/8 in69.09s, including1272 native
groups. Both normal/sanitizer builds have no warnings/errors. Changes remain
local; latest published launcher is DAY1-41.
