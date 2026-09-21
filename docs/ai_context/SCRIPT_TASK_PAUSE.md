# Original script task suspension and resumption

M33's opening scene reached an explicit native boundary at frame58,549:
opcode5F at `801CDAFC`, handler `80018300`. The two original handlers are now
translated in `func_80017018_port.c` and wired into the VM dispatcher:

| Handler | Original span | Words | Behavior |
|---|---|---:|---|
| `80018300` / opcode5F | `[18300,18364)` | 25 | Set flag40 on other tasks in all three actor chains, excluding the current task |
| `80018364` / opcode60 | `[18364,183E8)` | 33 | Clear flag40; when a saved PC exists, restore it, set delay1 and clear flag20 |

Resume clears flag40 even when the saved PC is zero. It preserves other
flags and fields. Physical RAM accesses, including actor address zero, use
the native cached alias without changing pointer comparisons. Neither
original routine calls another function. The58-word source span has SHA-256
`605f5b21d57821f8f56477f5bdf4338132ab895d9345735a23e9b2dcb4527ca0`.
Matching sources, executable layout and the index were not changed.

## Independent comparison

```sh
python3 pc_port/tools/pe_script_task_pause_oracle.py \
  --build-dir /tmp/pe-day2-release \
  --capture pc_port/build/day2-victory-evidence/pe-m32-control-connected.bin \
  --write-header
```

All **518 original/native cases pass**, plus an isolated suspend/resume
sequence reconstructed from the reached M33 task. The run executes143 unique
instruction PCs. It verifies every executed instruction and following word
against the original EXE and compares return values and all RAM below1FE000.
Only the original execution stack is excluded. Generated fixture ranges
cover every original write in the compared RAM.

Cases include empty and populated chains, different current tasks, flags
0/20/40/60/FFFF, saved and absent PCs, physical actor zero, and full VM
dispatch followed by the original wait opcode. The native VM argument bank
is preseeded with its final argument value, so it remains part of the full
RAM comparison. No callee contract substitutes for original execution.

The first run passed all synthetic cases but failed its capture lookup,
which incorrectly assumed the active task was in chainA0. The corrected
lookup walks all three chains and finds M33's active task in chainA8.
Final log: `/tmp/pe-script-pause-final-oracle.log`. Generated regression:
`retail_task_pause_cases.h`, exercised by `test_task_pause.h` in the native
suite. Release and Debug builds pass. The broad regression passes all
**1,393 native tests** and **10/10 CTest checks excluding the old route**
(71.72seconds, native69.66). Logs: `/tmp/pe-script-pause-ctest.log` and
`/tmp/pe-script-pause-lasttest.log`.

## Connected route

The original M32 floor routine matches native execution on16 captured
five-unit steps, with cached walls both retained and cleared. All compared
RAM and original instruction words match. Log:
`/tmp/pe-m32-gate-floor-compare.log`. The floor geometry requires approaching
the gate control from the south: cross west atz3300, then move north within
x[-17050,-16900]. This controller change opens the original gate and enters
M33 at58,538 without modifying gameplay RAM.

The pre-port boundary capture is
`pc_port/build/day2-victory-evidence/pe-m32-control-connected.bin`, SHA-256
`f79b6d7b0f4d5b78b9bf1d6cb3ec01b447d46c37fafc28f1f230cfc0b140a5f9`.

The fresh post-port coldboot passes the former boundary, completes the
opening dialogue and reaches its66,000-frame cap with no unresolved native
callback. Aya remains at the entrance because the controller emits only
Cross in M33. The original module3 rectangle x[-4000,-530],z[-900,900]
triggers the next scene. The next replay adds normal movement toward
(-1000,0), while retaining confirmation pulses.

Post-port capture: `pc_port/build/day2-victory-evidence/pe-script-pause-connected.bin`,
SHA-256 `c5ce742fe02ba8cf91a8a06a9256e04138f2ded44e356e2af9cc2695c3922348`.
The next replay, `/tmp/pe-m33-forward-connected.log`, crosses the original
trigger at approximately(-536,219), completes the scene and enters the M34
boss encounter. It stops at **60,096**, story6C/arrival21/tokenA8003248,
at the next explicit native boundary: opcodeD1, handler`80019D84`, script
`801B5E0C`, actor`800BF490`. All three earlier victories repeat unchanged.
The complete2MB capture is
`pc_port/build/day2-victory-evidence/pe-m33-forward-connected.bin`, SHA-256
`2d654c4dd8f000bbdc9b3cbc51f9fa02245229c86f968f25e736e0e4dbc33e70`.

The next missing handler is the original13-word `[19D84,19DB8)` wrapper:
read the signed message ID and call the existing `375E0` with mode1 and a
single signed-1 list terminator. It follows D0's configured window geometry
and differs from the already ported ordinary message-open opcode0D's mode0.
It has been inspected but is not yet translated or verified. All builds,
regression checks and connected replay processes from this turn are terminal.
Full Day2 and whole-route retail fidelity remain unproved.
