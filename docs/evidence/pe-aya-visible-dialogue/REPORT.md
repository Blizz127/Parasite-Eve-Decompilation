# Visible Aya, model anchors and opening dialogue

2026-09-04. Native PC-port changes; not a matching-C claim. Retail authority:
`build/disc1.candidate.exe`, SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Latest visible verification

INV11 verifies the field inventory through ordinary keyboard input. The
reload screen transfers one round from pistol to crate (6/6→5/7), confirms
both saved and live counts, then restores6/6. After the same replay's seven
shots and scripted battle exit, Aya has14/45 HP. Medicine 1 is moved from
slot4 to6 and used there; HP becomes45 in both live and saved records, the
item disappears, and movement resumes after closing the menu.
See `inventory-proof-inv11.json`, `inventory-main-inv11.png`,
`inventory-reload-adjust-inv11.png`, `inventory-medicine-selected-inv11.png`
and `inventory-medicine-used-inv11.png`. `performance-inv11.png` again shows
the backdrop, three performers and staircase; `complete-encounter-inv11.json`
records the seven shot frames and scripted exit. No guest state was edited.
Source/build INV12 verifies history rollback and command undo (162 original
cases). INV13 adds battle menu entry, command completion and targeting (183
cases); INV14 restores medicine animation/healing, equipment command completion
and escape judgement (182 cases). Full normal and sanitizer suites pass2/2
each,1,178 native groups. A fresh INV14 visible replay is in progress; battle
medicine still needs live confirmation. The INV11 run exited after its proof.

NAM12 adds visible opening-name verification: ordinary Return adds a letter,
Circle removes it, the Default control restores Aya, and End closes the menu
and resumes dialogue12. See `name-entry-proof-nam12.json`,
`name-added-nam12.png` and `name-default-nam12.png`. The same replay's
`performance-nam12.png` shows the full backdrop, three performers and staircase.
Normal and ASan/UBSan CTest pass2/2 each,1,164 native cases at this point.
That replay also completed the first fight: seven shots across four ordinary
selections, Eve's scripted exit at19719, messages50–60, then restored movement
with18/45 HP and five rounds saved. `complete-encounter-nam12.json` and
`battle-return-nam12.png` record the result. All seven body slots released;
the sound queue is empty. No guest state was edited.

ATK35 completes the first encounter visibly with both sides attacking.
Seven shots across four ordinary command selections include a reload;
four Eve beams take Aya45→42→35→25→18. Her reaction particles render in
`aya-reaction-atk35.png`; VM96 requests Eve's scripted retreat at12474.
Messages50–60 finish, all battle body slots release, HP/ammo persist, and
ordinary Right input moves Aya X345→120. `battle-return-atk35.png` and
`complete-encounter-atk35.json` record the result. No guest HP, position or
progress writes were used. Name entry was still pending in that older run.

ATK34 (2026-09-05): `performance-atk34-20.png` shows the full stage and all
three performers. `eve-beam-atk34.png` records a single beam hit taking
Aya45→42; `pistol-effects-atk34.png` records the muzzle/casing frame after an
ordinary target-and-fire selection. Shot10762 applies damage10763 and
continues dialogue46/47. `pistol-effects-progress.json` captures these facts.
The next missing callback was D751C, Aya's reaction particles. ATK35 now
implements it and its two particle types and point glow; all114 original
comparisons and full normal/ASan/UBSan suites pass (1,152 native cases).
Its visible replay is underway; combat completion with effects remains pending.

ATK29 (2026-09-05): `eve-beam.png` shows Eve's full beam at frame10242.
The displayed HUD is42/45; post-update RAM is39/45 and audio queue0.
`performance-atk29-20.png` again shows the complete stage and performers.
All121 original beam/streak/ribbon call graphs match; full normal and
ASan/UBSan CTest pass2/2 each,1,145 native cases. This run exposed an
older partial player tick repeatedly applying one hit, then a checked-RAM
abort on physical address0 when the next charge starts. These are being
corrected; battle completion with Eve attacking is not yet proven.

ATK26 (2026-09-05): `eve-flare.png` shows the eye/hand flare and expanding
green rings at F614 draw tick24, captured in the visible window at
frame10029. `eve-flare-progress.json` records Aya45/45 and queue0. The
following beam callback8018FDC4 is paused before execution at frame10039.
All73 original flare/ring call graphs match; normal and ASan/UBSan CTest
pass2/2 each,1,142 native cases. The beam itself is not yet verified.

ATK25 (2026-09-05): `performance-atk25-20.png` again verifies the full
stage backdrop and performers in the visible replay. `eve-charge.png`
shows green particles orbiting Eve before her first attack.
`eve-charge-progress.json` records effect tick162, Aya45/45 and queue0;
the next callback8018F614 is paused before execution. All125 original
charging/sprite/math call graphs match, excluding unused packet stack
padding. Normal and ASan/UBSan CTest pass2/2 each,1,141 cases. This is
charging-effect proof; Eve's flare/beam and naming remain in progress.

ATK16–19 (2026-09-05): effect-owner cleanup and animation pause/resume
unblock Eve's first-hit dialogue through message 49 and return to mode 0.
`first-hit-dialogue.png` shows this intermediate visible result. Damage,
hit reactions and the death/reward/victory initialization now match 82 + 24
full original-instruction fixtures; owner cleanup matches 18 more.

The ATK17 replay then exposed audio FIFO corruption: 31 unconsumed commands
reached Aya's battle record. `audio-overflow-before.json` records the entry
addresses and words from a read-only capture. Its 36 HP is corruption, not
an enemy hit. ATK19 restores the command consumer and voice-control handlers;
67 full original call graphs match and 240 commands across host frame waits
preserve Aya's record. Event registration/enable and lock gates are covered.
Normal and ASan/UBSan CTest pass 2/2 each, **1,135 native cases**.
The host services the command portion of the timer only; full score
sequencing and audible SPU synthesis remain unported. The new visible
ATK19 replay fired seven times across four ordinary command selections,
including reload, and reached Eve's retreat/message50. Aya remained45/45,
AT refilled to9000 at speed50 and the sound queue drained tozero.
`repeated-shots-proof.json` records that result. The retreat then reached
unported VM96; no debug writes repaired HP, position or progression.

ATK20 restores that script request and the mode8-to12 cleanup, hit flashes,
effect-pool destruction and HP/ammo saving. All69 original call-graph
comparisons pass. Normal and ASan/UBSan CTest pass2/2 each,1,136 cases.
The fresh visible replay verifies the exit: seven shots/reload lead to
VM96 at frame12248, mode12 cleanup and messages50–60. Input returns to0,
and a normal Right key moves Aya from x345 to125. HP45/45 and ammo5
match saved inventory; all seven body slots are released. See
`scripted-exit-proof.json` and `battle-return-to-stage.png`. Eve attack
effects and naming still need work; this verifies the scripted retreat.

STG8 / ATK15 (2026-09-04): the performance camera now completes its
scripted pan. `performance-before-20.png` / `performance-before-26.png`
show the reported black scene and actors cut off at the top. The matching
`performance-fixed-20.png` / `performance-fixed-26.png` show the full actors
and stage backdrop after restoring 66268 and its 35558 frame call.
`performance-camera-proof.json` records the read-only camera values.
The queued Y=400→112 move previously never advanced, shifting the geometry
and background outside the view. The original pan now updates both.

User confirmed visible targeting and AT reset in ATK13. Full shot animation
and hit checks are now native and match 22 + 53 original-instruction cases;
ten pan cases also match. Normal and ASan/UBSan CTest pass 2/2 each,
**1,130 native cases**. The first normal shot is now observed: 23008 executes at frame 15480,
ammunition drops 6→5, and 28574 applies six damage at frame 15481.
The original intro script's HP changes 1000040→1000034, which advances
Eve's first-hit script to mode 7. `battle-target-menu.png`,
`first-shot-start.png`, `first-hit-eve.png` and `first-hit-proof.json`
record this step. No debug progress or HP changes were made. ATK16/17 resolve that mode-7 pause as described above.
Further effect ticks, visible victory and naming remain incomplete. The following HUD2 and
STG7 observations are historical milestones.

`battle-hud.png` and `battle-hud-proof.json` record the HUD2 visible replay:
HP **45/45**, AT **9000** (full), weapon-derived AT speed **50**, active mode
**0**. This is the original packet/font HUD, reached through ordinary visible
keyboard input. Full `33A40`/`34104`/`334AC` packets match eight retail execution
fixtures. Equipment binding `2F76C` and all its stat/inventory callees match
33 retail execution fixtures. GPU command 65h for the HP/AT labels exposed a
renderer boundary; raw sprites and texel transparency are now supported.
Both normal and ASan/UBSan CTest pass 2/2, **1,113 native cases**, at HUD2.
Attack prerequisites are being implemented next. Attack input, Eve's effect
callback, a won battle, and name selection are **not yet verified**.

The visible native window now completes the opera performance and allows
Aya to reach Eve on stage. `stage-backdrop.png` shows the restored scenery;
`auditorium-aisle.png` shows Aya in the aisle with floor collision enabled;
`eve-conversation.png` shows Aya and Eve with the complete stage background.

The missing scenery came from omitted `65E48` camera-follow updates: the
512-pixel view's Y=256 pan survived the switch to a 224-pixel view. Restored
projection, bounds and smoothing update the pan and geometry offsets.
The new `1AE40` floor path checks actor radius against triangle boundaries,
slides along walls and computes slope height. Actor-to-actor collision is
still unported. Authority: `55C00.s` (65E48), `AB74.s` (floor collision),
`24240.s` (35558 frame call sites).

`battle-entry.png` shows the visible transition completing, with Aya and
Eve in the battle view. The black transition exposed
missing `6C5BC` loader states, unserviced asynchronous SPU DMA, missing
music/effect-bank upload functions, and a packed sound-count decode error
in `6D078`: the scene's two entries were interpreted as 130. These paths
are now implemented/corrected, including `6914C`'s related texture-count
decode. The next script barrier was AC/196E8; AC and E3/1A3FC now register
the controller's lists and continue. The battle-ready tail also releases
movement input and initializes UI colors; `716A4` stops movement and
preserves idle animation. The script reached active mode 0, exposing a
crash on Eve's first effect update: the room's effect exports had not
been registered at `6B968..6BA08`. Registration is now restored, so
CE49C replaces stale arena bytes with the loaded effect descriptor.
Normal and ASan/UBSan CTest pass 2/2 each, with 1,109 native cases and
no skips. The updated visible replay completes the transition and remains
in active mode 0 for 4,470 observed frames, without the former crash.
`battle-control.png` shows the final visible view; `battle-control.json`
records zero loader state bytes, input mask 4, and the valid 80190B44 effect
descriptor. A one-second Left arrow input moves Aya from X=22 to X=1349.5;
releasing it restores idle command 4 and zero velocity. Right input returns
her to the visible stage. The window remains open with automation stopped.

The earlier STG7 run verified battle entry and directional movement. Its HUD
was absent (now addressed by HUD2 above). `D4698` does not yet execute the loaded
effect callback `80190A6C`. Enter did not open an attack menu. Full combat
and attacks remain incomplete; the stable scene is not proof of those paths.
Earlier results below describe the opening-dialogue milestone.

## Implemented

- `6BECC` texture states 1–3 load and upload Aya's texture/CLUT package.
  The former forced state 4 skipped this upload, leaving every sampled Aya
  texel transparent. A synchronous DrawSync precedes staging-buffer reuse.
- `3A6A8` computes world reference coordinates, transformed clip offsets,
  selected-joint anchors and their camera projections. VM `CD/19CEC` adopts
  the animated coordinates as the actor's 16.16 position. The opening now
  moves beyond the animation where Aya's companion previously stopped.
- Field entry reinitializes `371B0` using the new field's language stream.
  Previously CE90 retained M0431's pointer, now pointing into M0010 model data.
- `37870` emits the ordinary 12×12 font SPRTs, redraws each complete page,
  handles F7 as a newline, and advances past the current F8 on confirmation.
  It also draws the existing profile/name text. The background packet
  addresses in `371B0` were corrected from `800AECxx` to `8009ECxx`, matching
  the signed low half of the retail LUI/ADDIU addresses.
- Window input takes priority over the old automatic dialogue confirmation.
  Enter/Space/Z/X map to Cross; arrows map to the directional pad.

Authority ranges: `asm/disc1/5B1E4.s` (6BECC), `2A19C.s` (3A6A8),
`A44C.s` (19CEC), `2F174.s` (field-entry 371B0), `279B0.s` (371B0),
`28070.s` (37870), and `682BC.s` (24-bit AddPrim stores).

## Visible evidence and validation

The game was run in a visible 960×720 X11 window on `:10.0`, titled
`Parasite Eve - Live Native Port`. GDB read-only RAM/framebuffer captures
were taken from this windowed process. No headless game run was used.

- `aya-textures.png`: Aya becomes visible after the texture upload fix.
- `opening-animation.png`: her companion has advanced to her left.
- `aya-profile.png`: the real font atlas renders Aya's profile.

Normal and ASan/UBSan CTest passed 2/2, including all 1,090 native cases at
the dialogue milestone. POS1 covers rotated roots, indexed anchors, signed
coordinate transfer and GTE saturation. TXT1 verifies multiline font pixels,
background submission, page retention, F8 advance and FF confirmation. The
opening regression now requires nontransparent samples from Aya's actual
texture/CLUT packets. The existing BTL55 retail instruction/hash audit passes.

## Explicit remaining limits

The opening subsequently requests EF(0), an unported `16F10 → 4DCA4` menu,
after profile message 11. Without a shortcut it now reports that boundary
and retains its script instruction instead of silently abandoning the task.
The separate `--skip-opening-menu` option retains current defaults only for
Aya's EF(0) in M0010. This is a HOST_ADAPTED demo shortcut, not a translated
menu. It does not inherit automatically from `--skip-movie`.

Field control, stair access and battle movement are verified above. Text custom
frames, substituted numbers and extended glyph pages remain partial. GTE
projection helpers expose coordinate outputs rather than all PS1 flags and
FIFOs; polygon rasterization remains the documented native approximation.

## INV15–17: battle medicine and defeat fade (2026-09-05)

Visible INV15 ordinary input selected Medicine 1 after a natural hit. At frame
26425 Aya healed38→45HP; the item was consumed and the blue particles and
rings rendered. [Captured effect](battle-medicine-effect-inv15.png). Seven
subsequent shot events prove that healing releases command execution. This
run ended in natural player defeat, so it is not encounter-completion proof;
see [read-only event record](battle-medicine-progress-inv15.json).

INV17 restores the two missing fade starts and flags before the game-over
state waits for their completion.24 original-executable cases verify those
state changes; the presentation packets are outside this bounded comparison.
Normal and ASan/UBSan CTest each pass2/2 with1181native test groups.
INV16 also matches297 original cases for PE reservations, costs, discounts
and availability. PE menu construction/action execution remain unfinished.

INV17 fresh visible retry completed: name entry, full stage view and stairs,
Medicine 1 (42→45 HP), healing particles/rings, seven shots, Eve's scripted
exit at12938, final message60 and ordinary movement afterward. No native
stop occurred. [Event and movement proof](complete-encounter-inv17.json),
[stage](performance-inv17.png), [healing](battle-medicine-effect-inv17.png),
[completed encounter](battle-complete-inv17.png). The native window remains open.
The repaired defeat fade still needs its own fresh visible defeat replay.


## INV18–20: PE menu and command execution (2026-09-05)

Implemented the original PE menu and confirmation path, full ability application,
field synchronization, paired sound requests, and battle command/effect waiting
and recovery. Tests match 200 application, 161 menu, and 142 command cases from
the original executable. The test fixture includes the actual sound/effect tables;
Liberation phase0 now saves Aya's pose and restores the missing input/fade flags
and effect cleanup. Four older fixtures now provide the empty effect pool that
the original cleanup reads. Normal and ASan/UBSan CTest pass2/2 each with1184
native groups (53.03s/62.55s). The subsequently expanded saved-pose hash range
also passes in both builds. Full Liberation presentation and several PE effect
callbacks remain unfinished; these are bounded state comparisons, not a claim
that every ability has been visibly played.

The fresh visible retry again accepted the name, rendered the full performance
backdrop and all three actors, and reached Eve via the aisle/stairs. See
[stage capture](performance-inv20.png). Its first menu check found PE excluded
by the opening fight's mask7D; the helper mistakenly selected Equipment and
reached that unfinished page's explicit native boundary. No live PE cast occurred.
The second ordinary-input replay completed without native boundaries:
Medicine1 healed40→45 with particles/rings, seven shots, Eve exit at12945,
message60, HP17, and ordinary field movement Z633→593. See
[completion event record](complete-encounter-inv20.json),
[healing](battle-medicine-effect-inv20.png), and
[completed encounter](battle-complete-inv20.png). The post-battle menu mask7F enabled PE. Ordinary V/Down/Confirm opened
Heal1, displayed its HP preview and confirmation, then applied it atframe18514:
live and savedHP17→45, PE80→20 (cost60). Both menus closed normally with C.
See [field PE event record](field-pe-use-inv20.json) and
[Heal1 result](field-pe-after-inv20.png). Battle PE casts are still verified
by original-executable cases only. The window remains visible, field input
is unlocked, and all helpers have finished with no held keys.


## INV21: original equipment page foundation (2026-09-05)

The observed45EE4 Equipment boundary is being replaced with the original
page family. Native functions now construct equipped-item, replacement-item,
and property lists; filter weapons/armor and property donors; preserve cursor
state and preview inventory bank; and draw base/bonus totals, comparison colors,
and signed small numbers.232 original-executable cases cover both inventory
banks, type masks (including masked MIPS shifts), empty/missing equipment,
normal/upgrade layouts, cursor restoration and NULL delay-slot stores, signed
bonuses and capped display totals. The case generator is
`pc_port/tools/pe_equipment_menu_oracle.py`; the native comparison is
`pc_port/tests/test_equipment_menu.h`.

These are full original-call-graph memory/packet comparisons of the implemented
functions. They do not prove that the complete Equipment screen is playable:
43DA4 still retains its explicit boundary until the remaining input and item-name
callbacks are native. The visible INV20 game remains at the completed encounter;
no guest state was changed to test this new page foundation.

Final INV21 verification: normal and ASan/UBSan CTest both pass2/2 with1185
native groups (47.21s/57.34s). The expanded232-case comparison is included.
No builds or keyboard helpers remain active; the visible INV20 process is
still running with restored field control and HP45 after Heal1.


## INV22: ordinary equipment selection (2026-09-05)

The main menu now opens native weapon and armor pages, including equipped
item labels, replacement rows, comparison properties, navigation/cancellation,
field equip and battle confirmation/command publication.215 additional
original-executable cases compare results and RAM state, covering both equipment
kinds, inventory banks, capacity bonuses, full page drawing and input dispatch.
All comparisons pass. Full normal and ASan/UBSan CTest pass2/2 each with1186
native groups (42.81s/52.84s; /tmp/pe-inv22-{tests,san-tests}.log).
Tool/upgrade actions remain explicit boundaries.

INV22 visible verification completed with ordinary keyboard input. The second
attempt accepted Aya's name, rendered the full stage and reached Eve by the
side aisle/stairs. Medicine1 healed42→45 atframe29844 with particles/rings;
seven shots led to Eve's scripted exit31945 and message60, with10HP live/saved.
See complete-encounter-inv22.json and the performance, medicine effect and
battle-complete INV22 PNG/RAM/RGB files. The first attempt lost after the
keyboard helper waited on an unaccepted menu press; the second retries input.
The route helper also needed clearance around a seat corner; collision code
was unchanged. Neither attempt hit an unresolved native boundary.

After the fight, the Weapon page showed M84F/Club1, comparison stats and the
equipped marker. Selecting Club1 changed the actual weapon index0→2;
selecting M84F restored2→0. Down focused the weapon property list and showed
Rate of Fire:2; Up returned to the equipped item. The armor page showed N Vest, handled selection
of the already equipped item and cancellation. Closing both menus restored
field control; ordinaryUp movedZ633→598. Screenshots and read-only snapshots
are equipment-*-inv22; equipment-use-inv22.json records the checks. No guest
inventory, HP, position or progression writes were used. Battle equipment
confirmation and armor capacity changes remain reference-tested rather than
visibly exercised in this run.
