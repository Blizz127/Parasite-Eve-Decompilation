# Shared transition fade and script wait

DAY1/DAY2-67 audits the original85/86 fade-start commands, their callees,
68E24 packet/timer tick and9C wait. M0351I uses86(60) at8018F5B8, then
85(60) at801910C8 followed by9C at801910D4. The audit checks these decoded
commands against the pinned original script, rather than inferring their
operands from native tests.

85/18EB4 reads an unsigned halfword duration and calls66B60. This snapshots
CFE8/EA/EC into CFF0/F2/F4, sets target components to255, CFEE=2,CFEF=2,
CFF6=duration,CFF8=0. 86/18EE0 calls66C7C, which snapshots the same components,
sets target components to0, CFEE=6, duration and timer, retaining CFEF.
Addresses in this paragraph have800B prefix.

68E24 returns without drawing when CFEE&3 is0. For state2 it computes each
packet color byte as follows (source and target components are signed16-bit):

```c
denominator = max((unsigned short)duration - 1, 1);
product_bits = wrap32((target - source) * (unsigned short)timer);
color_byte = low8(source + signed32(product_bits) / denominator);
```

Division truncates toward zero. Original mult/mflo retains the low32 bits
before division. The earlier C port used a signed32-bit multiplication that
could overflow; it now explicitly wraps the product and interprets it as signed
using64-bit arithmetic. The denominator is always positive, so the original
compiler's division traps are unreachable; removing the host fallback does not
remove a reachable original trap.

Other nonzero low states copy target color bytes and do not advance the timer.
The tick links the color packet and draw-mode packet into the current buffer's
ordering table, preserving tag high bytes and24-bit addresses. In state2 it
then increments the16-bit timer. On timer>=duration, CFEE becomes0 when its
original mask0x04 was set, otherwise1. TimerFFFF wraps to0 before comparison.

9C/19410 returns1 when CFEE&3 is0 or1. Otherwise it subtracts8 from global
next-PC8009CE00, writes1 to current task+10 and returns0. Thus the next VM
attempt retries9C. For a60-tick fade,60 completed fade updates allow this wait
to resume; a zero or one duration still requires one tick after initialization.
This counts calls to the fade updater, not elapsed wall-clock frames.

Evidence: `pe_fade_wait_oracle.py` executes512 full original ticks across states,
both display buffers, duration/timer extremes and signed color extremes. It
also executes16 start/wait sequences for both commands, durations0/1/2/60 and
both initial buffers, checking each wait and tick:1056 total RAM/return
checkpoints. Native `DAY1_fade_wait` compares all checkpoints, including packet
links, color bytes, timer state, next-PC rewind and task delay.

Each sequence supplies resolved argument pointers and a current task; before
each draw tick it supplies an empty OT head and alternates display buffers.
This is a scheduler/presentation fixture. It does not execute the complete
field scheduler, draw the packet chain, prove real frame timing, or execute the
message/resource operations between the M0351I prologue and final selector.
Field3F3C4 calls68E24 at3F588 only on its eligible draw path; skipped draw paths
and the unported overlay message path require separate integration evidence.

All original spans are checksum-pinned in the oracle:18EB4..18F0C (both command
wrappers),19410..19450,66B60..66BD8,66C7C..66CE8 and68E24..6914C. Proprietary
script/executable data remains local; the checked-in header stores hashes and
return values only.

DAY1/DAY2-67 final validation: normal and sanitizer focused tests pass; full
CTest8/8 passes in67.33s (1267 native groups). Oracle regeneration check, Python
compilation and scoped whitespace checks pass. Both builds have no compiler
warnings/errors.
