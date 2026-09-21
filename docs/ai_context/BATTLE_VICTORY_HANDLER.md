# Original encounter-victory handler

The native `8002B0E8..8002B29C` handler now calls the translated effect cleanup
`703F4` before reward creation and `67CBC` after it, matching the original
instruction order. Phase3 calls the existing full `295E4` translation, restoring
HUD colors, actor teardown/status updates and inventory-bank setup previously
omitted by a local tail-only helper. The partial helper remains used elsewhere;
this change does not certify the player-death handler.

The phase2 comparison retains the full unsigned halfword at actor+1A instead
of narrowing it to a byte. Battle flags come from the original RAM word rather
than OR-ing in a separate host shadow. Phases0 and2 reload/increment the phase
byte after callees, as the original does. Physical RAM accesses, including a
zero actor address, use the native cached alias; `295E4`'s store at actor+194
was similarly normalized. The original109-word span has SHA-256
`9eabc01299179e888714b69f69487bac38ecd50f1f1e40f6ddcb8e632bfefd67`.

## Independent verification

```
python3 pc_port/tools/pe_battle_victory_oracle.py --compare-native \
  --build-dir /tmp/pe-day2-release --write-header \
  --capture /tmp/pe-level-forward-connected.bin
```

**83 original/native cases pass**, plus the current phase from the previous
connected junction capture, totaling **1,470 executed instruction PCs**.
Every executed instruction and its following/delay word is checked against
the source EXE. Native execution matches all RAM below `1FE000`, and generated
ranges cover every original write there. Only the original execution stack is
excluded. No callee is replaced by a made-up oracle contract.

Cases cover every phase and unused phase values, the phase0 command/flag gate,
dormant and active effects in both pools, independent pool errors, confirmation
values around1000, animation halfwords0/1/255/256/257/65535, alternate command
flags, music completion/wait states and physical actor RAM. Fixtures provide
real translated destructor callbacks and resources. The original cleanup and
reward/menu callees execute fully. Generated regression files are
`retail_battle_victory_cases.h` and `test_battle_victory.h`.
Log: `/tmp/pe-victory-final-oracle.log`.

The initial comparison passed the ordinary cases but its physical phase1 case
exposed a missing observed RAM range in the test generator. Extending the
range through the actor fields and adding physical phase3 produced the final
passing83-case run. This was a fixture-coverage correction, not an omitted
production store.

## Connected continuation

The opt-in ordinary-input controller now continues from m31 through the left
exit to m32. In m32 it selects the carried pistol via the weapon menu while
reserve ammunition remains, attacks, moves, and requests Heal1 normally.
The original room scripts determine navigation; no gameplay RAM is injected.

```
PE_ROUTE_REWARD_PILOT=1 PE_ROUTE_FRAMES=66000 \
PE_ROUTE_BATTLE_DUMP_BEGIN=49000 \
PE_ROUTE_RAM_DUMP=/tmp/pe-victory-forward-connected.bin \
/tmp/pe-day2-release/pe-route-boot-day2-tests > /tmp/pe-victory-forward-connected.log 2>&1
```

Release and Debug builds pass. The old mode2 unit tests now initialize real
empty effect pools, since cleanup is no longer skipped. Their earlier failure
was a missing-resource fixture at `BTL103_2b0e8_phase0_advance`; initialized
pools restore the intended test conditions. The affected BTL10/BTL11 groups
pass separately, and the final broad run passes **1,392/1,392 native tests**
and **10/10 CTest checks excluding the old route** (69.82 seconds, native67.47).
Logs: `/tmp/pe-victory-verified-{tests-build,ctest,lasttest}.log` and
`/tmp/pe-victory-debug-test-build.log`.

The first fresh66,000-frame run repeats both victories and reaches m31 at
54,520 without unresolved boundaries, endingHP33/maxHP53,XP16,internallevel2,
BP9,reserve36. Its normal navigation inputs do not advance the first-visit
conversation: actor2 waits at original script `8019E600` (opcode22, message10).
Capture `/tmp/pe-victory-forward-connected.bin`, SHA-256
`f2729282269ffcca53938116d3996ccaa6b0b817e4a7229a349c7d65de0c17a4`.
A second controller run centers before walking but reaches the same dialogue
wait. The pilot now emits normal Cross pulses during junction navigation and
handles both directions when moving to a remembered weapon-menu row.
The dialogue run completes message10/11 and receives control eventFF, moving
Aya to x35 before a controller-only threshold mismatch stalls it: centering
stopped within40 units while its next stage required30. Capture
`/tmp/pe-victory-dialogue-connected.bin`, SHA-256
`680fb6bdfa67f87f871b97fff4aa82672f12cd05b9bb174d55449affdfa06fd6`.
The next navigation run reaches x18,z967, exposing the same inconsistency
in its forward waypoint: stopping within40 of1000 cannot satisfy z980.
Explicit movement stages now consistently use40-unit arrival tolerances.
The resulting `/tmp/pe-victory-waypoints-connected.{log,bin}` reaches M32
at frame55,479, enters its encounter at55,831, equips the pistol normally at
56,021, and heals21->51 at56,336 and11->41 at56,863. Aya dies at57,189 and
restarts at57,398. This is a connected M32 arrival, not a third victory.
No unresolved native boundary appears. The old optional supply milestones
remain unmet (50/57). Its endpoint capture is after the restart, SHA-256
`c04f4b1efb78eaa4a1a7de9eedafff7a4a003ec0f8614f478e01681e497a750f`.
The next replay, `/tmp/pe-victory-ranged-connected.{log,bin}`, uses longer
ranged spacing and reports actual living battle bodies instead of selecting
an unrelated type2 script actor. Aya dies at56,943 near(-16280,3060), with
both type6 enemies alive at16/34HP. Its endpoint capture is after restart,
SHA-256 `99af644442b7875849fa095b27ef3e0cd89df241ee3a1920d87cc27704f9e2aa`.
The next controller follows the nearest living enemy, keeps spacing within
the pistol's observed1,071-unit range, and turns inward near room boundaries.
Replay `/tmp/pe-victory-nearest-connected.{log,bin}` pauses from56,199 through
the62,000-frame cap with living Aya25HP and both enemies34HP. Its capture
shows the controller confirming an Items submenu (focusID3), instead of PE.
SHA-256 `5fca56497e4bcc2b101123d0bda76447782680482a785bea32fe85cb99689589`.
M32 healing now selects main-menu row1 and PE-list8 row0/column0 explicitly,
backs out of unrelated item/equipment submenus, and waits on transient focus.
The next coldboot `/tmp/pe-victory-heal-menu-connected.{log,bin}` backs out
of Items, selects PE, and waits at the original confirmation list41 from
56,291 through the62,000-frame cap. The controller now confirms that dialog
when its original callback is `80046DBC`, selecting the first column (Yes).
Replay `/tmp/pe-victory-heal-confirm-connected.log` shows both enemies
defeated by57,631, victory mode2 at57,658 with35HP, and resumed field movement
by57,840. Navigation reaches(-16362,3590), before the closed exit gate.
The final RAM write failed because `/tmp` hit its quota; that zero-byte
capture is invalid, and the log has gaps. Eight earlier complete captures
were copied with hash verification to `pc_port/build/day2-victory-evidence`,
preserving their `/tmp` paths as symlinks and freeing16MB of temporary space.

The original M32 module3 tests the control rectangle x[-17050,-16900],
z[3400,3650], Cross and facing1500..2600. Message13 choice0 sends actor0
event0; its asynchronous `8019BB14` path sets persist22 bit1 and activates
the gate polygons. The next controller visits that control before the exit.
It also explicitly observes the third victory using two spawned type6
bodies, both retired, living Aya, mode9 and field control. The replay
`/tmp/pe-victory-gate-connected.log` verifies this at **57,791, HP35**.
Its complete2MB capture is
`pc_port/build/day2-victory-evidence/pe-victory-gate-connected.bin`, SHA-256
`c9c4612dfd6125e4c2b1ad5fa1e1fda6abf13ac73f237b7a02d3e423debe18cc`.
The64,000-frame endpoint has HP35/maxHP64, XP24, internallevel3 (display4),
BP28, reserve36, loaded4, pistolslot0, flags4080, mode9 and no menu focus.
Only Aya retains a body in the actor list. There is no unresolved native
callback; the old optional supply milestones still leave the check at50/57.

The gate remains closed (persist22=6, persist2B=2). Aya reaches approximately
(-16866,2097,3548), heading1536, just outside the control's rectangle. The
controller's westward approach is constrained there; the next step is to
derive a valid approach from the original floor geometry and script. M33
has not been reached. The log's rotation field incorrectly read unused+64;
that read-only diagnostic now uses the original heading halfword at+3A.
Final Release/Debug route builds pass; all replay processes have finished.
The original m31 module2 resumes after message10, then waits for message11
before publishing the control-release eventFF. This is why directional input
alone cannot advance the route.
Full Day2 and whole-route retail fidelity remain unproved.
