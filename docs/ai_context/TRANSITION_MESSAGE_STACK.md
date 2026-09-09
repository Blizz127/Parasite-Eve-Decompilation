# Transition message lists and original stack history

Original801942FC calls375E0 with a list at its local `sp+0x88`, after
initializing only its first halfword to0 at80194318. The opener consumes up
to five signed halfwords, stopping early on-1. The other eight bytes have no
writer in942FC. Their history must come from the preceding outer-loop call
sequence; defaulting them to zero is not justified.

Original subtitle helper801958D4 similarly initializes only `sp+0x10` at
801958F8. In its sole original update caller, the immediately preceding
F55C sampler writes the remaining list bytes through saved registers:

| Subtitle list offset | Last original writer | Value source |
| --- | --- | --- |
| +0,+1 | 801958F8 | Zero |
| +2,+3 | 8018F578 | High halfword of update caller's s2 |
| +4,+5 | 8018F574 | Low halfword of update caller's s3 |
| +6,+7 | 8018F574 | High halfword of update caller's s3 |
| +8,+9 | 801958F0 | Low halfword of update's s0 (local rotation pointer) |

The original outer9234C sets s2=0, then1 after starting its effect handle.
It sets s3=0x00FFFFFF at80192448/8019244C. Mode10 in942FC preserves s2/s3;
thus the consumed list is `[0,0,-1]`. The last two values are not read.
The normal native942FC wrapper supplies this proven subtitle context, while
its menu-list tail remains unbound. It stops before opening a menu message
when those bytes are needed, after the original preceding close/color writes.
This is an explicit unfinished outer-loop dependency, not full integration.

`python3 pc_port/tools/pe_transition_stack_audit.py` observes original stores
and375E0 entries without replacing any instruction or callee. Eight executions
vary mode0/10, initial stack fill00/A5, and caller s2=0/1, with s3=00FFFFFF.
They confirm the subtitle last writers and the absence of menu-tail writers
within942FC. Output: ignored `local/live/transition-stack-audit.json`.
The initialized outer register values are also checked in38 complete original/
native mode10 update cases through the normal native942FC wrapper.

The explicit `PE_TransitionUpdate(menu_tail,subtitle_tail)` entry supports
comparison of arbitrary historical values. The original-execution fixtures
seed the menu bytes on the guest stack and supply non-retail arbitrary s2/s3
registers for most subtitle cases, making incorrect fixed-zero tails visible.
Native tests include missing-context prefixes and zero-period sampler prefixes.
Neither this input interface nor those tests establish the remaining menu-tail
history. Trace constructor/renderer/SDK stack writes through the actual9234C
sequence before binding the final menu context.

The complete update comparison also exposed a separate existing375E0 defect:
original37628..37680 ANDs flags withFFEFFFFF andFFDFFFFF (combinedFFCFFFFF).
Native code usedFFEEFFFF, incorrectly clearing bit16 and retaining bit21.
The corrected mask preserves bit16 and clears bits20/21 before mode3 restores
bit20. Negative decimal arguments also now use unsigned left shifts to preserve
MIPS low-word behavior without host-language undefined behavior.

DAY1-55 extends the audit backward into96498 with constructor-entry SP801FEFD0.
Under its seven explicit provider contracts, list bytes4..7 retain the initial
stack fill in all eight mode0/1/2/10, fill00/A5 cases. Bytes0..3 were last written
by96F8C and8..9 by96EF8. Those provider contracts do not model stack side effects;
these are observed constructor writers, not a final caller history. The outer
loop also exposes the partial native3EB04 input cut and void VSync shim as
remaining dependencies. See TRANSITION_OUTER_LOOP.md before outer integration.

DAY1-56 restores825C0/82680/828F4/8292C/82974 and real installed callbacks.
The outer stack audit now also executes twelve complete original3EB04 digital
paths without replacing input/SDK callees. State6 with flagsC000 calls828F4;
83BD0 saves8008291C at the first four menu-list bytes, making the retained second
halfword8008 instead of0000. Busy rejection happens after that stack write and
does not remove it. Other tested controller paths preserve the constructor's
96F8C writer. Bytes4..7 retain initial00/A5 in every case. Remaining constructor
provider and earlier-frame histories prevent binding a final production tail.

Actual bootstrap stack (DAY1-57 correction): original80012320..12344 stores the
previous SP at1F8003FC and calls9234C with SP1F8003F8. Its constructor/input call
SP is1F8003C8, so update's menu starts1F800390. The audit's --scratchpad mode
reruns all20 constructor/input cases at that location; writer assertions pass
under the same explicit constructor providers. Earlier1FEF98 observations are
relocated comparison fixtures, not evidence of actual absolute stack values.
Whole-frame provenance must also account for renderer/SDK scratchpad writes.
Evidence: local/live/transition-outer-scratchpad-audit.json.

DAY1-58 observes original VSync at SP1F8003C8 with264 clock-sequence cases.
Negative/mode1 queries do not write menu1F800390..399. Waits leave the watchdog
word via73BC4/73BF0 in bytes0..3 and saved return-address low halfword3B20 via
73BDC in bytes8..9. Bytes4..7 retain the initialA5 fill. Timeout can leave the
watchdogFFFFFFFF, changing the second list halfword to-1. These are original
source dataflow observations under explicit device sequences, not final live
menu-tail values. See VSYNC_CONTRACT.md and local/live/vsync-stack-audit.json.

DAY1-59 restores the original VBlank RNG/four-timer callback and connects CPU
source0 to checked7440C dispatch.512 original comparisons plus four IRQ scenarios
pass normally and under ASan/UBSan; full suite1261 groups, CTest8/8. Unknown or
stopped callbacks prevent subsequent slot execution. Timer1 MMIO, host VBlank
production and VSync adapter remain outstanding; no new live-frame/menu-tail
or full-day acceptance is established. See VSYNC_CONTRACT.md.
