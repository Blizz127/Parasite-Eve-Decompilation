# Second sewer encounter: movement and projectile effects

Full Day 2 and whole-route fidelity remain unproved. The 62,000-frame,
57-milestone, 3,158-pad supply replay below is historical: its reward handler
omitted experience commits and pending item processing. It retained zero XP.

The reward and loot paths now commit 2 XP and collect 6 ammunition in the first
m0013i reward. The old fixed inputs reach the first sewer battle but lose it.
An adaptive normal-input replay now defeats the first sewer encounter,
completes level-up and loot, and restores field control at frame52,111, HP27.
[Level animation evidence](BATTLE_LEVEL_ANIMATION.md).
[Current loot evidence](BATTLE_LOOT_SCREEN.md).
The isolated movement, projectile, chest and related comparisons below remain
useful, but their old connected route must be revalidated.

## Original movement effect

The replay with opcode DD connected stopped at frame 53,141, during effect
49 initialization and the following opcode 6C. The original room C2 overlay
is loaded at `8018EFE8` from Disc 1 LBA15016, 69 sectors, 141312 bytes.
SHA-256: `c15d03313a578f7c7ba07f255345df8e17734e38dd807162426d0bc3a7fd7e94`.
Its effect code matches the connected RAM capture. Relevant addresses:

| Routine | Original address |
|---|---|
| Constructor | `8019151C` |
| Command handler | `801915AC` |
| Draw no-op | `801917F4` |
| Update dispatch | `801917FC` |
| Animation gate | `80191824` |
| Initial movement arc | `80191894` |
| Apex pause | `80191D10` |
| Resumed movement arc | `80191D18` |
| Cleanup and completion signal | `801920A0` |

`pc_port/game/boot/m28_movement_port.c` translates this complete graph. It
preserves original angle reduction, wrapped arithmetic, signed division,
GTE target transformation, collision checks, animation repeats, apex pause,
turning, landing and the script completion output. Original scratchpad
temporaries use local variables; persistent data stays in guest memory.
Division traps and unknown callbacks request an explicit unresolved stop.
Four EXE helpers are translated within this file: DFB20, DFB78, DFF80 and
DFFB8. The original instructions, rather than inferred gameplay, determine
the behavior.

Three supporting fixes connect this graph:

- `func_8006F39C_port.c` returns `index + 11` for the small effect pool,
  matching existing `src/func_8006F39C.c`. Previously it returned the local
  index, making later event calls resolve the wrong pool. It also dispatches
  the original m28 constructor after checking the loaded overlay signature.
- Opcode 6C calls `func_8006F6D4(*arg0, 1, *arg1, arg2, arg3, arg4)`.
  Output operands remain addresses. The event dispatcher invokes the room's
  movement command routine.
- Original `func_80069660` draws slots 11..21 and then updates them unless
  flags `0x104` pause updates. Its original call at `800356D0` is restored
  before field collision resolution. The existing main effect pump covers
  slots 0..10 separately.

`pe_m28_movement_oracle.py` executes original instructions from the verified
EXE and original room overlay. It also checks that every executed word and
delay-slot word agrees with those sources. All RAM below `1FE000` is compared
against a native invocation; the original scratch stack, scratchpad and GTE
register state are outside the comparison. The VM's native compatibility
frame is explicitly excluded. Generated hashes cover every original write
below the stack cutoff, with an additional write-coverage assertion.

**947 original/native calls and history frames pass.** They cover constructor
fields, every command, modes and states, both allocation pools and all free
slot positions, allocation failure, event dispatch, small-pool pause flags,
all five opcode-6C operand modes, movement histories, collision interruption,
apex pause/resume, following actors, target offsets, turning and animation
gates. The cases are retained in `retail_m28_movement_cases.h` and exercised
by `test_m28_movement.h` in the native suite.

Logs: `/tmp/pe-movement-oracle-final.log`, `/tmp/pe-movement-native.log`.
The first oracle attempt lacked a distinct scratchpad in its test runner;
that fixture defect was corrected before these comparisons passed.

## Connected encounter and shared projectile

With movement connected, the replay reaches frame 53,241. Aya takes a real
enemy hit (30 to 26 HP), queues a normal attack at 53,180, and returns to
active battle processing. The next boundary is room command `80193148`.
Its nine original instructions copy three command arguments to
`80193288..80193290` and return the block address. All 12 new command cases
are included in the 947 comparisons above.

The following replay reaches projectile callback `80192700`, mode 0, at the
same frame. The active large-pool effect is code73, owned by actor800BFE90,
descriptor80193250, slot801861A0. This is distinct from the movement effect
in small-pool slot8018D024.

The original projectile/particle code at `801924F8..80193148` is the same
788-word routine pair as the already translated backstage code at
`8018F004..8018FC54`. A complete instruction comparison verifies that the
only changes are 18 relocated internal jumps and three data references:

| Reference | Backstage | Sewer |
|---|---|---|
| Joint vector | `8018EFF4` | `8018F1CC` |
| Angle vector | `8018EFFC` | `8018F1D4` |
| Particle callback | `8018F004` | `801924F8` |

`m0013i_effect_port.c` now shares its projectile implementation through
explicit parameters for those addresses. The original backstage wrapper
retains its original values; `PE_M28ProjectileMain` supplies the sewer values.
The particle arithmetic is unchanged. The effect dispatcher selects the
room callback after checking the loaded m28 overlay signature.

`pe_m28_projectile_oracle.py` both verifies the 788-word relocation and
executes the complete original m28 graph. **120 original/native cases pass**,
covering initialization, launch, movement, collision, fading, particle
expiry, pool saturation, sound requests and both packet banks. The scope is
the original return values, persistent state and defined GPU packet fields;
the inherited sprite oracle excludes packet padding and unlinked stack
bytes. Fixtures and native tests are `retail_m28_projectile_cases.h` and
`test_m28_projectile.h`. Logs: `/tmp/pe-m28-projectile-{oracle,native}.log`.

The movement-stage full suite passed all 11 CTest checks, including
1,385 native tests and the fixed route (124.92 seconds); total193.16 seconds.
That run predates the shared projectile refactor. The latest full suite and
connected replay after both effects are recorded in ACTIVE_HANDOFF.md.

## Evidence and limits

Captures are read-only diagnostic outputs, never inputs used to advance the
connected route. Each continuation uses the 965 initial pad pairs in
`/tmp/pe-m28-battle-pad.txt`, then the input-only battle controller in
`/tmp/pe-m28-battle-route.c`. It approaches targets, attacks and requests Heal1
through normal menus. These exploratory runs retain the older m27 endpoint
assertion, which is expected to reject an endpoint already in m28.

Completed intermediate captures:

- `/tmp/pe-m28-movement-connected.{log,bin}`: frame53241, missing room command.
- `/tmp/pe-m28-command-connected.{log,bin}`: frame53241, missing projectile init.
- `/tmp/pe-m28-projectile-connected.{log,bin}`: latest replay after both effects.

The matching-C plan and retail executable were not changed by this work:
796 C /347 asm /2 rodata, 1145 spans, plan `b50a30290d4d`, EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. These native adaptations are not
additional byte-matching C registrations. The full repository verifier's
existing tracked-source metadata failure is unchanged; no index edits,
staging, commits or broad restores were performed.


The latest complete suite passes all 11 checks and 1,386 native tests after
both effects are connected (185.87 seconds; fixed route129.03 seconds).
Release and Debug builds succeed. The 62,000-frame connected continuation
has no unresolved boundary; the type8 enemy is defeated and the two type7
enemies remain at12HP. Aya has38HP. An input-controller navigation adjustment
is being tested; this is not yet a second-sewer victory.


### Retirement defect found during the continuation

The second navigator replay exposed movement cleanup through6FC18 while an
enemy died. The original801920A0 routine is now dispatched there, guarded by
the M28 overlay signature. The movement oracle now passes1003 cases, including
56 destruction combinations of slot state, ownership, force and animation
count. The native suite passed1386/1386 after regenerating those fixtures.

That navigator subsequently lost the battle: AyaHP0 at54484, new-game restart
at54752. The62000-frame ending in the opening room is not story progress.
`/tmp/pe-m28-destroy-connected.log` records that failed route attempt.

The old frame cut omitted most of original360B4. It cleared the animation
flag but left retired actors in the live list. The defeated type8 actor in
`pe-m28-projectile-connected.bin` retained a255-unit collision radius and
blocked Aya's approach. Full360B4 now releases owned effects, clears Aya/input
state when needed, releases the first matching model allocation through363F4,
and updates the active/free actor lists and actor count. Original3D82C is a
return-one leaf without RAM effects. The frame calls this at original35C2C.

The first cold-boot retry exposed6FE14 reading a pool before initialization.
The original reads physical RAM; `effect_slot` now uses its cached alias for
physical addresses, preserving the access rather than skipping cleanup.
`pe_actor_retirement_oracle.py` compares468 cases with original instructions:
467 synthetic cases covering list positions, parent retirement, Aya, model
allocation, both effect pools, count wrap, physical and uninitialized pools;
one real stuck-battle capture. All RAM below1FE000 matches, excluding the
original scratch stack. Executed instruction/delay-slot bytes are validated
against the original EXE. Generated fixture ranges cover every original write.

The connected retry and regression verification after this complete retirement
change are still in progress; the earlier all11-check result predates it.


### Verified fixed route and forward continuation

The complete route now passes all49milestones at54,500frames. The original
961-pad prefix is unchanged; `route_second_sewer_pads.h` adds587recorded raw
pads. Raw input intervals are[42713,45041),[50500,51200),[52344,54500).
The new suffix SHA256 is
`1d191a3c3caed7f4bfe121e7de6709c307f3878c9bf558f2bc923758fcaad896`
(comma-separated pairs without leadingcomma or newline).

Read-only per-frame observations require three spawned enemy bodies, no
remaining bodies, living Aya, mode9 and restored control for each victory.
The victories occur at51167 with30HP and54039 with12HP. Endpointm0028i has
story68/persist1=1B, taskPC801A5A78, no type7/8 actors, menu0, clubslot2,
keysC8/C9, stored status-copy22, musicF2=0 and overlay40000040. This replaces
the old incorrect assertion that retired enemies remain in the active list.

All11CTest checks pass in163.72seconds (route163.72/native72.17). The native
suite reports1,387/1,387 passed. Release and Debug builds succeed. Logs:
`/tmp/pe-second-sewer-all-ctest.log`, `/tmp/pe-second-sewer-all-lasttest.log`,
`/tmp/pe-second-sewer-debug-build.log`. The first fixed replay had an outdated
status-copy assertion36; actual22 was corrected before the passing suite.

The input-only forward controller centers Aya then walks to m28's forward
rectangle. It entersm0031i at55387, story68/persist1=1C, then originally stopped
at55612 on audio commandA1 / callback8008B780. The full96-word routine
8008B780..8008B900 is now translated in `pe_stream_commands.c`. It selects
active effect voices by group overlap or exact handle and applies the signed
volume ramp. Full-word duration0 becomes1; a nonzero duration whose signed
low halfword is0 stops at the original division fault before voice writes.
Routine SHA256:
`29e7f2ce0e7fc181da4e8f3051148e981e0eec846274ec987110fee5721a13f8`.

The audio fixture family passes964normal FIFO cases (483A1), plus4A1 and2A9
fault prefixes. Generated ranges cover all original changed persistent RAM;
A1 instruction/delay-slot bytes are checked against the verified EXE. The
actual55612capture also compares byte-for-byte below1FE000 after the full
native/original FIFO consumer. These checks concern command and voice RAM,
not audible synthesis or timer fidelity.

With A1 connected, the next replay reaches62,000frames without an unresolved
boundary in m0031i. AyaHP12, position(170,1048,-950), flags408, D1A04000,
mode9/menu0. Capture `/tmp/pe-m31-a1-connected.bin`, SHA256
`eeb2900eb601d8f8fbd6f421aa7895a4725126f3e22848bcbfa63fe9b8be595c`.
The new room's original script is at8019DBE8, SHA256
`6dcc77b3c1e9378433f2522aee5fdbcc8f42972142b67cd3de8765f7d318d555`.
Its forward exit to m0334i is the rectanglex[1300,1800]/z[2600,3300],
original transfer8019EB40. Navigation to that exit is the next live probe.


### Next required encounter

The original m31 scripts lead onward through the left exit to m0032i; the
upper exit visits optional m0334i, whose only transfer returns to m31. Both
room-entry probes complete62,000frames without an unresolved boundary. The
main-path replay entersm32 at56619, with12HP and control restored.

Continuing west through m32 starts another encounter at frame 57,351.
The initial replay stopped at 57,424 on effect 49's constructor `8019150C`
and draw callback `801917E4`. That missing dispatch is now implemented.
The original stopped capture `/tmp/pe-m33-approach-connected.bin` has SHA256
`7b0dd263d430b821d885163566ce957394a0e83ede553294f093118553579fb2`.

All 764 words of m32's movement graph `[80191504,801920F4)` match m28's
`[80191514,80192104)`, apart from 21 internal jump targets and five stored
callback addresses relocated by -16. The original m32 C2 package is at
LBA 15937, 48 sectors, SHA256
`cad2f1feb66352c54806de4291c6a7fb28fd610039860a6631aebc93047fffa8`.
The oracle verifies this complete relationship directly from the disc data.
Shared native init, command, and update bodies now take an explicit unsigned
relocation; cleanup and EXE arithmetic are unchanged. Overlay signatures
restrict each dispatcher to its original addresses.

`pe_m28_movement_oracle.py --room m0032i` passes **991 original/native cases**;
the default m28 run still passes **1,003**. The m32 suite excludes only m28's
12 separate room-command cases. Full RAM comparison, original instruction
and delay-slot checks, and generated write-range coverage are retained.
The new `retail_m32_movement_cases.h` is exercised by `test_m32_movement.h`.
Logs: `/tmp/pe-m32-movement-oracle.log`, `/tmp/pe-m28-relocated-oracle.log`.

### Parent poses and projection center

The original actor-loop call at `800356F8` invokes `8003601C` after optional
floor collision and before camera updates. The port now includes all 38
words of this parent synchronization routine: one list pass copies position
and rotation for actors with flag `400000` and a parent pointer. Sequential
loads and stores preserve overlapping poses and list order.

The lifecycle oracle adds 60 parent/list/flag cases and six overlapping-pose
cases to the existing 467 retirement fixtures. With one captured retirement
and 16 captured contact comparisons, **550 original/native cases pass**.
Log: `/tmp/pe-parent-sync-oracle.log`.

The original `80035B34` call to the existing `800661CC` also now executes
after effect updates, restoring projection center `(160,112)` before the
remaining actor work. All **11 CTest checks and 1,388 native tests pass**
with these changes; full-suite time 145.87 seconds. Release and Debug builds
succeed. Logs: `/tmp/pe-frame-complete-ctest.log`,
`/tmp/pe-frame-complete-debug-build.log`.

### Connected outcome and remaining work

The original controller now passes the missing m32 callbacks, heals through
normal Heal 1 menus, and loses at frame 57,796. A second input-only controller
waits for nearby grounded targets and tries to dodge airborne enemies;
it heals from 12 to 42 HP at 57,615, but loses at 57,788. Both enemies start
with 34 HP. These losses do not establish a gameplay defect and do not count
as story progress. Logs: `/tmp/pe-m32-movement-connected.log` and
`/tmp/pe-m32-ground-connected.log`.

Going north first reaches the wall at z=3602; going west at z=3400 still
triggers the encounter. The previously identified lower polygons govern
enemy behavior and do not prove an encounter bypass. The onward original
exit is `8019C888` to m0033i (`A80031C8`), at
x[-16680,-16080], z[3650,4000]. Winning the third encounter and reaching that
exit remain unproved. The canonical regression still ends after the two
verified m28 victories; full Day 2 is not complete.

A separate supply-room probe opens m0334i's first chest and increases reserve
ammunition from zero to 15 through the original item award. Repeated Cross
then reopens the chest and reaches unported opcode A9 (`80019540`) at
script PC `8019292C`, frame 57,733. This optional item-selection path remains
unimplemented. Capture: `/tmp/pe-m334-chest-connected.bin`.

A subsequent input-only replay steps away after opening the chest, activates
the room switch, and opens the fixed-item chest. At 65,000 frames it retains
15 reserve rounds and item 7 in inventory slot 4, with 12 HP. Original item 7's
record specifies a 90-point recovery; it has not yet been used. The replay
ends at the frame limit without an unresolved callback, but its return-path
inputs are caught at x=685, z=244. This remains a diagnostic continuation;
the canonical regression has not been extended. Capture:
`/tmp/pe-m334-waypoints-connected.bin`, SHA256
`765af3be311bd78607573bd5ca066e2e73810fc02d682e4203ab1cc5127b1410`.

## A9 chest selection and verified supply return

The former A9 stop is resolved by translating three original routines:

| Routine | Original span | Words | Behavior |
|---|---|---:|---|
| `80019540` | `[19540,19618)` | 54 | A9 wait, result publication, cancellation |
| `8004C34C` | `[4C34C,4C4B4)` | 90 | Selection window and saved cursor |
| `80055E14` | `[55E14,55FB4)` | 104 | Storage eligibility and protected/equipped exclusions |

They are implemented in `field_item_pickup_port.c`, with the original A9
entry wired into the VM. `pe_field_pickup_oracle.py` now covers 184 cases:
74 existing pickup cases plus 110 new A9/window/filter cases. With
`--compare-native`, all 110 new cases match native return values and RAM
below `1FE000`; the original execution stack is excluded. Executed instruction
and delay-slot bytes are verified against the original executable, and every
original write must be covered by the generated fixture ranges.

Both A9 phases also match full compared RAM using the observed chest capture:
`--capture-only --capture /tmp/pe-m334-chest-connected.bin --compare-native`.
This reconstructs only the isolated call context; it does not restore RAM
into a connected route. Logs: `/tmp/pe-a9-oracle.log` and
`/tmp/pe-a9-capture-oracle.log`.

A connected replay releases held buttons before selecting an item, as required
by the original menu input latch. It completes the storage operation:
inventory slot 0 becomes empty, persist[71] becomes item 256, chest flag 80
clears, and selection mode closes. Log/capture:
`/tmp/pe-a9-selection-connected.{log,bin}`. The earlier post-port replay left
the menu open because its pilot kept a direction held; no input behavior was
changed to override the original latch.

The independent supply continuation now returns to m0031i, uses item 7 through
Items/Use (12 to 45 HP), and equips carried pistol slot 0 through the equipment
menu. It retains 15 reserve rounds and keys C8/C9. Capture:
`/tmp/pe-supply-return-connected.bin`, SHA256
`eccd1f5bcabde763875f76389dceb4d4bd0e7d94c880d2232d67175069dd4e2f`.

The canonical regression retains its previous 1,548 input pairs and adds
1,610 recorded pairs in `route_sewer_supplies_pads.h`. The raw interval is
`[54500,62000)`; suffix SHA256 (without leading comma or trailing newline):
`864ac392b0e256c1b250571c805d3331343ed1f6368d92cc6417f6f7d7567fde`.
Eight new milestones record m0031i/m0334i entry, ammunition, switch, item 7,
return arrival, healing, and equipment. Both prior victories still require
three observed enemies to be retired before leaving their original rooms.
Later rooms reuse actor types 7/8 for chests, so those types are not treated
as surviving enemies at the new endpoint.

**Verification:** A9 full CTest passes all 11 checks and 1,388 native tests
in 150.48 seconds (`/tmp/pe-a9-ctest.log`). The subsequent extended route
passes all 57 milestones in 157.22 seconds
(`/tmp/pe-supply-fixed-ctest.log`). The endpoint pins m0031i/`8019E8B8`,
story68, arrival14E, HP/status45, reserve15, pistol0, keys, both chest flags,
switch state, and restored player control. The PSX executable SHA-1 remains
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

The supplied m0032i attempt kills one enemy but loses at frame 64,146 with
the second at 17 HP. This is not a third victory. Its diagnostic log is
`/tmp/pe-supplied-m32-fixed.log`; the main objective remains active.
