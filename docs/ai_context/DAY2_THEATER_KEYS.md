# Connected theater key traversal

The input-only cold-boot route collects the first theater key, item `C8`,
in m0020i. At frame 26500, inventory slot 5 at `800C0E52` contains `00C8`
and persist[24] at `800A7850` is `01000220`. Story remains `48`.
Capture/log: `/tmp/pe-key-second-contact.{bin,log}`. This is still Day 1;
it does not establish complete Day 2 traversal or full game fidelity.

The original scripts distinguish two contacts. The initial NPC contact
sets persist[24] bit `200` at `801A1758..801A1770`. After its animation, a
second contact plus Cross and the heading gate at `801A1850..801A1898`
sends Aya payload 3. That path awards C8 through A7 at `801A0BA4` and opens
the E8 pickup window at `801A0BDC`, then sets bit `20` at
`801A0BE8..801A0C00`. Both contacts and the inventory write occur through
the translated engine and unmodified room scripts.

The original heading gate accepts an angle below `200` or above `E00`,
meaning within 45 degrees of the target. Simply turning near the first
contact point is insufficient: the animated NPC collision center moves
from `(237,-300,260)` before the dialogue to `(386,-40,40)` afterward,
while its actor origin remains `(179,0,241)`. The second approach must
reach that collision center and satisfy the heading relative to the origin.

The previous route prefix ends at frame 22500 in m0020i. Append these
raw-pad pairs; the existing periodic Cross pulse continues throughout:

```text
22500:FFBF,22540:FF7F,22670:FFEF,22730:FFDF,22800:FFEF,
23000:FFDF,23010:FFFF,
24500:FFBF,24540:FF7F,24548:FFBF,24556:FFDF,24588:FFBF,
24596:FFDF,24652:FFEF,24700:FFFF
```

The lower aisle must be crossed far enough before turning toward the NPC.
Earlier attempts turned at `(113,-212)` and slid back along the obstacle.
The successful first approach passes `(-78,-229)`, then approaches from the
left. The return through the aisle reaches the second collision position
from below. No position, inventory, persistence, script commands or save
state is injected into the connected run.

To check whether the collision itself differed from retail,
`pc_port/tools/pe_live_floor_compare.py` builds a temporary native caller
against the selected runtime library and compares it with original MIPS
execution on local captures. It checks the executable SHA-1 and verifies
every executed instruction against that executable. Eight five-unit
directions, with cached-wall state retained or cleared, passed for each of
five captures: **80 cases**, every byte below RAM offset `1FE000` identical.
The top 8 KiB is excluded because it contains the oracle scratch stack;
GTE state, rendering and timing are not compared. No runtime collision
change was needed for these cases.

```sh
python3 pc_port/tools/pe_live_floor_compare.py --build-dir /tmp/pe-day2-release \
  /tmp/pe-m0020-approach.bin /tmp/pe-m0020-key.bin \
  /tmp/pe-m0020-key2.bin /tmp/pe-m0020-key3.bin /tmp/pe-m0020-key4.bin
```

Log: `/tmp/pe-live-floor-verified.log`. Captures remain local game data.
This diagnostic comparison does not replay them into the route harness.

The connected route also collects C9 in m0018i. It enters that room at
**30356**, after opening its hallway door at **30293**. The diary advances
story to `54` at **33565** and sets the C9/rehearsal-key flag at **33596**.
Original award PCs are `8019D8E8` (C9), `8019D8F4..8019D90C` (flag `80000`),
and `8019D834` (story). That flag opens m0012i's rehearsal-room door to m0319i.

The first 35000-frame attempt continued sending Cross and reopened the
diary repeatedly. Its animation was progressing normally. The harness
now stops periodic Cross at **33620**, after the key pickup, using
`PE_ROUTE_PULSE_END`. `PE_ROUTE_PULSE_RESUME` can resume those pulses for
subsequent dialogue or combat; its default is INT_MAX. This changes only controller input. At **35000**,
Aya's script task is finished, her flags are `8`, D1A0 is `4080`, and both
C8/C9 remain in inventory slots 5/6. Story is `54`, persist[1] is `12`,
persist[24] is `010C0220`, and m0018i module 3 is at `8019E374`.
Capture/log: `/tmp/pe-both-keys.{bin,log}`. Both item awards and the final
return of player control are asserted by the updated regression.

The previous harness default was **35000 frames**, **50 input pairs**, and
**34 observed milestones**, with a 600-second timeout for the longer Debug
run. The optional input sequence now supports 256 pairs; diagnostics record
persist[24] and the current key rooms. Both Debug and Release binaries were
rebuilt. Final Release CTest **PASS**, **91.16 seconds**:

```sh
source /tmp/pe-tools/env.sh
ctest --test-dir /tmp/pe-day2-release -R '^route-boot-day2-control-flow$' --output-on-failure
```

Log: `/tmp/pe-keys-final2-ctest.log`. No production runtime or native unit
tests changed this turn; the preceding **1380/1380 native** and **10/10 full
suite** results remain the production checks. The matching EXE is unchanged.
This verifies the theater-key traversal, not complete Day 2 or hardware,
audio, rendering and timing fidelity. Four documented HOST_ADAPTED
movie/menu skip functions remain.

A further connected 42000-frame probe exits m0018i, confirms the rehearsal-door
message after resuming Cross at 39000, and enters **m0319i at 39101**. At 42000,
Aya is at `(-120,2855)`, story `54`, persist[1]=17, persist[24]=`010C0260`.
Log/capture: `/tmp/pe-sewer-resume.{log,bin}`. This is beyond the current
35000-frame regression endpoint; its expected old-frontier assertion fails
while all 34 milestone observations pass. The next story-gated step is the
m0023i encounter. Subsequent entry, defeat diagnostics and ordinary healing
are recorded in [DAY2_REHEARSAL_ROUTE.md](DAY2_REHEARSAL_ROUTE.md).

The current 36500-frame regression additionally verifies ordinary healing
and the cabinet ammunition pickup; see [rehearsal route evidence](DAY2_REHEARSAL_ROUTE.md).
