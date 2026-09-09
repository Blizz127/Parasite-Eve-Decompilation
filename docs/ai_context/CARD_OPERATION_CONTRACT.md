# Card operation processor: original state contract

DAY1/DAY2-75, original41108..42020,966 words in asm/disc1/307CC.s.
Executable SHA1:452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
Span SHA256:a9033ae5110fc414ffe13fa229fc7ffe9bc17a9e563eae68fc8e49b712fd3c6a.

This is a source-derived decompilation of the full operation state graph.
The native func_80041108_port.c implements each entry path only through its
first unresolved callee. The post-call behavior below is not yet native or
verified with returning BIOS semantics. It must not be counted as completed
card I/O. The original local stack frame is78h bytes; transient filename and
directory buffers are at frame+18h and+20h. No absolute guest stack address is
invented by the native boundary adapter.

Record = A0ED4 + index*418h, wrapping32-bit. State byte+1 indexes the16-entry
jump table10F6C. Entries are not laid out in numerical state order:

| State | Entry | Original behavior |
| ---: | --- | --- |
|0|42000|Return unchanged.|
|1|41170|Flags must equal1, else40F80 cleanup. Wait for own status+8=4 and other status0/4. Then busyA1838=1 and state=record+0B.|
|2|412C0|Flags exactly1. Initialize directory records, enumerate BIOS directory entries, account blocks, and choose display/scan continuation. Detailed below.|
|3|415B0|Flags exactly1. Search candidate entries in alternating order around center+5; open/read each selected header.|
|4|417C0|Flags exactly1. Copy A1704 low byte into selected entry+45h, format name, open with10200h. Close a successful handle, then state6/timer10; failure follows retry path.|
|5|418B4|Flags exactly1. Format name and open with1. Successful handle goes to state7/timer30; failure retries.|
|6|41980|Flags exactly1. Format name and open with2. Success gives state9, buffer9EED0, timer30; failure retries.|
|7|41B18|Flags exactly1. Read signed min(remaining,1024). Advance buffer/decrease remaining on positive return. Completion closes, handle=-1,state12,error0,clearsA1854/busy, then42264. Nonpositive return retries.|
|8|41A58|Flags exactly1. Read signed min(remaining,128). Advance/decrease on positive return; completion sets state10. Nonpositive return retries.|
|9|41C0C|Flags exactly1. Write signed min(remaining,1024). Advance/decrease on positive return. Completion closes, handle=-1,state3,error0,clearsA1854,closes progress39,opens notice53. Nonpositive return retries.|
|10|41D04|Flags exactly1. Close handle, copy parsed header into selected entry if type1, mark entry read, advance scan and choose state3/12.|
|11|41E40|Flags exactly1. Format/open with1; close success, reformat/delete name. Successful deletion goes to state4/timer10; failed open/delete retries.|
|12|41FE8|Return if flags exactly1; otherwise40F80 cleanup.|
|13|411E4|Flags exactly5 or A1864=-1/state0. Wait for own status4, other0/4; then busy1/state14.|
|14|41270|Format device string10F60 into local frame+18h, call72784. Nonzero return storesA1864=12, zero stores-1; clear flag4,busy,state.|
|15|42000|Return unchanged; out-of-table state bytes also return.|

All flags comparisons above are equality, not merely bit tests. Other-slot
status address is A0EDC+(index==0?418h:0); it is not an unchecked1-index.

## Directory enumeration and continuation

State2 writes index+30h to byte2 of the string pointed to by92230, clears
record bytes2,3,6,4,7,0A in that order, and sets fifteen entry-type bytes to2
in reverse entry order. Entry stride44h, first type at record+1Ch.
It calls727B4(name,frame+20h). For each returned directory entry,71A04 compares
12 bytes with (Load32(92224)+6); matching names identify an entry through
(name byte13h *44h -1128h +record). Set type1, read-marker0, filename variant
at+29h=(name byte12h!=30h), and record+4=1. Add signed(directory size>>13)
to record+0A as a wrapping byte; continue through72794.

Successful enumeration exhausts the retry halfword+16 to0. No first entry
instead decrements it; return while its new signed value is positive. Then
scan15 entries: type2 gets read-marker1, other types reduce the block count.
Walk backward, changing type2 to3 until that remaining count is zero or the
scan exhausts. Set record+2=15. If A186C==0, set state15 and clear busy.
Otherwise search for any type1 entry. Such an entry is enough to continue;
without it, a block count below15 and nonzero4D27C are required. Continue by
state3/timer10,62CE4,4D4C4(derived record index,record+2), storing return to+5.
If unable to continue:62CE4,4D298(index),state12,busy0.

## Alternating search and transfers

State3 starts scan counter+6 and limit=2*record+2. Candidate is
center + ((counter&1)*2-1)*((low_byte(counter)+1)>>1), giving center,
center+1,center-1,center+2,center-2,... . Skip candidates outside0..14 or with
nonzero read-marker(entry+1Dh), incrementing the stored byte counter each time.
The loop compares the low byte after increment. Invalid synthetic counts above
127 can make that byte wrap without exhausting the doubled limit; no native
iteration cap is added to alter original behavior.

Recompute candidate from reloaded counter/limit, or useFF if exhausted, and
store to+3. FF means state12/busy0. Otherwise format filename9EE70 using
Load32(92224), unsigned(record>A0ED4), entry variant+30h, and candidate+41h
as a fifth stack argument. Open72734(name,1), always store handle to+0C.
Nonnegative handle sets state8, bufferA1720+index*128, remaining128, timer30;
if selected entry is type1,72744 seeks100h from origin0. Negative open retries.

Every transfer size uses a SIGNED16-bit remaining value, including negative
values passed through unchanged, capped at128 or1024. Positive transferred
bytes add to buffer+18 and subtract from remaining+14 modulo16-bit; its signed
new value determines completion. A return larger than requested is not clamped
by this routine. Real BIOS behavior must establish whether it can occur.

State10 closes first, then handle=-1. For type1 entries it copies header fields
fromA1720+index*128 into entry(record+1C+selected*44): halfwords28->24,26->26,
byte2A->28, words08->0C,0C->10,64->20, byte2B->2A, halves5C->2C,5E->2E,
words00/04->04/08 and10/14/18->14/18/1C. Paired/triple source reads precede
their destination stores; aliases must preserve that ordering. Mark entry+1=1,
increment scan byte+6; choose state3 while its low byte<2*count, else12/clearbusy.

## Retry and cleanup

A failed open/read/write/delete decrements unsigned halfword+16 but tests the
OLD signed halfword for positive before proceeding. On expiry, absent flag1
calls40F80 and returns. Otherwise40F80 executes first, then error byte+7 is
reloaded:1 opens two-line3D/3F notice,2 opens3E notice,other values no notice.
Finally clear+7 and set state12. Do not collapse cleanup into the final stores:
40F80 can close/delete, change selection, or reach further unresolved calls.

40F80..41108 is98 words. NonzeroA185C routes42228. Otherwise a nonnegative
handle is closed before setting-1. State9 additionally formats names and
attempts open up to10 times, closes/deletes on success. Then4D5CC(derivedindex),
clearstate/+4/A1854/busy,62CE4. This helper remains unported at DAY1/DAY2-75.

## Native evidence and remaining work

The native boundary metadata holds a known-argument mask and fifth argument.
Unmodeled stack arguments are zero placeholders marked UNKNOWN by that mask,
not claimed guest addresses. State2's727B4 destination and state14's71A84
destination require original stack authority. Other first calls compare all
semantically passed register arguments, and filename calls compare argument5.

pe_card_operation_oracle.py executes8192 original dispatch cases through the
first unresolved target or normal return. It varies state, exact flags, card
index, status gates, slot counts/center/read markers, and signed transfer sizes.
pe_card_driver_oracle.py now executes the restored dispatch instead of stopping
at41108, so quiet operations can allow both card slots to run. This is deeper
coverage of the same original driver, not a synthetic operation return.
Validation results are recorded in ACTIVE_HANDOFF.md. Returning BIOS effects,
post-call branches, guest-stack binding, real card events and whole menu/day
acceptance remain unfinished.

Next cleanup dependencies, read from original before implementation:
42228..42264 closes38,37,36 via62F3C then512AC(12,0).
4D5CC..4D690 retains list(2,36), closes40, invokes live9CFFC callback and clears
its slot only after return. If index==9CF44 it closes63,39, window41 via62F1C,
31, and index+37; when retained list exists, clears list+44 and selects it only
if631DC finds no active window. Preserve callback mutations and reload9CF44
after callback; do not copy the success suffix across an unresolved call.

Final DAY1/DAY2-75 validation:8192 original/native dispatch comparisons and112
updated driver/callback comparisons pass. Both generated-header checks pass.
Normal/sanitizer focused card suites PASS6 groups each,1270 skipped. Full normal
CTest8/8 PASS71.43s, including1276 native groups. No build warnings/errors;
Python and scoped whitespace checks pass. No post-BIOS, live, full-day or
release acceptance claim; changes remain local.

## DAY1/DAY2-76: shared cleanup and callback restoration

Implemented42228..42264 (15words) and4D5CC..4D690 (49words), plus40F80's
paths through returning cleanup or its first unresolved BIOS/formatting call.
40F80 remains incomplete after72774/71A84; no file-open/delete return is invented.
Original span pins:

- 40F80..41108:4b7854a10db5e134b7377f736d2f4207b72adca48cf9a96d766e19ca48c14212
- 42228..42264:1c958adc13bb55f6474559dd7dfb18594affde554547a516f78c4a03ab8e620c
- 4D5CC..4D690:5fb97d0fd44a3474d2bcded181f1973f74cbe470adcc406875b1e60645e3dbaf

4D5CC dispatches42910,42928,5C488 and62F9C using the live9CFFC value after
closing40. Unknown callbacks stop without clearing the callback slot or running
the suffix. Every newly raised stop propagates. It reloads9CF44 after callback,
retains the original list pointer, and restores selection only when631DC returns0.
40F80's inverse-stride arithmetic uses the original low32-bit product byC9484E2B
and arithmetic shift3, not host division. A185C routes42228 without clearing
record state. Nonnegative handle reaches72774 before setting handle-1. Negative
handle/state9 reaches71A84; other negative-handle paths call4D5CC, clear record
state/+4/A1854/busy and restore selection through62CE4.

41108 now invokes40F80, so invalid flags can take returning cleanup paths and
allow the driver to continue. Updated operation fixtures provide an empty valid
window list and explicitA185C=0/callback0/selectedindex0, replacing canary pointers
that were safe only while execution stopped before cleanup. Original expected
states are regenerated by executing the deeper graph, not copied from native.

Newpe_card_cleanup_oracle.py covers192 original cleanup paths and64 direct
window/callback cases. It includes both handle signs, A185C override, state9,
matching/mismatching selected index, supported/unknown callbacks, missing list,
active-window retention and no-active-window selection restoration. Explicit active flags and matching-index cases were added to ensure selection
restoration is exercised. A coverage assertion also incorrectly named a delay-slot
PC (4D664), which the reader does not add to visited_pcs; it now checks the
owning call instruction4D660. That was an audit-check error, not a retail bug.
Final validation is recorded in ACTIVE_HANDOFF.md.

Next formatter evidence:71A84..72308 is an executable SDK formatting routine,
not a BIOS trampoline. Original92224 points to10ED4 string
`bu%d0:BASLUS-00662000000%c%c`;92230 points to10F58 `bu 0:*`.
10F4C is `bu%ld0:%s`,10F60 is `bu%ld0:`. These formats and the original
formatter's signed/width/argument semantics need an original-execution audit.
Do not substitute host sprintf or invent transient guest stack addresses.

Initial native cleanup case8 matched RAM but failed known call arguments: the
sparse fixture omitted92224, so native supplied format pointer0 instead of the
original80010ED4. The fixture now copies that original word. This was fixture
input incompleteness, not a reason to alter cleanup semantics.

Formatter follow-up:71A84..72308 is545 words, SHA256
3eaf80663eeaeaa09560a87c5468b74afad3fa6ea1177d7a81b6f15eae4331e4.
Direct callees are72314(BIOS A0/1B),72324(BIOS A0/2E),72334 (overlap-aware
byte copy). Inventory saved locally inlocal/live/card-formatter-calls.json.

Final DAY1/DAY2-76:256 original/native cleanup cases PASS. Updated8192
operation and112driver/callback cases PASS; all header regeneration checks PASS.
Normal/sanitizer focused card suites PASS7 groups each,1270 skipped. Full normal
CTest8/8 PASS74.55s, including1277 native groups. Final builds have no warnings
or errors; Python/scoped whitespace checks PASS. All processes terminal.
Changes remain local; post-BIOS, live and whole Day1/Day2 acceptance unfinished.

## DAY1/DAY2-79: dispatcher with supplied guest call context

`PE_CardOperationFrame(index, caller_sp, incoming[32])` now preserves the41108
prologue's five saved-register writes and runs the shared operation implementation.
Its original frame iscaller_sp-78h: s0 at+60h, s1+64h, s2+68h, s3+6Ch, ra+70h.
Stores occur in source order s2,s0,ra,s3,s1. The existingfunc_80041108 entry remains
context-free and preserves its previously audited boundaries and cleanup paths.
The framed entry stops before40F80 on invalid flags, because cleanup's own frame
adapter has not yet been implemented; it does not invent callee stack effects.

For the supplied-context entry, directory enumeration now supplies the actual
frame+20h output argument. Formatting supplies frame as the formatter's caller
SP, and filename paths store selected+41h atframe+10h before calling it. The
formatter's saved-register inputs come from the exact dispatcher path:

| State | Formatter return address | Dispatcher s0/s1/s2/s3 at call | Next unresolved call |
| --- | --- | --- | --- |
|3|800416D8|record, incoming s1, index, incoming s3|72734(filename,1)|
|4|80041834|record,1,index,incoming s3|72734(filename,10200h)|
|5|80041908|record,1,index,incoming s3|72734(filename,1)|
|6|800419D4|record,1,index,incoming s3|72734(filename,2)|
|11|80041E98|record,incoming s1,record>A0ED4,1|72734(filename,1)|
|14|80041284|record,incoming s1,index,incoming s3|72784(frame+18h)|

s4..s7 retain their incoming values. State14 passes incominga3 as the formatter's
second spilled argument even though its device format consumes only index.
Filename formatting uses the live92224 format pointer, the live entry variant,
and the selected slot. Stop epochs prevent a failed/unresolved formatter from
falling through to file operations. File handles, operation completion, retries,
and other effects after those BIOS calls are not synthesized.

`pe_card_operation_frame_oracle.py` executes1024 original dispatcher calls at
two stack placements and compares record/global/output/stack memory plus the
first unresolved call and its known arguments. It uses the real filename/device
formats and original formatter execution, including state3 scanning, state4
variant writes, returning states and invalid-flag cleanup boundaries. The normal
and sanitizer focused runs pass both dispatcher groups (1278 unrelated groups
skipped). Exact fixture-header regeneration and Python compilation passed.

Parent-frame source trace:425DC allocates20h and calls41108 at42744, so incoming
ra is4274C, incoming s0 is slot index and s1 is the corresponding record pointer.
5C498 allocates18h and calls425DC at5C510. Thus, relative to5C498 entry SP, the
formatter caller SP is-B0h and its allocated frame begins-300h; the state14
device buffer is-98h. This relative identity does not establish an absolute live
SP or the preceding calls' scratch/register effects. Context propagation through
these native parents, cleanup40F80, and BIOS string/event/file implementation are
still required. This is not a live card-operation or full-day acceptance claim.

Stage79 full CTest passed all8 targets in73.43 seconds. Scoped whitespace
checks passed. No build or test process remains running at this checkpoint.

## DAY1/DAY2-80: cleanup with supplied guest call context

`PE_CardCleanupFrame` implements40F80's original50h frame through its first
unresolved callee. It readsA185C before frame writes, then stores s2 at+40h,
ra+48h,s3+44h,s1+3Ch,s0+38h. Override stops at42228; a nonnegative handle stops
at72774 before the handle becomes-1. A negative handle outside state9 computes
the original signed inverse-stride index and stops at4D5CC before its unbound
callee frame. Existing context-free cleanup continues to own its returning
window/callback behavior; the framed adapter does not erase those distinctions.

State9 with a negative handle establishes s0=index,s1=0,s2=record,s3=-1 and
calls the filename formatter with return41048 and fifth argument selected+41h.
After it returns, the adapter reads saved s0..s7 from the formatter's guest
frame before the next call. It formats `bu%ld0:%s` into cleanup-frame+18h using
s0 and filename9EE70, with return41064. The ordinary string conversion currently
stops at72314, leaving the original partial prefix and formatter stack effects.
No close/open/delete result, final NUL or cleanup suffix is fabricated.

The supplied-context dispatcher now calls this cleanup adapter for invalid
flags. It establishes incoming cleanup s0=record,s2=index,ra=42000; states4..10
also establish s1=flags, and state11 establishes s3=flags. Other saved registers
retain their incoming values. The original1/2/3/12 paths do not replace s1.

The source-pinned frame oracle now executes1280 original cases:1024 dispatcher
cases continuing into cleanup and256 direct cleanup entries. It includes two
stack placements, override/nonoverride, valid/negative handles, state9/non9,
real filename/device formats and the first unresolved call. Output, record,
global and full exercised stack regions are hashed. The72314 boundary test
checks its semantic pointer argument separately from the card-boundary payload.
This does not claim BIOS string semantics or absolute live stack propagation.

Remaining frame work includes42228/4D5CC and the driver/menu parents. BIOS string,
event and file authority remains open. Day2's station-scene predicates and route
expansion inDAY2_ROUTE_AUDIT also remain required; card helper progress does not
replace either day's complete inventory/decompilation acceptance.

Stage80 focused normal/sanitizer card checks passed8groups,1272skipped.
Original1280-case generation/header verification, Python compilation and scoped
whitespace checks passed. Both final builds were warning-free.

Stage80 full CTest passed8/8 in73.42s. All build/test sessions finished.
