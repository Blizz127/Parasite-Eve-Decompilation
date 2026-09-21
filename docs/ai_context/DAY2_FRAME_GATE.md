# Retail frame overlay gate (2026-09-12)

Original `800355C8` branches to `80035C04` when `800B0CD8 & 0x200`.
The native `func_80035558_walk_cut` previously applied this condition only
to field-menu input. Battle processing, floor collision, camera/model work,
resource ticks and animation still ran. The gate now skips that entire
middle body, after actor callbacks and before final contact/task cleanup.
The final pause-bit test remains in place. No matching `src/` or manifest
inputs changed.

`pe_frame_gate_oracle.py` runs 32 complete original call graphs without
mocked callees, using synthetic actor and task state. Cases combine battle,
pause and command flags, an absent/present actor, and an optional retired
task. They verify the original branch reaches 35C04, omits the battle and
floor sites, and records complete persistent guest RAM effects in the tested
ranges. `FGATE_retail_overlay_pause` compares the native bytes against those
results. Both the original oracle and focused native test pass.

Commands:

```
python3 pc_port/tools/pe_frame_gate_oracle.py --check
source /tmp/pe-tools/env.sh
cmake --build pc_port/build -j6
PE_TEST_FILTER=FGATE pc_port/build/pe-native-tests
ctest --test-dir pc_port/build --output-on-failure
```

Logs: `/tmp/pe-frame-gate-{build,focused,ctest}.log`.

A separate connected input probe released the held direction at frame 11000
and continued periodic Cross to frame 18000. It returned to the title at
frame 12481 and restarted the opening route; it did not complete the battle.
Releasing movement at frame 10000 instead completed the encounter: original
script threshold 1000000 was crossed at frame 11432 (enemy HP 999998), followed
by battle modes 6 at 11433, 7 at 11442 and 8 at 11588. Aya had 28 HP. The route
entered m0367i at frame 11913 (story 26, persist[1]=5), returned to m0005i at
12205 (story 27), then reached story 28 at 12417. At frame 14000, battle bit 2
and D244 were cleared and module 6 waited at 801B25E0. This extends the
connected route past the encounter; it still does not prove Day 2 completion.
The HP/AT/command trace used a temporary present hook under `/tmp`.
Log `/tmp/pe-battle-stats.log` exited 1 solely because it still asserted the
previous active-battle frontier.
These probes write no story, actor, command or position state.

## Connected m0011i endpoint

The input continuation is now:
`8990:FFDF,9140:FFEF,9600:FF7F,10000:FFFF,14000:FF7F,14386:FFFF,15000:FFDF`.
It reaches m0009i at **14386**, then crosses the hole polygon at 801930AC.
The existing periodic Cross accepts the dialogue's first choice. Story becomes
30 at **15373**, the script transfers to **m0011i at 15434**, and its arrival
event advances story to **38 at 16039**. At **17500**, module 3 waits at
**801A097C**, token **A80010C8**, persist[1]=9, Aya **800BED10** at x407,z125,
battle flag 2 clear and D244=0. Default regression now requires these room/PC/
battle conditions and **23 observed milestones**. No game state is injected.

Full native CTest passed **10/10**, **1379/1379 native tests**, zero skips.
That full run used the preceding 11000-frame route. The intermediate
15000-frame m0009i Debug route CTest also passed in 128.50s. The final
17500-frame regression is built in both configurations and **passes in
Release (46.87s)**, log `/tmp/pe-m0011-route-ctest.log`. Its final output is:

```
route: frames=17500 stop=frame-limit story=0x00000038 persist1=0x00000009 token=0xA80010C8
route: 23/23 milestones observed
PASS: boot -> m0005i battle -> m0367i -> m0009i -> m0011i route (23 milestones, frontier=m0011i mod3 pc=0x801A097C, battle cleared)
```

The original candidate remains byte-identical by cmp and SHA-1:
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

The next input gate is m0011i module 3's 801A0800 rectangle
(x=-442..412,z=1111..2959), leading to m0012i. From the current x407,z125,
FFEF produces +z under the identity camera matrix; the next run must test
that approach against the walkmesh and scripts. This is still Day 1.
