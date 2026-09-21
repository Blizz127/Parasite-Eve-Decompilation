# Rehearsal victory, supplies and club effects

**Current connected result:** ordinary controller input wins the rehearsal
encounter at44752 and restores control atstory60 in m0319i at45653.
Aya has36HP, both theater keys remain and the carried club is equipped.
The52000-frame run has no unresolved runtime boundary. It is still Day1;
fullDay2 and whole-game fidelity are unproved. Earlier attempts below are
retained as diagnostic history; the final section records the victory.

The connected cold-boot route reaches **m0023i at frame 42713**, story
`5B`, through m0319i. These are Day 1 rehearsal rooms, before the sewers.
The previous “sewer entry” label for m0319i was incorrect. This evidence
extends [the theater-key traversal](DAY2_THEATER_KEYS.md); it does not
establish complete Day 2 or whole-game fidelity.

The previous 35000-frame baseline input sequence can be extended with:

```text
35000:FFDF,35008:FFBF,35088:FFDF,35270:FFFF,
36000:FFDF,36090:FFEF,36610:FFFF,42000:FFEF,42400:FFFF
```

Set `PE_ROUTE_PULSE_RESUME=39000`. The route returns to m0012i at 35296,
sets the rehearsal-door flag at 39005, enters m0319i at 39101, and crosses
its required encounter trigger. The original module6 rectangle at
`801A34B0` spans x[-1271,1065], z[1265,1349]. The later sewer trigger is
separate and story gated; it is not used to bypass this encounter.

The 46000-frame stationary probe, `/tmp/pe-sewer-encounter.{log,bin}`,
reset to the opening at 43720. A repeat with read-only battle diagnostics
stopped at 43500, `/tmp/pe-rehearsal-battle.{log,bin}`, and established:

| Frame | Observation |
| --- | --- |
| 42934 | Battle starts; Aya HP25 |
| 42940 | Enemy HP1000092 |
| 42980 | Aya HP21 |
| 43160 | Aya HP16 |
| 43445 | Enemy HP1000078 |
| 43452 | Aya HP0, battle mode3 |

This is defeat, not victory or an unresolved runtime boundary. The native
player handler's terminal path in `func_8001D340_port.c` sets mode3 when
Aya's HP is nonpositive. No production change was made to suppress it.

The route harness now supports `PE_ROUTE_BATTLE_DUMP_BEGIN=<frame>`.
It logs battle HP, mode, command queue, AT, positions, enemy script PC,
PE, loaded ammunition and bullet reserve when tracked values change or every 120 frames. This diagnostic reads
guest state without writing it. It is disabled by default.

Ordinary healing is verified in a separate **35000-frame connected run**,
`/tmp/pe-healed.{log,bin}`. Append these pairs to the default sequence,
before any later room-navigation pairs:

```text
34000:EFFF,34002:FFFF,34030:BFFF,34032:FFFF,
34060:FFBF,34062:FFFF,34090:FFBF,34092:FFFF,
34120:BFFF,34122:FFFF,34150:BFFF,34152:FFFF,
34190:DFFF,34192:FFFF,34220:DFFF,34222:FFFF
```

These inputs open Triangle, choose Items, move down twice to inventory
index4, select Medicine, choose Use, and cancel back to the field. The
carried item6 is consumed (`800C0E50=0`), Aya HP rises **25 to45**, global
HP is45, D1A0 returns to4080, and overlay flags return to40000048. The
existing **34-milestone route assertions PASS**, with both keys retained
and player control restored. No inventory, HP, position, story or save
state was injected into this connected run.

The cabinet pickup is also verified through cold-boot controller input,
`/tmp/pe-ammo-cabinet.{log,bin}`. From the healed position, append:

```text
35000:FF7F,35040:FFBF,35064:FF7F,35080:FFBF,35104:FF7F,
35112:FFBF,35128:FF7F,35136:FFBF,35144:FF7F,35176:FFBF,35224:FFFF
```

Resume periodic Cross at35300. Module4's cabinet trigger at8019E3B4
requires x[-738,-643], z[-437,-293] and heading0..800. Aya's payload2
handler opens the cabinet, sets persist[25] bit4000, awards item1 at
8019DCC4 and sets persist[24] bit80000000 at **35381**. The item record
has category16 and quantity6. At36500, reserve bullets at800A1E6E are6,
the weapon still has5 loaded, HP is45, both keys remain, and player control
is restored. A floor-only path proposal was checked against the connected
movement; it was not loaded into the running game.

The regression now incorporates healing and cabinet inputs: **36500
frames,77 pad pairs,35 milestones**. Its endpoint additionally asserts
Medicine consumption, live/saved HP45, six reserve bullets and the cabinet
flags. Build and CTest commands:

```sh
source /tmp/pe-tools/env.sh
cmake --build /tmp/pe-day2-release --target pe-route-boot-day2-tests -j4
ctest --test-dir /tmp/pe-day2-release -R '^route-boot-day2-control-flow$' --output-on-failure
```

The extra ammunition is necessary for further battle work. The earlier
35000-frame baseline has only five loaded bullets and zero reserve. A
moving encounter probe exhausted those bullets and stopped damaging Eve,
then reached HP0 at44332. Healing alone does not resolve ammunition loss.
That failed probe is `/tmp/pe-rehearsal-orbit.{log,bin}` and is not victory
evidence.

Original-MIPS callbacks on a diagnostic copy independently confirm that
menu path: `5C174(0)`, main-window `43DA4(...,10000)`, two list
`63E0C(...,4000)` events, item-window `44444(...,10000)`, and Use
`44B0C(...,10000)`. This direct callback check establishes the intended
menu navigation; the connected pad replay establishes actual traversal.

Both route binaries were rebuilt after adding the diagnostic. The matching
executable remains unchanged. Verification on 2026-09-12:

```sh
sha1sum build/extracted/disc1/SLUS_006.62 build/disc1.candidate.exe
python3 tools/build/disc1_plan.py --check
```

Both hashes are `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; the plan has
1145 spans (795 C,348 assembly,2 rodata). No new matching C is claimed.
Full encounter victory and the following sewer traversal remain unproved.
Captures and extracted scripts remain local game data.

## Empty weapon update stack retention

The supplied, moving encounter replay used ordinary Heal1 at43669,
restoring24->45HP, then reached a real native boundary at44942:
`PE_M0023I_Flash`. Aya remained alive with29HP; Eve had1000006HP,
six above the scripted victory threshold. `/tmp/pe-rehearsal-supplied.log`
and its local RAM capture preserve this failed run.

Original69594 call-graph traces show the preceding CE934 room update writes
Z=0 to801FEF5C. Empty C9B90/CD8F0 weapon updates, including their C251C /
C2758 frames, do not overwrite the flash's borrowed six bytes801FEF58..5D.
The native tracker discarded this known Z unconditionally. It now preserves
known bytes across these shallow updates; deeper constructor/callback graphs
still invalidate tracking until their original stack writers are bound.
No fallback position is supplied.

`pe_m0023i_pump_oracle.py` now covers58 histories /812 original frames,
including empty active weapon and impact updates in both banks and slot
orders, with delay/loop/stop programs. Original reference generation passes
(`/tmp/pe-empty-weapon-pump-oracle.log`), and the native comparison passes
(`/tmp/pe-empty-weapon-pump-native.log`). The fresh-RAM unknown-Z boundary
assertion remains effective. Hash comparisons use the established oracle
ranges and defined GPU packet bytes; they do not establish whole-machine
or undefined-padding identity.

The matching EXE is unchanged by this PC-runtime tracking correction.

Both Debug and Release builds complete. Full Release native tests pass
**1380/1380** (`/tmp/pe-empty-weapon-native-full.log`); full CTest passes
**10/10 in164.67s**, including the35-milestone route in105.51s
(`/tmp/pe-empty-weapon-full-ctest.log`). The same supplied connected input
runs49000frames after the correction with no unresolved stop; its old
34-milestone endpoint assertion reports the expected changed-room failure.
AtendAyaHP18, Eve1000006, noammunition, PE menuopen. This clears the flash
boundary but does not prove encounter victory.

## Club equipment and newly translated effects

With the gun empty, ordinary battle-menu input switches to carried slot2:
Triangle, main-menu row2, equipment row0, replacement list row1, confirmation.
The connected run queues407 at44629 and applies the club; Aya approaches to
melee distance and queues its first attack at44732. The next frame exposed
missing effect constructor800CE084, with Aya36HP and Eve1000006HP.
`/tmp/pe-club-replay.{log,bin}` records this boundary.

The native club path now translates constructorCE084, update wrapperCE16C,
target-originCE1FC, spark initializerCE2B4 and drawCE3B4. It reuses existing
shared storage/VM/rendering and generated matching CE144/CE464/CE470 leaves.
Descriptor constructor, parameter, update and draw dispatch are connected.
New `pe_club_effect_oracle.py` executes61 complete original cases covering
owner types, target-list termination, random seeds and signed positions,
both drawbanks, shared VM creation, lifetime, parameter dispatch and allocation.
All61 match native results (`/tmp/pe-club-oracle-compact.log` and
`/tmp/pe-club-native2.log`). No expected gameplay result is synthesized.

The next connected replay passed construction, then hit flash at44744.
Original pump traces (`/tmp/pe-club-stack-writers.log`) prove the empty
club update preserves the preceding room writer, just like the empty pistol
updates. The tracker now includes CE144/CE16C and the empty CE3AC leaf;
unknown deeper graphs retain the explicit boundary. The expanded pump
comparison and further connected replay are pending; these61 cases alone
do not establish victory or whole-game fidelity.

## Connected victory and fixed controller regression

`/tmp/pe-club-stack-replay.{log,bin}` completes52000frames with only the
four existing HOST_ADAPTED movie/menu skips. It uses the supplied route,
ordinary Heal1, and an ordinary switch to the club after all11bullets are
spent. No positions, HP, inventory, story or command queue entries are seeded.

| Frame | Connected observation |
| --- | --- |
| 38709 | Enter m0319i through the unlocked rehearsal door |
| 42713 | Enter m0023i /story5B |
| 43669 | Heal1 command387 queued through battle menu |
| 44629 | Club equipment command407 queued; carried slot2 selected |
| 44752 | Club hit changes EveHP1000006->999990; AyaHP36 |
| 44753 | Original encounter exit enters mode8 |
| 45041 | Story5E and m0367i victory scene |
| 45525 | Return to m0319i /story5F |
| 45653 | Story60; player control restored |

The actual return is m0319i, directly from m0367i. Earlier predictions that
required a m0023i return were not established by connected evidence.
At52000, Aya is at(0,0,2105), flags8, task0; D1A0=4080, saved/liveHP36,
module6PC801A355C, C8/C9retained, reserve0, equippedslot2. Battle mode12 is
retained while the battle-active bit is clear; it is not an active fight.

The regression now records **751 fixed pad pairs** in
`pc_port/tests/route_rehearsal_pads.h`:86supplies/entry pairs plus665 final
raw battle-pad changes. Automatic Cross is suppressed only during
[42713,45041), because those raw words already include Cross and its menu
releases. `PE_ROUTE_EXACT_PAD_BEGIN/END` expose that input-only interval;
the array capacity is1024. The comma-separated751pair sequence has SHA256
`72d06612ba7360fb8f08e37203ee7ee938cac738d3fd7c7114c9252c2e1d8431`
without a trailing newline. The regression uses no live-state controller.

The new endpoint is **46000frames /41milestones**, requiring story60,
m0319i module6PC801A355C, HP36, clubslot2, both keys, consumed Medicine,
opened cabinet and player control. Its fixed-input CTest verification
**passes121.54s** (`/tmp/pe-victory-fixed-ctest.log`, captured46000RAM
`/tmp/pe-victory-fixed.bin`). The full connected52000run above
proves the route; this repeat checks its recorded deterministic inputs.

Combined effect-pump coverage is now **70histories /980frames**, including
12empty-club histories. Original and native both pass
(`/tmp/pe-club-pump-oracle.log`, `/tmp/pe-club-pump-native.log`). No new
packet masks or guessed stack values were introduced. The remaining9CTestchecks pass61.05s, including fullnative **1381/1381**
(`/tmp/pe-club-final-ctest.log`, `/tmp/pe-club-final-lasttest.log`). All10checks
pass across the two commands. BothDebug/Releasebuilds complete. The
original/candidateEXE hashes and795C/348asm/2rodata plan were rechecked
after these changes and remain unchanged.

## Connected sewer entrance

The fixed victory inputs extend with46500:FF7F,46684:FFEF,47244:FFFF.
A diagnostic-copy floor proposal identified module7's exit rectangle;
ordinary connected movement and the default dialogue choice then enter
**m0026i /story68 at47348**. The52000frame run
`/tmp/pe-sewer-door-connected.{log,bin}` has no unresolved boundary and
retains all41earlier milestones. Its old rehearsal endpoint assertion
correctly reports the further room.

The m0026i snapshot is Aya(20,1160,-6700), flags408, D1A04000. Module0
cycles water-sound opcodeBF plus timed delays; this is an intentional loop,
not proof of a stalled descent. Module2 waits for its next exit rectangle
x[-1600,1600],z[-5600,-5200], before settingpersist1=1A and enteringm0027i.
The next connected input adds49000:FFEF,49250:FFFF and reaches
**m0027i at49176**, story68/persist1=1A. At55000, Aya is(0,773,-4335),
HP36, D1A04000, with no unresolved boundary
(`/tmp/pe-sewer-first-connected.{log,bin}`). The first hallway encounter,
triggered around z[-3791,-3648], remains under test.
