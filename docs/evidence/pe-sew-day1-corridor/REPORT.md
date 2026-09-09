# SEW1–SEW17: Day 1 ordinary-input replay past the Eve encounter (2026-09-05)

Goal set by the user: retail accuracy from New Game into the sewers. This
session drove the visible native port forward with ordinary keyboard input
only, stopped at each native boundary, restored the original code, and
re-ran. No guest RAM writes were used for any proof; every value below was
read from the inspector's snapshots.

## Tooling (all in-repo now; a host reboot wiped the previous /tmp helpers)

| Tool | Purpose |
| --- | --- |
| `pc_port/tools/pe_live_gdb.py` | read-only gdb inspector: RAM/RGB/VRAM snapshots, stop snapshot, optional hardware watchpoints, per-task tracer |
| `pc_port/tools/pe_live_drive.py` | xdotool keys, state readback, PNG/VRAM conversion, position-feedback `walk_to` |
| `pc_port/tools/pe_live_replay.py` | phased replay: opening → naming → courtyard → lobby → auditorium → aisle → Eve battle → stage exit → backstage hole → under-stage → corridor |
| `tools/pe_retail_replay.lua` | PCSX-Redux retail replay (blocked by an emulator segfault after the STR skip; see ACTIVE_HANDOFF) |

## Findings and fixes (func_80017018 script VM unless noted)

1. **Unported opcodes silently suspended tasks.** The VM fallthrough returned
   0 without rescheduling; a task left with delay 0 never ran again. Eve's
   under-stage script parked at 801A074C on opcode 5D while Aya polled a
   shared word Eve never set. Every unported table slot is now an explicit
   `PE_PORT_STOP_UNRESOLVED_BOUNDARY` with the opcode PC retained and a
   `[VM] unported script opcode` line.
2. **Opcodes wired from matching `src/` C or retail words:** 5C, 5D, 93 (SEW1);
   F0, 19, 25, 26, 27, 29, 2B, 2C, 3E, 46, 47, 49, 4D, 69, 8A, 4C/8D/8E/90,
   99, CA, 37/39/42/50/78/B9/BA/C2/C8/C9/BE/C0 (SEW2); 23, 72, 7C, 7D, 7E, 7F,
   B4, B5, BB, BC, D0 with native 659F8/65A60/65A9C/67678/676CC/67730/3746C/
   37454/375B4/375C4 (SEW3, `func_800659F8_port.c`); CB (SEW4); 6D/6E with
   6F820 (SEW5); AF with 1ACE0 (SEW6); 45 (SEW7); 18, 92, A5, C3, DB, 48 with
   665A0 (SEW8, `func_80013988_port.c`); BF, D2 (SEW11); the M0013I room
   command callback 8018FC54 keyed on its code words, plus D4698 forwarding
   retail's fourth argument (SEW12).
3. **Retail gives the player control at the top of the aisle** after the
   performance (the earlier automatic run down the aisle was an artifact of
   the silent-yield path). The Eve encounter is a walk-in region (x −820…995,
   z −202…2328, gated by 0x11 ≤ g74 < 0x28).
4. **Map draws were never frozen**: the corridor map (M0013I) renders; the
   first screenshot was taken during the fade-in.

## Route reached with ordinary input

Naming → courtyard → lobby → chandelier/performance → aisle (seat route) →
scripted Eve encounter (mode 12 exit, messages 50–60) → stage-right exit →
backstage hole trigger → under-stage Eve+child scene (now completes) →
backstage corridor (M0013I) → random rat encounter (renders, HUD live) →
stops at the M0013I main effect callback 8018F20C (>670 words, GTE).

## Verification

Normal native suite (build tree): `Results: 1200 run, 1179 passed, 1 failed
(B54KY: disc path only when run from the wrong cwd), 20 skipped`.
CTest normal and ASan/UBSan: **2/2 each** on the final code
(`local/live/ctest-asan5.log`, 58.35 s). Tests added:
SEW1_opcodes_5c_5d_93, SEW1_unported_opcode_is_explicit_boundary,
SEW2_leaf_opcodes, SEW3_container_leaves, SEW4_heading_opcode_cb,
SEW5_progress_flag_opcodes, SEW6_walkmesh_place_opcode_af,
SEW7_heading_to_actor_opcode_45, SEW8_wait_opcodes, SEW9_walk_opcode_db,
SEW10_camera_pan_opcode_48, SEW11_opcodes_bf_d2, SEW12_m0013i_command_callback.

## Open (see ACTIVE_HANDOFF for detail)

- Eve's dress fragment after her retreat (user report): later runs proved
  the message chain does run. The field reload re-spawns Eve while g74 is
  still 0x27, then her retreat clip runs. Root motion, reused actor tasks,
  and remaining destination-rendering cuts are still under investigation.
- Dest tick 3AF14 branches 0x800/0x10/0x20/0x40/4/8/1 are still cuts.
- Remaining unported script table slots and the route through the end of
  the sewers (the current user objective extends beyond entry).

## SEW14: corridor projectile callback

Run 14 resumed from the live post-Eve stage and reproduced M0013I's
8018F20C mode-0 boundary at frame 113537, story g74=0x40, during the rat
encounter. The native translation in `m0013i_effect_port.c` covers the
complete charging/motion/collision/fade/draw callback and its particle
callback 8018F004. Shared helpers D3F64 and C6B90 are native. D3F64 consumes
the sound playback handle, revealing that the existing 6DCE4 wrapper had
dropped its return value; that wrapper now forwards the original result.

Instruction authority is Disc 1 user sectors 12007–12008 (4096 bytes),
SHA256 `5be7c97af6b8fe3f51dc34350c6e16c2bc0a659e187ed7fd7aa5d6506ed063a1`.
The overlay's code matches the live dump; its first two header words are
relocated at runtime. No captured game data was added to the repository.

Verification commands:

```sh
python3 pc_port/tools/pe_m0013i_effect_oracle.py --write-header
cmake --build pc_port/build -j 8
ctest --test-dir pc_port/build --output-on-failure
cmake --build pc_port/build-asan -j 8
ctest --test-dir pc_port/build-asan --output-on-failure
```

All 117 original-instruction cases match native return values and state:
initialization variants, launch transition, movement directions, polygon
exit, player collision, attack flags, successful sound allocation and
cleanup, full/free particle pools, particle expiry, both packet banks and
depth rejection. The comparison includes defined GPU packet fields and
ordering-table links; transient stack padding and unlinked GPU fields are
masked consistently with the existing sprite oracle. No callee is replaced
by a reconstructed oracle formula. CTest normal and ASan/UBSan passed 2/2
each (1202 native groups, 45.16 / 58.44 seconds). Live run 15 passed the
former boundary: native projectile capture at frame 33190, rat defeated
and battle flag cleared at 33520, player control returned at 33670. Aya
remained at 21/45 HP; the enemy record reached zero HP. Captures and replay
log are under `local/live/sewers15-*`. The later corridor/key/sewer route
and the existing Eve rendering discrepancy remain open.

## SEW15: dressing-room mirror

Run 15 entered corridor door action 6 through ordinary Left input at
X~535/Z-1944. The transition loaded A80020C8, then stopped at frame 74034:
opcode 6A at 801A2E14 starts event code 45, whose constructor is 8018EFFC.
The final snapshot is `local/live/sewers15-stop-*`. The earlier suspicion
of a floor collision bug was disproved by isolated original/native 1AE40
executions from the captured state; both permit X535. No collision code changed.

`mirror_effect_port.c` translates the room overlay at 8018EFF4..8018FB44:
constructor, target actor lookup, mirror endpoints/visibility commands,
reflected translation and rotation, source animation, and reversed-winding
polygon insertion for all four packet formats and both display banks.
Scheduler dispatch checks overlay instructions before using reused addresses.
Original authority: Disc 1 Form 1 sectors 12868–12869, SHA256
`bff195a272f6b7fc19a4eba453c8ad6b138537b7dfa8c64a0da44c534777a77f`.

`python3 pc_port/tools/pe_mirror_effect_oracle.py --write-header --dump`
executes 87 complete original instruction graphs with synthetic actor/model
fixtures, including lifecycle scheduler entry points. No callees are replaced.
Native state and packet hashes match all cases. Normal CTest 2/2 PASS,
45.99 seconds, 1203 native test groups (`local/live/ctest-sew15.log`).
ASan/UBSan CTest also passes 2/2 (51.46 s). Live run 16 room traversal
is still pending at this entry.

## Window resizing correction (user report)

X11 previously ignored ConfigureNotify and blitted using startup dimensions.
The new image buffer follows client dimensions and is released through
XDestroyImage. Both platform backends share complete-frame nearest-neighbor
scaling that fits the 4:3 image with centered black borders. Win32 also uses
resizable/maximizable window chrome and reads the current client size.

Actual X11 pixel checks with `local/live/resize_smoke.c` and
`python3 local/live/check_resize.py` passed at 960x720, 1001x713, 713x1001,
240x180, and restored 640x480. Each check verifies the four source quadrants,
all image corners, and black borders. `local/live/resize-large.png` is the
960x720 capture. Maximize (1280x909) and restore (640x480) also pass all
pixel checks (`check_maximize.py`); the smoke app closed on Escape before
xdotool could send key-up. Win32 runtime was not tested on the Linux host.

SEW15 live run 16 reached the mirror but aborted while looking up its
source actor. The original opcode 6B wrapper (187C0) dereferences argument
pointers before passing args 2/3/4 to 6F6D4; the existing port passed those
pointers directly. Corrected all three loads. The mirror oracle now includes
full original wrapper dispatch (92 cases total), and the M0013I oracle
also covers its command block through that wrapper (120 cases total).
Final normal and ASan/UBSan suites both PASS 2/2, 1203 native groups
(49.11 s / 60.11 s; `ctest-{,asan-}sew15-final.log`). Live run 17 is
in progress. Signal-stop snapshots were added to the read-only inspector.

SEW15 live proof: run 17 initialized and configured the mirror at frame
29350, returned player control at 29400, and rendered the moving reflection
without a boundary. `sewers17-calls.log` records source0/id0, endpoints
(838,323)/(838,-690), visibility1. At Aya (462,-242), mirrored X=1214 as
expected across X838; both figures are visible in `sewers17-mirror-close.png`.
The retained state is `sewers17-mirror-proof-*`. Medicine1 was subsequently
used through the ordinary inventory menu, consumed, and restored HP7→45.

## SEW16: retire room effect registrations before overlay replacement

Run17 left the mirror room through normal Down input and stopped at frame43020.
The corridor overlay had replaced 8018FB44, while slot0 still used that mirror
descriptor: its draw/tick callback reads became instruction words A7A2004A and
96030004. Evidence is `local/live/sewers17-stop-backtrace.txt` and `-stop-*`.

The original 121-word func_800696F0 (800696F0..800698D4, SHA256
8ea0ad39f179aef98cb62db32e52ab5e44a9cbdf9ab92f0bbbe2ad6fc9fa8089) first
calls module finalizers when D1A0&0x80, clears that bit, then clears registry
entries 8..84 and E1044 indices30..103. The previous native cut omitted the
first half and cleared the descriptors *pointed to* by the entries instead
of the entries themselves. The full walk now follows the original stores.
Core finalizers C7DD4/C8F18/C9C10/CA7A8/CD970/CCF90/CE1EC/CBFB4 are original
`jr ra; move v0,zero` leaves. The mirror finalizer is covered by the existing
overlay signature dispatch. Unknown callbacks still stop explicitly.

`python3 pc_port/tools/pe_room_cleanup_oracle.py --write-header` executes the
original cleanup and real callbacks across 18 cases: flag clear/set, 0/1/8
package entries, missing descriptors, null callbacks, edge codes7/8/84/85/255,
and preservation of descriptors and table boundaries. Native comparison
passes all18; CTest normal2/2 passes (1204 groups,42.14s), log
`local/live/ctest-sew16-final.log`. First test run exposed fixture issues:
BTL86's fake descriptor overlapped its registry; SEW16 needed the established
host D1A0 synchronization around the native call. Both fixtures are corrected.
ASan/UBSan CTest also passes2/2 (1204 groups,50.04s), log
`local/live/ctest-asan-sew16-final.log`. Run18 passed mirror entry at23290, rat return22530. Its host exec session
then closed the game when the replay finished; no game boundary recorded.
The launch helper now uses setsid and a separate exec, and run19 is underway.
The mirror exit remains unverified in live play at this entry.

SEW16 live validation: run20 entered mirror22250, exit trigger22910, corridor
control22940. Snapshot after exit has D1A0=4040 and registry[45]=0. Evidence:
`sewers20-exit-proof-*` and fully faded-in `sewers20-corridor-after-exit.png`.
This proves the previous stale callback boundary is passed with normal input.

## SEW17: mandatory theater-key award and field pickup dialog

Original M0020I script (tokenA8002048, chunk2LBA13260) awards item200 at
801A0BA4 through opcodeA7, opens its pickup dialog through E8 at801A0BDC,
then sets global24 bit20. These are required by the corridor's locked doors.
`field_item_pickup_port.c` translates the original194B0/15BAC wrappers,
532B4 item lookup, 4F490 pickup constructor, 4F644/F730/F798/F7D8 display/input,
and 50204/5022C/51060 bonus-list drawing. Full source authority is the verified
Disc1 EXE SHA1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching-C claim.

`python3 pc_port/tools/pe_field_pickup_oracle.py --write-header` runs74
complete original instruction cases, including inventory full, key/weapon/
armor dialogs, bonus slots, both render banks, initial script yield/retry,
confirm/cancel, and input event dispatch. Native test mirrors the original
memory/result output and verifies no stub/stop. Normal CTest2/2 PASS1205
native groups44.88s; ASan/UBSan2/2 PASS81.59s, final log files
`local/live/ctest-sew17-final.log` and `ctest-asan-sew17-final.log`.
Live key pickup remains pending: current run20 began before this build.

Read-only inspector destination token now reads native D_8009D280 instead
of its stale guest-memory copy. Run19 accidentally selected retained menu
cursor ArrangeItems while trying to heal and stopped at46DFC; this optional
page remains an explicit boundary. Future replay verifies UseItem selection.
Mirror code EFF4..FB72 is shared byte-for-byte by M0017I (oracle source at
LBA12868) and M0021I (live room atLBA13469), SHA256
5cd8e8eb4ad71e0fd11c30c015f40c2ffdd98230d0fbc1b674a15e2324993fd1.


## SEW18 investigation: missing body-contact tasks (not yet ported)

Run20 entered M0020I at28020 and approached the body using normal input.
At Aya X395.5875/Z189.3808, native actor+A4 contact chains remain empty.
Executing the original608-word36448 on a copy of the same RAM creates two
tasks: body script801A1620 at8009D4C8 and Aya script801A09F0 at8009D4F4.
Sixteen words change. Reproduce with `python3 local/live/contact_live_oracle.py`;
proof `sewers20-contact-proof-*`, `contact-live-diff.txt`, and
`sewers20-contact-proof.png`. No writes to the live game. Earlier positions
outside contact range correctly produced no differences in the original too.

Native35558 currently omits the original35C1C call36448 and35C24 call12774.
They need the complete contact pass and task retirement, with35F54 movement
rollback. Also identified existing29810 callback-address error: lui8002 plus
signed addiuD268 forms8001D268, but the native cut published8002D268. The
former is the54-word original contact callback; the latter is unrelated code.
This is the next required port before the body can award the theater key.


## SEW18: actor contact and interaction-task retirement

`actor_contact_port.c` translates original36448 (608 words),35F54 (50),
1D268 (54) and12774 (55), wired after animation updates in35558. The second
pause check is preserved. Battle actor construction now uses callback8001D268;
original lui8002/addiuD268 uses a signed low half, which the old constant missed.

`python3 pc_port/tools/pe_actor_contact_oracle.py --write-header` generates214
synthetic cases by executing the checksum-pinned original instructions and
all reached callees. Cases cover proximity limits, signed radius scaling,
actor filters, task duplicates, rollback with parents/siblings, sphere contacts,
battle callback conditions, and all three task chains. Native comparison passes.
On a copy of the captured run20 body approach, original and native36448 also
produce identical non-stack RAM below801F0000, including both new contact tasks.
Diagnostic sources and RAM stay ignored in `local/live/`.

Validation: `cmake --build pc_port/build -j 6`; normal CTest2/2 PASS47.96s,
ASan/UBSan CTest2/2 PASS63.38s;1206 native groups. Logs
`local/live/ctest-sew18.log` and `local/live/ctest-asan-sew18.log`.
Run21 live proof: examination completed28390; pickup popup30270 and capture
30870. The visible popup says Theater Key. The captured inventory800C0E48
contains item200 and g24=01000220. Confirming the popup restores control.
Medicine1 through the visible inventory healedHP10→45 and consumed item6;
key200 remains. Capture `sewers21-key-popup.png`, `sewers21-key-proof-*`,
`sewers21-key-healed-proof-*` in ignored `local/live/`. No gameplay RAM writes.
Replay `theater_key` phase records the demonstrated chair approach and input
sequence. The end-of-sewers objective remains active and incomplete.


## SEW19: Rehearse Key and script sound requests

Run21 entered door7/M0018I at48810. This is the dressing room with the diary,
not the piano room. The demonstrated approach(300,-300),(0,0),(-181,223),
then Up120ms sets heading800; Cross opens the diary. Advancing its text reaches
the Rehearse Key popup57870. Capture61630 records g24=010C0220 and item201
in inventory alongside200. Popup and RAM are ignored
`local/live/sewers21-rehearse-key.png` and `sewers21-rehearse-key-proof-*`.
Confirmed pickup, exited diary room66900, unlocked the final corridor door,
then entered rehearsal roomM0319I at73150, HP45/45. Eve and her piano are visible.
All gameplay advancement used normal keyboard input; no gameplay RAM writes.

Separately, `func_80015DAC_port.c` now handles original EA sound keys300 and
350..353: room sound lookup and queueing, attenuation bounds, current actor,
selected actor, and explicit fixed-point position. Output handles and missing
sound/actor behavior follow the original instructions. Music-related EA cases
remain partial; this does not claim complete audio playback.

`python3 pc_port/tools/pe_script_sound_oracle.py --write-header` executes the
checksum-pinned original executable for64 synthetic cases, including all
reached projection, lookup, validation and FIFO callees. Native64/64 PASS.
Normal CTest2/2 PASS66.35s and ASan/UBSan2/2 PASS80.17s,1207 test groups;
logs `local/live/ctest-sew19.log`, `local/live/ctest-asan-sew19.log`.
Run21 still uses the precedingSEW18 binary; the sound changes need a later
live replay. The end-of-sewers objective remains incomplete.


## SEW20: rehearsal battle attack timing

Run21 advanced from M0319I piano dialogue into M0023I, g74=5B, HP45.
At frame80464 it stopped explicitly at unported198C4, script801D8F8C,
Eve actor800BF210. Original opcodeB0 calls2FAD8 to store two32-bit timing
values in the actor attack record, with an8-bit index. Both routines are now
translated in func_8002FAA4_port.c and wired into the VM.

`python3 pc_port/tools/pe_attack_timing_oracle.py --write-header --dump`
PASS45 complete-original cases; native SEW20 groupPASS, including fullVM
execution. A COPY of `local/live/sewers21-stop-ram.bin` matches original
execution across all non-stack RAM below801F0000: only800A5D7C/800A5D80
change10000→20000. Native comparison fixture `attack_timing_native_live.c`
and both binary snapshots remain ignored in local/live. No gameplay RAM writes.
Normal CTest2/2 PASS47.64s and ASan/UBSan2/2 PASS50.87s,1208 groups;
logs `ctest-sew20.log`, `ctest-asan-sew20.log`. Run22 is replaying the new
binary; live B0 completion and the end of the sewers remain unverified.


## SEW21/22: music volume and embedded script polygons

SEW21 restores EA205/206/207, the signed-byte channel records at
800B0DB4/DB5, and C0/C1/C2 command producers. Missing handles return
without modifying channel/queue state; durations preserve original32-bit
shift behavior, command volumes are7-bit and saved volume is8-bit.
`pe_music_volume_oracle.py --write-header --dump` executes69 complete
original call graphs (both slots, signed identifiers/handles, duration
overflow, FIFO offsets); nativeSEW21 PASS.

SEW22 restores original1A390..1A3FC, opcodeE2, used by the next Eve
battle's script at801D90F4. It computes a polygon pointer relative to
actor script base using a halfword offset and calls original/native1CAB0.
`pe_script_polygon_oracle.py --write-header --dump` executes48 original
cases with empty, convex, concave and signed-extreme polygons, points on
edges/interior/exterior, wrapper and completeVM calls; nativeSEW22 PASS.
Both new groups are part of1210 native groups. Normal CTest2/2 PASS70.23s,
Release2/2 PASS33.99s, ASan/UBSan2/2 PASS114.66s. Logs
`ctest-sew22.log`, `ctest-dist-sew22.log`, `ctest-asan-sew22.log`.
Windowscrossbuild PASS, runtime not executed on this Linux host.
These sources are native ports, not claims of new matching decompilation.
Run22 usesSEW20 and is still replaying; these additions need laterliveproof.


## Launcher publication: PE-SEW22-7fc95fa875ba

User explicitly requested publication to the launcher update channel.
Built Linux Release and Windows x64 via dockcross/windows-static-x64;
packaged both discs and cue sidecars for each platform. Linux extracted
wrapper boots its bundled Disc1 through120frames and stops atframe-limit;
both extracted disc hashes match metadata. Windows package structure and
metadata verified; its executable was cross-compiled, not run here.

Normal, Release and ASan/UBSan CTest eachPASS2/2 (1210native groups);
70.23s,33.99s,114.66s respectively. Linux archive592459551bytes,
SHA256b336b27b39891f667c68e9d70627ed9f07e5b12ee1df57d204bc73789ec2ac8d;
Windows592516977bytes,
SHA25605cd8b7d6ea0a8a0950695efbfe12f8eb783e0f056fc71225894819b70df1000.
PrivateR2 remote full SHAreadbacks match, both signed archive rangeGETs
return206, pinned/mutable channel docs readbackmatch, signed channelGET
returns thisbuild. Independent subsequent rclone read confirms dev.json
nowPE-SEW22-7fc95fa875ba replacingPE-SEW16-25673ab3c853.
`local/live/publish-sew22/published-summary.json` contains nonsecret evidence;
signed URLs remain private in that ignored directory. Sourcecommit7821e7c3
is dirty; exact runtime binaries are identified by package metadata hashes.

User expanded the objective toALL Day1 at100% retail accuracy. Publication
does not claim that objective achieved. Scope and unresolved acceptance
items are in `docs/ai_context/DAY1_RETAIL_ACCURACY.md`.


## Run22 rehearsal encounter and full-loader audit

Run22 healed17→45 through normalMedicine1 use, obtainedRehearseKey201
at50860, confirmed50920, enteredpiano room54730, triggeredEve56620.
At57481 it passed the formerlymissingB0 timing command, then stopped
explicitlyatE2/1A390, pc801D90F4. It usedSEW20, whereas the publishedSEW22
alreadycontainsE2. Original/native E2 execution on a COPY of this stopped
RAM matchesallnon-stackRAM below801F0000; only800BF314 changes0→1.
Run23 now replaysSEW22. No gameplay RAM writes wereused.

Independent full-original execution also reveals a pre-existing loader gap:
6CDA4 mode1,emptyXAarchive,initialF0=0,stack_flag=0 returns0/F0=0 inretail,
but currentnative returns1/F0=7. This disproves full equivalence despite
older selected-state BTL6 tests passing. Evidenceignored
`cda4-empty-before.bin`, `cda4-empty-original.bin`, `cda4-empty-native.bin`,
`cda4_empty_native.c`; original181-word disassembly `music-loader-cda4.txt`.
The native ignores stack_flag and omits the internal loop. Full restore and
updated original-execution tests remain required for100%Day1 accuracy.


## DAY1-1: full loader loop restored; replay defeat evidence corrected

The discrepancy above is resolved by `func_8006CDA4_port.c`. Full-original
execution covers336 cases and1198 controlled CD/SPU provider calls, including
empty archives, all four modes, retry/failure, upload/poll transitions and
blocking/yield behavior. This is a native port, not a matching-C claim.
Provider behavior is an explicit test contract, not full audio validation.

Commands and results:

- `python3 pc_port/tools/pe_disc_loader_loop_oracle.py --write-header`: PASS.
- `ctest --test-dir pc_port/build --output-on-failure`:3/3 PASS44.77s.
- `ctest --test-dir pc_port/build-asan --output-on-failure`:3/3 PASS49.94s.

The suites include1210 native groups and336 standalone loader cases. Logs
are `local/live/ctest-day1-1-final.log` and `ctest-asan-day1-1-final.log`.
Legacy fixtures now seed genuine pending/completed I/O instead of relying on
the former early returns. The empty-archive original/native RAM comparison
matches below801F0000, excluding original stack effects.

Run23 lost the corridor rat fight. Its old helper's cleared-battle-flag check
falsely recorded victory, then advanced into new-game naming. That log entry
must not count as progress. `sewers23-rat-check.png` records the naming screen;
frame43180 shows modeFFFFFFFF. The helper now checks zero HP and defeat mode
before its victory predicate or another keypress; direct fixture checks pass.
A fresh run should heal before the corridor fight. No gameplay RAM writes
were used. DAY1-1 source and replay corrections are not in published SEW22.


## DAY1-2: complete music-bank loader control flow

`func_8006D2B8_port.c` translates original8006D2B8..8006D60C (214 words),
SHA256 `ea86f38da35cb0224e6a50f581e8e02044d4e91528a33ac29d95a7c856fe0029`.
Original instruction execution provides864 cases and216 controlled provider
calls for bank lookup, signed channel IDs, channel reuse/busy returns,
unloading duplicate channels, copying and internal blocking/retry.
`pe-music-bank-tests` links the production translation with wrappers for
6CDA4 and86FF8, comparing their arguments/order and final RAM/return.
The oracle models BIOS memcpy explicitly. This verifies the routine under
those provider contracts, not full CD/SPU providers or audible playback.
EA200/201/203 integration remains open; this is not a matching-C claim.

- `python3 pc_port/tools/pe_music_bank_oracle.py --write-header`:864 cases PASS.
- `ctest --test-dir pc_port/build --output-on-failure`:4/4 PASS52.48s.
- `ctest --test-dir pc_port/build-asan --output-on-failure`:4/4 PASS58.24s.

Logs `local/live/ctest-day1-2.log`, `ctest-asan-day1-2.log`. The full suites
include1210 native groups,336 disc-loader and864 music-bank cases.
DAY1-2 is not published; live run24 was launched on DAY1-1 before this build.
Run24 first Eve start13450, scriptedexit15150, fieldcontrol16170 HP18;
Medicine1 normal Use Item menu heals18→45 at18730, screenshot
`local/live/sewers24-healed.png`. Replay then resumes toward the corridor.

Run24 corridor rat victory25210, fieldcontrol25340 with HP34 and g74=0x48;
key200 popup30740/confirmed30840, key201 popup37580/confirmed37680.
Its diary exit waypoint then encountered a table obstruction; the helper
stopped while the game remained responsive. Preserve this run and recover
with normal movement rather than restarting. Live binary SHA256 read from
`/proc/1973210/exe`: `9f0c85f5f25ad6712d9b99a5aa592b656727aee7f01e9035ce6f2ea899a97214`.

`python3 pc_port/tools/pe_day1_route_audit.py` independently verifies25
inspected original room-transfer commands across11 chunks. See
`docs/ai_context/DAY1_ROUTE_AUDIT.md` for addresses and scope limits.
The M0000I sentinel path still reaches bootstrap6ECEC/8019234C; Day1
completion remains unproven. Static edges are not live coverage evidence.


## Run24 terminal evidence: M0023I effect callback

Diary obstruction recovered via(300,200),(300,-300), now the replay exit
route. Diaryexit45210, rehearsaldoor47300, entry47440, Eve trigger49620.
At50516 the process stopped with unresolved callback8018F3C8 mode0,
data80186230, extraFD6DFB9D; HP34, g74=5B, tokenA80021C8. The scene
advanced beyond the previous E2 stop, with last script cursor801D928C.
This is not a completed rehearsal battle. Both game and helpers are terminal.
Evidence `local/live/sewers24-stop-{ram.bin,state.json,backtrace.txt}`.

Independent Disc1 extraction M0023I chunk2 (LBA13904,172 sectors) matches
both live effect code spans exactly:

- Main8018F3C8..8018F710,210 words, SHA256
  `b39ab2a332607673c32fa36e41b2b2428dcc58020316681b553df9ab4b79863b`.
- Particle8018F004..8018F3C8,241 words, SHA256
  `2e2c57c73f6b63fab5ffa871e5b66be5fd3ca84e29c158e3f5259b87d4f6a221`.

Complete mode0/1/2 translation and original/native proof remain the next
implementation step. Full per-word disassembly is ignored at
`local/live/rehearsal-effect-f004-f3c8.txt`; unknown GTE encodings are retained
as words rather than terminating disassembly. Overlay-address reuse must be
handled: M0013I also has a different callback at8018F004.


## DAY1-3: run24 particle effect translated

`m0023i_effect_port.c` restores all modes of originalF3C8 (210 words) and
F004 (241 words), with code-identified overlay dispatch. The11-word90644
command is also restored: mode1 sets descriptor+12 to1, all modes return
80190758. Its original SHA256 is
`06bd7eb3f2fba46e855437b0c3bbc727f28f6bbb34f799f9e73bc7d373d7364c`.

`python3 pc_port/tools/pe_m0023i_effect_oracle.py --write-header` executes
complete original instructions and real callees for167 cases. Coverage includes
initialization, all particle states, random emission seeds, full pools, signed
timers and lifetime boundaries, both GPU packet banks, culling, ribbon/sprite
rendering, complete pool update/draw calls and descriptor command modes.
Only undefined packet padding/unlinked stack fields use the existing masks;
defined packet fields, ordering links and persistent state are compared.
No callback or callee behavior is substituted in this oracle.

- `ctest --test-dir pc_port/build --output-on-failure`:4/4 PASS46.44s.
- `ctest --test-dir pc_port/build-asan --output-on-failure`:4/4 PASS54.19s.

The native suite now contains1211 groups. Logs:
`local/live/ctest-day1-3-final.log`, `ctest-asan-day1-3-final.log`;
original-case log `oracle-day1-3-final.log`.

On a COPY of run24 stop RAM, originalF3C8(mode0,80186230) and native
PE_EffectCallback both return252 and match every RAM byte below801F0000
(excluding original stack). Reproduction native fixture
`local/live/m0023i_native_live.c`, outputs `m0023i-{original,native}-live.bin`.
The game was not modified or resumed from this copied state.

This is not full M0023I acceptance. Descriptor80190720 also listsF710
(321 words) andFC14 (652 words), both still unported. Their code matches
original Disc1 M0023I; full disassembly is `m0023i-other-effects.txt`.
F710 calls the still-missing439-word D004C drawing function and reads a
stack position before its later joint-position call; the caller/stack behavior
must be resolved from original evidence. Descriptor+34 points to bytecode,
not another native callback. These findings guide the next port before
another full route replay. DAY1-3 is not published; no live run is active.
