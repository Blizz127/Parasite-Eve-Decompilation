# Day 1 retail-accuracy acceptance

User objective, expanded 2026-09-05: finish all of Day 1 at 100% retail
accuracy. This supersedes the earlier end-of-sewers milestone. A complete
playthrough is necessary but does not by itself prove accuracy or coverage.
No completion date or small-effort estimate is established.

Final deliverable: the complete, verified Day 1 implementation, correct host
window resizing/maximizing, validated Linux and Windows packages, and the
published launcher update for that final build. SEW22 satisfies the earlier
publication request. The latest checkpoint PE-DAY1-9-0c462bc18581 was
published2026-09-06; the fully accepted Day1 build will require another release.

## Evidence and scope

The original USA Disc 1 executable and original disc scripts/assets are the
behavioral authority. Checksum-pinned original instruction execution validates
translated routines. Native tests alone do not establish retail equivalence.
Native ports belong under `pc_port/`; new matching C under `src/` requires
the matching rebuild/checksum harness. Proprietary assets remain ignored.

First establish Day 1's exact terminal state and transition from original
scripts. Derive the full scene, optional interaction, enemy, item, and menu
inventory from those scripts. Do not infer coverage from names or a single
route. Record the original conditions for branches and how each was exercised.

## Acceptance ledger

| Area | Current evidence | Required closure |
| --- | --- | --- |
| Opening, naming, theater, first Eve | Multiple native input replays; original routine oracles | Compare full presentation, timing, inputs and all Day 1 branches against retail |
| Backstage/corridor/key route | Run21 obtained Theater Key and Rehearse Key, healed, reached piano scene | Reverify final release; enumerate optional rooms, interactions and items |
| Rehearsal Eve encounter | Particle/fan/beam/flash routine comparisons; DAY1-11 covers644 synthetic pump frames and64 actual-room copy frames with concurrent pistol draws | Verify live encounter through victory, all attacks, reactions, rewards and transition |
| Sewers and remaining Day 1 scenes | Not yet played through in the native port | Derive original route/branches, port missing graphs, verify every encounter and Day 1 ending |
| Combat and progression | Large suite of original-execution comparisons for first-Eve systems | Exercise every Day 1 enemy/action/status, defeat/recovery, rewards, progression, PE and equipment case |
| Inventory and field interactions | Key awards, medicine use, equipment and numerous menu routines verified | Audit every reachable menu action, full inventory/cancellation cases, save/load and persistence |
| Rendering and animation | Opening/field/battle screenshots; model/camera/effect oracles | Retail visual comparison throughout Day 1, including movies, fades, masks, effects, geometry and animation timing |
| Audio | Sound and music command queue translations; SEW19 sound, SEW21 volume/fade, DAY1-15 EA200/201/203/204 start/stop oracles | Restore remaining EA paths, score sequencing and audible SPU synthesis; compare music/effects timing and output |
| Host presentation and controls | Linux resize/maximize checks; frame pacing; Windows compiles | Verify packaged controls/presentation on both supported platforms without changing retail gameplay semantics |
| Release | PE-DAY1-9-0c462bc18581 published for Linux/Windows; both discs, remote hashes and channel readback verified | Repeat package verification for the final Day 1 build; Windows runtime remains untested |

## Known gaps that prevent a 100% claim

`pc_port/platform/pe_stream.c` explicitly states that full score sequencing and
audible SPU synthesis are unported. DAY1-15 wires EA200/201/203/204 start/stop, but queued commands are not
evidence of audible playback. Remaining music cases, score sequencing and
SPU synthesis must still be restored. `func_8005C498_port.c` also contains an unresolved field-menu boundary.
Audit other reached no-op/partial paths against original instructions rather
than treating a lack of crashes as completion.

Run22 completed both key pickups and reached the rehearsal encounter, then
stopped at E2 because it used the SEW20 binary. Run23 used SEW22 and lost the corridor rat fight; its false victory log is excluded from evidence. The replay now stops on defeat.
The E2 fix matches original execution on a copy of the stopped battle RAM. Test results and launcher publication must
identify the actual binary under observation.

## Working order

1. Launcher publication is complete (SEW22). Preserve existing evidence and
   verify subsequent changes on a fresh native run, healing before the rat fight.
2. Continue the mandatory route in order, using normal game inputs and read-only
   inspection; recover each missing routine from original evidence.
3. Complete the Day 1 branch inventory and close optional/recovery paths while
   restoring shared systems, including audio and movies.
4. Run the complete original/native comparisons and visible retail acceptance
   routes on the final implementation. Investigate all differences.
5. Mark completion only when the scope ledger has direct evidence for every
   requirement and no Day 1 omissions remain. Keep the goal active otherwise.

## Full disc-loader discrepancy resolved in DAY1-1

The original complete 6CDA4 routine returns0/F0=0 for an empty XA archive;
the former native dispatcher returned1/F0=7. The native port now restores the
181-word routine's internal loop, read-error retry and blocking/yield behavior.
The complete-original oracle and production native comparison pass336 cases
with1198 calls to controlled CD/SPU providers. The original counterexample
also matches all non-stack RAM below801F0000 after the fix.

Normal and ASan/UBSan CTest each pass3/3, including1210 native groups and
336 loader cases. Evidence: `local/live/ctest-day1-1-final.log` and
`ctest-asan-day1-1-final.log`. This verifies loader control flow under the
stated provider contracts; full providers, audible playback and live Day 1
coverage remain open. DAY1-1 is not part of the published SEW22 packages.

## Music-bank loader translated in DAY1-2

The complete original6D2B8 music-bank routine now has a native translation.
Its864 original-execution comparisons cover lookup, signed channel/bank IDs,
busy/existing channels, unloading duplicate channels, data copying and
blocking/retry control flow. The216 calls to the loader/queue use controlled
contracts; BIOS memcpy is also an explicit contract. Normal and sanitizer
CTest both pass4/4. This routine is not yet wired into EA200/201/203; those
commands, provider integration and audible playback still require completion.
DAY1-2 is not published. Run24 uses DAY1-1 and healed18→45 after first Eve
before continuing to the corridor, via normal inventory input.

## Partial route inventory and a later transition gap

[DAY1_ROUTE_AUDIT.md](DAY1_ROUTE_AUDIT.md) records27 verified original room
transfers across12 chunks, including alternate exits and the M0000I sentinel.
This is partial static evidence, not proof of every reachable branch or the
Day 1 terminal state. The sentinel dispatch calls6ECEC (translated in DAY1-8)
and8019234C (still a bootstrap stub). Restore and verify that later transition as part of
this objective; do not treat arrival at M0036I or g74=0x80 as completion.

Run24 later completed both keys and entered rehearsal47440. At50516 it
stopped explicitly at M0023I effect callback8018F3C8 with HP34. Its210-word
main and241-word particle code match the original disc chunk; they still
need native translation and complete original-execution comparisons.

## DAY1-3: rehearsal particles and command

The callback reached by run24 (8018F3C8), its particle8018F004 and command
80190644 now have native translations. All167 complete original-execution
cases pass, including the real pool, GTE, ribbon and sprite callees. The
run24 initialization copy matches all RAM below801F0000 and returns252 in
both implementations. Normal and sanitizer suites each pass4/4 (1211 native
groups plus the disc/music loader suites). No new live replay or publication
has occurred. Descriptor callbacks8018F710 and8018FC14 remain to be restored;
full rehearsal battle and full Day1 acceptance are still open.

## DAY1-4: shared triangle fan

The original439-word D004C fan renderer now has a native translation, with74
original-execution comparisons passing. Normal CTest passes4/4 in49.76s and
ASan/UBSan passes4/4 in53.98s (1212 native groups). Logs are
`local/live/ctest-day1-4.log` and `ctest-asan-day1-4.log`.
This shared routine supports both remaining rehearsal callbacks; those
callbacks and live encounter verification remain open. Original-code probes
also establish F710's dependence on historical caller stack bytes for some
generated coordinates. That behavior requires faithful handling before its
translation can be accepted. DAY1-4 is not in the published launcher build.

## DAY1-5: rehearsal beam and collision notification

Callback8018FC14 now implements the complete original652-word routine.
All187 original/native comparisons pass, including real shared drawing and
collision callees. Coverage checks require10 consumed collision latches and
all four mesh packet formats. Normal CTest passes4/4 in52.13s and sanitizer
CTest passes4/4 in55.98s (1213 native groups). Logs:
`local/live/oracle-day1-5-beam.log`, `ctest-day1-5.log`,
`ctest-asan-day1-5.log`.

On a copy of run24 RAM with an explicit test data buffer and projection
controls, original and native initialization, update and draw return0 and
match every non-stack byte after each mode (including mapped scratchpad).
They use the actual room mesh8019778C and allocate1684 drawing bytes.
`local/live/m0023i-beam-live-comparison.json` records the comparison.
This is copied-state evidence, not a new live encounter. F710 remains
unported, full Day1 acceptance is open, and DAY1-5 is not published.

## DAY1-6: flash callback verified, caller integration open

The321-word F710 callback now has a native translation with its borrowed
stack vector represented as explicit input/output. All173 original/native
cases pass, including12 paired flashes and comparison of the retained bytes.
Normal CTest passes4/4 in52.89s; sanitizer CTest passes4/4 in58.44s
(1214 native groups). Logs: `local/live/oracle-day1-6-flash.log`,
`ctest-day1-6.log`, `ctest-asan-day1-6.log`.

Production mode2/state0 still stops when the original stack context is
unavailable. [M0023I_FLASH_STACK.md](M0023I_FLASH_STACK.md) records original
writer instructions and the two-flash dependency found in the original
effect pump. Wider field-loop observation remains incomplete; callback tests
do not close that gap. No new live encounter or launcher publication occurred.

## DAY1-7: flash caller connected; live run25 started

The original35558 field-loop trace now runs with supported GTE flags and
confirms the borrowed vector's writers, including paused-frame retention.
The native69594 pump tracks those writes and invalidates unknown contexts.
All84 original/native pump frames pass across six histories;64 copied-state
frames with actual rehearsal assets also match every defined non-stack byte,
through the beam phase. GPU output arenas are explicit fresh fixture inputs;
effect state and original CPU stack persist between frames. This is stronger
caller evidence, but does not replace live battle acceptance.

Normal CTest4/4 PASS52.84s; ASan/UBSan4/4 PASS58.78s;1215 native groups.
All173 flash and187 beam original return/hash pairs remain unchanged after
the test interpreter changes. Run25 now uses this build with normal keyboard
input and read-only captures. Full Day1 coverage and final release remain
open; the launcher still serves SEW22.

## DAY1-8: transition asset loader

The full214-word6ECEC loader is translated and connected. All48 original/native
provider-contract cases pass, covering14064 calls, both texture banks, pending
and failed reads, retries,265 TIM uploads, and retained versus reread image-base
values. Normal CTest5/5 PASS47.83s; sanitizer5/5 PASS55.92s. The actual CD/TIM/SDK
providers and the loaded8019234C frame loop still require transition acceptance.

## DAY1-9: rehearsal actor turn

Run25 passed the previous M0023I effect boundary and stopped at script76,
80014BA0, frame49234 HP34/g74=5B. The full128-word command now turns actors
using the original task state, direction choice, signed halfword steps and
angle wrap conditions. All520 original/native cases pass, including1560 VM
frames. Temporary VM argument storage is excluded because native uses a fixed
scratch vector where the original uses its CPU stack. The direct command on
a copy of run25's stop RAM returns0 in both implementations and matches all
2097152 RAM bytes without exclusions. Normal CTest5/5 PASS60.79s,
Release5/5 PASS24.57s, ASan/UBSan5/5 PASS113.27s;1216 native groups.
Windows runtime crossbuild passed; its execution is untested. Both extracted
packages passed binary/disc/cue verification; Linux wrapper120-frame boot passed.

The user requested publication of the latest PC port on2026-09-06. Checkpoint
PE-DAY1-9-0c462bc18581 is now published for Linux and Windows. Full remote
archive hashes, signed ranged GETs and channel readbacks verified; independent
remote channel check also passed. Summary in
`local/live/publish-day1-9/published-summary.json`. The all-Day1 goal remains active.

## DAY1-10: Arrange Items and shared sorting

Run26 stopped when opening Arrange Items (46DFC). The complete menu graph,
confirmation/cancel paths, label drawing and original category/stat sorting
now have native translations, including the original quicksort pivot/equal-key
order and equipped-index relocation. The main field-menu command and shared
input/draw dispatchers are connected. Unknown sorting callbacks stop explicitly.

The checksum-pinned original oracle executes542 cases without replacing its
callees. All542 native comparisons PASS in normal and ASan/UBSan builds. Coverage includes carried/storage inventory,
empty/single/full lists, duplicate sort keys, signed stat modifiers, all submenu
branches, event precedence, both GPU banks and packet-arena wrap.

Six consecutive steps from copied run26 RAM (open/draw/confirm/draw/sort/draw)
match every byte below801F0000, excluding only the original CPU stack region.
The copy synchronizes the captured host D1A0 value. Evidence:
`local/live/inventory-sort-live-comparison.json`; test reproduction:
`python3 pc_port/tools/pe_inventory_sort_oracle.py --write-header`.
Full normal CTest5/5 PASS65.17s and ASan/UBSan5/5 PASS84.44s
(initial506 cases; final542, including corrected GPU-bank fixtures, additionally pass both focused runs). Windows runtime crossbuild PASS; execution remains untested. Run27 visibly
opened/cancelled the menu and cancelled/reopened/confirmed all three sort
submenus, preserving every item and both equipped identities. Screenshots and
order/frame records: `docs/evidence/pe-day1-arrange-items/`. Medicine1 then
healed16→45HP; the mandatory route replay continues. This does not establish full menu/save or Day1 acceptance.

## DAY1-11: rehearsal flashes alongside pistol effects

Run27 passed actor-turn14BA0 and entered the battle, then stopped48900 on
unknown flash stack Z. Original tracing identified casing instructionC9EF8
as the writer (signed age times64), and CA444/450 as the distinct muzzle
writer. The native69594 draw context now tracks both and preserves values
across original empty weapon draws. Unknown callbacks still invalidate.

644 original/native frames PASS in normal and sanitizer builds, including
both GPU banks, signed ages, draw ordering, pause and empty callbacks.
Actual run27 copied state matches every defined non-stack byte over64 full
pumps with weapon updates and beams. Details and limitations:
`M0023I_FLASH_STACK.md`. Live run28 passed the flash boundary and stopped on script36 at49014. FullDay1 accuracy,
Windows execution and final package publication remain unfinished.

## DAY1-12: tracked actor approach (script36)

Run28 passed the flash path, then stopped49014 (regular snapshot49010) at
13E84, pc801D94D4, actor800BF210, HP4 alive. This is a missing command,
not a completed or defeated encounter. The233-word original graph now runs
natively: actor lookup, retained identity/rate, live target tracking, steering,
fixed-point movement, arrival and VM retry. Signed-edge arithmetic in the
existing arctangent helper preserves32-bit wrap without C undefined behavior.

536 histories /840 steps compare complete original handler/VM execution,
including camera speed scaling, missing/removed/reused targets and signed
edges. Normal and sanitizer focused checks PASS. Copied run28 handler
matches every non-stack byte; fullVM additionally excludes its established
host argument vector80120F80..80120FBF. Copy-only preparation restores the
current opcode and the VM's already-consumed delay. No gameplay RAM writes.
Evidence `local/live/script-follow-live-comparison.json` and its probes.

Full normal CTest5/5 PASS59.98s, sanitizer5/5 PASS72.19s,1218 native groups.
Windows runtime crossbuild PASS; execution untested. New Linux runtime
SHA25694e7b4b7a452927fa53cc5d84fd525e21e731d1d82dc79e148517ef0a0b91799.
Fresh live actor-follow/rehearsal victory, remaining Day1 scope and final
publication remain necessary. Run28 exposed the need for an ordinary battle
movement/healing strategy rather than relying entirely on Cross taps.

## DAY1-13 hit initialization restored (2026-09-06)

Restored original BE180.s CD980/CDA5C/CDC24 through PE_WeaponCallback in
C2414_port: next hit target/terminal marker, copied impact coordinates,
conditional sound FIFO request/texture selection, randomized particle and
spark records. The separate CDD0C/CDE90 draw callbacks and C3B04 renderer
remain explicit boundaries. No complete hit-rendering or live hit claim.

`pe_hit_init_oracle.py --write-header`:90 full original/native cases PASS in
normal and ASan/UBSan builds. Covers nine RNG seeds, signed coordinates,
target indices/next-target flags, disabled/invalid/valid sound banks and
complete C2758 creation of all three descriptor types. Existing CEDA8 tests
cover actual texture transfers; these fixtures use cache hits. Pinned original
executable authority, no substituted original callees or matching-C claim.

Normal CTest5/5 PASS65.59s; sanitizer5/5 PASS82.69s,1219 native groups.
Windows runtime crossbuild PASS (Docker dockcross/windows-static-x64).
A direct host attempt could not use the container-owned CMake cache; rerun
inside its original container succeeded. Windows execution remains untested.
Logs `local/live/{oracle-day1-13-hit-init,native-day1-13-hit-init,
native-asan-day1-13-hit-init,ctest-day1-13,ctest-asan-day1-13,
build-win-day1-13-runtime}.log`. Linux runtime SHA256:
6ce8bc7d8bda15ee07633f2682b646754d965137aa31bc57fad7c1d04a48aaa0.

## DAY1-14: hit-draw renderer and callbacks

Callback `800CDD0C` / `800CDE90` and shared renderer `800C3B04` now have
native translations from the original 488-word / 97-word / 44-word
graphs, including the three COP2 wrappers they call. `pe_hit_draw_oracle.py
--write-header` executes 96 complete original cases; native comparison
PASS in normal and ASan/UBSan. No live hit-draw or matching-C claim.
Normal CTest 5/5 PASS 67.93s (1220 native groups). Linux runtime
SHA256 `699923ad70707f51bc4ef0b9ff9273a5158f5a27237ddbc09fb0706c8de51b8f`.
Run30 on the prior DAY1-13 binary reached stage-exit heal, then host-quit
before rehearsal. Run31 is the next live replay on this binary.

## DAY1-15: EA200/201/203/204 start/stop wired

`func_80015DAC_default_cut` no longer returns success for unused music
start/stop keys. Keys 200/201/203/204 now follow the original 15DAC
graphs through `6D2B8` and the `86464`/`86498`/`864F8`/`86770` producers.
64 original-execution cases that do not enter `6CDA4` match native RAM
and return values (`pe_music_start_oracle.py --write-header`;
`PE_TEST_FILTER=DAY1_music_start` PASS; 1221 native groups).

This is not audible SPU synthesis and not score sequencing. Blocking CD
loads that return `v0=1` from `6D2B8` still need live provider evidence;
the native yield path copies `16758` (`D_8009CE00 -= 0x20`,
`*(D_8009D300+0x10) = 1`). Full Day 1 acceptance remains open.

## DAY1-16: two M0000I overlay leaves

`func_80191DE8` and `func_8019BF8C` match 9 complete original executions
from the loaded transition overlay. This is not the 253-word `8019234C`
frame loop and not the 1474-word `80196498` init. Full Day 1 acceptance
remains open. `PE_TEST_FILTER=DAY1_m0000i_leaves` PASS (1222 groups).

## DAY1-17: M0000I `80193AB0` camera-add leaf

`func_80193AB0` (43 words, SHA256
`766697b440a25e78d5c6a5ffdcc31ef20eac5693dca2db7132022a3ca69ca409`)
matches 5 complete original overlay executions plus the prior 9 leaf
cases (14 total). Original `lh` counter at `8019C058`: signed `blez`
returns without writes; otherwise decrement and add 10 stride-16
records from `801EA268` into dest starting `8019CAA8` (words 0/1/2,
`addu` wrap). One `jal` from `8019234C` at `8019264C`. Not matching C.
Not the `8019234C` frame loop. `PE_TEST_FILTER=DAY1_m0000i_leaves` PASS
(1222 groups). Full Day 1 acceptance remains open.

## DAY1-18: M0000I fade stepper `80194108` / `801941A4`

`func_801941A4` (86 words, SHA256
`a15f132798e64eb0b1af4017da3f62159d6eb058f63511459ae8f90cd5e7d529`)
builds a semi-trans PolyG4 fullscreen quad (320×240) from `*8019C9C0`,
AddPrim, then DR_MODE `a3=64` and a second AddPrim. `func_80194108`
(39 words, SHA256
`2568db564110f734694094eac85f6a0adfa73263e2d151f304b8b8d0d93c0701`)
is the signed fade stepper that calls it: `s16>0` draws `min(level,255)`
and adds 16 only when `<255`; `s16<-254` draws 0 and clears; otherwise
draws `(level-1)&0xFF` and subtracts 16. Returns the next `s16`. Called
from `8019234C`. Not matching C. 31 original overlay cases PASS,
including `v0` on the stepper. `PE_TEST_FILTER=DAY1_m0000i_leaves` PASS
(1222 groups). `8019234C` / `80196498` remain unported. Full Day 1
acceptance remains open.

## DAY1-19: M0000I `80192740` OT-link (`C02C==0`)

`func_80192740` (48 words, SHA256
`3f01fcd7ae8cdfc7cbaba7e0efb473012ef7a8f9f82bd0e4e1eaf9d0c0174845`)
matches 4 complete original overlay executions with `*8019C02C==0` (no
`jal 80193B5C`). If `*8019C1F0==1`, AddPrim dest `801EA598` or `+36`
when `*8019C9C0!=8019C1F8` into OT word `+0x3FFC`. The `C02C!=0` /
`80193B5C` graph is unported. 35 original overlay cases PASS.
`PE_TEST_FILTER=DAY1_m0000i_leaves` PASS (1222 groups). `8019234C` /
`80196498` remain unported. Full Day 1 acceptance remains open.

## DAY1-20: M0000I `80193B5C` fade sprites (`flag!=1` graph)

`func_80193B5C` (363 words, SHA256
`405eaed226fc1c7529fda0d1319095801abefe4d74a64dbcf4ca94af66e11aa8`)
plus `func_80077BE4` (SetPolyFT4) and `func_80079274` (RotAverage3,
`SZ3>>2`). First compared original overlay executions with per-slot
`801EA394` flags not 1 (no RTPT/FT4 fill). Clamp returns and the
`8019CA80` OT+0x28 link match, including `92740` with `C02C=10`
(`v0=0x12`). `8019234C` / `80196498` remain unported. Full Day 1
acceptance remains open.

## DAY1-21: M0000I `80193B5C` RTPT/FT4 (`flag==1`)

Original overlay slot0 (`lbu 801EA394==1`) fills TILE+FT4 packets.
Positive fade skips `addiu t0,t3,255` at 80193C24 (`bgez` lands on
80193C28); brightness is `lhu(fade)` then `sll 16` / `srl 17` and
`srl 19`. Negative fade adds 255 first. Native half/third now follow
that split. Two original overlay executions (a0=1 and a0=-8, slot0
flag 1, zero RT/verts → SXY 0) match native packet bytes, OT links,
and `v0`. `pe_m0000i_leaves_oracle.py --write-header`: 48 original
overlay cases. `PE_TEST_FILTER=DAY1_m0000i_leaves` PASS (1222 groups).
`80191E30` (51w) and `80191EFC` (77w) still need unported 78A94/78B38
Push/PopMatrix helpers. Full Day 1 acceptance remains open.

## DAY1-22: `80078E04` / `80078E94` GTE RT/TR

Original 12-word SetRotMatrix (`ctc2 $0..$4` from five MATRIX words)
and 8-word SetTransMatrix (`ctc2 $5..$7` from +0x14). Native
`PE_GTE_LoadRT33` / TR stores follow that layout. Layout test
`PE_TEST_FILTER=DAY1_gte_rt_leaves`. Python `execute()` ctc2 lands in
cop_control, but its RTPT path does not yet consume those words the
same way as native GTE, so RTPT-after-ctc2 is not hash-compared.
`80191E30` still needs 78A94/78B38. Full Day 1 acceptance remains open.

## DAY1-23: Push/Pop/`6EC6C`; blocking EA200 SPU IRQ

`func_80078A94` / `func_80078B38` follow the original 40-word
Push/PopMatrix bodies (depth `D_800963E8`, stack `D_800963EC`,
`cfc2`/`ctc2` `$0..$7`). Overflow/underflow print arms are collapsed
like other `71A74` sites. `func_8006EC6C` matches the original 5-word
`sll 16` / `sra 14` walk: `a0 + *(a0 + (int16)a1 * 4)`. Three original
EXE executions PASS (`pe_gte_stack_leaves_oracle.py`). Native
`PE_TEST_FILTER=DAY1_gte_rt_leaves` includes the stack round-trip.
Python `execute()` does not write cfc2 into guest RAM, so Push/Pop are
not hash-compared.

Run35's skip-movie stall was EA200 in `6CDA4` state 10: `870E0` kept
reading busy `D_8009D24C` because the host DMA4 IRQ
(`PE_SpuDma_Service`) only ran at VSync. The original blocking poll
never reaches VSync; the host now services that IRQ inside the
blocking loop. After the pump, dest-ready completed, EA200 finished,
and dest `0xA8001048` presented (915 frames / 20s). Not matching C.
`80191E30` is now a native 51-word translation (6EC6C / Push /
SetTrans / SetRot / 6DF50 / Pop). Run36 died on `8CA84` cmd `0x11`;
run37 died in the lobby on cmd `0xC1`. Both commands and the
`8A354` id filter are now native; 81 original audio-command graphs
PASS. Full Day 1 acceptance remains open.



## DAY1-25: explicit start/end music-volume consumer

EA207's existing C2 producer now has its original 8008B410 consumer
(92 words; SHA256
`1c93fead13abb7debfb312356fd02e47a337db821bc19c3bab9417fb986043f2`).
Native translation preserves owner selection, masked endpoints, signed
division, zero-duration substitution, halfword countdown and voice flags.
`pe_audio_commands_oracle.py --write-header`: 196 original graphs PASS
native and ASan/UBSan; C2 asserts changed persistent RAM is inside the
compared ranges. This proves command application, not audible sequencing.

Normal CTest 5/5 PASS (60.64s), after supplying the boot music arena to
the isolated BTL94 M0367I fixture. Focused BTL94 ASan/UBSan also PASS.
Run38 ended with host-quit at 149257, story g74=1; helper route timeouts
provide no rehearsal or sewer acceptance. Full Day1 scope remains open.


## DAY1-26: transition sound position to active voice state

80191EFC (59 words; SHA256
`0b26f974cb4a5b285e42182b6798d484dcc74e99843073d22642efb010a347af`)
now translates the original camera/position projection and volume/pan
requests. Its 868F0/86A28 producers and A0/A2 consumers (8B698/8BA3C)
are also native. Handle masks FFFF and 3FF intentionally remain distinct.

184 original graphs compare both queued commands and consumed voice RAM;
382 original audio-consumer cases cover the expanded command table.
Both focused native groups PASS normally and under ASan/UBSan. Original
execution asserts changed non-stack RAM lies inside the compared ranges;
native checks restoration of camera, matrix stack and temporary scratchpad.
Full normal CTest 5/5 PASS (62.46s; 1224 groups).

The call site at 80192554 is established from the original 8019234C.
That frame loop, its initialization and rendering/input graph remain
unported. No live transition or audible SPU verification is claimed.
Full Day1 route/content/platform acceptance remains open.


## DAY1-27: transition object pool and constructors

Thirteen original M0000I routines now have native translations:
90998 initialization, 91580 record reset, 915DC/91678 allocation/free,
91740/91754 iteration, 917BC/91834 depth-list attach/detach, 9959C
relative metadata lookup, 90AEC/90B78/90C1C constructors and 90D08 removal.
The original 96498 initializer calls the init and all three constructors.
This does not restore the rest of 96498 or the transition frame loop.

`pe_transition_pool_oracle.py --write-header` executes 2589 original
history steps. Native return values and persistent RAM match after each
step, including all 200 slots, iteration, head/middle/tail removal,
permuted complete drain/refill, slot reuse, reinitialization, both parent
depths, stack arguments and signed/truncated metadata indices. The oracle
asserts all changed non-stack RAM is compared. Partial initialization and
untouched bytes are tested using three distinct starting patterns.
Focused ASan/UBSan PASS; normal CTest 5/5 PASS (54.85s; 1225 groups).

Original code ranges and SHA256:
- 80190998..80190D3C: `2cdda535f38ce7887a542c03e66274c60fbd831873aff6be55c4d73fb9557215`
- 80191580..80191854: `2483607765c2070a5e88e2c8b3dfcf7823286ee86642b94504fda5d8c710786e`
- 8019959C..801995BC: `b7a1140bc3490c9ca2ed30f37f8725f348a6afa525d0b383efdabbc07e48b582`

No live transition or scope-wide Day1 acceptance claim. Remaining transition
initialization/rendering/input, route/content, audiovisual and platform work
remain open.


## DAY1-28: transition gradient and fade packet initialization

80195F6C (331 words; SHA256
`112b2d3c7bdaedac44c218a012e4b56385b77dbbe75f1e0eeccbb3ed3cbd1bf6`)
now creates both frame-buffer banks of background gradients, top/bottom
fade strips, and draw-mode packets. The original call is in 96498.

160 original history steps compare packet bytes, preserved bytes and
92740 ordering-table links, including reinitialization after linking,
both banks and five initial patterns. Native and ASan/UBSan PASS; all
changed non-stack RAM is inside compared ranges. Full CTest 5/5 PASS
(52.46s; 1226 groups). This is packet/state evidence, not live transition
visual acceptance. The remaining transition initializer/frame loop and
full Day1 content/platform verification remain open.

Fresh run39 passed naming with ordinary inputs (D154 cleared), then
courtyard3730 and lobby4100. Run38 had failed in its naming helper;
its later route timeouts did not establish a game traversal failure.
No gameplay RAM writes or new sewer/transition acceptance claim.


## DAY1-29: transition lighting/color/fog setup

Original EXE 78E34/78E64 matrix setters, 78FC4/78FE4 background/far
colors and 77E64 fog near/far setup are now native. Wrapped products and
shifts, signed division, short-span no-op, signed slope clamps and the
original arithmetic-trap boundaries are preserved. Far-color storage
alone does not supply missing depth-cue rendering commands.

`pe_transition_gte_oracle.py --write-header`: 123 original cfc2 register
readbacks plus 2 arithmetic-trap cases PASS native and ASan/UBSan. The
readback harness follows the untouched retail graph; halfword-width
terminal matrix controls and DQA are masked accordingly. Original DIV
zero is reported by the interpreter before its subsequent BREAK; overflow
reaches the original BREAK instruction. Full CTest 5/5 PASS (82.71s;
1227 native groups). This is setup evidence, not live transition lighting.

Run39, on the preceding DAY1-28 binary, cleared first Eve (14600->16290),
regained control17310, exited stage19590, used Medicine21->45 at20840,
and passed backstage21890, understage22860 and corridor24460. Ordinary
input only. The route helper remains active toward keys/rehearsal;
no new rehearsal victory, sewer or Day1-end acceptance claim.


## DAY1-30: rehearsal contact crash; GPU quick-fill hardware behavior

Run39 crashed in the rehearsal battle at frame49207. Original1D268 on
its saved RAM returns with all2MiB unchanged: it reads physical address0
through the enemy's idle action pointer. The port now preserves this
read through the cached RAM alias. The279-case original contact oracle
includes65 new physical/cached-address cases, varied kind bytes, whole
contact passes, and the observed crash state. Native and ASan/UBSan PASS;
byte1 cases prevent accepting a fabricated null-pointer early return.

GP0(02h) now uses the hardware RGB ordering,16-pixel alignment/rounding
and independent VRAM wraps. Twelve full-VRAM checks include both frame
buffers, masks, drawing environment, zero sizes and full-width rounding.
Normal and sanitizer fill tests PASS; five presentation tests PASS.
Authority: https://psx-spx.consoledev.net/graphicsprocessingunitgpu/
and https://psx-spx.consoledev.net/memorymap/ . Full CTest5/5 PASS
(57.74s;1228 groups). SDK ClearImage remains a host-only shim; this cut
does not establish correct retail frame-buffer clearing through that SDK.

Fresh run40 is replaying ordinary inputs on the corrected build. Naming
accepted; courtyard-to-stage, healing/keys and rehearsal helpers are
sequenced. No new rehearsal victory or Day1-completion acceptance yet.


## DAY1-31: ClearImage worker packets, queue ownership and GPU status

Restored original76434..76664 worker with in-place signed RECT clamps,
fast-fill versus precise-rectangle packets, drawing-area/offset restore,
and real76B98 submission. Guest RECT direct76C34 and queued76EE4 paths
now recognize76434. The180 original-executable cases model only the
GPU info/submission providers and compare exact packet/RECT results.
Native GPU integration checks both buffers, precise bounds, environment
restoration and queue ownership after the source RECT changes.

GPUSTAT now reflects E1/E6 drawing state and embedded polygon tpage
updates, fixing a missing readback required by the original clear worker.
Hardware authority: https://psx-spx.consoledev.net/graphicsprocessingunitgpu/ .
Targeted native and ASan/UBSan clear/status tests PASS. Final CTest5/5
PASS (58.73s;1232 groups). General GP1 info latch remains unimplemented;
this worker uses explicit v2 info3/4/5 platform contracts. SDK74F44 remains
the old host-only shim pending mutable transient-RECT integration: no
claim that production SDK frame-buffer clearing is complete.

Run40 on DAY1-30 remains live: first Eve exited16580, regained control17730,
stage_exit22060, Medicine21->45 at22290, corridor rat defeated26390,
mirror exit27410, theater key32430 and room exit32890. Ordinary keys only.
The sequential driver continues keys/healing then rehearsal. No new
rehearsal victory or full Day1 acceptance yet.


## DAY1-32: production SDK ClearImage uses retail GPU dispatch

Replaced74F44's host-only shim with its validator/jtb/packed-RGB dispatch.
Direct native RECT width/height clamps are preserved; queued work owns
an8-byte guest copy and cannot retain the caller's pointer. The shared
original clear worker now serves both typed and guest paths. SDK clearing
writes VRAM without premature host presentation. Boot fixtures/canaries
now include original static dispatch data and packet writes; RGB(0,0,1)
quantizes to black with column1023 preserved by the original rectangle.

Seven original SDK argument/return cases and180 original worker cases
PASS. Normal and ASan/UBSan clear/streaming checks PASS. Full CTest5/5
PASS (55.19s;1235 groups). General GP1 info latch and transition caller/
frame-loop work remain open; this is not full transition visual acceptance.

Run40 recovered a waypoint stall, reached rehearsal49430 and healed to45.
The moving battle strategy ended in defeat, without the earlier contact
abort. Game returned to opening; subsequent manual close caused X11
BadDrawable exit1 (not a gameplay crash). Run41 now replays the updated
SDK build from the opening, with no gameplay RAM writes. Rehearsal
victory, optional-resource coverage and full Day1 acceptance remain open.


## DAY1-33: transition double-buffer setup and current-bank reset

Translated original9BD78/9BF50: two draw/disp environments, packet
cursors, ordering tables, geometry/light/fog setup, both frame-buffer
clears, display enable/current-bank publication and per-bank reset.
80 original history steps compare state using explicit hardware provider
contracts; native checks also inspect complete VRAM, mask/presentation,
GTE readbacks and unresolved-provider prefixes. Native and ASan/UBSan
PASS; full CTest5/5 PASS (56.98s;1237 groups). Transition initialization/
render loop is still incomplete, so no live transition acceptance yet.

Run41 cleared first Eve, reached backstage21650 and declined the hole
prompt after the backup-arrival line. Backtracked through the original
backstage exit polygon to stageA80002C8 at38100. The optional police
visit and supplies are being checked using ordinary inputs, with this
walkthrough as a route hint: https://shrines.rpgclassics.com/psx/pe/day1.shtml .
No healing/ammo or optional-item receipt has yet been verified in this run.


## DAY1-34: transition selection initializer and package read retries

Restored original91854 scene/encounter/flag selection and91C94 two-stage
package reads. Preserve untouched fields and second-span choice while
rereading sector table/base on retries. Explicit stop epochs prevent loops
past unresolved providers. Original-instruction provider-contract oracles:
2304 initializer cases and64 package cases/832 CD calls. Native and
ASan/UBSan PASS, plus4 initializer and18 package stop prefixes. Full
CTest6/6 PASS (77.11s;1237 native groups). This verifies control flow and
state around explicit providers, not real package I/O or the visible
transition. Outer96498/9234C still missing; Day1 acceptance remains open.

Run41 remains live on DAY1-32. Ordinary input backtracking reached center
stage80550 and front stage81050, then stalled at(1624,1409) before the
positive stairs82980. Approaching the stair base triggered descent85470 and returned to
auditoriumA8000248/control85880; the last helper waypoint crossed back
to stage by86670. The stair route is now identified.
No optional police healing/ammo or rehearsal victory accepted yet.


## DAY1-35: cyclic transition path interpolation

Restored originalF55C sampler with original ratan2, heading wrap,
fractional position/heading, turn clamp/deadzone, signed path counts,
sticky completion flag and alias-preserving output writes. 1440 original
instruction cases and two zero-divisor prefix checks; native and
ASan/UBSan PASS. Full CTest6/6 PASS72.80s (1239 native groups). This closes
a constructor dependency, not the outer transition or its visual acceptance.
96498/9234C and remaining camera/object/frame routines still incomplete.

Run41 returned to auditoriumA8000248/control102740 via(2250,1800) on
stage; now attempting the auditorium aisle. Ordinary X11 input only;
no police resources or optional-item acceptance yet.


## DAY1-36: camera path selection, start and advance

Restored95994/95D3C/95E4C over the original package lookup and newly ported
sampler. Preserve paired eye/target deltas, from/to vectors, MIPS-masked
shift counts, byte-counter wrap/exhaustion, signed IDs and completed first
sample state at a later failure. Temporary guest scratch is restored.
2048 original history steps and six zero-divisor prefixes match native and
ASan/UBSan; full CTest6/6 PASS89.53s (1241 native groups). Constructor and
render-loop closure remain incomplete; no visible transition acceptance.

Run41 remains live in auditorium on DAY1-32. Ordinary-input aisle attempts
reached z5176. The offset route subsequently triggered a backstage
young-girl encounter by127410 (HP21/45,g74=48), captured in
sewers41-offset-result.png. No optional
police resources, room completion or rehearsal victory accepted yet.


## DAY1-37: direct camera target setup

Restored original95BC8 with exact alias-sensitive read/write ordering for
both camera target vectors, current positions, deltas, accumulators and
shift counters. 4624 original-instruction history steps match native and
ASan/UBSan. A terminated normal build and stale-binary skipped tests were
rejected as evidence; completed rebuilds passed the actual target test.
Full CTest6/6 PASS89.31s (1242 native groups). Next is the full96498
constructor; its direct dependencies are now ported. Visible transition
and full Day1 accuracy remain unverified.

Run41 advanced the young-girl scene to g74=56,roomA80010C8/control at
133500. The old backstage backdrop persists through movement, so the room
rendering is not accepted. The field menu works and shows Medicine1;
used through its popup, healing21->45; menu fully closed148320.
Initial empty-slot action moved the item before cancel/reopen recovered
normal use. No optional police supplies or
rehearsal victory accepted yet.


## DAY1-37 launcher publication — 2026-09-07

User-requested interim build PE-DAY1-37-40d88b0bf8ad compiled and published
for Linux and Windows. Release CTest 6/6 PASS (1242 native groups); Windows
Wine startup 120 frames PASS, not native Windows OS validation. Extracted
packages passed binary/disc/cue/metadata checks and Linux wrapper startup.
Private remote checksum checks passed. Default public channel fetched without
a cache query names DAY1-37; full public downloads of both platforms match
expected SHA256, size and embedded metadata. Evidence:
local/live/publish-day1-37/launcher-verification.json and
local/live/public-verification-day1-37.log. Replaced public DAY1-10.
Full Day1 retail parity remains in development; constructor96498 is not yet
implemented. Publication adds distribution evidence, not new gameplay acceptance.


## DAY1-38: complete transition constructor

Restored whole96498..97BA0, including partial record initialization, all
object placement lists, resource-relative lookups and entry-mode camera tails.
720 full original-instruction reference cases match native and ASan/UBSan,
covering all10 initial camera variants and pool occupancies55/121/199.
Original execution uses real pool/packet/resource/path/camera callees; scene
setup, window setup and GTE setters use explicit contracts. Six stop prefixes
also match (provider interruptions and zero-period path faults).
CTest7/7 PASS105.91s,1242 native groups; the subsequent expanded constructor
cases and fault tests separately PASS in normal and sanitized binaries.
Evidence local/live/{oracle-day1-38-constructor-full,oracle-day1-38-constructor-faults,
native-day1-38-constructor-final,native-asan-day1-38-constructor-final,ctest-day1-38}.log.
This proves constructor state/control behavior within those cases, not visible
transition parity. Frame loop9234C and its remaining update/render callees are
still unported. DAY1-38 is local; launcher remains DAY1-37.

Run41 previously recorded as live actually ended normally with host-quit at
frame151332,22:52:30. Exit0 and captured stop/backtrace confirm this; no new
gameplay acceptance follows from the quit. Latest HP45,g74=56,Aya(67,0,255).


## DAY1-39: transition exit routing and effect fade command

Whole92030 and helpers868AC/38D48 restored.3840 original routing cases and304
provider stop prefixes match native and ASan/UBSan. Graphics/audio calls use
explicit contracts, so this proves state/control behavior, not presentation.
Added missing A9/8B978 effect fade consumer; expanded real original FIFO
comparison to481 graphs including99 A9 cases, plus2 zero-divisor prefixes.
Both native and sanitized audio comparison pass. Full CTest8/8 PASS78.61s,
1242 native groups. Evidence in local/live/*day1-39*.log. Not published.
Audible fade and full transition still require integration/runtime evidence.

New user report: random battle froze, small black line at image bottom.
Neither is accepted/fixed yet. Requested room/platform/build; no live game on
shared machine to inspect. Integer aspect-fit centering can leave one extra
bottom pixel, but actual screenshot/dimensions needed before attributing line.


## DAY1-40: retail NTSC vertical display placement

Found live224-row source with screen.y8; host previously placed it at row0,
leaving16 black rows below. Restored original PutDispEnv vertical range and
clipping arithmetic, including default240 height, and blanked unused lines.
88 original instruction cases match native and ASan; CTest8/8 PASS74.23s,
1243 native groups. Independent new-binary live capture display40-final.png
shows image rows8..231 and equal8-row borders. Fix is local, not published.
This verifies NTSC vertical positioning; full GPU/PAL/interlace parity is not
claimed. Old run42 (DAY1-39) remains active to investigate user's random battle
freeze. Opening Eve battle completed14900..16680 and field control17760; this
is not the reported random-battle reproduction. Room/platform details pending.


## DAY1-41: missing Windows controls; live corridor progress

Win32 backend only mapped Confirm/arrows. Added all9 missing digital buttons
to match Linux. Real window-message test validates14 buttons, repeat/release,
combinations and focus reset. Dockcross Release and Wine input test PASS;
Windows executable startup120frames PASS. Native Windows OS proof remains
missing; no claim this caused reported random freeze. Launcher unchanged37.
Evidence local/live/{build-win-day1-41,wine-day1-41-input,wine-day1-41-startup}.log.

Run42 ordinary Medicine19->45, first scripted corridor rat defeated24080 and
fieldcontrol24230/g74=72/HP41. Theater Key acquired31270, Rehearse Key popup
37650/diary_key39050. No battle freeze encountered on this route so far;
unidentified user random-battle report stays open. Helper67859/game82826 active.

Run42 follow-up: route helper67859 timed out near the rehearsal door; ordinary
input recovery43198 entered45060. Battle helper13164 attempted healing atAT700
(main battle commands require9000), timed out, and Aya subsequently died.
Frame52560 reset HP is defeat, not victory. At59710 the game visibly returned
to opening street dialogue (sewers42-after-defeat.png). No crash/stop or user
random-encounter reproduction established. No live helper remains.

DAY1-41 launcher publication: PE-DAY1-41-01d9ff4d63a8 now serves both platforms.
Linux Release CTest8/8 PASS56.08s,1243 native groups. Extracted packages and
Linux wrapper startup passed. Both public full downloads matched SHA256/size
and metadata; default channel confirmed without cache query. Evidence:
local/live/publish-day1-41/launcher-verification.json. Display and Windows input
fixes shipped; unidentified random battle freeze and full Day1 acceptance remain
open. Wine validation does not establish native Windows OS acceptance.

## DAY1-42 — original camera basis graph (local, 2026-09-08)

Restored whole8018F344..8018F55C and its SDK78134/78194 vector normalization
graph. Original GTE square/GPF/OP and inverse-root lookup arithmetic, signed ADD
overflow boundaries, degenerate-vector Z adjustment, matrix padding and final
negative rotated-eye translation preserved. pe_transition_basis_oracle.py
executes original functions with real callees, including source SHA pins and
persistent write checks. 587 cases (267 normalizer /320 basis),34 signed ADD
trap prefixes; zero vectors, signed extremes, overlapping input/output storage,
18 terminal GTE control/data words for nontrapping cases. No terminal GTE claim
for trap prefixes, which compare persistent RAM and explicit native stop.
Normal and ASan/UBSan targeted PASS. Full CTest8/8 PASS66.43s,1244 native groups.
Evidence local/live/{oracle-day1-42-basis,native-day1-42-basis,
native-asan-day1-42-basis,ctest-day1-42}.log. LOCAL only, launcher remains41.
Outer8F05C/8F92C camera calls and9234C frame plus update/render callees remain
unfinished; this does not establish live transition or full Day1 acceptance.

## DAY1-43 — whole transition camera setup (local, 2026-09-08)

Restored whole8018F05C..8018F344 plus SDK787D4..78934 concatenation and
799E4..79C70 alternate Euler rotation. Original template/delta/normalization,
both angle sets, basis setup, camera matrices and final GTE setters preserved.
Concatenation retains alias-sensitive store ordering, full-word matrix padding,
short translation inputs and signed ADD traps. Temporary scratch128 bytes at
1F800280 restored on return and overflow. pe_transition_view_oracle.py runs
all original callees with source hash pins, persistent write checks and18
terminal GTE control/data words for successful returns. 524 cases:300 whole
views,124 concatenations,100 rotations, including28 overflow prefixes (RAM/stop
checks only for traps). Initial isolated SDK fixture IR0 mismatch corrected;
expanded normal and ASan/UBSan targeted tests PASS. Full CTest8/8 PASS75.47s,
1245 native groups. Logs local/live/{oracle-day1-43-view,native-day1-43-view,
native-asan-day1-43-view,ctest-day1-43}.log. LOCAL only, launcher41. Projection
8F92C,update942FC,render92800/93478 and outer9234C still missing; no live full
transition or scope-wide Day1 parity claim. User random-battle freeze open.

## DAY1-44: transition boundary planes and matrix-stack correction (2026-09-08)

Previous goal turn was progress: whole transition camera setup verified.
Current worktree already contained an unverified func_8018F92C_port.c draft
and linkage. Audited it against the complete original8018F92C..8018FFF4
(434 words), SHA2569c77bdeac144aeac997604f2c0a5b7cd52aab9b582bba9be26590296b045246e.
Its SDK792D4 transform/FLAG and791D0 cross-product helpers are included;
whole routine computes four planes, offsets, widths and wrapped squared
lengths, then restores the matrix stack. pe_transition_bounds_oracle.py pins
the original EXE/overlay and runs all real callees with strict FLAG checks.
600 original/native cases:200 whole routines,200 transforms,200 cross products;
input/output aliases, origin aliases of plane outputs, zero/large coordinates,
three valid matrix stack depths, persistent RAM and18 terminal GTE words.
Scratch112 bytes at1F800300 are verified restored. Full-stack diagnostics
and live outer transition acceptance are not covered by these cases.

First mismatch exposed shared PushMatrix's missing upper-halfword write:
func_800C3B04_port.c now writes full signed RT33 at matrix+16. Original CFC2/SW
writes all32 bits; the previous implementation only wrote18 matrix bytes.
Also corrected test interpreter CFC2 matrix-final-element sign extension
(registers4/12/20), per hardware documentation:
https://psx-spx.consoledev.net/geometrytransformationenginegte/ .
Second mismatch was fixture omission: wrapped negative squared lengths read
original bytes before the usual sqrt table. Bounds fixture now loads95D00
through963DC; no altered sqrt behavior. Removed temporary mismatch dumps.

Normal and ASan/UBSan targeted tests each PASS1 actual group,600 cases.
Full normal CTest8/8 PASS78.11s,1246/1246 native groups; all tool handles ended0.
Original view524 and basis587 regression executions also PASS after CFC2 fix.
Evidence local/live/{oracle-day1-44-bounds,native-day1-44-bounds,
native-asan-day1-44-bounds,ctest-day1-44,oracle-day1-44-view-regression,
oracle-day1-44-basis-regression}.log. LOCAL only; launcher remainsDAY1-41.

Next:942FC update,92800/93478 render and outer9234C remain unported.
Saved partial942FC disassembly at local/live/original-801942FC-region.txt;
inspected prefix calls95994,6DF50,6EC6C,8F55C,95BC8. Establish full extent
and dependencies before translation. Day2 full decompilation inventory remains
required by current goal; prior Day1-only handoff wording is superseded above.
User's unidentified random battle freeze remains unresolved; no live transition
or full Day1/Day2 acceptance is claimed by this arithmetic verification.


## DAY1-45: scene motion plus initial Day 2 inventory (2026-09-08)

Previous goal turn was progress: full bounds comparisons and matrix-stack fix.
Restored original93478..938E8 (284 words),938E8..939B0 (50 words), and
939B0..93AB0 (64 words) in func_80193478_port.c, public declarations and normal
runtime linkage. Combined398-word SHA256
ffc912797b4704567e6968ec68b5c1cabc72d327dd8c822438ef4ed3577e5b9a.
Four real path samples update scene actors, accumulate/clamp bank, subtract
1024 yaw, copy companion state in original block order, advance both clocks.
Preserves asymmetric retail behavior936D0..93728: tests second actor's bank
but changes first actor's bank. Both path-seeding siblings sample10 points;
one stores16.16 positions, the other wrapped signed32 deltas divided by32.
All three preserve temporary scratch and stop at original zero-period prefixes.

pe_transition_motion_oracle.py executes untouched original graphs and real
6EC6C/F55C/79FB4.1566 steps:1080 scene-motion history steps,480 seed/delta
steps,6 zero-period prefixes. Covers24 bank seeds,5 actor/companion alias
layouts including overlapping block copies,3 clock values includingFFFFFFFF,
signed16 IDs/high ignored bits,constant and turning paths. Original persistent
write whitelist checked; native compares all affected ranges and24 scratch
bytes. Normal and ASan/UBSan targeted PASS1 actual group each. Full CTest8/8
PASS69.22s,1247/1247 native groups; final build/test handles ended0. Logs
local/live/{oracle-day1-45-motion,native-day1-45-motion,
native-asan-day1-45-motion,ctest-day1-45}.log. LOCAL; launcher remains41.

Day2 scope now has docs/ai_context/DAY2_ROUTE_AUDIT.md and reproducible
pe_day2_route_audit.py. Original4 entry/shared scripts (M0351I,M0042I,M0041I,
M0043I),26 modules/3636 decoded command boundaries;16 pinned transfers and
6 immediate story writes. g74 means decimal74 (argument4A), not index74hex.
Tool validates original EXE and script hashes; output local/live/day2-route-audit.json.
Shared later-story branches (140/218 etc.) are not labelled Day2 without
predicate proof. This starts inventory, not full Day2 semantic decompilation.

Next missing transition: full942FC update ends958D4 (1398 words); its helper
958D4..95994 is48 words. Saved original-801942FC-whole.txt includes both,
combined SHA256c8bfdc9dd6a45967188d48a0dadf7f5778c888c2b0b4d43e66175d15d8f704ae.
Its callees include camera sampling,95994,95BC8,95D3C,sound6DF50/868AC/86C5C
and dialogue3746C/38940/375E0. The92800..93478 renderer (798 words) and its
90D3C/90E04/91114 graph still need restoration; whole9234C is still unported.
Saved original-80192800-whole.txt and original-80193478-whole.txt for review.
Continue full Day1/Day2 goal, including remaining optional/recovery/audio paths.
No live transition completion, user random-battle freeze fix, or full-day
acceptance is claimed. Final packages/publication remain unfinished.


## DAY1-46: whole transition update and message-state fix (2026-09-08)

Previous goal turn was progress: scene motion translations and initial Day2 audit.
Restored942FC..958D4 (1398 words), subtitle958D4..95994 (48 words), and
shared38940..38954 color setter (5 words). Combined overlay update/subtitle
SHA256c8bfdc9dd6a45967188d48a0dadf7f5778c888c2b0b4d43e66175d15d8f704ae.
New func_801942FC_port.c covers controller mode/selection/navigation, camera
samples and interpolation, timed subtitles/colors, message changes, exit
requests, and real sound/volume producers. Public/runtime linkage present.

Original375E0 reads up to5 halfwords while update/subtitle initialize only
one. Preserved remaining bytes through explicit context inputs instead of
inventing zeroes. pe_transition_stack_audit.py traces8 original executions:
subtitle tail comes from F55C saved s2/s3 and helper saved s0. Outer9234C's
s2=0/1,s3=00FFFFFF proves consumed subtitle list[0,0,-1]. Normal942FC wrapper
now supplies this context and passes38 complete original comparisons. Menu
tail has no writer within942FC and remains UNBOUND in the normal wrapper;
it stops at the original message-open prefix when needed. Full implementation
is available as PE_TransitionUpdate(menu_tail,subtitle_tail) for explicit
historical context. Do not equate this with integrated outer-loop completion.
Detailed evidence/remaining binding: TRANSITION_MESSAGE_STACK.md.

Full-original comparisons found a real preexisting375E0 bug: maskFFEEFFFF
incorrectly cleared bit16 and retained bit21. Original37628..37680 combines
FFEFFFFF & FFDFFFFF = FFCFFFFF; corrected native mask. Negative decimal
arguments now shift unsigned low words, removing host UB without changing
MIPS behavior. This shared message fix is used by normal Day1 dialogue.

pe_transition_update_oracle.py passes1180 original/native cases with real
6EC6C/F55C/79FB4, dialogue helpers,6DF50 sound lookup/AKAO validation/queue,
and868AC/86C5C volume producers.1024 controller/state combinations,152 timed
intro cases,4 stop prefixes (missing menu/subtitle context and each of two
zero-period intro samples). Original persistent write whitelist plus native
all-affected-range hashes; caller scratch160 bytes and subtitle10 bytes
restored. Arbitrary borrowed digits exercise signed conversion and flag bits.
First664-case comparison exposed mask bug; expanded1180 comparison passes.
Normal and ASan/UBSan targeted PASS1 actual group each. Full CTest8/8 PASS
65.93s,1248/1248 native groups; final builds/tests ended0. Logs local/live/
{oracle-day1-46-update,native-day1-46-update,native-asan-day1-46-update,
ctest-day1-46}.log; stack report transition-stack-audit.json. LOCAL only,
launcher remainsDAY1-41. No runtime/menu transition or freeze fix claimed.

Next:92800..93478 renderer and90D3C/90E04/91114 dependencies, then9234C outer
loop. Trace original constructor/renderer/SDK writes to update's menu tail
(sp+8A..90) while implementing outer-loop binding. Render graph and retained
menu history are required; no zero substitution. Continue Day2 branch/control
flow inventory from DAY2_ROUTE_AUDIT.md and all remaining Day1 coverage/audio/
optional/recovery/release requirements. User random-battle report unresolved.


## DAY1-47: complete transition visibility graph (2026-09-08)

Previous goal turn was progress: full update comparison and shared message fix.
Restored all8018FFF4..80190998 (617 words), SHA256
50dbed98c4cc99a403f96fb34619f3d06538d03a6d9902220f94fc1026047d3d.
New func_801904B0_port.c implementsFFF4 long-point,90124 short-point,
90254 two-plane radius and904B0 four-plane radius tests; declarations and
runtime linkage present. Preserves wrapped32-bit plane products/sums, strict
point vs inclusive expanded boundaries, signed16(radius*8+30), signed division
and original DIV/BREAK stops. Later planes still evaluate after earlier rejection;
short-circuiting would incorrectly hide later division traps.

pe_transition_visibility_oracle.py runs untouched original code with EXE/overlay
and617-word SHA pins.2356 cases:1600 random alias/radius/plane cases,512 targeted
sign/zero/overflow combinations,4 later-trap-after-rejection cases,240 cases
using plane records generated by original8F92C geometry.58 DIV/BREAK prefixes,
1003 accepted results; all persistent RAM unchanged in original and native.
Normal and ASan/UBSan targeted PASS1 actual group each. Full CTest8/8 PASS
60.19s,1249/1249 native groups; final build/test handles ended0. Evidence
local/live/{oracle-day1-47-visibility,native-day1-47-visibility,
native-asan-day1-47-visibility,ctest-day1-47}.log. LOCAL, launcher remains41.
No live transition/optional-route/full-day or random-freeze acceptance claimed.

Mapped remaining renderer graph in TRANSITION_RENDER_FRONTIER.md. Four packet
emitters97BA0/995BC/9A318/9B1D0 all require missing SDK79384/79414 projection,
NCLIP,FLAG and AVSZ3/4 helpers. Contiguous80-word SDK slice SHA256
418c0e10f86f636b682b2353689a5ce9a3f0719d39d63a9e371c6714f35e3028;
saved original-80079384-projection.txt. Existing coordinate helpers alone do
not provide exact flags, conditional stores and average-depth behavior.
Next: restore/verify SDK pair, then complete packet emitters and90D3C/90E04/
91114/92800 wrappers. Original packet emitter disassembly/hash evidence saved
as original-80197BA0-whole.txt etc. 91580..91854 and9959C are already ported;
do not retranslate them as part of those spans. Then9234C outer loop plus
retained menu history still required. Continue Day2 coverage from its audit
and all remaining Day1 scope; no smaller completion objective replaces them.


## DAY1-48: shared SDK projection helpers restored (2026-09-08)

Previous goal turn was progress: complete visibility graph and2356 comparisons.
Restored EXE79384/79414, contiguous80-word span incl8-byte padding, SHA256
418c0e10f86f636b682b2353689a5ce9a3f0719d39d63a9e371c6714f35e3028.
New func_80079384_port.c exposes triangle and quad projection/clipping leaves,
with declarations and CMake linkage. Preserves unconditional firstFLAG store,
nonpositive-winding early return, coordinate/cue/depth write ordering, and quad
fourth-vertex load after first-three coordinate stores (including RAM aliases).
Shared pe_gte.c now computes projection FLAG, maintains SXY/SZ FIFOs, supplies
NCLIP and AVSZ4. projection_flags is explicitly an RTPS/RTPT result snapshot,
not a claim that all existing GTE commands implement retained FLAG state.

pe_projection_oracle.py pins original EXE and80-word slice, executes untouched
leaves under strict GTE flags, produces3304 cases:2800 random/rational-matrix
cases with9 alias layouts plus504 threshold/winding/signed-depth-scale cases.
1103 zero,1109 positive,1092 negative clip results. Native compares256-byte
input/output region and18 hardware registers, including FIFO/MAC/IR state.
Test oracle gained AVSZ4 and optional final_gte snapshot. Independent ISA tests
cover four-term average, signed scale, truncated depths, positive/negative MAC0
overflow, depth saturation and flag reset;21 test_gte_oracle.py tests pass.

Normal and ASan/UBSan targeted PASS1 actual group each (3304 cases); full CTest
8/8 PASS58.51s,1250 native groups. Build sessions27830/46994 and CTest6777
ended0. Evidence local/live/{oracle-day1-48-projection,oracle-day1-48-gte,
native-day1-48-projection,native-asan-day1-48-projection,ctest-day1-48}.log.
py_compile and scoped diff whitespace check pass. Changes LOCAL; launcher41.

Next: implement packet emitters, starting9B1D0 (746 words). Updated
TRANSITION_RENDER_FRONTIER.md with eight-stream strides/packet sizes and
critical inline-GTE exceptions: stream4 copies retained scratch coordinates;
stream5 uses AVSZ3 before fourth projection. Do not replace these with generic
SDK calls. Then restore wrappers90D3C/90E04/91114, renderer92800, outer9234C
and retained menu-stack context. All Day1/Day2, live/release acceptance and
unlocated random-battle freeze remain unfinished. No broader fidelity claimed.


## DAY1-49: fixed-depth model packet emitter restored (2026-09-08)

Previous goal turn was progress: SDK projection restoration and3304 comparisons.
Implemented all8019B1D0..8019BD78 (746 words), SHA256
2c946ccd523fdfd5dbcc32a802e0545f54e8f1e8f18d0a656a47f04b3f11f658.
New func_8019B1D0_port.c, compat declaration and CMake linkage. Eight original
primitive streams with dynamic count reads, relative submodel/stream pointers,
projection rejection, exact packet layout/cursor increments, fixed-depth OT
linking and final cursor publication. Original FT3 uses retained scratch SXY;
FT4 executes AVSZ3 before fourth RTPS. Ordered RAM reads/writes preserve input,
packet and OT aliases. Unused SDK stack-depth word borrows/restores scratch+3FC;
original persistent scratch+0..+20 remains visible across streams/calls.

pe_fixed_model_oracle.py pins full EXE/overlay and746-word slice, executes
untouched original and real SDK leaves with strict flags.1280 cases cover all
256 stream masks x5 alias layouts: ordinary packets, packet/source overlap at
+4/+16, OT entry overlapping packet tag, OT entry overlapping model source.
Seeded scratch and1..3 mixed positive/negative/collinear/random polygons per
stream. Original PC coverage verifies all8 accepted paths execute. Native checks
all1024 scratch bytes,32KiB source/packet/OT/descriptor RAM, two globals, and16
GTE data registers. Header regeneration matches checked-in text. No ISA execution
in production. Test interpreter optional visited_pcs is test-only coverage.

Normal and ASan/UBSan targeted PASS1 actual group each (1280 cases). Full CTest
8/8 PASS63.42s;1251/1251 native groups.21 GTE ISA tests pass. Build/targeted
sessions77897/15345, oracle97970, CTest94213 all ended0. Evidence
local/live/{oracle-day1-49-fixed-model,oracle-day1-49-gte,
native-day1-49-fixed-model,native-asan-day1-49-fixed-model,ctest-day1-49}.log.
Changes LOCAL; launcher41. This emitter is not yet called by a completed native
transition renderer; surrounding wrappers/render loop remain unported.

Next:995BC (855 words), then9A318/97BA0; surrounding90D3C/90E04/91114,
92800 and9234C. TRANSITION_RENDER_FRONTIER.md records995BC asymmetric depth
clipping/shift and rejection scratch history. Keep those original differences;
do not parameterize away their order. Full Day1/Day2 coverage, live transition,
release acceptance and unlocated random-battle freeze remain unfinished.


## DAY1-50: depth-sorted model packet emitter restored (2026-09-08)

Previous goal turn was progress: full fixed-depth emitter and1280 comparisons.
Implemented801995BC..8019A318,855 words, SHA256
9e05144086c6f9bfa77edeb3dbf5bb41ae8d10d5f82629af07bfaad6d9c8af69.
New func_801995BC_port.c with compat declaration and CMake linkage. All eight
original streams restored, preserving relative submodel pointers, dynamic count
reads, packet/cursor layout and ordered OT linking. Streams0/1 bounds-check the
unshifted biased depth; stream2 shifts first. All three update scratch depth even
on winding rejection. Streams3..7 use inline RTPT/NCLIP/AVSZ3 with their original
signed depth check and late fourth-vertex projection. Cursor publishes after
stream3 (including empty) and again at return. No borrowed scratch/frame state
needed. Surrounding transition render wrappers remain unported.

pe_depth_model_oracle.py pins full EXE/overlay and855-word source slice.
1280 untouched original executions:768 mixed stream/packet-source overlap cases,
480 targeted depth-boundary/winding cases,16 OT packet/source overlaps and16
cases putting the descriptor into later-stream color data to expose the
mid-function cursor publication. All8 accepted paths verified by original PC
coverage. Depth cases cover negative/zero/small-positive sums,4095/4096 and
shifted16383/16384 thresholds, plus signed wrapping. Native compares all scratch,
source/packet/OT/descriptor regions and16 GTE data registers, not just packet count.

Normal and ASan/UBSan targeted PASS1 actual group each (1280 cases). Full CTest
8/8 PASS58.52s;1252/1252 native groups. Initial1264-case build56712, final
build/targeted/CTest88941, sanitizer92963 and final oracle85275 all ended0.
Evidence local/live/{oracle-day1-50-depth-model,native-day1-50-depth-model,
native-asan-day1-50-depth-model,ctest-day1-50}.log and build-day1-50-final.log.
py_compile and scoped whitespace checks pass; final build logs have no warnings.
Changes LOCAL, launcher remains41. No live transition/full-day fidelity claimed.

Next:9A318 (942 words) then97BA0 (1663 words); wrappers90D3C/90E04/91114,
renderer92800 and outer9234C/menu-stack history. TRANSITION_RENDER_FRONTIER.md
records9A318's shifted-depth differences, quarter-strength RGB byte writes,
retained secondary color padding, uniform color801EA264 and absence of995BC's
mid-function cursor publication. Preserve these differences when translating.
Full Day1/Day2, live/release acceptance and unlocated random-battle freeze remain
unfinished. This cut does not change the complete objective.


## DAY1-51: color-adjusted model packet emitter restored (2026-09-08)

Previous goal turn was progress: complete995BC and1280 original comparisons.
Implemented8019A318..8019B1D0,942 words, SHA256
cb887e9575fa14d4044f7d590846e5f0d5c29d1580889af040cb5262a584a2d4.
New func_8019A318_port.c with compat declaration and CMake linkage. Eight
original streams, their distinct shifted/unshifted depth acceptance and rejected
scratch-depth history, byte-ordered quarter-strength RGB for G3/G4, retained
secondary color fourth bytes, and textured uniform color801EA264. GT3/GT4
load the uniform word once after first RGB writes, then repeat it. Only final
cursor publication;995BC's mid-function publication is not copied here.

pe_color_model_oracle.py pins EXE/overlay and942-word source, executes unchanged
original with real SDK leaves/strict flags.1304 cases: previous1280 mixed stream,
depth-boundary, winding, packet/source/OT/descriptor overlap patterns adapted to
this emitter, plus24 packet/global-color overlaps at three positions for all8
primitive kinds. All8 accepted paths covered in original PC trace. Comparisons
include scratch, source/packet/OT/descriptor and color-global RAM plus16 GTE data
registers. Ordered byte/word writes, retained padding and colors all match.

Normal and ASan/UBSan targeted PASS1 actual group each (1304 cases). Full CTest
8/8 PASS65.86s;1253/1253 native groups. Original oracle20286, build/targeted
98679, sanitizer61158 and CTest23301 all ended0. Evidence local/live/
{oracle-day1-51-color-model,native-day1-51-color-model,
native-asan-day1-51-color-model,ctest-day1-51}.log. py_compile and scoped
whitespace checks pass. LOCAL; launcher remains41. Native surrounding transition
render loop is still unfinished; no live or full-day fidelity acceptance claimed.

Next: final emitter97BA0 (1663 words). Updated TRANSITION_RENDER_FRONTIER.md
with direct model-header ABI, larger textured source layouts, bias801EA5E0,
cached scratch bias, near FT3/FT4 four-packet subdivision and asymmetric500/501
thresholds, three UV-layout depth bands510/701, and GT depth checks. Restore full
subdivision rather than substituting the simpler emitters. Then wrappers
90D3C/90E04/91114, renderer92800, outer9234C/menu-stack history and continuing
full Day1/Day2 work. Live/release acceptance and random-battle freeze remain open.


## DAY1-52: final subdivision packet emitter restored (2026-09-08)

Previous goal turn was progress: complete9A318 and1304 original comparisons.
Restored80197BA0..8019959C,1663 words, SHA256
8a823f37071d3fc46dba908ad18da507e51145bbd576efe69d59af85fce3f4a3.
New func_80197BA0_port.c, compat declaration and CMake linkage. Complete static
native C transcription,1504 statements/37 branch labels with original address
annotations, including all near FT3/FT4 subdivision, midpoint/UV arithmetic,
three texture-depth bands, original scratch/packet/OT order and all8 streams.
pe_subdiv_model_translate.py reads the pinned original overlay offline and
reproduces the C file; runtime has no instruction fetch/decode or register array.
Named scalar locals/native spill frame replace the original nonescaping frame
and ABI saves. Exact flow is retained rather than replacing subdivisions with
simpler emitters. Reproducibility check passes.

pe_subdiv_model_oracle.py executes untouched original+SDK with strict flags and
random incoming caller registers.2240 cases:768 mixed masks/packet-source aliases,
1440 depth/subdivision/UV-boundary and winding cases,16 OT aliases and16 descriptor
aliases. Explicit thresholds around499/500/501,509/510,700/701 and quantization
bias-3/0/+3; all8 accepted streams and both near/far textured paths execute.
Native compares scratch/source/packet/OT/descriptor/global RAM plus16 GTE data
registers. Original cursor diagnostic confirms near FT3 four linked32-byte
packets/cursor+128, FT4 four linked40-byte packets/cursor+160, far one packet.
The staggered cursor update positions are preserved; no original cursor bug or
random-battle-freeze fix is claimed.

Normal and ASan/UBSan targeted PASS1 actual group each (2240 cases). Full CTest
8/8 PASS67.48s;1254/1254 native groups. Oracle92246, build/targeted1199,
sanitizer65240 and CTest3035 all ended0. Evidence local/live/
{oracle-day1-52-subdiv-model,native-day1-52-subdiv-model,
native-asan-day1-52-subdiv-model,ctest-day1-52,original-day1-52-subdiv-cursors}.log.
Build logs have no warnings. py_compile, static reproduction and scoped
whitespace checks pass. Changes LOCAL; launcher remains41.

All four packet emitters are implemented, but transition rendering is NOT yet
integrated. Next90D3C (50 words) now has all prerequisites: rotate object+28 to
object+8, two16-byte block matrix copy to*8019BFF0, concatenate8019CC30, set
translation/rotation, call97BA0(load(object+4)). Preserve blockwise aliases and
SDK signed-translation-overflow stops. Then90E04/91114,92800 and9234C with
retained menu-list context. TRANSITION_RENDER_FRONTIER.md holds source details.
Full Day1/Day2 coverage, live transition, final packages/release and unlocated
random-battle freeze remain unfinished; objective unchanged.


## DAY1-53: all object-render wrappers restored (2026-09-08)

Previous goal turn was progress: complete subdivision emitter and2240 comparisons.
Implemented all90D3C..91580,529 words, SHA256
d430a28dfa21a41f473027f8bc3c6247511b48a0440d938af53abe89cef742d7.
New func_80190D3C_port.c contains three wrappers plus compat declarations/CMake
linkage.90D3C rotates, block-copies, concatenates, sets GTE matrices and calls
97BA0.90E04 preserves cull-before-mutation, forced visibility evaluation, template
left multiplication and color/fixed-depth dispatch.91114 preserves matrix-before-
cull ordering, mode2 right multiplication/temporary translation writes then
restoration, signed depth/index bands and middle-band mode2 omission. Matrix
copies load all four words before each16-byte store group. SDK/visibility stop
epochs terminate at original arithmetic prefixes. Borrowed point/angle scratch
+300..313 is restored; renderer scratch+0..7D remains visible.

pe_object_render_oracle.py pins complete source/EXE/overlay and executes real
original visibility/matrix/packet graphs.453 cases reach all four emitters:
128 each90D3C/90E04,19291114, plus5 divide/signed-add stop prefixes. Vary matrix
aliases (self,+4,view destination), angles/template, draw/color modes, culling,
forced draw, depth bands and distinct submodel header masks. Compare all fixture
RAM plus28 GTE data/control values. Initial test failure was a fixture omission
of the original sin/cos table; loading shared DAY1_view_tables fixes the fixture.
No production behavior was changed to fit that failed fixture.

Normal and ASan/UBSan targeted PASS1 actual group each (453 cases). Full CTest
8/8 PASS67.59s;1255/1255 native groups. Final oracle33194, build/targeted59638,
sanitizer80860 and CTest4858 all ended0; initial93375 failed before fixture fix.
Evidence local/live/{oracle-day1-53-object-render,native-day1-53-object-render,
native-asan-day1-53-object-render,ctest-day1-53}.log. Final build logs have no
warnings. py_compile and scoped whitespace checks pass. LOCAL; launcher41.

Next: main renderer92800 (798 words), now with all direct dependencies restored.
Source local/live/original-80192800-whole.txt includes ordered object mutations,
depth/color passes and conditional two-call71A54 random animation. Then restore
9234C outer loop and retained menu-stack provenance. All four emitters and three
wrappers are native, but the live transition loop is not yet integrated or
accepted. TRANSITION_RENDER_FRONTIER.md records source boundaries and details.
Full Day1/Day2 scope, live/release acceptance and unlocated random-battle freeze
remain unfinished. Do not claim complete transition/full-day fidelity.


## DAY1-54: main transition renderer restored (2026-09-08)

Implemented all798 words of80192800..80193478 in func_80192800_port.c;
source SHA256926ba5886cd51536f747a8514c1ae8cd268b44b75aeaae8492f72112eef84690.
Preserves ordered depth passes, four post-draw advances followed by separate
wrap checks,24/52 model toggling, conditional two-call BIOS random animation,
grouped matrix/angle copies, dynamic signed half-count object traversal, color
bytes, conditional LOD passes and final subdivision depth500. Calls restored
native wrappers and propagates their stop epochs. No runtime instruction decoder.

pe_transition_render_oracle.py executes the complete original graph with real
wrappers, SDK matrices, visibility and all packet emitters.128 cases cover all51
original call sites, mode0/1/2, effect gates, signed/odd object counts, wrap/overflow
values, object aliases, visibility bands, model masks and RNG advancement.
Compare packet/OT/scratch/source/object/global RAM and28 GTE components. Source,
EXE and overlay authority are pinned. Normal targeted group passes all128 cases.
Initial oracle coverage assertion expected53 sites; full source contains51 and
all51 were executed. Corrected to compare against the source-derived site set.

Normal and ASan/UBSan targeted checks pass1 actual group each (128 cases).
Full CTest8/8 PASS;1256/1256 native groups. Oracle71024, build56624,
sanitizer21788 and CTest89786 ended0. Evidence in local/live/
{oracle-day1-54-render,native-day1-54-render,native-asan-day1-54-render,
ctest-day1-54}.log. Build logs have no warnings; py_compile and scoped diff checks
pass. LOCAL; launcher remains41.
Next: restore9234C outer frame loop and trace retained menu-stack provenance
through constructor/SDK/frame calls before binding update menu context. Complete
renderer comparison does not establish live transition or full-day acceptance.
Full Day1/Day2 scope and unlocated random-battle freeze remain unfinished.


## DAY1-55: outer-loop decompilation and OT compaction (2026-09-08)

Previous goal turn was progress: main renderer restored with128 full-graph cases.
Decompiled253-word9234C..92740 into TRANSITION_OUTER_LOOP.md, preserving frame
order, reset-versus-normal exit, retained effect handle and empty-run state,
double-buffer descriptor switching, and SDK call/result requirements.
Implemented its35-word925A0..9262C OT loop as PE_TransitionCompactOT in
transition_ordering_table_port.c, with compat/CMake linkage. Source SHA256
 e3bbb8893991be52590560e3222bffa28dcdec3c44eaf5ff1be22fa387a1464a.
pe_transition_ot_oracle.py executes original code for512 cases across empty,
occupied, alternating, long-run, mixed and descriptor/bucket-alias tables.
Compares persistent table RAM and returned retained empty-run index.

Normal and ASan/UBSan targeted checks pass1 actual group each (512 cases).
Full CTest8/8 passes,1257/1257 native groups. Oracle56748, native58311,
sanitizer32400, stack audit65537 and CTest11002 all ended0. Logs:
local/live/{oracle-day1-55-ot,native-day1-55-ot,native-asan-day1-55-ot,
ctest-day1-55}.log. Build logs have no warnings; py_compile and scoped whitespace
checks pass. The native outer loop remains unintegrated pending the dependencies
above; this verification covers its compaction block and constructor observation.

pe_transition_outer_stack_audit.py runs the real constructor at outer SP801FEFD0
under its existing explicit provider contracts. Eight mode/fill cases confirm
menu-list bytes4..7 have no observed constructor writer and retain initial00/A5.
Do not bind a zero menu tail from this partial history. Evidence:
local/live/transition-outer-stack-audit.json and TRANSITION_MESSAGE_STACK.md.

Two additional live-loop dependencies surfaced: existing3EB04 is only a digital
edge cut (missing controller init/hold counters/priority/analog paths and SDK
825C0/82974/82680/8292C/828F4), and HostFB_VSync returns void although the outer
loop consumes its vblank/scanline query results. No input/timing shortcut was
introduced. Next restore those original input/SDK/timing paths, trace their
stack writes, then integrate9234C with proven menu context. Bootstrap9234C still
stops explicitly. Full Day1/Day2 scope, release/live acceptance, and unlocated
random-battle freeze remain unfinished. LOCAL; launcher41.


## DAY1-56: original controller query/configuration SDK (2026-09-08)

Previous turn was progress: full outer-loop decompilation, native OT compaction,
and constructor stack evidence. Restored ten controller functions (246 source
words) in pe_controller_sdk_port.c:825C0,82680,828F4,8292C,82974 and their real
84B20,84F8C,835A4,83BB8,83D04 helpers. Uses original guest controller slots and
the callbacks already installed by native844E4 in pe_save.c. State queries retain
conditional2/3-to1 and6-to4 translation; info queries preserve signed index
bounds. Configuration calls preserve busy checks, byte truncation, queue writes
and callback IDs. Unrecognized installed callbacks stop explicitly. No host pad
state fallback or invented controller success. Source SHA256
fa58951f31f09efabde626872321aeacd180bc41e70fdcafcec1eea1cc446918.

pe_controller_sdk_oracle.py runs1280 complete original cases without replacing
callbacks, comparing results and guest record/table/buffer RAM. Native target
passes all1280 plus two unknown-callback stop-prefix checks. Configuration queued
serial callbacks are retained as guest IDs; their later execution remains separate
work. Complete game input3EB04 is still only its previous digital cut.

Normal and ASan/UBSan targeted checks pass1 actual group each (1280 original
cases plus2 unknown-callback prefixes). Full CTest8/8 PASS71.33s;1258/1258 native
groups. Oracle99506, native63442, sanitizer60595, extended audit11179 and CTest
57345 all ended0. Logs local/live/{oracle-day1-56-controller,
native-day1-56-controller,native-asan-day1-56-controller,ctest-day1-56}.log.
Build logs have no warnings; py_compile and scoped whitespace checks pass.

Extended pe_transition_outer_stack_audit.py through twelve complete original
3EB04 digital-controller executions after constructor, varying disconnect,
state1/2/6, flags4000/C000, busy/ready and stack fill00/A5. No input/SDK providers.
When state6/C000 chooses828F4, original83BD0 saves return address8008291C into
menu-list bytes0..3 (even when busy). Otherwise their last writer remains96F8C.
Bytes4..7 still retain the initial fill; constructor provider-stack gaps remain.
This proves caller history varies and must not be replaced with a universal tail.

Next restore complete3EB04 game input behavior and reconcile D_8009D1A0 host/
guest ownership, then VSync query/timing and remaining menu-stack provenance.
After that integrate9234C with live acceptance. Full Day1/Day2 scope, random-battle
freeze, platform/release acceptance remain unfinished. LOCAL; launcher41.


## DAY1-57: complete game input and shared flags (2026-09-08)

Previous turn was progress: controller SDK restored and original stack-history
paths observed. Replaced the digital-edge-only3EB04 cut with all348 original
words through3F074. Source SHA256
c6a27aa96c180ee441866ac86dfc995683fb48c2366dfd864f0bbd03aeb4660b.
Now preserves disconnect initialization, controller state/configuration calls,
32 hold counters, the special nine-step input sequence, all priority masks,
analog menu-versus-game thresholds and ordered press/release edges. Calls real
controller SDK and menu lookup; unknown SDK callbacks propagate stop epochs.

D_8009D1A0 and D_8009D280 now use PE_GUEST_U32 aliases in psx_compat.h. Removed
host scalar definitions and stale extern declarations, so named and address-based
readers/writers share original storage. This fixes the discovered split between
input/initialization and battle/frame state. No duplicated synchronization copy.

pe_game_input_oracle.py executes2816 complete original inputs with real SDK and
menu calls. Cases cover pad identity/state, configuration/busy status, flags,
held counters, analog boundary values and both menu/game behavior, plus special
sequence progress/completion. Native comparisons check all fixture RAM including
shared flags. The first oracle coverage check exposed correlated fixture bits
that suppressed mode setup; varying mode availability independently covers it.
Normal target passes all2816. Wider initial native run1251/1259: seven old digital
fixtures lacked controller reply context, and3E974's footprint expected a separate
host flag. Updated explicit test controller setup and original38-word footprint.
Production input behavior was not weakened to retain the old partial fixtures.

Normal and ASan/UBSan targeted checks pass1 actual group each (2816 cases).
Final CTest8/8 PASS65.14s;1259/1259 native groups. Final57290 and sanitizer57082
ended0; earlier75430 exposed two stale stable-destination expectations: original
connected state2 clears setup bit4000 before frame flags are processed. Updated
their exact flags expectations, retaining destination assertions. Logs:
local/live/{oracle-day1-57-input,native-day1-57-input,native-asan-day1-57-input,
ctest-day1-57-final}.log. Builds have no warnings; scoped diff/py_compile pass.

Bootstrap source80012320..12344 explicitly switches to SP1F8003F8 before9234C.
The audit now supports --scratchpad: constructor/input SP1F8003C8, menu1F800390.
All20 actual-scratchpad constructor/input observations pass the same writer
assertions under existing constructor provider contracts. Log
local/live/transition-outer-scratchpad-audit.json (8364 ended0). Earlier relocated
RAM-stack observations are not live address/value proof; future whole-frame
stack auditing must use the scratchpad location and account for shared scratch
writes. No final menu-tail binding was made.

Next restore VSync return/timing semantics and remaining original menu-stack
provenance before9234C integration. Controller queued serial callbacks retain
original IDs; their later scheduling/execution and live host-input acceptance
remain separate work. Full Day1/Day2 scope, random-battle freeze and release
acceptance remain unfinished. LOCAL; launcher41.


## DAY1-58: full VSync SDK semantics and device traces (2026-09-08)

Previous turn was progress: complete game input, unified flags and actual-stack
audit. Implemented132-word73A44..73C54 as PE_RetailVSync with explicit clock/BIOS
operations in platform/pe_vsync.c/.h, linked by CMake. Source SHA256
 e356692b0a159f0f9e07da321a2ea515c4789379094ff39d6d77f42858169af4.
Preserves stable timer reads, entry-time return delta, negative absolute-counter
queries, signed relative waits, GPU field synchronization, both baseline writes,
and exact watchdog/timeout BIOS order. No runtime instruction decoding.

pe_vsync_oracle.py executes original VSync and wait instructions using device
read sequences and BIOS contracts; it does not replace either callee.264 cases,
4846 RLE events, compare every global/device read/write, timeout service and
return value. Native targeted group passes all264 including eight stalled-device
cases. This establishes the SDK algorithm against explicit devices, not a live
host timer. VSYNC_CONTRACT.md records the source and integration requirements.

Normal and ASan/UBSan targeted checks pass1 actual group each (264 traces).
Full CTest8/8 PASS66.36s;1260/1260 native groups. Oracle47188, native79172,
sanitizer78888, CTest29625 and scratchpad audit65540 all ended0. Logs:
local/live/{oracle-day1-58-vsync,native-day1-58-vsync,native-asan-day1-58-vsync,
ctest-day1-58,oracle-day1-58-vsync-scratchpad}.log. No build warnings; py_compile
and scoped whitespace checks pass. Initial oracle-only run failed on an observer
variable name (simm versus si), corrected before any comparison results.

Scratchpad audit local/live/vsync-stack-audit.json confirms264 original cases at
SP1F8003C8. Query modes leave menu1F800390..399 untouched. Waits write watchdog
bytes0..3 through73BC4/73BF0 and saved-return low halfword3B20 atbytes8..9 through
73BDC. Bytes4..7 retain initialA5. Timeout can leaveFFFF in the second halfword;
do not infer a universal tail or live timing from the device-sequence fixtures.

HostFB_VSync is still the old void host provider. The platform lacks timer1 MMIO,
and CPU IRQ delivery currently recognizes the DMA handler only; source0/7440C
still reaches an indirect boundary. Next restore timer1 and source0 delivery with
existing IRQ masks/generation/BIOS policy, implement the clock adapter, and bind
public VSync. Then finish actual-scratchpad frame history and9234C integration.
Do not equate the shim's invocation count or GPU frame counter with956AC/timer1.
Full Day1/Day2, live/release acceptance and random-battle freeze remain unfinished.
LOCAL; launcher41.


DAY1-59 restores the original VBlank RNG/four-timer callback and connects CPU
source0 to checked7440C dispatch.512 original comparisons plus four IRQ scenarios
pass normally and under ASan/UBSan; full suite1261 groups, CTest8/8. Unknown or
stopped callbacks prevent subsequent slot execution. Timer1 MMIO, host VBlank
production and VSync adapter remain outstanding; no new live-frame/menu-tail
or full-day acceptance is established. See VSYNC_CONTRACT.md.

DAY1-60 restores ResetCallback's timer1 mode107 setup, with an explicit
HBlank/VBlank edge counter for that original configuration.32 original setup
prefixes and native hardware-contract tests pass; ASan/UBSan targeted pass,
CTest8/8 and1262 native groups pass. GPU edge scheduling, BIOS delivery policy,
clock adapter and full-frame/live validation remain unfinished. Details and
hardware source: VSYNC_CONTRACT.md.

DAY1-61 restores all318 PutDispEnv words and GP1(05..08) display-register
state/readback.2048 original software comparisons and ASan/UBSan checks pass;
full CTest8/8,1263 native groups. This supplies original mode/range inputs for
the remaining GPU scheduler. It does not yet establish scanline timing, public
VSync integration, final menu-stack history or live transition acceptance.
See DISPENV_CONTRACT.md.

DAY1-62 connects explicit GPU blanking edges to timer1/IRQ0 and fixes critical-
section IRQ deferral. It restores BIOS clear-control veneers and controller
reset's formerly skipped VBlank policy write.64 original ABI cases and32 native
edge scenarios pass; ASan/UBSan targeted pass, CTest8/8 and1264 native groups.
Raster signal generation, BIOS handler processing and public VSync integration
remain outstanding. See VSYNC_CONTRACT.md.

DAY1/DAY2-64 restores missing VM D3 conversion and consolidates D2 timer reads
with2048 original-handler comparisons.256 additional entry-script chains now
execute original D2/D3 before the calculation. Native VM dispatch and sanitizer
checks pass; full CTest8/8 and1265 groups. Earlier asynchronous scene gates and
live timer scheduling remain unproven. See DAY2_ENTRY_DECOMP.md.

### DAY1/DAY2-65: shared exit input/state predicates

Original-handler audit now checks1,050 M0351I input/state combinations and all
six exits from8018F694. Corrected opcode11 selector1 to newly pressed input100.
An additional256 chains execute the permitted gate prefix through original
D2/D3 and calculation, matching existing chain traces and persistent writes.
Prior772 selector/96 calculation/256 timer cases also pass; Python compile and
scoped whitespace checks pass. No native code changed; latest native validation
remains DAY1-64. Async scene execution, map loading and full-day coverage remain
unverified. Details:DAY2_ENTRY_DECOMP.md; log:local/live/oracle-day1-65-entry-gates.log.

### DAY1/DAY2-66: complete held-input query selector

Restored opcode11 selector3 in130B4; it previously returned silently without
writing an output. Preserves the original destructive mask store, pointer
reloads, signed-leading-bit indexing, unchecked wrapped counter address and
GTE data30/31 effects. Normal and sanitizer DAY1_input_query groups pass2560
original-state comparisons and a native VM continuation case. Original entry
checks also pass unchanged. Details:INPUT_QUERY_CONTRACT.md. This handler fix
is local; live transition and full-day acceptance remain open.

DAY1/DAY2-66 final regression:8/8 CTest targets pass in66.52s, with1266 native
groups. Both builds have no compiler warnings/errors; Python compilation and
scoped whitespace checks pass. All changes remain local.

### DAY1/DAY2-67: fade arithmetic and wait resumption

Corrected native68E24 color interpolation to retain original mult/mflo32-bit
wrap before signed division, eliminating signed-overflow undefined behavior.
Original512 tick cases and16 command-start/wait sequences produce1056 RAM and
return checkpoints, all matching normal/sanitizer DAY1_fade_wait runs. Includes
the transition's60-tick start operands and wait next-PC/task-delay effects.
Details:FADE_WAIT_CONTRACT.md. Complete scene scheduler, GPU presentation and
wall-clock timing are outside this bounded proof; full-day acceptance remains
open. Changes local.

DAY1/DAY2-67 final validation: normal and sanitizer focused tests pass; full
CTest8/8 passes in67.33s (1267 native groups). Oracle regeneration check, Python
compilation and scoped whitespace checks pass. Both builds have no compiler
warnings/errors.

### DAY1/DAY2-68: restore exit menu entry and constructor

Added original E7/15AF0 and4D18C constructor,107 words combined, with native
VM dispatch.128 original-state cases match normal/sanitizer native tests,
including yield/retry, scene bypass, saved cursor normalization, menu/help
allocation, flag updates and event/resource resets. A VM fixture verifies
initial yield/rewind and the next scheduler pass's bypass/continuation.
Original execution uses an explicit BIOS A28 bzero memory contract. Details:
EXIT_MENU_CONTRACT.md. Constructor-installed4D2DC/4FDE8/4FDA4 callbacks remain
unported; full menu interaction, transition and whole-day acceptance stay open.
Changes local.

DAY1/DAY2-68 final validation:128 original-state cases pass normal and sanitizer
checks; VM yield/continuation passes. FullCTest8/8 passes in68.01s,1268 native
groups. Original entry-path checks, Python compilation and scoped whitespace
checks pass. Final builds have no compiler warnings/errors. Changes remain local.

### DAY1/DAY2-69: exit-menu drawing and option availability

Restored42770/4FDA4/4FDE8/50C08 (73 words) and connected the menu draw/cell
predicate dispatchers.64 complete original drawing graphs and1024 predicate
cases match normal/sanitizer native checks, including packet exhaustion,
clipping, enabled-mask and glyph state. No VBlank progression is introduced.
Details:EXIT_MENU_CONTRACT.md. These use synthetic text/resource fixtures;
retail text/GPU presentation and input callback4D2DC remain unverified.

DAY1/DAY2-69 final regression:8/8 CTest targets pass in68.34s,1269 native
groups. Original128-case E7 constructor regression and generated drawing
header check pass. Normal/sanitizer focused groups, Python compilation and
scoped whitespace checks pass. Final builds have no compiler warnings/errors.

DAY1/DAY2-70 restores seven card-record/selection helpers required by the input
path and fixes42798 continuing past an unresolved72774 call.256 original
chains match normal/sanitizer native states, distinguishing completion from
stopping at the call. Handles/states now remain unchanged on that stop, and
42A10 omits subsequent resets. See CARD_RECORD_CONTRACT.md for ordered input
callback decompilation and remaining modal/card/BIOS dependencies. Native input
callback4D2DC remains unported; full menu interaction is not yet established.

DAY1/DAY2-70 final validation:256 original/native chains pass normal and
sanitizer checks; three EV1 cleanup groups pass under sanitizers. FullCTest8/8
passes in68.46s,1270 native groups. Header regeneration, Python compilation
and scoped whitespace checks pass; final builds have no warnings/errors.
Changes remain local.

DAY1/DAY2-71 restores42848/4DAA4 (151 original words): flag-gated modal
creation, existing-window reuse, both modes, missing first text and measured
layout.112 original/native cases match normal and sanitizer tests. The audit
also exposed resource-bank scalar copies diverging from guest RAM; D048/D04C/
D050/D054/D058/D064 now use canonical guest lvalues. See CARD_RECORD_CONTRACT.md
for details and synthetic-text limits. Input4D2DC, modal callback50580 and the
notice42910 dispatch arm remain outstanding; no full interactive acceptance.

DAY1/DAY2-71 final validation:112 original/native modal cases pass; full normal
CTest8/8 passes in70.12s, and the full sanitizer suite passes1271/1271 groups.
Original64 drawing/1024 predicate regression and generated header check pass.
Python compilation and scoped whitespace checks pass. All functional changes
were tested; subsequent ownership-comment cleanup changed no executable code.
Changes remain local, with full Day1/Day2 and interactive acceptance unfinished.
