# M0351I: closed Day 1 exit / Day 2 entry script regions

Authority: original Disc1 M0351I script, SHA256
1cc11664e59100e6378107b99ade037704e1917702182264e459c7d1172b7203.
Module1 starts8018F4C8. All persist indices below are decimal: story is74
(encoded4A), the calculation output is311 (encoded137). This document
semantically decompiles closed regions, not the entire shared scene.

Reproduce with `python3 pc_port/tools/pe_day2_entry_paths.py`. Original MIPS
handlers17294,1731C,12850,173F4,130B4,19DB8,19DF4 execute each command; a pointer-bank binding
fixture supplies argument addresses and the actor's module base. Unknown
opcodes/modes stop the audit. No asynchronous scene command is silently skipped.
The combined handler spans/checksum are pinned in the tool and output JSON.
Full per-case command paths and persist writes are saved in ignored
`local/live/day2-entry-paths.json`; proprietary script bytes stay local.

## Final selector:801910DC through transfer/yield boundary

```c
// Entered only after the preceding scene/poll sequence has reached801910DC.
if ((int32_t)persist[74] <= 0x80) {
    persist[1] = 0;             // 80191104
    persist[74] = 0x88;         // 80191114
    request_map("M0042I");      // 80191124: stop before handler in audit
} else if (persist[74] == 0x138) {
    persist[74] = 0x140;        // 80191158
    request_map("M0092I");      // 80191168: stop before handler in audit
} else {
    yield_for(1);              // 80191174: stop before handler in audit
    goto poll_at_8018F694;      // 80191180: module base + (0xE6 << 1)
}
```

The <= comparison is signed. It is not an equality test for80. All story
values0..300hex plus7FFFFFFF,80000000,FFFFFFFF were checked:772 original-handler
cases. Boundary values test arithmetic semantics, not membership in valid story
states. The M0042I branch clears persist1 before advancing story; the M0092I and
waiting branches retain persist1. Both destination commands are checked against
their original packed names and immediate operand modes.

This closes the final selector predicate formerly missing from DAY2_ROUTE_AUDIT.
It does not prove the earlier interaction/event conditions that reach it, actual
map loading, or M0042I's subsequent scene progression. The existing Day1 M0000I
selector proof supplies M0351I at story80; composing these endpoints still
requires the intervening scene and loader behavior.

## Day1 calculation:8018F7C8..8018FB88

The preceding story80 branch resets persist310, reads timer slot2 and calls
D3 to produce local13/local14/local15. The initial96 cases begin after those
calls with explicit local13/local14 inputs. DAY1-64 adds256 original-handler
chains beginning at8018F770, including the reset, D2, intervening copies and D3;
only the initial timer count remains a fixture input.
The49-command closed block computes a wrapped32-bit quantity
`time = local[13] * 60 + local[14]`, then assigns persist311:

| Signed time condition, tested in order | Assigned value before final writes |
| --- | --- |
| >100 | 100 |
| >70 | 700 - (time-70)*40 |
| >60 | 700 - (time-60)*30 |
| >50 | 700 - (time-50)*20 |
| >40 | 800 - (time-40)*10 |
| ==40 | 800 |
| <40 | 800 + (40-time)*5 |

ALU arithmetic wraps to32 bits; comparisons interpret those bits as signed.
After the branches rejoin at8018FB40, the script writes100 if persist311 is
signed-less-than100. Then8018FB78 **unconditionally writes800**. Control reaches
8018FB88, the next story comparison, with persist311=800 on every path through
this block. Preserve the intermediate instructions in a faithful port; do not
use the earlier formula as the final value or remove writes based only on the
final state.

Evidence:96 original-handler executions cover threshold-adjacent and signed
input values. Separately, the tool retains both edges of every branch, validates
all targets as forward command boundaries inside this region, and verifies that
removing8018FB78 makes the exit unreachable. That structural dominance check
proves every possible path to this exit includes the constant assignment,
without relying on sample coverage or assumptions about branch feasibility.

The broader module later uses persist311 as an argument to opcodeE9/resource45E
at80191038. Its game-facing meaning is not inferred here. Several later-story
calculation blocks also exist; they remain separate decompilation work.

## Remaining entry-scene work

M0351I has385 decoded commands across two modules. The asynchronous prologue,
other story branches, resource operations, message choice and fade operations still require
full semantic/call-graph evidence. M0042I and shared station rooms retain the
unresolved predicates/inventory in DAY2_ROUTE_AUDIT. Neither whole-day coverage
nor native/live transition acceptance is established by these closed regions.

## DAY1-64: timer command implementation and earlier chain entry

Original D2/D3 handlers19DB8..19F04 comprise83 words, SHA256
f684e1f70afe44e8a2195316e020111fecf4c6ad3f99e2e1e3b0ad297662effe.
Native owner is game/boot/func_80019DB8_port.c. D2 reads the word at
A76A4+12*index without imposing a new four-slot bound. D3 now has a real VM
dispatch arm; it previously reached the unported-command boundary.

D3's signed multiply-high sequences implement truncation toward zero:

```c
*out0 = read_input_again() / 216000;
*out1 = (read_input_again() % 216000) / 3600;
*out2 = (read_input_again() % 3600) / 60;
```

With a60-tick-per-second count these represent hours, minutes and seconds.
The helper establishes conversion factors, not host timing accuracy. It reloads
the input pointer/value after each output store and reloads destination pointers;
aliased arguments can therefore change subsequent results. The native port
preserves that order. `pe_script_timer_oracle.py` compares2048 original-handler
D2/D3 cases, including signed limits, division boundaries and overlapping input,
output and already-consumed pointer slots. A native VM test executes D3 and then
the next opcode, proving the dispatch path is connected.

The expanded entry audit executes256 timer-to-calculation chains at8018F770
through8018FB88. Original instructions read slot2, fill/copy locals, convert
components and run the49-command calculation before the final800 write. This
closes the earlier D2/D3 fixture boundary for those chains. It does not prove
when the live timer advances or the asynchronous gates before8018F770.

## DAY1/DAY2-65: input and persistent-state gates

The poll at8018F694 uses opcode11 with selector1, mask0x100 and local2 as
its output. Original handler130B4 reads8009D1F4 for selector1, returning
`(pressed & mask) == mask`. This is input polling; the earlier tentative
"mailbox" description was incorrect. Input routine3EB04 derives8009D1F4 as
`(current_held ^ previous_held) & current_held`, after input filtering. This
identifies newly pressed bits without assigning a physical button name here.

Ordered control flow, with decimal persistent indices:

```c
local[2] = (pressed & 0x100) == 0x100; // 8018F694
if (local[2] == 0) goto wait_at_80191174;
if (persist[1] == 0x3E6) goto fade_at_801910C8;
if (persist[8] == 0) goto message_at_80191054;
if ((int32_t)persist[8] <= 0) goto story_block_at_80190BC8;
if (persist[74] != 0x80) goto story_block_at_8018FB88;
// 8018F770: reset persist310, read/convert timer2, calculate persist311.
```

The fourth test only receives nonzero persist8, so its taken path consists of
negative signed values. Ordering matters: persist1==3E6 bypasses all later
gates even if persist8 is zero or negative. No persistent assignments occur
within this gate region. The names of exit labels describe the next decoded
operation/block; the gate audit stops before those operations execute.

Evidence:1,050 original-handler cases combine seven pressed masks, five
persist1 values, six persist8 values and five story values. They verify all
six exits, signed extremes, overlapping pressed bits and persistent-state
preservation. Handler130B4's complete77-word source span130B4..131E8 is pinned
with SHA25641ee2a1659cb0f8d1905ad71fb9f8b99408d6647c1a55127e789122ae7e25bbd.
These gate cases use only selector1; they do not validate selector3's native
implementation. An additional256 chains start at8018F694 with pressed100,
persist1=0,persist8=1,story80 and varied timer values. Their eleven-command
gate prefix reaches the existing timer/calculation chain, with identical
subsequent command traces, components and persistent writes.

The pointer-bank fixture and supplied input/state/timer values remain explicit
boundaries. Actual frame input production, asynchronous prologue, message/fade
execution and subsequent map transfer are not proved by this audit.

DAY1/DAY2-66 restores selector3 of the shared opcode11 handler, including its
mask-word overwrite, held-counter lookup and GTE LZCS/LZCR side effects.
The entry gate here uses selector1 and retains the same proven behavior.
See INPUT_QUERY_CONTRACT.md for the full handler semantics and2560-case
original/native comparison. All entry-path checks were rerun successfully.

DAY1/DAY2-67 closes a bounded fade-start/tick/wait sequence for the85(60)/9C
commands at801910C8/801910D4, and the prologue86(60) command. The original
wait rewinds next-PC by8 and yields with task delay1 until fade state is0/1.
512 original ticks and16 sequences verify packet/timer state at1056 checkpoints;
60 updates release the60-duration wait. Native interpolation now wraps the
32-bit product before signed division. See FADE_WAIT_CONTRACT.md for the
scheduler/OT fixtures; full asynchronous scene execution remains open.

DAY1/DAY2-68 restores E7 at801910C0 and its menu36 constructor. The first
eligible call sets task bit20, delay1 and rewinds8; the retry opens the menu,
sets scene/input flags, clears task bit20 and continues. A scene1000 flag
bypasses initialization.128 original/native cases verify full constructor
state, including saved cursor2 normalization and existing/new help windows.
See EXIT_MENU_CONTRACT.md. The constructed menu's callbacks remain unported,
so this does not establish the complete interactive path into the fade.

DAY1/DAY2-69 restores menu36's list/row drawing and enabled-option predicate.
64 original drawing graphs and1024 predicate cases match native/sanitizer
checks, including packet-buffer exhaustion and enabled masks. Options0/1 read
record bit0; option2 is always enabled. Input callback4D2DC remains unported,
so the complete interactive path into the exit fade still requires work.
See EXIT_MENU_CONTRACT.md for source spans, fixtures and limitations.
