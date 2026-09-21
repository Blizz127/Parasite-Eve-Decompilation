## CURRENT 2026-09-21: first branch merge landed — 810 -> 964 c spans on main

The native PC port is the deliverable; matching decomp is the means. Nothing is
committed to a scratch branch any more: **this is on `main` and pushed**
(`8e0dc06b`). Standing rules unchanged (no MIPS interpreter, recovered asm worth
zero to the port).

1. **`cursor/cd-sector-backpressure-6f51` merged.** It was a *parallel* decomp
   line (910 c spans of its own) against main's 810, almost disjoint, so the merge
   is a span-level union: `964 c / 401 asm`, `funcs 521/1153`, `c_words 11156`,
   `tierA 908`. `EXACT_REBUILD_GATE=PASS` with the retail SHA-1 unchanged;
   `VERIFY_SWEEP=PASS leaves=964`; port suite 1405/1405.

2. **Four lessons worth keeping** (full detail in
   `docs/ai_context/BRANCH_CONSOLIDATION_PLAN.md`): a blind file harvest breaks the
   repo's invariants because the branch's *YAML rows* are half the change;
   "same address -> main wins" silently dropped 36 leaves, so rank
   `rodata > c > asm`; the rebuild gate catches what preflight only warns about;
   and a preflight WARN is a real defect, not noise.

3. **Open follow-up — the dispatch-fold family.** Six branch leaves
   (`func_80012E7C`, `func_8002FE78`, `func_8003010C`, `func_8004AE1C`,
   `func_80051CC4`, `func_800C3238`) build under per-leaf
   `MASPSX_DISPATCH_FOLD=jtbl_X` profiles and fail `ERROR: .rodata N != pool table
   M for jtbl_X` regardless of whether the profile is applied. Excluded from the
   merge; resolving the jump-table carve would take main to ~970 c spans.

4. **Machine consolidation continues.** Settings/tools scouting is in
   `docs/ai_context/AGENT_SETTINGS_CONSOLIDATION.md`; 39 remote branches classified
   in the plan doc. `matts-macbook` is offline and `macserver` (100.72.127.15)
   still refuses this box's key, so the Mac row remains unread.

---

## PRIOR 2026-09-21: GOAL 4H #3 — the mesh colour transfer pair (810), port checked again

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.
Goal contract: `docs/ai_context/GOAL_4H_QUEUE_ROUND2.md`. Nothing committed.

1. **`func_800C6EF8` and `func_800C6F4C` (0x54 / 21 words each) are matching C.**
   Mirror word-array transfers between a mesh record and the fixed buffer
   `D_800E2370`: the record holds a byte offset in its `+8` halfword and a word count
   in `+0xA`, and the count is **re-read every iteration** (the stores could alias the
   record), which is why retail keeps `lhu 0xA(a0)` inside the loop. Both `LINK_EXACT`
   at the default `-O2 -G0`; YAML `[0xB76F8 c][0xB774C c][0xB77A0 asm]`.

2. **The spelling fix: the counter increment belongs in the loop body, before the
   copy.**
   ```c
   while (i < *(unsigned short *)(record + 0xA)) { i++; *dst++ = *src++; }
   ```
   Written as a `for (i = 0; i < count; i++)` header the increment is scheduled after
   the load and exactly three words differ. Sixth source-spelling fix of the session.

3. **Proof.** `disc1_preflight.py --deep` PASS (810 c / 347 asm / 2 rodata);
   `EXACT_REBUILD_GATE=PASS` with
   `sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
   `VERIFY_SWEEP=PASS leaves=810`, `plan=184dab537bab…`. Report:
   `docs/evidence/phase5gb-mesh-colour-pair/REPORT.md`.

4. **Metric note (third time).** Plan 808 → **810** c spans, but `funcs` stays
   **377/980** and `c_words` 6834 — these two sit outside the direct-call closure like
   the 42E34 cluster and the arena pair. The plan count plus the gate are the evidence.

5. **Port verification, round three.** The port implements both in
   `pc_port/game/boot/func_800C71E4_port.c` and **agrees exactly**: same source
   derivation (`mesh + *(u16 *)(mesh + 8)`), same count field re-read every iteration
   (`PE_LoadU16(mesh+10u)` in the loop condition), same word granularity, mirror
   operands swapped the same way. The only cosmetic difference is `unsigned i` versus
   the decompiled `int i`, which the `lhu` / `blez` / `slt` sequence makes equivalent
   for a 16-bit count — so there is **no signed/unsigned trap at `count >= 0x8000`**
   for either implementation. `PORTVERIFY_matched_leaves` now pins the pair and covers
   nine functions.

6. **Next.** `func_800C6FA0` (the effect-level sibling of this pair, 0xF8 = 62 words,
   already partly present in the port) and the queue from `port_priority.py`.

---

## PRIOR 2026-09-19: GOAL 4H — the arena push/pop pair (808) + port checked again

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.
Goal contract: `docs/ai_context/GOAL_4H_QUEUE_CONTINUE.md`. Nothing committed.

1. **`func_8005E8C4` (push, 0x50) and `func_8005E914` (pop, 0x54) are matching C.**
   They move the arena write cursor `D_8009D12C` (gp+0x3BC) by eight bytes against
   the window `0x800A2270..0x800A22B0`, exchanging `D_8009D124` / `D_8009D128`
   (gp+0x3B4 / gp+0x3B8). Both `LINK_EXACT` at `-O2 -G8`; YAML
   `[0x4F0C4 c func_8005E8C4][0x4F114 c func_8005E914]`.

2. **Two spelling devices, both load-bearing.** Reading both source words into
   locals *before* the cursor moves keeps the two `lw`s above the stores (inline
   loads leave the second one below the first store: 7 words off); and declaring the
   arena bound as a 3-word object pushes it above the `-G8` small-data threshold so
   its address comes out absolute, as retail has it — the same device as Phase 5FY,
   again without a `MASPSX_FORCE_ABSOLUTE_SYMBOLS` entry.

3. **Proof.** `disc1_preflight.py --deep` PASS (808 c / 347 asm / 2 rodata);
   `EXACT_REBUILD_GATE=PASS` with
   `sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
   `VERIFY_SWEEP=PASS leaves=808`, `plan=49257b756e66…`. Report:
   `docs/evidence/phase5ga-arena-pair/REPORT.md`.

4. **Metric note.** c spans 806 → **808** but `funcs` stays **377/980** and
   `c_words` 6834: like the 42E34 cluster these two are reached through the menu
   draw path, not the direct-call closure, so the union metric cannot see them.
   `port_priority.py` regenerated.

5. **Port verification, round two.** `func_8005E8C4`/`func_8005E914` ↔
   `pc_port/game/boot/func_8005EED4_port.c`: **agrees** on cursor arithmetic, bounds
   and word order (the port stores through the documented `menu_ram()` KUSEG
   mirror). The single difference is that the port elides the `func_800527C0(2)` /
   `(3)` call on the out-of-bounds path — behaviourally nil, because that retail
   callee is an empty `jr $ra; nop` stub and the port elides the same call
   elsewhere with that reason stated (`func_8005ED18_port.c`). What the decomp adds
   is the previously unrecorded **report codes (2 = push overflow, 3 = pop
   underflow)** and the arena window. `PORTVERIFY_matched_leaves` now covers seven
   functions; suite **1405/1405**.

6. **Next.** `func_80077404` (the poll side of the `func_800773D0` timer pair) and
   the queue top from `port_priority.py`.

---

## PRIOR 2026-09-19: GOAL 6H — three more leaves (806) and the port verified against the decomp

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.
Goal contract: `docs/ai_context/GOAL_6H_DECOMP_PORT.md`. Nothing committed.

1. **`func_8005270C` (0x58) closed — the Phase 5FW residual.** The blocker was
   addressing, not structure: retail computes `&D_800B0E08` once into `$a0`
   (`lui`+`addiu`) and loads through it twice. Under `-G8` a 4-byte declaration
   makes cc1 address it gp-relative, and `MASPSX_FORCE_ABSOLUTE_SYMBOLS` only
   rewrites that into a *fused* `lui`/`lw` — a shape cc1 never emits for a live
   address register. **Declaring the pointer as a 3-word object** (above the `-G8`
   threshold) makes cc1 emit the genuine absolute address and it matches with no
   knob. Element 0 is the same 4-byte pointer, so no emitted access changes.

2. **`func_80052C08` (0x64) and `func_800773D0` (0x34) landed.** The first needed a
   spelling change: retail's 0xFF-terminator scan carries `addiu dst,dst,1` in the
   branch delay slot of the *exit* test, so the post-increment form with an
   explicit step back (`while (*dst++ != 0xFF) { } dst--;`) rotates both loops the
   way retail does; the pre-increment spelling is 13 words off. `func_800773D0` was
   word-exact first try (arm the GPU deadline from VSync + `0xF0`, clear the poll
   word).

3. **Proof.** All three `LINK_EXACT`; `disc1_preflight.py --deep` PASS (806 c / 348
   asm / 2 rodata); `EXACT_REBUILD_GATE=PASS` twice (804 then 806 spans) with
   `sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
   `VERIFY_SWEEP=PASS`, `plan=87f1d99d909d…`. Report:
   `docs/evidence/phase5fy-5fz-sixhour/REPORT.md`.

4. **Metrics moved for real this time.** The union grew 979 → **980** (`tierA` 734
   → 735): `funcs` 374/979 → **377/980**, `c_words` 6776 → **6834**, `asm_funcs`
   529 → **527**; matching c spans 804 → **806**; `port_priority` 55 → **52**
   active candidates, new top `func_8005E8C4`.

5. **Port verified against the new decomp** — this is the second half of the goal.
   Every leaf landed this session has a hand-written port counterpart, and the
   authoritative C turns them into checked statements;
   `pc_port/tests/test_port_verify_decomp.h` (`PORTVERIFY_matched_leaves`) pins
   them in the native suite. Verdicts: `func_80062F3C` ↔ `func_80062D2C_port.c`
   (`func_80062A34` reproduces the walk — head `0x8009D154`, `+0x20`/`+0x24`,
   NULL-on-miss); `func_800773D0` ↔ `func_80076C34_port.c` (deadline/poll words);
   `func_8005270C` ↔ `field_message_port.c`; `func_80052764` ↔
   `battle_reward_port.c`; `func_80052C08` ↔ `func_8004F910_port.c`. **No semantic
   drift found**; the only differences are the documented host substitutions
   (VSync query, sound-package stub) and one factoring difference
   (`func_80052C08` inlines the copy in retail, the port calls `func_80052BCC`).

6. **Reusable method — try spelling before flags.** Four leaves this session were
   unblocked by source form, not compiler options: `volatile int *p` (two loads
   through one address), `int one = 1;` (loop constant hoisted into the entry
   block), the post-increment scan plus `dst--` (loop rotation), and a non-small
   declaration (absolute vs forced-absolute addressing).

7. **Next.** `func_8005E8C4` (new queue top); `func_80077404` is the poll side of
   the `func_800773D0` pair and the natural follow-up in that region.

---

## PRIOR 2026-09-18: `func_80062F3C` matched — queue top cleared, gate PASS (803)

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.

1. **`func_80062F3C` (24 words / 0x60) is matching C.** Walks the node list rooted
   at `D_8009D154` (`gp+0x3E4`) for the entry with `+0x20 == 1 && +0x24 == arg`,
   then calls `func_8006269C` with that node (the cursor stays in `$a0`, so a miss
   passes NULL). File `src/func_80062F3C.c`, YAML `[0x5373C, c, func_80062F3C]`
   (the whole former asm span), profile `era_o2_g8` (`-O2 -G8`, already holds
   `func_80062A34`/`func_80052764`), staged not committed.

2. **The lever was the source, not a flag: `int one = 1;` is load-bearing.** Every
   structural spelling (pointer casts, struct fields, `for`, nested `if`, volatile
   head) produced the *same* 6-word prologue difference — retail hoists the loop
   constant into the entry block (`addiu a1,zero,1` before the guard) and keeps
   `addiu sp` first, while the naive `== 1` materialises the constant inside the
   loop body and sinks the frame adjust. Holding the constant in a local gives it a
   register home and GCC hoists it exactly as retail did. Swapping the comparison
   operands (`one == x`) flips two `bne` operand orders (2 mismatches), so the
   operand order matters too. Full attempt table:
   `docs/evidence/phase5fx-80062f3c/REPORT.md`.

3. **Proof.** `era_link_check.py` → `LINK_EXACT`; `disc1_preflight.py --deep` →
   PASS (803 c / 348 asm / 2 rodata); `EXACT_REBUILD_GATE=PASS`,
   `sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
   `VERIFY_SWEEP=PASS leaves=803`, `plan=2b52a0ac44ea…`.

4. **Metric.** Unlike the 42E34 cluster, this leaf **is** in the 979-function
   union, so the numbers move: `funcs` 373→**374/979**, `c_words` 6752→**6776**
   (+24 = 0x60 exactly), `asm_funcs` 530→**529**. `native_c` stays 258/979.

5. **Reusable lesson for the parked leaves.** `func_8005270C` (0x58, 11/22 words
   off) has the *same* signature — a value that retail hoists into the entry block
   (there it is `&D_800B0E08` in `$a0`, not a constant). The thing to try there is a
   source spelling that gives the address a register home (a named pointer local
   used for both the test and the argument), exactly as `int one = 1;` did here.

6. **Next.** `func_80052C08` (25 w), `func_800773D0` (13 w), then the queue;
   `func_8005270C` is the closest parked win.

---

## PRIOR 2026-09-18: 42E34 cluster — four more matching leaves, gate PASS (802)

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.

Resumed at the top of the repaired candidate queue. The former `[0x42E34, asm]`
span turned out to be a five-function cluster, and four of the five landed:

1. **`func_80052634` / `func_8005267C` / `func_800526C4` (0x48 each)** — void twins
   of `func_800525EC` with sound ids `0x44D`/`0x44E`/`0x44F`. The same
   volatile-pointer form applies unchanged; all three were `LINK_EXACT` at
   `-O2 -G0` (default `era_o2_g0`) on the first attempt.

2. **`func_80052764` (0x2C)** — the fade-stop companion: if a target is published
   in `D_8009D01C`, call `func_800866A4(target, 0)` and clear it. `D_8009D01C` is
   gp-relative (`0x2AC($gp)`), so this leaf uses the existing `era_o2_g8` profile
   (the neighbouring `func_80052790` is already in that group) — `LINK_EXACT`.

3. **Symbol resolution, reusable.** `_gp = 0x8009CD70` (from the generated
   `build/abs_syms.ld`), so `_gp + 0x2AC = 0x8009D01C`; that cross-checks against
   the port's own `GM_D_8009D01C` in `pc_port/game/boot/field_message_port.c`
   (`/* gp+0x2AC: func_8005270C fade target */`). `disc1_build.py` probe-links
   first and **auto-derives `D_<addr> = 0x<addr>`** for any undefined `D_` symbol,
   so naming data globals `D_<their VRAM address>` is all that is required.

4. **Residual — `func_8005270C` (0x58), left as `asm`.** Structure fully
   understood (the port already implements it in `field_message_port.c`), but the
   codegen is not matched: retail keeps `&D_800B0E08` in `$a0` across the branch so
   both loads go through it. Best attempt is the volatile-pointer if/else form at
   `-O1 -G8` — identical structure (22 instructions, `j`/`nop` over the else arm,
   `move v0,zero`, one store) but the address is materialised twice, 11/22 words.
   `MASPSX_SYMBOL_AT_TEMP`, `MASPSX_SYMBOL_LOAD_DEST_TEMP`,
   `MASPSX_THREE_WORD_SYMBOL_STORE` and `MASPSX_FILL_STORE_DELAY_SLOT` each changed
   nothing. Details + the attempt table: `docs/evidence/phase5fw-42e34-cluster/REPORT.md`.

5. **Proof.** `disc1_preflight.py --deep` → PASS (802 c / 349 asm / 2 rodata), then
   `EXACT_REBUILD_GATE=PASS` with
   `sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
   `VERIFY_SWEEP=PASS leaves=802`, `plan=dc4ec73840…`. New sources are staged, not
   committed.

6. **Metric honesty.** `funcs` stays `373/979` and `asm_funcs` `530/…`: these four
   leaves are **outside** the 979-function direct-call union, so the coverage
   metric cannot see them. What moves is the plan (798 → **802** c spans) and the
   gate. `port_priority.py` regenerated: 60 → **56** active candidates, 111 → 115
   integrated; new top candidate `func_80062F3C`.

7. **Next.** `func_80062F3C` (24 w), then `func_80052C08` (25 w), `func_800773D0`
   (13 w); the `func_8005270C` register-allocation puzzle is a known `<8`-mismatch
   target for a C-spelling hunt.

---

## PRIOR 2026-09-18: route autopilot hand-over — drive to a fight, then play it

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.

Requested: "start the port and put me to the second Eve fight". Two blockers were
found; one is environmental, one was a real gap and is now fixed.

1. **Finding — this box has no display.** No Xorg, no Xwayland, no Wayland
   compositor; `/tmp/.X11-unix` empty; no X11/Wayland sockets (`ss -lx`); `DISPLAY`
   unset. The port's window backend is X11 (`dlopen` libX11 + `XOpenDisplay`), so no
   window can be opened here. Everything below was verified `--headless`.

2. **Fixed — the autopilot could not hand the pad back.** `--route-pad` installs
   `RoutePadSource`, which returned the route mask unconditionally, so the player
   could only watch a bot play or start from boot. `pc_port/src/port_main.c` now
   has `--hand-over-at <frame>` (release at a route frame) and `--hand-over-key`
   (release when the player presses a button; combined with the frame gate this
   reads "wait for the frame, then my button"). `RouteHandOverReady()` is checked
   first in `RoutePadSource`; afterwards the pad comes from `HostWindow_PadRaw()`
   (idle `0xFFFF` with no window, so a headless hand-over just stops driving).
   Evidence: `docs/evidence/pe-handover-route-pad/REPORT.md`.

3. **Verified run** (`--hand-over-at 52500`): the autopilot booted the retail disc,
   skipped movie/opening menu, drove Day 1 through the first Eve fight (m0023i) and
   the first sewer fight (`route: sewer victory room=1 frame=52111 HP=27`), then
   printed `[ROUTE] hand-over at frame 52500 (--hand-over-at): pad is the player's`
   — token `m0027i`, `victories=1`, `mode=9`, `stop_reason=frame-limit`, exit 0. The
   captured 320x240 frame is a live field scene (sewer corridor, Aya lit).

4. **Frame anchors for "the second Eve fight"** (from the recorded pilot): first
   sewer victory 52111, `m0028i` (second hallway) ~53.0–53.5k, second sewer victory
   53823. Use `--hand-over-at 53400` to be dropped in the second hallway, or
   `--hand-over-key` to grab control whenever ready.

5. **Commands.** On a machine with a display:

   ```sh
   cd /home/blizz/dev/Parasite-Eve-Decompilation
   ./pc_port/build/parasite-eve-port --route-pad --hand-over-at 53400 \
     --disc-image "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"
   ```

   Cross is Return/Space/Z/X. On a machine without a display add
   `--headless --max-frames N --screenshot out.ppm` instead.

6. **Scope.** `pc_port/src/port_main.c` is the CLI host and is *not* linked into
   `pe-native-tests`, so the 1404-case suite and both routes are unaffected; every
   target builds clean. The hand-over changes only which host source answers the
   pad poll — no guest state is touched.

---

## PRIOR 2026-09-18: first matching-C leaf registered from the repaired queue — exact rebuild PASS

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.

The repaired candidate order (`func_800525EC` first) was worked, and the leaf is
now registered and proven by the exact-rebuild gate.

1. **`func_800525EC` (18 words / 0x48) is matching C.** Menu sound helper:
   `if (D_800B0E08) func_8006DF50(D_800B0E08, 0x44C, 0x100, 0x80, 0x7F)`. File
   `src/func_800525EC.c` (staged, not committed); YAML
   `[0x42DEC, c, func_800525EC]` (mid-42D94 carve: asm prefix 0x58, C 0x48, asm
   resumes 0x42E34); profile = the YAML default `era_o2_g0` (`-O2 -G0`), so **no
   per-leaf profile assignment was needed**.

2. **The whole difficulty was codegen, and the sweep is the record.** Retail
   materialises `&D_800B0E08` once into `$a0` and reads through it twice. A plain
   `extern int` folds the two reads into one load (16 word mismatches); a plain
   `volatile int` materialises the address twice (11); reading through a
   `volatile int *p = &D_800B0E08` gives the single materialisation plus two loads
   and is **word-exact at `-O2 -G0`** (0 mismatches). Full write-up:
   `docs/evidence/phase5fv-525ec/REPORT.md`.

3. **Proof.** `era_link_check.py` → `LINK_EXACT` (18/18 words, zero pad);
   `disc1_preflight.py --deep` → PASS (798 c / 349 asm); then the full gate:
   `EXACT_REBUILD_GATE=PASS`,
   `sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
   `VERIFY_SWEEP=PASS leaves=798`, `plan=aaaa8873cd47…`.

4. **Metric movement (and non-movement).** `route_coverage.py`: `funcs` 372→**373**
   /979, `c_words` 6734→**6752**, `asm_funcs` 531→**530**. The *port* metric
   `native_c` stays **258/979** — different metric, unchanged.
   `docs/generated/NATIVE_CANDIDATE_PRIORITY.md` regenerated: baseline 797→**798**
   spans, candidates 61→**60** (the leaf left the pool); top candidate is now
   `func_8005267C`, its twin (18 words, sound `0x44D`).

5. **Two staging rules the gate enforces** (both hit on the first attempt): a new
   YAML `c` source must be `git add`-ed (`staging gap — a YAML C source is not
   git-tracked yet`), and `docs/generated/DISC1_MATCHING_STATUS.md` must be
   regenerated (`python3 tools/build/disc1_plan.py --write-status`). Nothing is
   committed — the new source is staged only, matching the repo convention.

6. **Port untouched.** No `pc_port/` change in this step, so the native suite and
   both routes are unaffected: re-verified full suite **1404/1404**, and the last
   route runs remain plain 45/57 / pilot 57/57.

7. **Next.** `func_8005267C` is the closest win (same shape, `0x44D`, 18 w), then
   `func_80062F3C` (24 w), `func_80052C08` (25 w), `func_800773D0` (13 w) from the
   repaired order. Reusable recipe:
   `python3 tools/analysis/era_link_check.py src/<name>.c <vram> <size> [flags]`
   (no YAML edit needed to iterate).

---

## PRIOR 2026-09-18: field-menu tree complete + port-priority tool repaired

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.

Two things landed: the field-menu tree is now fully native (**zero named
boundaries**), and the port-priority tool that ranks matching work is repaired and
now reports the true baseline.

1. **Tree closed.** The last named boundary — the modal window draw
   `PE_MenuDrawCallback_8004B5DC` — is native, together with its two untranslated
   callees:
   - `func_8005FCAC` (81 w) — signed-number printer: icon `0x52` for a negative
     (2 digits) / `0x89` for a positive (3 digits); leading zeros become a blank
     (`func_8005F874(-1)`); `x += 5` per cell.
   - `func_8005ED18` (108 w) — icon/sprite packet builder: reserves 0x28 bytes from
     the menu packet pool, writes the colour word at `+4` (with `+7 = 0x2C` the GPU
     command and `+3 = 9` the word count), the UV rectangle, the mode-2 interior
     UVs, the texture page, then links the packet into the ordering table.
   - `func_8004B5DC` (29 w) — the modal draw itself: `func_8005E8A4(0x10,0x0A)`,
     `func_8005FCAC(8 - func_8005E884())`,
     `func_8005EB58(func_80073A44(-1) & 0x10)`, icon `0x7B`, `func_8005ED18(0x7B,2)`.
   Grepping the three dispatchers now shows only the *generic* fallback arms
   (`PE_MenuInputCallback`, `PE_MenuDrawCallback`); no tree-specific symbol remains.

2. **Address-mirror note.** Retail writes the packet through a possibly-null
   pointer; the PS1 maps low addresses onto the same RAM, but the port's range
   guard aborts. `func_8005ED18` therefore routes the packet and ordering-table
   pointers through the hardware KUSEG mirror (`< 0x200000 -> |0x80000000`), the
   same convention as `menu_ram()` in `func_8005EED4_port.c`: same physical RAM, no
   abort, identical observable state.

3. **Port-priority tool repaired (two stale inputs).**
   `tools/progress/port_priority.py` exited with `ERROR: priority frontier label
   disagrees with generated native metrics`:
   - `configs/USA/port_priority.json` still described a cut inside
     `func_80030894`, but that body is **complete** (788/788 words) and
     `native_metrics` reports `production_frontier = None`. The schema could not
     express that state; it now accepts `label: null` (with an empty
     `remaining_direct_callees`, and still requires callees when a label is set),
     and the generated table renders "none — body is complete".
   - `linked_native_sources()` still required a `PORT_SRCS` list that the project
     folded into `PLATFORM/BOOTSTRAP/GAME_SRCS`; it now uses
     `optional_cmake_sources`, matching `native_metrics`.
   `--write-status` / `--check-status` now pass, and the regenerated
   `docs/generated/NATIVE_CANDIDATE_PRIORITY.md` replaced a badly stale table:
   **matching baseline 797 exact C spans (the stale doc said 411)**, 61 active
   candidates (was 140), frontier "none". Top matching targets are now
   `func_800525EC` (18 w), `func_8005267C` (18 w), `func_80062F3C` (24 w),
   `func_80052C08` (25 w), `func_800773D0` (13 w).

4. **Verification.** Focused `DAY2_field_menu_input_4ae1c` PASS (parts 7–14 cover
   the modal draw, `func_8005FCAC`'s three cases via the x advance, and
   `func_8005ED18`'s allocation / colour word / link word). Full suite
   **1404/1404**. Plain route unchanged (`45/57`, only the four HOST_ADAPTED skips,
   no unresolved boundary). Pilot unchanged (**57/57 PASS**). Logs:
   `route_modal_plain.log`, `route_modal_pilot.log`.

5. **Honesty.** `route_coverage.py`: `disc=1 funcs=372/979 native_c=258/979`
   unchanged — hand TUs derived from asm are not matching-C leaves, and the
   priority work is tooling. `gen_decomp_ports.py --verify`: 269 TUs, 0 drift.
   `git diff --check` clean; no game data; no interpreter; nothing committed.

6. **Next.** The real remaining work is (a) matching-C leaves, starting from the
   repaired candidate order (`func_800525EC` first), and (b) the guarded callback
   boundaries (`PE_MenuConfirmationCallback`/`PE_MenuNoticeCallback`/
   `PE_MenuCellEnabled`/`PE_InventorySort*`) that a deeper play session would need.
   Both routes are bounded by the synthesized pad program
   (`g_supply_pad_end = 62000`), not by port gaps, so extending play needs new pad
   capture rather than more stubs.

7. **Matching-leaf recipe + `func_800525EC` head start.** A single leaf can be
   tested *without* touching `configs/USA/disc1.yaml`:

   ```sh
   export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
   python3 tools/analysis/era_link_check.py src/func_800525EC.c 0x800525EC 0x48 [flags]
   ```

   `era_link_check.py` compiles the source, links it at the retail VMA and prints a
   word-for-word diff against the SHA-1-exact EXE (`ROM` vs `LNK`), so a candidate
   can be iterated with no YAML/profile edits; only a verified leaf should be
   registered (`check_leaf.sh` = link check + `disc1_preflight.py --deep`).
   Findings for `func_800525EC` (18 words / 0x48, `%hi/%lo D_800B0E08`, test,
   then `func_8006DF50(sound, 0x44C, 0x100, 0x80, 0x7F)`):

   - a plain `extern int D_800B0E08;` gets CSE'd to one load — retail loads the
     global twice (test into `v0`, reload into `a0` for the call), so the source
     needs `volatile` (or an equivalent double access);
   - best attempts (volatile, era cc1) reach **11/18 words** at `-O1 -G0` /
     `-O1 -G0 -fomit-frame-pointer`, but emit 20 words vs 18: retail keeps
     `addiu sp` → `lui a0` → `addiu a0` → `sw ra` (mine saves `ra` first) and
     drops the extra pair. Not yet matched — needs the profile/maspsx knob sweep
     the era batches use. The experiment's `src/func_800525EC.c` was removed so no
     unmatched C is left in `src/`.

---

## PRIOR 2026-09-18: field-menu tree closed (equipment / modal / close pages)

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.
Goal contract: `docs/ai_context/GOAL_4H_FIELD_MENU_TREE.md`.

The residual named by the previous handoff was the six remaining **named**
boundary arms in the field-menu dispatchers plus the `func_8004AE1C` case-4/5
close arm. All are translated now; 363 words of split asm became native hand TUs.

1. **Six new leaves** (one function per file, address/word span quoted in each
   header):
   - `func_8004B214` (96 w) — Equipment **window draw** (`window+0x30`): tints the
     two rows of three lane arrows from the list index and the `func_8005E54C()`
     pad-focus bits (0x1000 / 0x4000), icons 0x4A/0x4B.
   - `func_8004B394` (104 w) — Equipment **input** (`window+0x2C`): Up/Down clamp
     the packed colour byte lane `index*8` (`+2` clamps at 0xE8, `-2` at 0x20)
     through `func_800614A0/AC`; confirm clears the pair then republishes list0
     with `-1`; cancel restores `D_8009D260`.
   - `func_8004B650` (47 w) — Modal input (`window+0x2C`): 0x1000/0x4000 scroll
     `func_8005E850(0,∓1)` + `func_8005267C`; 0x10000 closes; 0x40 scrolls by
     `D_8009D264 - func_8005E884()` then closes.
   - `func_80050438` (33 w) — Equipment list1 per-cell draw (reached through
     `func_8004B534 -> func_800638D8`): alpha `0x80<<(cell*8)`, text id
     `cell+0x35`, digit `((D_8009D14C>>(cell*8))&0xFF - 0x20) >> 1` (arithmetic).
   - `func_8005D994` (62 w) — close page: view bytes 0x32/0x62, `D_800C0E24 =
     0xFFFFF`, the seven-stat row (index 0 → all 0x3E8), then the commit.
   - `func_8005247C` (21 w) — commit: `func_8005218C()` then `record+0x1C` →
     `+0x0E` and `+0x0C` (full heal to the recomputed max).

2. **`func_8004AE1C` case 4/5 is native.** It used to emit the named
   `PE_MenuInputClose` boundary; it now runs `func_8005D994(index-4)`,
   `func_80062F1C(node)`, `func_800439D8()`, `func_800525EC()` and falls into the
   shared tail, which calls `func_800525EC()` a second time (preserved:
   `asm/disc1/37CD0.s:4122`, the previous increment's comment already flagged it).

3. **Dispatch bug fixed.** The previous increment wired `0x8004B214` into
   `menu_callback` — the **input** dispatcher. `window+0x30` is the *draw* slot,
   so the Equipment draw was unreachable and the equipment *input* was routed to
   the wrong symbol. `0x8004B214` now lives in `menu_draw_callback`; the input
   dispatcher keeps `0x8004B394`/`0x8004B650` (`window+0x2C`). The route never
   opens the Equipment page, which is why the latent inversion survived.

4. **Only one named boundary remains in the tree:** `PE_MenuDrawCallback_8004B5DC`
   (the modal window's own draw). Its callees `func_8005FCAC` (0x144 = 81 w) and
   `func_8005ED18` (0x1B0 = 108 w) are the next residual.

5. **Verification.** Focused `DAY2_field_menu_input_4ae1c` PASS, with new parts
   8–12 (equipment draw / up / down / per-cell, equipment confirm + cancel, modal
   four arms, close page, jump-table case 4). Full suite `1404/1404`. Plain route
   unchanged: `frames=62000 stop=frame-limit`, 45/57, only the four HOST_ADAPTED
   skips, no unresolved boundary (`route_tree_plain.log`). Pilot unchanged:
   57/57 PASS (`route_tree_pilot.log`).

6. **Honesty.** `route_coverage.py --quiet --no-history`: `disc=1 funcs=372/979
   native_c=258/979` **unchanged** — these are asm-derived hand TUs, not
   matching-C leaves, so the metric does not move. `gen_decomp_ports.py --verify`:
   269 TUs, 0 drift. `git diff --check` clean; no game data; no interpreter;
   nothing committed.

7. **Harness gotcha worth an hour.** These draws allocate from the menu packet
   pool (`0x8009D100`/`0x8009D104`, `PE_MenuPacketAlloc`). A unit test that calls
   them directly must seed the pool (`menu4ae1c_draw_setup`) first, otherwise the
   allocator returns small integers (20, 28) and a packet writer (`func_80077C84`)
   stores to an invalid guest address and aborts (`FATAL: PE_StoreU8 ... 0x17`).
   The route always has a seeded pool — harness requirement, not a port bug.

---

## PRIOR 2026-09-18: milestone 55/56 re-pinned — pilot 57/57, Items/Use verified native

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter. `{SCRATCH}` = `/tmp/pe-2nd`.

The residual named by the previous goal ("milestone 55 normal item use") turned
out to be a **stale test pin, not a port gap**. The port already runs the whole
field Items/Use chain natively; the harness could not see it.

1. **The item use is native and correct.** Live trace (`PE_ROUTE_ITEM_TRACE=1`)
   of the reward pilot: `LOOT_PILOT_HEAL_DONE 58044 hp=53 item4=0` at token
   `A80030C8` (m0031i) — item7 left slot 4 and HP went 33 → 53. The chain is
   `func_80044B0C` (menu_callback `0x80044B0C`) → action 0 Use →
   `func_80057834` → `func_800516B4` → `func_80023E14` (heal 90, then clamp to
   `record+28`) + `func_80057D30` (remove from inventory). All native.
   The pistol selection is `LOOT_PILOT_EQUIP_QUEUED 58917 command=407 slot=0`.

2. **Two milestone pins were route-specific and unsatisfiable.**
   - Row 55 compared HP to the absolute 45 — Aya's **max** HP in the run that
     authored it. The route now levels her to max 53 (`record+28`), so the pin
     is "HP is full after the Items/Use" now (strictly stronger).
   - Row 56 only sampled the gun slot **on the item-use frame**, but the field
     item use (58044, gun=2) always precedes the equip (58917), so it could
     never fire. It now observes `0x800BE834==407 && 0x800C0E20==0` after the
     item7 pickup.

3. **The endpoint pin was stale too.** It required the run to end at the frozen
   m0031i token/PC with HP45, `persist[1]=0x14E` and `Aya+0x98` bit 0x200 clear.
   The pilot advances into the **m0032i fight** (that is where milestone 56's
   equipment command is issued) and cannot park at m0031i: observed endpoint
   `token=0xA80031C8 pc=0x00000000 hp=27/64 persist1=0x20 aya98=0x600`. The
   endpoint check now asserts the durable state — persist[74]=0x68, persist[24]
   bits, C8/C9 retained, item6/7 consumed, chest 0x280 + switch bit4, pistol
   slot 0, two victories, field control, no unresolved boundary — and prints the
   observed endpoint on `route: endpoint …`. The room-local bits (persist[1],
   Aya+0x98) are reported, not required. The frozen m0031i branch is preserved
   and additionally requires `persist[1]=0x14E`.

4. **Result.** Pilot: `frames=62000 stop=frame-limit story=0x68 token=0xA80031C8`,
   **57/57 milestones, PASS** (`route_m55c.log`, exit 0). Plain route unchanged:
   `frames=62000 stop=frame-limit story=0x09 persist1=0x0A token=0xA8000148`,
   45/57, no unresolved boundary (`route_plain3.log`) — the plain route never
   drives the sewer battles, so the supply observations stay 00.

5. **Coverage unchanged (honest).** `route_coverage.py --quiet --no-history`:
   `disc=1 funcs=372/979 c_words=6734 native_c=258/979 native_c_words=27622`.
   This work is a test-pin correction plus diagnostics; it adds no native leaf.
   `gen_decomp_ports.py --verify`: 269 TUs, 0 drift.

6. **Diagnostics added** (env-gated, off by default, read-only):
   `PE_ROUTE_ITEM_TRACE=1` prints `ITEM_TRACE` on every change of token / HP /
   item slot / gun slot, and the harness always prints the `route: endpoint …`
   line. `git diff --check` clean; no game data; no interpreter. Next: the
   equipment and modal sub-pages (`0x8004B214`/`B394`/`B650`/`0x8004B5DC`/
   `0x80050438`) and the close page (`func_8005D994` + `func_8005247C`), now the
   only named boundary arms left in the field-menu tree.

---

## PRIOR 2026-09-18: field-menu input tree 0x8004AE1C — plain route boundary closed

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter.

The previous session left the plain disc-1 route stopping on
`BOOTSTRAP_RET PE_MenuInputCallback` (raw callback `0x8004AE1C`) at
`frames=61623`. That whole input tree is now native, and the plain route reaches
the frame cap with **no unresolved boundary stub at all**. `{SCRATCH}` for this
goal is `/tmp/pe-2nd`.

1. **Executed-path native leaf: `func_8004AE1C`** (72 words,
   `[0x8004AE1C,0x8004AF3C)`, asm/disc1/37CD0.s) is
   `pc_port/game/boot/func_8004AE1C_port.c` — the field-menu Items/Escape
   `menu_callback` (`0x8004AE1C`). Its jump table is retail data, not a guess:
   `asm/disc1/data/800.rodata.s:1309 jtbl_80011034` =
   `{0x8004AE84, 0x8004AE94, 0x8004AEA4, 0x8004AEB4, 0x8004AEC4, 0x8004AEC4}`,
   i.e. cases 0..3 → the four sub-page constructors, 4/5 → the shared close arm,
   and ≥6 → the `func_800525EC`/return-1 tail (reached **once** for ≥6, **twice**
   for 4/5 — the same double-`525EC` fall-through is preserved). Test:
   `DAY2_field_menu_input_4ae1c`.

2. **The sub-page tree is native.** Constructors `func_8004AF3C` (Items, case
   0), `func_8004B03C` (Escape, case 1), `func_8004B13C` (Equipment, case 2 —
   two cross-linked lists 0x2E/0x31) and `func_8004B584` (modal, case 3) live in
   `pc_port/game/boot/`. Input handlers `func_8004AFA4` (Items confirm →
   `func_80052790` writes `D_8009D020`) and `func_8004B0A4` (Escape confirm →
   `func_800649D0`). List-draw wrappers `func_8004FF58` / `func_8004FF80` /
   `func_8004B534` / `func_8004B55C` are hand adapters: a bare `func_*` used as
   a value is a guest code address, so `gen_decomp_ports.py --check` reports
   "indirect call through local" (the same E5/E6 class as `func_8004FF30`).
   Per-cell draws `func_80050C70` (Items) and `func_80050CB4` (Escape) are
   native and dispatched from `menu_draw_callback`.

3. **Named residuals, not silent skips.** Everything in the tree that is not
   translated is an explicit named boundary arm: input
   `PE_MenuInputCallback_8004B214` / `_8004B394` / `_8004B650`, draws
   `PE_MenuDrawCallback_8004B5DC` and `PE_MenuDrawCell_80050438`, and the
   case-4/5 close page `PE_MenuInputClose` (needs `func_8005D994`, whose own
   callee `func_8005247C` is absent).

4. **Route evidence — the boundary moved twice, then closed.**
   - pre-leaf: `frames=61623 stop=unresolved-boundary story=0x09 persist1=0x0A
     token=0xA8000148`, 45/57, `PE_MenuInputCallback x1` (raw `0x8004AE1C`).
   - after 1–2 but before `0x80050C70` was wired into `menu_draw_callback`:
     still `frames=61623 stop=unresolved-boundary`, but now
     `PE_MenuDrawCallback x1` with `[MENU] Unported drawing callback 80050C70` —
     the handler ran natively and the render stopped exactly one level deeper.
   - final plain run (`route_menu2.log`): `frames=62000 stop=frame-limit
     story=0x00000009 persist1=0x0000000A token=0xA8000148`, **45/57**,
     boundary stubs = the four documented HOST_ADAPTED movie/menu skips only
     (**no unresolved boundary**).
   - pilot re-run (`route_menu_pilot.log`): `frames=62000 stop=frame-limit
     story=0x00000068 persist1=0x00000020 token=0xA80031C8`, 55/57, all four
     HOST_ADAPTED — identical to the pre-leaf pilot; only miss is milestone 55
     "normal item use restores 45 HP".
   The route harness gained an env-gated `PE_ROUTE_FRAME_TRACE=1` per-10-frame
   stderr trace (off by default) so a slow frame-limited run can be told apart
   from a hang: the 62000-frame run is ~100 frames/s (~11 minutes).

5. **Two coverage numbers (unchanged, must not drop).**
   `python3 tools/analysis/route_coverage.py --quiet --no-history`:
   `disc=1 funcs=372/979 c_words=6734 … native_c=258/979 native_c_words=27622`
   (`plan=5aceb911…`). The whole tree is the indirect-callback subtree that the
   979-function direct-call union excludes (`ROUTE_COVERAGE.md` §Metric scope),
   so translating it does not move `native_c` — the honest outcome, not a metric
   to fudge. `gen_decomp_ports.py --verify`: 269 TUs, 0 drift.

6. **No regression / hygiene.** Full native suite 1404 run / 1404 passed /
   0 skipped (`/tmp/pe-2nd/native_full2.log`), including all three menu tests,
   `DAY2_cd_sector_device` and `DAY2_cd_dma`. `git diff --check` clean; no game
   data added (no ISO/BIN/CUE/CHD/`SLUS_*`); no interpreter. Next holes, in
   order: milestone 55 normal item use (the pilot's only miss — the Items confirm
   `func_8004AFA4` is native, so the gap is downstream of it), the
   equipment/modal sub-pages (`0x8004B214`/`B394`/`B650`/`B5DC`/`0x80050438`),
   then the close page `func_8005D994` + `func_8005247C`.

---

## PRIOR 2026-09-18: shared CD data-ready dispatcher + field-menu page 4AD9C

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter.

1. **CD/stream gap closed: the command-poll event dispatch now reaches the
   native data-ready leaves.** `func_8007B010` (B178 loop) and
   `func_8007B558` (B820 loop) consumed `func_8007AAB4` events and raised an
   indirection boundary (`func_8007B010_afb8_callback`,
   `func_8007B558_afb8_callback`, `..._afb4_...`) for callbacks whose
   translations already exist. They now share one address-keyed dispatcher,
   `PE_Cd_DispatchDataCallback` (`pc_port/game/boot/cd_stream_port.c`,
   declared in `pc_port/platform/pe_sdk.h`), with the same target set
   `func_8007C13C` already used (7F960 / 7E964 / 80164 / 80778 / 7F88C /
   813E8). Consequence: a blocking command wait (`func_80080D5C` →
   `func_80080DC4`) that has a pending data-ready event now runs the real
   chain (80778 → 7F88C → 813E8 → `func_8007C564`) instead of stopping.
   Tests: `DAY2_movie_player` (see 2) and the unchanged
   `DAY2_cd_stream`/`DAY2_cd_dispatch`/`DAY2_cd_queue_recovery` oracles.

2. **Movie player 121C04: the enabled-device stream now really delivers.**
   `DAY2_movie_player` had one enabled-device arm that stopped because the
   fixture's searched file carried no Form-1 video chunk (the assembler
   dropped every record), and the docs called that stop a device frontier.
   The test now has three arms: (a) disabled device → recorded
   `movie_player_search_wait`; (b) enabled device + non-video fixture → the
   assembled record is really DMA3'd into the pool (`D_800A34A0==0x80150000`)
   and then dropped by the stream-start filter (`D_800B6918` != record frame),
   so the one and only stub is `movie_retry_wait` — no fabricated slice; (c)
   **enabled device + a real Form-1 chunk whose frame word equals record+6 →
   the physical stream delivers, 7C214 promotes the record and the first
   frame is handed off** (`D_800A3494==0x1234`, `D_800B0DBA==4`,
   `D_800B0DBC==1`, no stop). Arm (c) is the frontier DAY2_MOVIE_UPDATER.md
   named. `MOVPLY_SeedRecord` now documents that record+6/record+8 are the
   stream-start/frame-limit words `func_8007C304` publishes into
   `D_800B6918`/`D_800C0DBC`. **The same frontier is closed for the
   updater**: `DAY2_movie_updater` gained the enabled-device
   readiness/Setloc retry completion case — no ready record forces
   `func_80122040`'s acquisition timeout, 7C2A0 repositions to LBA 1, the
   real 80D5C → 80DC4 queue issues Setloc *with* the stack response buffer
   (`D_80122414==0x11010200`), 81314 opens the stream, the physical stream
   delivers and 121270 hands the frame off (`D_800A3494==0x1234`, bank
   flipped, `D_800B0DBC` 65535→0, no stub). The fixture helpers
   `CdStreamSeedRegisters` / `CdStreamWriteVideoSector` live in
   test_cd_device.h and are shared by both groups.

3. **Executed-path native leaf: `func_8004AD9C`** (32 words,
   asm/disc1/37CD0.s) is now
   `pc_port/game/boot/func_8004AD9C_port.c` and is called by the command-5
   arm of `func_80043DA4` (original call site asm/disc1/340EC.s:487, `jal`
   with `a0 = list`). It really constructs its window/list through the native
   0x80062D2C / 0x8006322C / 0x80062CB8 / 0x800647D0 and stores the original
   guest callback identities (window+0x2C = `0x8004AE1C`, list+0x30 =
   `0x8004FF30`). Test: `DAY2_field_menu_page_4ad9c`. **Route evidence:** the
   plain route now reports `PE_MenuDrawCallback x1` as its only boundary stub
   where it used to report `func_8004AD9C x1`, at the same frame (61593) and
   the same 45/57 milestones — the boundary moved one level deeper instead of
   the route stalling on the page constructor. **Named residual:** the list
   callback 0x8004FF30 (now `PE_MenuDrawCallback`), then the 0x8004AE1C input
   handler (72 words) and its six sub-pages (still only names in
   `func_80063E0C_port.c`'s `menu_callback` default arm,
   `PE_MenuInputCallback`).

4. **Executed-path native leaf: `func_8004FF30`** (10 words,
   `[0x8004FF30,0x8004FF58)`, matched C leaf `src/func_8004FF30.c`) is now
   `pc_port/game/boot/func_8004FF30_port.c`: a hand adapter (bare `func_*`
   used as a value is a guest code address, so `gen_decomp_ports.py` skips
   the leaf, rules E5/E6 — same class as `func_8004AD9C`) that forwards
   `slot` to `func_800638D8(slot, 0x80050C50)`. Wired into
   `func_800638D8_port.c`'s `menu_draw_callback` (`0x8004FF30`, plus the
   already-ported per-cell draw `0x80050C50`). Test:
   `DAY2_field_menu_draw_4ff30` — direct wrapper entry, equivalence with
   `func_800638D8(list, 0x80050C50)` (identical `D_8009D164/168` + packet
   pool), and the route's `func_80062830(list)` dispatch via slot `+0x30`;
   the list is parked at zero visible/total rows so no per-cell text draw
   (which would need the PE.IMG archive) is invoked. **Route evidence:**
   the plain route's only boundary stub is now `BOOTSTRAP_RET
   PE_MenuInputCallback x1` at `frames=61623` (was `PE_MenuDrawCallback x1`
   at 61593) with the same story/persist/token (`0x09`/`0x0A`/`0xA8000148`,
   m0002i) and the same 45/57 milestones — the boundary moved exactly one
   level deeper, onto the 0x8004AE1C input handler. Log
   `route_4ff30.log`. **Named residual:** the 0x8004AE1C input handler tree
   (72 words + six sub-pages, still only names in the `menu_callback`
   default arm).

4. **Two coverage numbers (unchanged, must not drop).**
   `python3 tools/analysis/route_coverage.py --quiet --no-history`:
   `disc=1 funcs=372/979 c_words=6734 … native_c=258/979 native_c_words=27622`.
   Honest caveat found while picking leaf 3: the `--top` "remaining asm"
   list and `native_c` are *filename*-based
   (`pc_port/game/{boot,decomp}/func_*_port.c`), and the Tier-A closure walks
   direct calls only. Most `--top` entries already have native definitions in
   differently-named TUs (`func_80079FB4` in `func_8001A15C_port.c`,
   `func_8003708C` in `func_80012850_port.c`, `func_80073A44` VSync,
   `func_80080B44` as the static `PE_Cd_IntToPos`), and the indirect
   field-menu subtree (including `func_8004AD9C`) is outside the 979-function
   union, so leaf 3 does not move `native_c`. The number is a lower bound; do
   not "fix" it to move the metric without deciding that explicitly.

5. **Disc-1 baseline (recorded, not invented).** `{SCRATCH}=/tmp/pe-4h`; the
   final runs from the frozen tree are `*_final.log` and reproduce the earlier
   ones exactly (deterministic):
   - plain `pc_port/build/pe-route-boot-day2-tests` (`route_baseline_final.log`;
     `route_baseline.log` is the pre-leaf run):
     `frames=61593 stop=unresolved-boundary story=0x00000009
     persist1=0x0000000A token=0xA8000148` (m0002i), 45/57 milestones.
      Before leaf 3 the only boundary stub was `BOOTSTRAP_RET func_8004AD9C
      x1`; after leaf 3 it is `BOOTSTRAP_RET PE_MenuDrawCallback x1` at the
      same frame, token and milestone count. After leaf 4 (`func_8004FF30`,
      `route_4ff30.log`) it is `BOOTSTRAP_RET PE_MenuInputCallback x1` at
      `frames=61623` — 30 frames further, same story/persist/token
      (`0x09`/`0x0A`/`0xA8000148`, m0002i) and same 45/57. The plain
      invocation has never reached m0027i; it needs the pilot below.
   - `PE_ROUTE_REWARD_PILOT=1 pc_port/build/pe-route-boot-day2-tests`
     (`route_baseline_pilot_final.log`; `route_baseline_pilot.log` is the
     earlier run): `frames=62000 stop=frame-limit story=0x00000068
     persist1=0x00000020 token=0xA80031C8`, 55/57, **all four boundary stubs
     HOST_ADAPTED (no unresolved boundary)**; missing milestone 55 “normal
     item use restores 45 HP”. This run reaches the documented pin (token
     `0xA80023C8` m0027i is in the pass set) with `persist[74]=0x68` and both
     sewer victories.
   The baseline doc's command line omitted `PE_ROUTE_REWARD_PILOT=1`; it is
   corrected there. Harness source still rejects `PE_Disc_BootKind==2`
   (test_route_boot_day2.c:588), and the in-memory Disc-2 identity fixture
   (`test_disc_findfile_disc2_idf`, `\FMV2\PEDISC02.IDF;1` → LBA 41) still
   passes in the full suite, so Disc 2 is not being passed off as Disc 1.

6. **No regression:** full native suite 1403 run / 1403 passed / 0 skipped
   (`/tmp/pe-4h/native_full4.log`), including `DAY2_cd_sector_device` and
   `DAY2_cd_dma` (`cd_sector.log`, `cd_dma.log`), `DAY2_movie_player`,
   `DAY2_movie_updater`, `DAY2_field_menu_page_4ad9c` and the new
   `DAY2_field_menu_draw_4ff30`. Pilot re-run after leaf 4
   (`route_4ff30_pilot.log`): `frames=62000 stop=frame-limit
   story=0x00000068 persist1=0x00000020 token=0xA80031C8`, 55/57, all four
   boundary stubs HOST_ADAPTED — identical to the pre-leaf pilot.
   `git diff --check` clean; no game data added (no ISO/BIN/CUE/CHD/SLUS in
   the tree); no interpreter. Next holes, in order: the 0x8004AE1C
   field-menu input handler tree (72 words + six sub-pages,
   `PE_MenuInputCallback`), then the remaining disc-1 route milestones
   (milestone 55 item use).

---

## PRIOR 2026-09-18: port-first — CD B0CD0 catchup, two coverage numbers, two-disc, HD-2D architecture

The native PC port is the deliverable; matching decomp is the means. Recovered
assembly is worth zero to the port. No MIPS interpreter.

1. **DAY2-158 B0CD0 catchup-miss (CD driver).** `func_8007C564` defers when
   MDEC output DMA is busy: it writes `D_800B0CD0` and returns without BFRD.
   The next ReadN/ReadS cadence used to `CD_device_sector_overrun` the unread
   sector. `PE_CdReg_ServiceDevice` now **holds** that pending sector while
   the halfword is set, and when output DMA is idle **retries 7C564** so the
   held sector is BFRD'd. A clear unread sector still overruns.
   Tests: `DAY2_cd_sector_device` (hold + overrun), `DAY2_cd_dma` (idle pump
   assembles). Docs: `docs/ai_context/DAY2_CD_SECTOR_DEVICE.md`.
   Disc-1 gameplay baseline: `docs/generated/DISC1_GAMEPLAY_BASELINE.md`
   (`pe-route-boot-day2-tests` → m0027i). The harness rejects a Disc 2 image.

2. **Two coverage numbers.** `python3 tools/analysis/route_coverage.py --quiet`
   now emits both:
   - `funcs=` / `c_words=` — matching YAML `c` on the boot→Day-2 route (rebuild)
   - `native_c=` / `native_c_words=` — executed-path **native** C
     (`pc_port/game/{boot,decomp}/func_*_port.c`). That second number is what
     predicts a playable restylable build.
   Live: `disc=1 funcs=372/979 c_words=6734 … native_c=258/979 native_c_words=27622`.
   `disc=2` is reserved until a disc-2 route exists.

3. **Two-disc.** `PE_Disc_BootKind` / `PE_Disc_VolumeId` identify
   SLUS_006.62 vs SLUS_006.68 from the ISO. Boot EXE load already follows
   `SYSTEM.CNF`. The EXEs are byte-identical; `PE.IMG` and FMV tracks
   diverge. `PE_Disc_SetActive` is the swap point. No game data in git.

4. **Rendering (verified).** Prerendered TIM plates + real-time characters
   in an ordering table (no depth buffer). Evidence:
   `func_800718D0` TIM walker, DrawOTag/`func_80076C34` DMA2.
   `docs/ai_context/RENDERING_ARCHITECTURE.md`. HD-2D = upscale plates +
   better models, not re-render whole scenes.

Matching-C parks for `func_8006AD40` / `func_80069B08` stay asm (compiler
residuals). Do not treat `pc_port/` as matching C.

---

## PRIOR 2026-09-18: M34 = tail weak point + rotated-camera aim; supply52 live

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### M34 solved structurally: break the type-4 tail, aim with a transposed pad

47 supply runs kited the *head*; **not one ever damaged anything**, so M34
could never end. Two independent root causes are now identified and fixed.

**(1) The objective is the type-4 tail, not the head.** Original M0034I script
(`PE.IMG` LBA 16597, base `0x8018EFE8`, script blob chunk `0x25B18` / VA
`0x801B4B00`, 6 modules / 1341 commands; branch base = module start,
`target = module_base + imm*2`):

- `0x801B5A2C..0x801B5B2C` reads tag `0x2C` (= slot `+0x10` = HP,
  `func_80030220` case 44), **adds `0xF4240` (1,000,000)** and writes tags
  `0x2C`/`0x2D`/`0x3C` — that is the “million-point offset”; the real resource
  HP is 120 (head) / 80 (tail).
- `0x801B8718` `op 09 sub 9` = `(1000000 < HP)`; `0x801B8730 op 05` loops until
  **HP ≤ 1,000,000**, then `op 0xCE` zeroes tags `0x2C`/`0x2D`, spawns
  sub-script `0x19A0`, plays the outro (second `op 0x89` @`0x801B8898`,
  `op 0x95/0x96`, wait mode `0xC`) and `op 0x31` exits.
- The HP read is `op 0x8B` @`0x801B8700` with **type = 4**
  (`func_80018080` walks `D20C` for `actor+0x0C == 4`, `actor+0x0D == 0`).
- Damage (`func_80028574`) from the live capture
  (`pe-m34-survival-connected.bin`): aya attack `rec+0x1E`=45, pistol atk 12,
  `wf&15`=2 → `power = (45/5+12)*scales[2]/100 = 12`,
  `defense = body+0x8C / shots`:

| actor | type | HP | `+0x8C` def | dmg/hit | hits to 1,000,000 |
| --- | --- | ---: | ---: | ---: | ---: |
| head | 3 | 1,000,120 | 200 | **1** | 120 (impossible, 40 rounds) |
| tail | 4 | 1,000,080 | 6 | **9** | **9** (≈5 x2-shot attacks) |
- Hit gate `func_80021278`: any `distance>1800` is a hard miss, where
  `distance = world*1000/range` and `range = weapon+2 = 1071`. So the tail is
  **unhittable beyond world ≈1928** and reliable within ≈1071. The west lane
  (x≈-2600, where Aya spawns) is the only firing position.

**(2) The M34 arena camera is rotated, and the pilot's aim silently inverted.**
Live pads proved `Left`→world south `(0,-1)` and `Down`→world east `(1,0)`,
i.e. the engine applies the very matrix at `0x800BD000/4` to the pad
(`M=[[0,-1],[1,0]]`, `M*M=-I`). `RoutePilotAimPad` used the untransposed form,
so every aim came out **exactly reversed** here (that is why the hunt still
drifted east and why 47 runs “fled” into the boss). The transposed form
(`RoutePilotAimPadT`) fixed it: supply50 drove Aya to the west wall with
`t2=1,055,546` (inside the 1071 reliable band) — the first time any run was in
firing position. Because the effective convention drifts with the action, the
hunt now flips transposed/normal whenever 90 frames pass without progress
toward the lane (`g_m34_aim_t`).

### Pilot state (`pc_port/tests/route_reward_sewer_pilot.h`)

- `tail_hunt` = tail alive & `tailhp>1,000,000` & no heal/equip pending → camp
  `lane_x≈tx+750`, Z works toward the tail's Z (unpins the north wall) and
  dodges the head when `h2<600k`; transposed/normal aim is adaptive.
- `attack_tail` = tail alive, pistol loaded, no heal pending, `t2≤2.5M`,
  `h2≥250k`. Mode 1 **commits the ATB on the type-4 target** instead of
  Circle-cancelling, and (supply52 fix) will not Cross until `target==selected`
  — supply51 proved the old `!can_heal` shortcut committed with `selected=0`
  and shot the armored **head** (headhp 1000120→1000118, 2 rounds wasted).
- Stuck-break latch is suppressed while hunting (it was cycling raw cardinal
  pads and overriding the hunt aim).
- Diagnostics: `M34_TAIL_HP`, `M34_TAIL_HUNT`, `M34_TAIL_ATK`,
  `M34_TAIL_BROKEN`, plus `taildmg=`/`headhp=` on the 30f `M34_PILOT` line.

### Story path

- Supply45 (**killed**, `/tmp/pe-m34-pilot-supply45.log`): latch-suspend fix
  changed nothing — 4 heals, died ~65310, boss HP untouched.
- Supply46 (no lane logic) → `t2` never fell below 6.6M, died.
- Supply47 (lane camp, untransposed) → drifted east, `t2` 5.6M, died.
- Supply48/49 (lane camp + stuck-break suppressed) → still east, died.
- Supply50 (transposed aim) → **reached the west wall**, `t2=1,055,546`, died
  pinned at `(-2839,780)` because `hp>24` blocked every mode-1 commit and the
  Z-dodge pointed into the wall.
- Supply51 (hp gate removed, adaptive aim, Z unpin) → the ATB did commit but
  hit the **head** twice (see above), died.
- Supply52 PID **1554290** → `/tmp/pe-m34-pilot-supply52.log`
  (`PE_ROUTE_REWARD_PILOT=1`, `PE_ROUTE_FRAMES=100000`). Live. Not at `0x80`.
  Success signal: `M34_TAIL_HP … dmg=` rising → `M34_TAIL_BROKEN` → script
  retires the pair → `M0359I`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-17: M34 weak point found — the type-4 tail, not the head; supply46 live

Matching-C lane (bootstrap / main loop). `func_8003F3C4` (field-tick, 229
words, `0x8003F3C4` / `0x394`) is now a YAML `c` span, `LINK_EXACT` at the
retail VMA. Profile `era_o2_g8_force_3f3c4_absolute`. Evidence
`docs/evidence/func-8003F3C4/REPORT.md`. Command:

```
MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009CDD8,D_8009D1F4,D_8009D238,D_8009CDDC,D_8009D250,D_8009D26C,D_800B0CEA,D_800B0CD8,D_800BCFE8 \
  tools/analysis/check_leaf.sh func_8003F3C4 0x8003F3C4 0x394 -O2 -G8
```

`disc1_plan.py --check` → `1147 spans (797 c, 348 asm, 2 rodata)` (includes
this leaf plus sibling carves). Deep `--only func_8003F3C4` PASS. Full
`disc1_preflight.py --deep` PASS. `scripts/exact_rebuild.sh --preflight-fast`
build `EXACT_MATCH`; `scripts/verify_us.sh` `VERIFY_US=PASS`; packed SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

### Boot-spine status (direct `jal` from `func_8001220C` + initial PC)

Matched C: `func_8003E610`, `func_8006A5BC`, `func_8006A64C`,
`func_8003E680`, `func_8006ECEC`, `func_8006F044`, `func_8006E834`,
`func_8006E9A0`, `func_80074D28`, **`func_8003F3C4`**.

Accepted-residual / PARK (not counted as C): `func_8001220C` (prologue
save-batching), `func_800725DC` (per-TU skew), `func_800698D4` (dbr
liveness), `func_80073A44` (prologue-schedule), `func_8006A9E4` (dbr-sched
+ copy-loop encoding; PARK `docs/evidence/func-8006A9E4/PARK.md`,
still `in-progress-checkpoint` not the 24-leaf ACCEPTED-RESIDUAL list),
`func_80072534` (handwritten crt0; `docs/evidence/func-80072534/PARK.md`).

`func_80069B08` (376w) has a size-exact draft (`src/func_80069B08.c`,
1504 bytes / `0x5E0`, frame `0x58`, CD-read loops match) but is **not**
YAML `c`: prologue save-batching plus jumptable `jtbl_80011388` 3-word
`$at` form vs patch-5 shrinking other indexed loads. PARK
`docs/evidence/func-80069B08/PARK.md`, disposition `in-progress-checkpoint`.

`func_8006AD40` draft is **frame-exact 0x30** (inlined CLUT loops, no `$s7`).
The `0x80` `bnez` delay now carries `addu $v0,$zero,$zero` (test stays in
`$v0`); work2 skip delay, both CLUT packs, walks, and work4 `jal` delays
match. Residual is **exactly one extra join `move $2,$0` before the
epilogue** (1568 vs 1564, 12 word mismatches, all the epilogue shifted by
one). cc1 2.7.2 will not credit a not-taken-arm delay fill to the taken
arm, so `return 0` stays at the join. PARK
`docs/evidence/func-8006AD40/PARK.md`. Not YAML `c`. Overlay jals
`func_8019234C` / `func_801235DC` / `func_801909B4` are overlay-only.

### Next subsystem after bootstrap/main loop

Disc-1 spine matchable callees are closed or parked. `func_80069B08`
needs a per-symbol `MASPSX_SYMBOL_AT_TEMP` (or a prologue-interleave
lever) before it can be `c`. Next subsystem: field VM (`func_80017018` +
`D_800910A0` handlers).

Do not treat `pc_port/` translations as matching C.

---

## PRIOR 2026-09-17: M34 weak point found — the type-4 tail, not the head; supply46 live

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### M34 is winnable: break the type-4 tail (original script, not a guess)

44 straight supply runs kited the *head* and healed; **none ever tried to
damage the tail, and none ever won** — because the pilot's M34 branch was
survival-only (`shoot_floor` 2.2M/3.2M never held; the ATB was
Circle-cancelled, never committed to an attack).

Reading the **original M0034I script** (`PE.IMG` LBA 16597, base
`0x8018EFE8`, script blob at chunk `0x25B18` / VA `0x801B4B00`, 6 modules
1341 commands; module-relative branch base = module start, `target =
module_base + imm*2`) settles it:

- `0x801B5A2C..0x801B5B2C` (module 3 setup) reads tag `0x2C` (= slot `+0x10`
  = HP, `func_80030220` case 44), **adds `0xF4240` (1,000,000)**, and writes
  it to tags `0x2C`/`0x2D`/`0x3C`. That is the observed “million-point
  offset” — the real resource HP is 120 (head) / 80 (tail).
- `0x801B8718` `op 09 sub 9` = `(1000000 < HP)`; `0x801B8730 op 05` loops
  while true and only falls through to `0x801B8758` when **HP ≤ 1,000,000**.
  `0x801B876C/0x801B8784` (`op 0xCE`) then zero tags `0x2C`/`0x2D` (retire),
  spawn sub-script `0x19A0`, and continue to the second `op 0x89`
  (`0x801B8898`), outro `op 0x95/0x96`, wait for mode `0xC`, then
  `op 0x31` → exit.
- The HP read is `op 0x8B` at `0x801B8700` with **type = 4**
  (`func_80018080`: walk `D20C` for `actor+0x0C == 4` and `actor+0x0D == 0`,
  read tag `0x2C`). So the script polls the **type-4 tail's** HP.

Damage (`func_80028574`) with the live capture (`pe-m34-survival-connected.bin`):
aya attack `rec+0x1E`=45, pistol atk 12, `wf&15`=2 shots →
`power = (45/5 + 12)*scales[2]/100 = 12`; `defense = body+0x8C / shots`.

| actor | type | HP | tag `+0x8C` def | dmg/hit | hits to reach 1,000,000 |
| --- | --- | ---: | ---: | ---: | ---: |
| head | 3 | 1,000,120 | 200 | **1** | 120 (impossible with 40 rounds) |
| tail | 4 | 1,000,080 | 6 | **9** | **9** (≈5 attack commands) |

So the alligator is a **weak-point boss**: shoot the stationary tail ~9
times and the script retires the pair and transfers to `M0359I`. This is the
first concrete, finite M34 objective found; the prior “kite forever” framing
was unwinnable by construction.

### Pilot fix (`pc_port/tests/route_reward_sewer_pilot.h`)

- `attack_tail` = tail alive (`body+0x10 > 1,000,000`), pistol loaded,
  `heal_start/equip_start < 0`, `hp > 24`, `t2 ≤ 1.4M`, `h2 ≥ 400k`.
- mode 1 (ATB full): `cancel_atb && !attack_tail` — the ATB is now
  **committed on the type-4 target** (existing target scan already picks
  type 4; Cross at `%16==3` and `%16==11`) instead of Circle-cancelled.
- mode 0: fires/reloads on the tail when `t2 ≤ 900k` and `h2 ≥ 450k`.
- Diagnostics: `M34_TAIL_HP`, `M34_TAIL_ATK`, `M34_TAIL_BROKEN`, and
  `taildmg=`/`headhp=` on the 30f `M34_PILOT` line.

### Story path

- Supply45 (**ended/killed**, `/tmp/pe-m34-pilot-supply45.log`): the
  latch-suspend fix changed nothing — 4 heals landed, hp36@64980, died
  ~65310 (hp13) → `A8001048`@66000 → `A8000148`. Boss HP untouched. Never
  `0x80`. Confirms the structural (not micro-tuning) diagnosis above.
- Supply46 PID **1507185** → `/tmp/pe-m34-pilot-supply46.log`
  (`PE_ROUTE_REWARD_PILOT=1`, `PE_ROUTE_FRAMES=100000`), launched from the
  repo root (needs `local/pe_disc1.path`). Live. Not at `0x80`.
  Success signal: `M34_TAIL_HP … dmg=` rising → `M34_TAIL_BROKEN` → the
  second `op 0x89` / outro / `M0359I`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-17: latch survives ATB flicker; supply45 (4 heals, died 65310)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply44 (**ended**, `/tmp/pe-m34-pilot-supply44.log`): best run yet —
  **4 heals landed** (8→38@63000; 24→54@63810; 14→44@64230; 6→36@64980),
  first run still in `A8003248`@65000. Died ~65310 (hp13, h2≈139k) →
  soft-fail `A8001048`@66000. Never `0x80`.
- Root: latched stuck-break disarmed within ~15f both times (65097→65115,
  65216→~65230) with no mode print, no heal, no band exit, no 300u escape.
  Cause: ATB-full `mode=1` flicker between 30f prints — the old code reset
  the latch on ANY `mode!=0`/heal frame, while the Circle-cancel itself
  worked. Aya then vibrated pinned (FF3F/FFCF) while the head closed.
- Fix in `route_reward_sewer_pilot.h`: latch now **suspends, not resets** —
  only band exit or 300u net escape clears `w_break`; `mode/heal` frames
  only pause the override and the window (`w_t0=0`). ATB-cancel and heal
  nav untouched (heal/equip blocks overwrite mask downstream anyway).
  Rebuilt `pe-route-boot-day2-tests`.
- Supply45 PID **1494113** → `/tmp/pe-m34-pilot-supply45.log`
  (`PE_ROUTE_REWARD_PILOT=1`, `PE_ROUTE_FRAMES=100000`). Live. Not at `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-17: pocket-margin + windowed stuck-break; supply44 (4 heals, died 65310)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply43 (**ended**, `/tmp/pe-m34-pilot-supply43.log`): heals 1–3 all
  landed (hp8→38@63030; hp7→37@63360; hp18→48@64020). Then 31-dmg hit
  hp48→17@64050 at h2≈1.9M (ranged), PE starved (pe=7/13), soft-fail
  `A8001048`@65000 → `A8000148`. Never `0x80`.
- Root (new, precise): 200f+ pin @(-796,-399) 63450–63660, pads flickering
  FF3F/FFCF every frame, head closing h2 1.75M→303k, hit hp37→18@63690.
  Two defects: (1) `az=-399` sat **1u above the old ±400 Z-pocket trigger**,
  so the east-dominant break never armed and want stayed (1800,-2400) into
  the east wall; (2) the anchor-reset stuck detector never fired — shove/
  slide ≥25u kept resetting `stuck_n` (no `M34_STUCK_BREAK` 63391–63970).
- Fix in `route_reward_sewer_pilot.h`: all Z-pocket triggers ±400 → **±350**
  (matches stuck band + `z_clear` + ATB-cancel; removes the 350–400 deadband
  where Aya neither breaks center nor shoots); stuck detector is now a
  **windowed net-displacement** latch (50f window, <120u → latched cardinal
  cycle until 300u net escape; oscillation cannot reset it). Rebuilt
  `pe-route-boot-day2-tests`.
- Supply44 PID **1474936** → `/tmp/pe-m34-pilot-supply44.log` — **4 heals**,
  alive in `A8003248`@65000, died ~65310 (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.62: stuck pad-cycle + hug heal floor; supply43 live

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply42 (**340199**) reached **`A8003248`**@63000; early kite cleared
  north→south better than supply41; heals to hp54 / hp39. Still died pinned
  @(-749,+412) pad=FFBF → `A8000148` mode=-1@66000. Never `0x80`.
  Killed **340199**.
- Fix: when immobile >40f in wall/Z-pocket bands, cycle raw R/U/L/D pads
  (`M34_STUCK_BREAK`); critical heal hug floor `hp≤8` now `h2≥20k` (was
  50k — pe=59 starved at h2=24k). Rebuilt `pe-route-boot-day2-tests`.
- Supply43 PID **351295** → `/tmp/pe-m34-pilot-supply43.log`
  (`PE_ROUTE_REWARD_PILOT=1`, `PE_ROUTE_FRAMES=100000`). Live. Not at `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.61: east-wall Z-pocket west+center; supply42 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply41 (**331404**) reached **`A8003248`**@63000; heals 8→38@63000 and
  7→37@63360. Soft-fail `A8001048`@64000. Never `0x80`. Killed **331404**.
- Root: east-dominant Z-pocket (`want_x=2400`) cleared west→east to
  **x≈-747**, then camera-mapped pad=FFBF pinned against the east wall in
  the north pocket @(-747,+735) for ~400f while the head closed.
- Fix: when `ax>=-1000` and `|az|≥400`, push **west + strong center Z**
  (`want_x=-1200`, `want_z=±2400`) instead of further east. Rebuilt
  `pe-route-boot-day2-tests`.
- Supply42 PID **340199** → `/tmp/pe-m34-pilot-supply42.log` — longer
  survival then east-wall FFBF pin (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.60: east-dominant Z-pocket + !can_heal shoot; supply41 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply40 (**321169**) re-proved path: m31 heal → `A8003148` →
  **`A8003248`**@63000. Heals landed (8→38@63000; 7→37@63600;
  18→48@64050). Soft-fail `A8001048`@65000. Never `0x80`. Killed **321169**.
- Root: Z center-break used ±2400 with X capped to 800 → AimPad Z-wins and
  froze on the `|az|≈400` band (south @(-765,-411) then north
  @(-746,+400) pad=FF9F). After PE spent, `flee_floor=3.2M` also blocked
  all Cross → heal→pocket-oscillate→chip until pe<60.
- Fix in `route_reward_sewer_pilot.h`: Z-pocket sites are east-dominant
  (`want_x=2400`, `want_z=±600`); don't dig deeper into `|az|≥400`;
  `!can_heal` shoot floor lowered to 1.2M with Z-clear gate. Rebuilt
  `pe-route-boot-day2-tests`.
- Supply41 PID **331404** → `/tmp/pe-m34-pilot-supply41.log` — east-wall
  north-pocket pin (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.59: west-wall skips Z center-break; supply40 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply39 (**312075**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Heal1 landed
  (8→38@63000; `HEAL_STARVE` is item4=0 noise — PE meter heal). Soft-fail
  `A8001048`@64000 → `A8000148`. Never `0x80`. Killed **312075**.
- Root: ungated Z center-break at west wall (`ax<=-2100`) capped
  `want_x` 2000→800; AimPad(800,-2400)→pad=FF7F Left. Supply38's
  `want_x=2000` cleared west to x≈-930; supply39 hugged @(-2176,460)
  pad=FF3F h2≈63k and soft-failed before heal2.
- Fix in `route_reward_sewer_pilot.h`: all Z center-break sites now also
  require `ax>-2100` (west corridor keeps pure-east / uncapped X).
  Rebuilt `pe-route-boot-day2-tests`.
- Supply40 PID **321169** → `/tmp/pe-m34-pilot-supply40.log` —
  Z-pocket oscillation after heals (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.58: Z-wall center-break ungated; supply39 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply38 (**306693**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Heals 1–4 landed
  (8→38@63000; 7→37@63600; 9→39@64050; 18→48@64860). Soft-fail
  `A8001048`@66000 → `A8000148`. Never `0x80`. Killed **306693**.
- Root: east-wall `want_z=(az>=hz)?2400` dug further north to
  **(-735,771)**; polarity (`hz>=az`) blocked center-break because head
  was south, and `h2>=1M` never armed (freeze at h2≈984k→434k) —
  pad=FF9F freeze while head closed.
- Fix in `route_reward_sewer_pilot.h`: any `|az|≥400` with `h2≥200k`
  forces center-break ±2400 (no polarity / 1M gate) in flee, east-wall
  re-apply, and corridor final pocket. Rebuilt
  `pe-route-boot-day2-tests`.
- Supply39 PID **312075** → `/tmp/pe-m34-pilot-supply39.log` —
  west-pocket hug after heal1 (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.57: flee-under-ATB-cancel + strong Z break; supply38 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply37 (**299297**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Heal1 hp8→38@63030.
  Heal2/3/4 all landed (7→37@63690; 9→39@64170; 18→48@64920). Soft-fail
  `A8001048`@66000 → `A8000148`. Never `0x80`. Killed **299297**.
- Root: (1) Circle-cancel pulsed `pad=0xDFFF` only — 7/8 frames `FFFF`
  froze mid-flee south @(-787,-763) then north @(-735,767) while head
  closed; (2) Z center-break ±600 lost to camera AimPad (east+north →
  pad Down) and dug az -411→-763.
- Fix in `route_reward_sewer_pilot.h`: (1) while canceling ATB, keep
  flee `AimPad` under Circle (`mask&=0xDFFF` on alternate frames);
  (2) Z-pocket center-break ±2400 with |X| capped so AimPad Z wins.
  Rebuilt `pe-route-boot-day2-tests`.
- Supply38 PID **306693** → `/tmp/pe-m34-pilot-supply38.log` —
  north-pocket freeze after heal4 (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.56: ATB cancel-while-flee + Z reclamp; supply37 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply36 (**287404**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Heal1 landed
  (hp8→38@63002). Heal2 **never landed**. Soft-fail
  `A8001048`@64000. Death @63379 hp=0 @(-793,-746). Never `0x80`.
  Killed **287404**.
- Root: (1) east-wall branch `ax≥-1100 && hx<ax` overwrote the ±400
  Z center-break with raw `z_away=-1200` @ax=-983 → dug south to
  az=-772; (2) AT naturally hit 9000 → mode=1@63160 while still
  below `flee_floor` (h2≈3.19M < 3.2M); Circle-cancel was gated on
  `h2≥500k` so keep-fire froze mid-flee → long-range hit hp38→17 →
  heal2 starved → death.
- Fix in `route_reward_sewer_pilot.h`: (1) Circle-cancel whenever
  `h2 < flee_floor` or `|az|≥400` (Z-pocket); keep-fire only past
  `flee_floor` and clear of Z walls; (2) re-apply Z center-break
  after the east-wall `want_z` overwrite. Rebuilt
  `pe-route-boot-day2-tests` via `/tmp/pe-tools` make.
- Supply37 PID **299297** → `/tmp/pe-m34-pilot-supply37.log` —
  Circle-only cancel froze N/S pockets despite heals 2–4 (see CURRENT).
  Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.55: no-shoot-while-flee + south pocket; supply36 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply35 (**275575**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. West-pin fix held
  (east kite after heal1; no FFCF west freeze). Both PE heals landed
  (heal1 hp8→38@63002; heal2 hp7→37@63526). Soft-fail
  `A8001048`@64000. Death @63749 hp=0. Never `0x80`. Killed **275575**.
- Root: (1) `!can_heal` Cross @h2≈2.8M opened ATB mode=1@63160 while
  still below `flee_floor` → froze @(-791,-598), long-range hit
  hp38→7; (2) east kite dug south to az≈-613 (old Z-clamp only at
  ±700 + head-north polarity blocked push-to-center) → south pocket
  @(-808,-613) through heal2 → chip hp37→16 → head close → death.
- Fix in `route_reward_sewer_pilot.h`: (1) never Cross/reload while
  `h2 < flee_floor`; (2) Z wall flip ±700→±400; when `h2≥1M` break
  Z-pocket toward arena center even if that closes slightly on head.
  Rebuilt `pe-route-boot-day2-tests`.
- Supply36 PID **287404** → `/tmp/pe-m34-pilot-supply36.log` —
  ATB freeze mid-flee + south pocket; heal2 starved (see CURRENT).
  Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.54: west-wall hug priority; supply35 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply34 (**259441**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Both PE heals
  landed again (heal1 hp8→38@63030; heal2 hp19→49@63420). Soft-fail
  `A8001048`@64000 → `A8000148`. Never `0x80`. Killed **259441**.
- Root: `h2<400k` both-axis away overrode west-wall east escape —
  @63420 ax=-2689 hx=-2073 h2=388k → `x_away=-1500` / `z_away=+1200`
  (NW into walls) pad=**FFCF** freeze @-2825,775 while head closed
  (h2→24k; hp49→26).
- Fix in `route_reward_sewer_pilot.h`: west-pinned `ax≤-2100` always
  keeps east (+ Z-away with north/south wall flip); hug floor only
  when not west-pinned. Rebuilt `pe-route-boot-day2-tests`.
- Supply35 PID **275575** → `/tmp/pe-m34-pilot-supply35.log` —
  south-pocket ATB freeze after both heals (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.53: east-wall polarity; supply34 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply33 (**247052**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. East-rotate fix
  held: heal1 **landed** (hp8→38@63030); east kite @x≈**-1093**
  h2≈3.0M. Heal2 **also landed** (hp10→40@63540). Soft-fail
  `A8001048`@64000 → `A8000148`. Never `0x80`. Killed **247052**.
- Root: unconditional east-wall bounce `ax≥-1100 → want_x=-400`
  reversed east flee while head was still west (hx≈-2162@63090;
  hx≈-1293 h2=167k after heal2) → pad Left into the charge
  (hp38→10@63360; heal2 chip hp40→21@63600).
- Fix in `route_reward_sewer_pilot.h`: (1) east-wall bounce only when
  `hx≥ax` (west is away); else keep east + Z-away; (2) both-axis hug
  flee widened `h2<150k` → `h2<400k`. Rebuilt
  `pe-route-boot-day2-tests`.
- Supply34 PID **259441** → `/tmp/pe-m34-pilot-supply34.log` —
  west-wall hug into NW corner after both heals (see CURRENT).
  Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.52: east-rotate + heal2 ATB; supply33 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply32 (**236248**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Heal1 landed
  (hp8→38@63030). East kite reached x≈**-1210** h2≈912k then pad
  **FF7F Left** walked west/south → hit hp38→17@63180. Heal2 **starved**
  on ATB: @63240 hp17 pe64 at=2652 h2=2.0M; @63360 hp13 at=8892
  h2=26k (108 short of gate 9000). Soft-fail `A8001048`@64000 →
  `A8000148`. Never `0x80`. No FFDF hug this run. Killed **236248**.
- Root: (1) tail-rotate skip was `ax≤-1500` only — at ax=-1210 /
  hx=-2162 / want_x=+952 the rotate flipped east→(-80,-952) and
  camera AimPad mapped it Left; (2) east bounce needed `h2<900k` so
  missed at h2=912k; (3) reduced ATB (`2500`) only for `hp≤12`, so
  chip-damage heal2 at hp13–17 waited for 9000 until head closed.
- Fix in `route_reward_sewer_pilot.h`: (1) never reverse east flee when
  `want_x>0 && rx≤0 && (hx<ax || ax≤-1100)`; (2) east bounce when
  `ax≤-1200 && hx<ax` (no h2 gate) else `h2<1.5M`; (3) `atb_need=2500`
  and closer h2 floor through `hp≤20`. Rebuilt
  `pe-route-boot-day2-tests`.
- Supply33 PID **247052** → `/tmp/pe-m34-pilot-supply33.log` —
  east-wall bounce into boss after both PE heals (see CURRENT).
  Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.51: Z-clamp polarity flip; supply32 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply31 (**220778**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Heal1 landed
  (hp8→38@63030). East kite held @x≈**-1520** h2≈2.2M until head
  charged. @63330 h2=817k pad=FF3F; @63360 **FFDF freeze**
  @-1361,-702 h2=20k (ax drifted past bounce `-1400`). Heal2 **armed
  and landed** (floor 100k worked: `LOOT_PILOT_HEAL`@63407 h2=116756 →
  hp10→40@63570) but stayed hugged (h2≈70–155k) → soft-fail
  `A8001048`@64000 → `A8000148`. Never `0x80`. Killed **220778**.
- Root: supply30 Z-clamp guard had **inverted polarity** —
  `az≤-700 && want_z<0 && hz≥az` flipped south-flee **north into** a
  northern head. Bounce also dropped once `ax>-1400`.
- Fix in `route_reward_sewer_pilot.h`: (1) Z-clamp polarity corrected
  (`hz≥az` for south push / `hz≤az` for north push) and skipped when
  `h2<200k`; (2) east bounce widened to `ax≤-1200`; (3) `h2<150k`
  forces both-axis away-from-head. Rebuilt `pe-route-boot-day2-tests`.
- Supply32 PID **236248** → `/tmp/pe-m34-pilot-supply32.log` —
  east-rotate + heal2 ATB starve (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.50: M34 east Z-clamp; supply31 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply30 (**192206**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Heal1 landed
  (hp8→38@63030). Tail-rotate skip **held** east kite @x≈**-1520**
  h2≈2.2M. Head charged; `az≤-700` Z-clamp flipped want_z south→north
  **into** head → pad **FFDF** freeze @-1520,-714; hp38→10. Heal2
  starved @63420: pe88 **at=9000** but h2=173770 < critical floor 250k
  and west-pin emergency needed `ax≤-2100`. Soft-fail `A8001048`@64000
  → `A8000148`. Never `0x80`. Process ended at frame-limit.
- Fix in `route_reward_sewer_pilot.h`: (1) arena Z clamps only when they
  do not reverse Z-away from head; (2) east-corridor bounce
  `ax≤-1400 && h2<900k` → strong east + Z-away; (3) critical heal
  `hp≤12` h2 floor **100k**; west/east emergency `ax≤-1400 &&
  h2≥100k`. Rebuilt `pe-route-boot-day2-tests`.
- Supply31 PID **220778** → `/tmp/pe-m34-pilot-supply31.log` —
  Z-clamp polarity + hug (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.49: M34 post-heal kite/ATB; supply30 (failed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply29 (**187674**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Heal1 **landed**
  (`LOOT_PILOT_HEAL`@62820 hp8→38@63030). Post-heal kite: east to
  x≈**-1668** then **tail-rotate** of flee flipped want east→west/south
  → back to x≈**-2116** z≈**-761**. Head slam hp38→7@63210. Second heal
  starved: @63240 hp7 pe64 h2=2.0M but **at=2652<9000**; by
  at=8892@63360 head closed (h2=42k, hp3). Soft-fail `A8001048`
  mode=`0xFFFFFFFF`@64000 → `A8000148`. Never `0x80`. Killed **187674**.
- Fix in `route_reward_sewer_pilot.h`: (1) skip flee tail-rotate when it
  would reverse an east escape at `ax≤-1500`; (2) critical PE heal
  (`hp≤12`) ATB gate **2500** (else 9000). Rebuilt
  `pe-route-boot-day2-tests`.
- Supply30 PID **192206** → `/tmp/pe-m34-pilot-supply30.log` — east
  Z-clamp into head + heal2 h2 floor (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.48: M34 hard-Up west-pin; supply29 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply28 (**179062**) re-proved path: m31 heal → `A8003148` →
  `A80031C8` → **`A8003248`**@63000. Supply27 heal floors **worked**:
  `LOOT_PILOT_HEAL`@62820 hp8→38@63030; second heal@63183 hp19→49@63360.
  But hard-pad **Up (FFEF)** drove west **-2344→-2839**, then pinned
  forever @x=**-2839** pad=FFEF while head closed (-2048→-2753). Soft-fail
  `A8001048` mode=`0xFFFFFFFF`@64000 → `A8000148`. Never `0x80`.
  Killed **179062**.
- Root: pad Up/Down are camera-relative — hard Up mapped **west into the
  wall**, not world-Z strafe. Also `az≥700` north-clamp flipped Z-away
  toward the head at pocket Z≈780 when applied before west override.
- Fix in `route_reward_sewer_pilot.h`: drop hard-pad; west bounce **after**
  az clamps; at wall (`ax≤-2500`) / head-east corridor keep **strong east
  + Z-away** via AimPad only. Rebuilt `pe-route-boot-day2-tests`.
- Supply29 PID **187674** → `/tmp/pe-m34-pilot-supply29.log` — post-heal
  kite/ATB starve (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.47: M34 heal-cancel bug; supply28 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply27 (**172492**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. Entered M34 hp27 →
  west pocket x≈**-2344**; first contact `HEAL_STARVE`@62820 hp8
  **h2=320450** pe=87 at=9000 — **no PE heal ever armed**. Pad stayed
  **FFDF (Right)** while head sat east (hx≈-2243). Soft-fail
  `A8001048` mode=`0xFFFFFFFF`@64000 → `A8000148`. Never `0x80`.
  Killed **172492**.
- Root causes: (1) AimPad camera-rotated lateral `want` into Right →
  east-into-boss; (2) emergency heal gate h2≥400k never met at starve
  h2=320k; (3) **same-frame cancel** `heal_start=-1` when h2<600k made
  the 400k–599k emergency window impossible.
- Fix: closer heal floors + cancel only h2<80k/hp>8; hard Up/Down (later
  proven wrong — see CURRENT). Supply28 tested that heal fix.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.46: M34 east-into-boss; supply27 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply26 (**166836**) re-proved path: m31 `HEAL_DONE`@58044 →
  `A8003148` → `A80031C8` → **`A8003248`**@63000. PE heal1 hp8→38@63090
  at x≈**-2297**. Pure-east flee ran **into head** (hx east of aya,
  h2~0.7–1.1M) → hp38→17@63180; pe rebuilt to 64 but second heal blocked
  (h2~660k < 1.5M gate). Soft-fail `A8001048` mode=`0xFFFFFFFF`@64000 →
  `A8000148`. Never `0x80`. Killed **166836**.
- Fix in `route_reward_sewer_pilot.h`: (1) west pin at **ax≤-2100**: if
  head is east/close, **strafe Z first** then east (not pure-east into
  boss); (2) emergency second heal when hp≤20 + west-pinned + h2≥400k;
  (3) ATB Circle-out threshold matched to -2100. Rebuilt
  `pe-route-boot-day2-tests`.
- Supply27 PID **172492** → `/tmp/pe-m34-pilot-supply27.log` —
  heal-starve soft-fail (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.45: M34 ATB Up-freeze; supply26 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. **Never observed live `0x80` / `M0036I`.**

### Story path

- Supply25 (**153541**) reached **`A8003248`**@62572; PE heal1
  hp8→38@63090. East bias held x≈**-2316** (not -2700). Then
  `!can_heal` ATB mode=1 pressed **Up forever** (`target!=selected` →
  pad FFEF/FFFF) from f=63270→99990; pe frozen ~56; frame budget
  exhausted. FAIL milestone 55 (item7 already spent earlier — expected
  miss). Never soft-fail, never `0x80`. Process exited naturally.
- Fix in `route_reward_sewer_pilot.h`: (1) ATB: Circle-out when
  west-pinned + `!can_heal`; else Cross when `!can_heal` / target match /
  nav≥3 (Down toward higher target, not Up-only); (2) pure-east flee
  `want_x=2000, want_z=0` at ax≤-2300 so AimPad X wins over Z. Rebuilt
  `pe-route-boot-day2-tests`.
- Supply26 PID **166836** → `/tmp/pe-m34-pilot-supply26.log` —
  east-into-boss soft-fail (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.44: M34 west-flee root cause; supply25 (exited)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply24 (**147625**) re-proved full path: m31 `HEAL_DONE`@58044 →
  `A8003148`@59000 → `A80031C8` → **`A8003248`**@63000. Two mid-M34 PE
  heals (hp8→38@63150; hp17→47@63510). Then pinned at **x≈-2700** —
  flee-from-head from ax≈-2300 drove further west (bias only at -2700).
  After heal2, hp47→16 in ~90f; `!can_heal` + hp≤24 Circle-cancelled all
  ATB → soft-fail `A8000148` mode=`0xFFFFFFFF`@65000. Never `0x80`.
  Killed **147625**.
- Fix in `route_reward_sewer_pilot.h`: (1) west escape from **ax≤-2300**
  toward arena center (`want_x=1000`, forced Z-strafe — do not let |Z|
  dominate into the corner); (2) heal-spent fire earlier (h2≥400k / %4)
  + desperate shots hp>12 h2≥700k; (3) when `!can_heal` keep ATB Cross
  instead of Circle-cancel at hp≤24. Rebuilt
  `make pe-route-boot-day2-tests`.
- Supply25 PID **153541** → `/tmp/pe-m34-pilot-supply25.log` —
  ATB Up-freeze (see CURRENT). Never `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.43: M34 wall-pin + heal-spent fire; supply24 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply23 (**142891**) re-proved path through M34: m31
  `HEAL_DONE`@58044 → `A8003148` → `A80031C8` → **`A8003248`**@63000.
  Two mid-M34 PE heals landed (hp8→38@63120; hp17→47@63450). Then
  pinned at **pos=-2830** (west wall) while head closed h2≈1.9M→0.16M;
  `!can_heal` suppressed all ATB Cross → pure kite → hp47→26→2 →
  soft-fail `A8000148` mode=`0xFFFFFFFF`@65000. Killed **142891**.
  Never `0x80`.
- Fix in `route_reward_sewer_pilot.h`: (1) bias flee off arena walls
  (ax≤-2700 / ≥-1100, az≥700 / ≤-700) + prefer wall-strafe; (2) when
  heal spent but hp>24, still Cross at h2≥600000 (finishing damage).
  Rebuilt `make pe-route-boot-day2-tests`.
- Supply24 PID **147625** → `/tmp/pe-m34-pilot-supply24.log` — reached
  M34, two PE heals, west-pin death (see CURRENT).

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.42: M34 reached; post-heal kite; supply23 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply22 (**137883**) proved the full post-heal → m32 → **M34** path:
  `HEAL_DONE`@58044 → `HEAL_EXIT` Circle → **`A8003148`**@58560 →
  `A80031C8` → **`A8003248`**@63000. Mid-M34 PE heal worked
  (`LOOT_PILOT_HEAL`@62931 hp8 → hp38@63120 via id0→8→41). Then
  re-hugged boss at h2≈1.9M, hp38→17, no second heal (PE spent,
  item7=0) → soft-fail `A8001048` mode=`0xFFFFFFFF`@64000. Killed
  **137883**. Never `0x80`.
- Fix: widen M34 flee floor to 3.2M when heal unavailable / hp≤24;
  stop ATB Cross unless hp>24 **and** can_heal; Circle-cancel ATB
  sooner; log `LOOT_PILOT_HEAL_STARVE`. Rebuilt
  `source /tmp/pe-tools/env.sh && make pe-route-boot-day2-tests`.
- Supply23 PID **142891** → `/tmp/pe-m34-pilot-supply23.log` — reached
  M34, two PE heals, wall-pin death (see CURRENT).

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.41: post-DONE exit site; supply22 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply21 (**134001**) proved post-heal **menu exit works**:
  `HEAL_DONE`@58044 → `DFFF` id1→id0→clear → walk → **`A8003148`**@58560
  (`EQUIP_QUEUED`@58917). Then m32 battle re-armed `LOOT_PILOT_HEAL`@59766
  with **item7=0** (PE heal); cancel-only inside
  `RoutePilotItemHealMask` starved PE nav → Circling id0 → soft-fail
  `A8001048` mode=`0xFFFFFFFF`@61000. Killed **134001**. Never M34 /
  `A8003248` / `0x80`.
- Fix split: (1) field post-consume exit is **call-site cancel-only**
  (`LOOT_PILOT_HEAL_EXIT`, `DFFF` on 28f cadence) — do not navigate;
  (2) restore `RoutePilotItemHealMask` `!item7` PE/menu nav for m32.
- Supply22 PID **137883** → `/tmp/pe-m34-pilot-supply22.log` — reached
  M34 then died (see CURRENT).

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.40: post-heal cancel-only; supply21 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply20 (**129631**) healed again on m31 `A80030C8`:
  `LOOT_PILOT_HEAL`@57927 → `HEAL_DONE`@58044 (hp33→53, item4=0). Then
  post-consume exit: id1 `DFFF` → id0 → Down+Cross opened **id=8**;
  `HEAL_MENU` id=8 pad=`BFFF` (Cross) through f=64000+ — never focus
  clear / m32 / `A8003148` / `A8003248`. Killed **129631**.
- Root cause: no-item7 exit path navigated root (Down on id0) and Crossed
  into id=8; Cross does not dismiss that submenu. `DFFF` had already
  cancelled id1→id0.
- Pilot fix (too broad): cancel-only `DFFF` inside
  `RoutePilotItemHealMask` for all `!item7`. Rebuilt via cmake make.
- Supply21 PID **134001** → `/tmp/pe-m34-pilot-supply21.log` — exited to
  `A8003148`, then PE-heal starved (see CURRENT).

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.39: post-heal Circle exit; supply20 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply19 (**125239**) **healed successfully** on m31 `A80030C8`:
  `LOOT_PILOT_HEAL`@57927 → `HEAL_PULSE` → `HEAL_MENU` id0→1→2 →
  `HEAL_DONE`@58044 (hp33→53, item4=0). Then stuck: Items focus `id=1`
  (`focus=800A2520`), `LOOT_PILOT_DISMISS` pad=`FFFF` through f=62400+ —
  never m32/`A8003148`/`A8003248`. Killed **125239**.
- Root cause: clearing `heal_start` at `HEAL_DONE` handed the pad to
  dismiss; Cross on `id=1` re-opens Use and never exits the menu.
- Pilot fix in `route_reward_sewer_pilot.h`: (1) keep heal ownership after
  item consume until focus is gone (Circle out via
  `RoutePilotItemHealMask`); (2) dismiss Circle-only on submenu ids
  1/2/3/5/7/8. Rebuilt via direct gcc (`/tmp/pe-tools/.../gcc`).
- Supply20 PID **129631** → `/tmp/pe-m34-pilot-supply20.log` — healed,
  then stuck id=8 Cross (see CURRENT).

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.38: heal open pulse; supply19 (killed)

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply18 (**119380**) repeated supply17: clear-patch `LOOT_PILOT_HEAL`
  f=57927 (`pos≈37,1199`, item7, hp33), then pad=`FFFF` through f=84000+ —
  never `HEAL_MENU`/`HEAL_DONE`, never m32/`A8003248`. Killed **119380**.
- Dismiss-hold alone was insufficient: focus stayed 0 (Items never opened).
  Also `d1a0` bit2 chatter routed the m31 block into Circle-only and starved
  Triangle on those frames.
- Pilot fix: (1) heal/equip own the pad even when `d1a0&4`; (2) until focus,
  alternate longer Triangle holds with Circle clears + `LOOT_PILOT_HEAL_PULSE`
  diagnostics. Rebuilt via direct gcc (cmake libarchive still broken).
- Supply19 PID **125239** → `/tmp/pe-m34-pilot-supply19.log` — heal path
  later succeeded; post-heal dismiss stuck (see CURRENT).

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.37: heal dismiss hold; supply18 live

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply17 (**113583**) cleared the door wedge: staged walk reached clear
  patch, `LOOT_PILOT_HEAL` at f=57927 (`pos≈37,1199`, item7, hp33). Then
  idled pad=`FFFF` ~f=58000..82000+ — never `HEAL_MENU`/`HEAL_DONE`, never
  m32/`A8003248`. Killed **113583**.
- Root cause: m31 field Items opens under `mode=9`+focus; the loot
  `LOOT_PILOT_DISMISS` early-return ate the menu before
  `RoutePilotItemHealMask` ran (same failure class as supply15 after arm).
- Pilot fix: skip mode=9 dismiss while `heal_start>=0` or `equip_start>=0`;
  re-pulse Triangle every 28f until focus appears. Rebuilt via direct gcc.
- Supply18 PID **119380** → `/tmp/pe-m34-pilot-supply18.log`
  (`PE_ROUTE_REWARD_PILOT=1`, `PE_ROUTE_FRAMES=100000`). Not at `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.36: heal-wait X-stage; supply17 live

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply16 (**105522**) reached m31 `A80030C8` with item7 after m334, then
  wedged at door spawn `(751,2502)` on `LOOT_PILOT_HEAL_WAIT` pad=`FFBF`
  (~f=57600..100000) — never `HEAL_MENU`/`HEAL_DONE`, never m32/`A8003248`.
  FAIL milestone 55. Battle-room `LOOT_PILOT_HEAL` at f=51273/52951 on
  `A80023C8`/`A8002448` was incidental (cleared on leave).
- Root cause: HEAL_WAIT aimed `(0,1000)` diagonally from the alcove; camera
  kept only Down held into the wall. Clear-patch arm gate itself was fine.
- Pilot fix: stage walk — if `az>1300 && abs(ax)>40` then `tx=0,tz=az`
  (center X), else `tx=0,tz=1000`. Same staging on `HEAL_ABORT`. Log now
  prints `tx=`. Rebuilt via direct gcc (cmake libarchive broken).
- Supply17 PID **113583** → `/tmp/pe-m34-pilot-supply17.log`
  (`PE_ROUTE_REWARD_PILOT=1`, `PE_ROUTE_FRAMES=100000`). Not at `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.35: heal clear-patch gate; supply16 live

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply15 (100294) reached m31 `A80030C8` with item7 but idled at return
  door `(1020,2615)` pad=FFFF after `LOOT_PILOT_HEAL` ~f=57515 — no
  `HEAL_MENU`/`HEAL_DONE`, never m32/M34. Killed **100294**.
- Root cause: heal armed on the m334→m31 transition frame while ax/az were
  still prior-room coords (`az<2000` gate); spawn then snapped to the door.
- Pilot fix: arm only near clear patch `abs(ax)<=80 && abs(az-1000)<=200`;
  abort+walk if heal has no focus off-clear after 40f (`LOOT_PILOT_HEAL_ABORT`).
- Supply16 PID **105522** → `/tmp/pe-m34-pilot-supply16.log`
  (`PE_ROUTE_REWARD_PILOT=1`, `PE_ROUTE_FRAMES=100000`). Not at `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.34: m31 door clear + item7; supply15 live

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply13 failed: m32 battle heal stall `id2@row0` Cross, HP17, no M34
  (killed **88419**). Oracle Use is on **m31 field** `A80030C8`, not m32.
- Supply14 (95845) held at m334-return door `(1020,2615)` mode=9 with item7;
  AT gate never armed heal. Killed; pilot now walks to `(0,1000)` then opens
  Items (no AT requirement); 28f cadence + `use_armed` Circle cancel.
- Supply15 PID **100294** → `/tmp/pe-m34-pilot-supply15.log`. Not at `0x80`.

### Title / FMV

- Unchanged: frontier **`0x801911C0`**; canary `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.33: item7 Use row2 fix; supply13 live

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply12 **failed** on item7 Use: m32 `A8003148` ~f=59142..70000+ stuck
  `LOOT_PILOT_HEAL_MENU` id1↔id2 Cross toggle; HP frozen 17; never
  `A8003248`; no `ROUTE_EXIT` (killed PIDs 77243/77201).
- Root cause (vs SUPPLY_MENU oracle 61020..61170): Cross on id1@row0 opens
  Use@row1 (cancel toggle). Correct: **Down to row2** (slot4 item7) → Cross
  → id2@row0 → Cross (HP12→45). supply11 Up+Cross on id2@row0 was also wrong.
- Pilot fix in `route_reward_sewer_pilot.h`; m31 field heal gate `hp<45`.
- Supply13 running PID **88419** → `/tmp/pe-m34-pilot-supply13.log`
  (`PE_ROUTE_FRAMES=100000`).

### Title / FMV

- Unchanged: body 90064→8F468→merge→present; frontier **`0x801911C0`**
  continue; canary `0x801911F8`. Next cut: title loop at `0x801911C0`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.32: title 8F468 body; supply12 in sewers

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply12 running (`/tmp/pe-m34-pilot-supply12.log`): theater + sewers;
  ~f=56000 in m0334. Item7 Use Cross-on-id=2 (no Up). **Later failed:**
  m32 heal menu id1↔id2 forever.

### Title / FMV

- Ported `func_8018F468` + `func_80192FE8` + `func_8019319C`; one loop body
  (90064 → 8F468 → list merge → present). Pad-seeded frontier **`0x801911C0`**
  continue. Canary still `0x801911F8`. Test
  `B54KR_801909B4_title_input_pump_with_pad` PASS.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.31: supply11 55/57; item7 Use row fix

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply11: through sewers + item7 pickup; **MISS** Use (pilot Up on id=2
  row1 → stuck row0 Cross) and pistol. 55/57. Log supply11.
- Fix: Cross immediately on Use id=2. Supply12 → `/tmp/pe-m34-pilot-supply12.log`.

### Title / FMV

- `func_80190064` ported; frontier **`func_8018F468`**.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.30: m0018 diary clear; title → 8018F468

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply11 theater carve-out: m0018 diary completed → **m0319i**
  (`A80614C8` ~f=39000). Log `/tmp/pe-m34-pilot-supply11.log`. Next: Eve
  rehearsal → sewer → M34 → `0x80`.

### Title / FMV

- Ported **`func_80190064`** (+ `func_8003FFCC`); pad-seeded title frontier
  advanced to **`func_8018F468`**. Canary path still `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.29: m0018 diary; theater-field dismiss carve-out

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Supply10: mode=2 dismiss OK; walked m0012→m0020→**m0018** (~f=31000), then
  mode=9+focus dismiss stole diary pads (stuck A8001448).
- Fix: no mode=9 dismiss on theater field tokens (m0012/m0020/m0018/m0319).
- Supply11 → `/tmp/pe-m34-pilot-supply11.log`.

### Title / FMV

- Frontier still **`func_80190064`** (383w overlay; `jal` after `3EB04`).
  Next callees: `8018F468`. Canary: `0x801911F8`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.15: F434 cleared; M34 combat death / heal gate

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- Writer-snaps run (`/tmp/pe-m34-writer-snaps.log`): **no F434 / WeaponCallback
  stop**. Aya kited through ~60330–60900, HP 35→14→0 at **f=60901**, then
  frame-limit at 65000 (`story` collapsed after death). Milestone still 50/57.
- F434 writer snapshots are the durable fix for the prior `known=0x32` /
  clobbered `words[]` boundary.
- New frontier: M34 **combat**. Heal never fired (`LOOT_PILOT_HEAL` absent);
  gate required `h2>=1800000` while critical HP sat at ~1.3M. Pilot now
  heals at `hp<=16` regardless of head range, fires denser mid/pocket.
- Connected re-verify: `/tmp/pe-m34-pilot-heal-crit.log`. Not a win until
  `story` leaves `0x6C` toward `0x80`.

### Title / FMV

- Frontier still `func_801909B4_title_loop_425DC` @ `0x801911F0`.
  Opening FMV without skip-movie is live; skip remains on the connected
  route.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.6: M34 pilot kite/reload; title cut before card pump

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete.

### Story path

- M34 pilot: no heal mid-charge; kite/fire; Cross-reload when empty.
- Headless `--route-pad` present-hook fix so `g_frame` advances.
- Verification run (`PE_ROUTE_REWARD_PILOT=1`, 61000 frames): Aya **moves**
  in M34 (no longer frozen at -873), but stops at **f=60323** on
  `PE_WeaponCallback` BOOTSTRAP_RET (`story=0x6C`, still m0034i). Not a
  win; next is bind/allow that weapon callback or avoid the path.
  Log: `/tmp/pe-m34-pilot-kite-reload.log`.

### Title / FMV

- Title present + `8018FBC0` + loop bank-flip live; frontier
  `func_801909B4_title_loop_425DC` @ `0x801911F0` (calling `425DC` after
  `42538` hits card-status BIOS stop — cut before it).
- `B54KR_801909B4_prefix_effects_and_canary` → **PASS** after that cut.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.5: Title present + object alloc; frontier title loop

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. New Game reaches title setup without skip-movie; title
main loop remains the named cut (or `--skip-opening-menu`).

### Opening FMV / title

- Ported `func_8018F2F4` (title-bank LoadImage/PutDrawEnv×2) and
  `func_8018FBC0` (52-byte freelist title object).
- `func_801909B4` after FMV/restore: `8F2F4` → VSync → SetDispMask(1) →
  freelist seed → `8FBC0(1)` + callback `0x8019319C` → frontier
  `0x801911C0` (`func_801909B4_title_main_loop`).
  `--skip-opening-menu` returns New-Game after present + freelist (before
  `8FBC0` / title loop).
- Verified (`PE_TEST_FILTER` / `PE_DISC1_BIN`):
  - `B54KR_801909B4_prefix_effects_and_canary` → **PASS** (frontier
    title main loop)
  - `B54KR_801909B4_dirty_repeat_and_alternate_cut` → PASS
  - `B54KY_saved_bit_zero_skips_disc_prefix` → PASS
  - `OPEN1_909B4_enters_fmv_without_skip_movie` → PASS
  - `B54KY_192CE8_real_disc_issue_poll_and_boundary` → PASS
- Still required: full FMV001 without continue budget; title main loop
  (`80190064` / `8018F468` / …); drop route `PE_Port_SetSkipMovie(1)`;
  XA; cold boot to `persist[74]=0x80`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.4: New Game enters real FMV without skip-movie

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. Opening FMV is reachable from `func_801909B4` without
`PE_Port_SkipMovie`; title join is still a named cut / opening-menu skip.

### Opening FMV / New Game

- `func_801909B4`: Disc-1 saved bit → `90660` → `92CE8` → post-movie
  env restore → title join `0x80191120` (`jal 0x8018F2F4`).
  `--skip-movie` remains HOST_ADAPTED; `--skip-opening-menu` returns
  New-Game from the title join after a real (or budgeted) FMV.
- Verified (`PE_DISC1_BIN` + `PE_TEST_FILTER`):
  - `OPEN1_909B4_enters_fmv_without_skip_movie` → **PASS**
  - `B54KY_192CE8_real_disc_issue_poll_and_boundary` → PASS
  - `B54KR_801909B4_prefix_effects_and_canary` → PASS (frontier
    `func_8018F2F4`)
- Still required: full FMV001 (~2077 frames) without continue budget;
  translate or retire title `8018F2F4`; drop skip-movie from connected
  route; XA; cold boot to `persist[74]=0x80`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.3: continue-frame live; media loop budgeted

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. Opening FMV first frame + continue-frame decode live on
real Disc 1 FMV001. Production `92CE8` media loop is unlimited by default;
tests may set `PE_Port_SetMovieContinueBudget(N)`.

### Opening FMV progress

- Ported `func_80191DC8` (DMA1 twin of `214D4`) and `func_80192934`
  (continue-frame twin of `122040`); IRQ dispatch recognizes `0x80191DC8`.
- `func_80192CE8` runs `3EB04` → `92934` → skip/`70E54` until `B0DBA==0`.
  Host budget 0 stops at named `func_80192CE8_media_loop` (B54KY uses 1).
  FMV001 is ~2077 frames — full play is not a unit-test default.
- Verified (`PE_DISC1_BIN` + `PE_TEST_FILTER`):
  - `B54KY_192CE8_real_disc_issue_poll_and_boundary` → **PASS**
  - `B54KAD_fmv2_filename_threshold` → PASS
  - `B54KAE_movie_state_setup` → PASS
  - `DAY2_movie_autonomous` → PASS
- Still required: drop `PE_Port_SkipMovie` only when New Game stays
  stop-free through full opening STR; XA; connected route to
  `persist[74]=0x80`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.2: `92CE8` enters media loop; frontier `func_80192934`

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`). Day 1 is
**not** complete. Opening FMV first frame decodes live; `func_80192CE8` now
enters the media loop and cuts at the continue-frame worker.

### Opening FMV progress

- Real Disc 1 FMV001 first frame: live C89C→7C394→EC (`B54KY` PASS).
- `func_80192CE8` extended past `80192E08`: after `924F8`, if `B0DBA!=0` and
  `B0DBC>0`, named frontier `func_80192934` (overlay twin of `122040`,
  ~197 words at `80192934..80192C48`). Pad/`func_8003EB04` seeding is part
  of that next rung.
- Verified: `B54KY` / `B54KAD` / `B54KAE` PASS with `PE_DISC1_BIN`.
- Still required: translate `func_80192934` (+ `3EB04` before it), then
  drop `PE_Port_SkipMovie` only when New Game stays stop-free; XA; connected
  route to `persist[74]=0x80`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no commits
unless asked.

---

## PRIOR 2026-09-15 cont.: Real Disc 1 opening frame decodes through C89C/EC

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`), including
movies/sounds. Day 1 is **not** complete. Opening-FMV first frame now runs
live on real FMV001; frontier is the `func_80192CE8` media-loop cut.

### Opening FMV (`func_801924F8` got_frame) — complete first frame

- Authenticated tail `80192814..80192930` runs live
  `func_8010C89C` → `func_8007C394` → EC on **real Disc 1 FMV001.STR**
  (9 video chunks + interleaved XA; `B374=10`, `BE998=9`).
- E0 uses `HostFB_StreamTick`; DMA IRQ may clear `B89F4` inside the tick
  before the loop samples it — a nonzero `func_80191B64` result is the
  complete-frame signal (do not `7C214` every poll).
- B54KY harness needs autonomous-style `CdDeviceSeed` + `func_8007EC14`
  before stream open (`ED58`-only left `9B554=1` so CdInit was a no-op).
- Verified (`PE_DISC1_BIN` + `PE_TEST_FILTER`):
  - `B54KY_192CE8_real_disc_issue_poll_and_boundary` → **PASS**
    (frontier `func_80192CE8_80192E08_cut`)
  - `B54KAD_fmv2_filename_threshold` → PASS
  - `B54KAE_movie_state_setup` → PASS
  - `DAY2_movie_autonomous` → PASS
  - `MV1D_c89c` → 6/6 PASS (prior)
- Still required for skip-free New Game: extend `func_80192CE8` past
  `80192E08`, then remove `PE_Port_SkipMovie` early-return in
  `func_801909B4` only when that path does not unresolved-stop. XA still
  undecoded. Connected cold boot still short of `persist[74]=0x80`.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no
commits unless asked. No matching `src/` claims this session.

---

## PRIOR 2026-09-15: Opening `got_frame` C89C live for empty-VLC; real STR gated

Goal ACTIVE — Day-1 retail fidelity (boot → `persist[74]=0x80`), including
movies/sounds. Day 1 is **not** complete. Prior M34 burst work still stands;
this session advanced the opening-FMV decoder frontier only.

### Opening FMV (`func_801924F8` got_frame)

- Authenticated tail `80192814..80192930` (PE.IMG ov133 carve) is now
  transcribed through `func_8010C89C` → `func_8007C394` → EC stores
  (`[D0DBD]=0`, `B0DBC=1`, `B0DBA++`) in
  `pc_port/game/boot/func_801924F8_port.c`.
- **Live C89C** when the VLC source is the CDQ2d empty `FF FF` terminator
  at `0x8010CBFC`, or when E0 observed `B89F4` (last-chunk complete).
- **Real Disc 1 FMV001** still stops at named boundary
  `func_8010C89C_needs_complete_frame` (mid-frame promote + live table
  previously `PE_StoreU16` at `0x80200000`). Not silent decode.
- Verified (rebuild via pe-tools gcc + archive replace; cmake wrapper
  missing `libarchive`):
  - `PE_TEST_FILTER=B54KAD_fmv2_filename_threshold` → PASS
  - `PE_TEST_FILTER=B54KAE_movie` → PASS
  - `PE_TEST_FILTER=B54KY_192CE8_real_disc` → PASS (gate stub)
  - `PE_TEST_FILTER=MV1D_c89c` → 6/6 PASS
- Still required for skip-free New Game: last-chunk delivery into this
  E0 path, then `func_80192CE8` past `80192E08`, then remove
  `PE_Port_SkipMovie` early-return in `func_801909B4` only when that
  path does not unresolved-stop. XA still undecoded.

See `docs/ai_context/DAY1_FIDELITY_GAPS.md`. Preserve dirty tree; no
commits unless asked. No matching `src/` claims this session.

---

## PRIOR 2026-09-13: M34 native burst bound and verified; pilot defeat is the next obstacle

Goal ACTIVE. Major production/source/connected progress; fullDay2 and
whole-route fidelity UNPROVED. All sessions terminal; no pending questions.
Preserve huge dirty tree; no matching src/asm/YAML/index edits, staging,
commits or gameplay RAM injection/restores. Copies only isolated oracles.

NEW production: F434 is NOW WIRED through native retained-stack provenance
in m34_boss_effect_port.c. New PE_M34Stack* APIs in pe_port_compat.h; field
wrapper35558 scopes persistent6words/knownmask/RAMgeneration, actor35E04
calls scoped; 17018 scopesVMdepth/opcode. Current22-handler whitelist from
observedsourcehistory is NOT generalstaticcallgraph/branchproof. Unknown
handlers/callbacks,nestedVM,generation/overlaychangesinvalidate. Slotowner
mustmatchesavedcueactor; activefieldrequired. No capturedconstanttuple.

6DE80_port.c PE_SpatialSoundRequest sharesactual6DED4calculationandoptionally
returnscomputedvolume. 2FAF8actualclipcrossingrecordsvolume/body/targetonce
afterrequest; no second sound call. Missingpackage/failedcalculationunknown.
Other6DED4/86608callsinvalidate. 18774->6F39C->6914CidlebranchrecordsS1
80010690 (original17020/17024constant). M34Command API now6args;6F6D4forwards
args5/6, storesinprovenancerecord. Tests/coreoraclecallersupdatedto6args.
Effect/weaponcallbacksnotifyrecord. F434itselfpreservesallsixoriginalwords.

NEWsecond-shotpartialwriters: F830->C61A8 atoriginalF434SP offsets:
-76lowhalf=vertex2.z(C62A0), highhalfremainsvolume;
-72whole=vertex3.x/zeroY(C6264/C625C);
-68lowhalf=vertex3.z(C62B8), highhalfsavedactor;
-36lowhalf=trailworldZ(FDE4->C42A4 C4544), highhalfsavedVMS1;
-32/-28whole=F830secondscaledmatrixfirst4elements(78D0C/78D4C).
SharedPE_EffectPointQuadC61A8 reportsverticesviaPE_M34StackPointQuad;
M34main/trailrenderersrecordpackedmatrix/worldZ. Partialstorespreservehigh
halvesandcannotmakeunknownhighhalfknown. AllM34callbacksallowedhere;
unverifiedweapon/callbackgraphs stillinvalidate. OptionalPE_M34_STACK_TRACE=1
logsactualwritersandinitializerinputs; no gameplayRAMwrites.

FIRSTbindingcoldboot74410terminalF434secondshot60275(mask0), 3victories
repeat52111/53823/57791, optional50/57 exit1. ThisisINTERMEDIATEhistorical
implementationbeforepartialwritebinding. /tmp/pe-m34-bound-projectile-connected.log.
Captured60270..60275 in day2-victory-evidence/m34-bound-projectile. Lastcapture
partiallystoppedframe;60274beforesecondinitializerSHA256
5a6f278b6444d1d883fa8f50c0da9a6530f40144aab7a36f910eb3063b7f5dc8.
First60271record72bytesmatchesoriginaloracle,captureSHA256
5a4e42341ff47eff2293eb5505c7e81ad3fe022f90682f80fa3ef849178b1386.

FINALbindingcoldboot49356terminalframe-limit62000,no unresolvedcallbacks.
Fiveprojectiles60271/60275/60279/60283/60287, actualdynamicinputtupleslogged
in /tmp/pe-m34-burst-bound-connected.log. Threevictoriesrepeat. Thenreset
60742story0/title,60746NewGame,60753M0010I. ThisisNOTboss/Day2completion.
17captures60274..60290 day2-victory-evidence/m34-burst-bound.
60275bothprojectile72byterecordsmatchsourceoracle.SHA256
a85adc7cbfe633249f169c30b83069de5f4f76a2468cd4efc3944028ad6a37a1.
Fourthconnectedinputmatchescontinuousoriginaltarget4exactly.

DEFEATconfirmedbydiagnosticcoldboot35762terminal60800frame-limit,50/57exit1.
/tmp/pe-m34-burst-outcome.log; envPE_ROUTE_BATTLE_DUMP_BEGIN=60100,
PE_ROUTE_REWARD_PILOT=1,PE_M34_STACK_TRACE=1,PE_ROUTE_FRAMES=60800,
PE_ROUTE_RAM_DUMP=pc_port/build/day2-victory-evidence/pe-m34-burst-outcome.bin.
HP35->25 at60304,->21 at60499,->0/mode3at60533. Ayaessentiallystationary
(-873,11)untilcontact; PE92, reserve36 remain. First2shotsmiss;shots60405/
60421reduceenemy0HP1000120->1000118, loadedammo0. Originalmillionoffsetnotbug.
At60290enemy0actor800BF490body800A5D5CHP1000120type3,pos(-2126,-317),flags80;
enemy1actor800BF710body800A5E38HP1000080type4,pos(-3354,-109),flags20E0.
Aya record800B8A20, gun800B0CB0, pistolslot0. Pilot currently onlyadapts
M27/M28/M32, NOT M34; defaultCrosspulseperiod8 makesherstandstill.

VERIFICATIONcurrentproduction: Release/DebugfullbuildPASS72390/46399;
1398/1398native0failed/skipped,10/10non-routeCTest62.57secPASS20998.
Logs/tmp/pe-m34-burst-stack-{build,debug-build,ctest}.log.
Newnativecontextregression coversdynamicnonzeroargs5/6,partialwordwrites,
crossframepreservation, unknownweaponcallbacks, nestedVM andresetisolation.
No productionchangesafterpassingchecks. Docs/diagnostics onlyafterward.
EXEsbytecmp/SHA1still452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

NEWpe_m34_stack_binding_oracle.py usesactualnative35558 andoriginalcounter/
mailbox/35558 calls on eachcapturedframe, carryingoriginalstack/GTE/scratch
andnativeprovenance. Nativeisolatedcapturesnotconnectedrestores. 5envelope
histories(firstvolumes87,0,49,127,127), TWOinputsandfull72byterecordsallmatch,
10508PCs sourcechecked,79817PASS. Command --end-frame60274 --build-dir
/tmp/pe-day2-release withdirectoryday2-victory-evidence/m34-burst-window
(symlinkscaptures60260..70fromm34-destination-window,60271..74fromfirstboundrun).
Reportnative-stack-binding.jsonSHA256
3a364c2f16126ffe3a93283888b405119c02ea3719f067b4048d594b0e218034;
/tmp/pe-m34-burst-stack-oracle.log. IMPORTANT136othernon-stackRAMbytesdiffer
eachfinalfieldcomparison, alladdressesreported; notfullframe/nativeproof.
Earlierfirst-only5histories9135PCs140otherbytesdiff,reportinm34-destination-window
SHA2566fb6b96a1f4d1e597321c228d7ab3174c3f54e34eaba0827854725e5f35c1994.

pe_m34_frame_device_probe.py now --initializersN --instruction-budgetN.
Fresh60260 --modecontinuous --initializers2 --instruction-budget5000000:
6995PASS15frames3159708instructions12615PCs; sixwords
2BB,F821,800B02DD,8001023C,015CFF1A,0 withmixedwritersabove.
/tmp/pe-m34-second-projectile-stack.jsonSHA256
f12c887b80cf5eee9ae52bf35b9d0f31fea1221f8800f8fa92973bfd349c6542.
--initializers4 --instruction-budget8000000:59161PASS23frames4973703insns
12631PCs,tuple1DF,F91E,800B0248,800101CD,0426FF8E,0 matchingconnected4th.
/tmp/pe-m34-fourth-projectile-stack.jsonSHA256
7ca39cef2e81d80c61174a439545e4677de7fdca466fcf3b901333e76a9402bd.
Allsameexplicitdevice/BIOScontracts; NO BIOSinternal/IRQ/raster/timingproof.
Fifthconnectedinputnotyetcontinuoussourcecompared.

NEXT: ordinary-inputM34survival/targeting/healingpilot (existing
pc_port/tests/route_reward_sewer_pilot.h), provefightthenremainingDay2.
Do not treatstationarypilotdefeatasproductionprogress/completion; noexternal
impasse. Investigateoriginalenemybody/targetflagsasneeded, noHP/ammo/state
injection. Newlyencounteredwriters/opcodesmustbeverified; currentbridge
is scoped,notgeneralstackemulation. Fullframe136bytedifferencesremainopen.
[M34_STACK_BINDING.md](M34_STACK_BINDING.md) currentproofandlimits.

## PREVIOUS 2026-09-13: destination snapshot repaired; continuous M34 stack history reaches F434

Goal ACTIVE. Production fix and source-history progress; fullDay2/whole-route
fidelity UNPROVED. All sessions terminal. Preserve dirty tree; no matching
src/asm/YAML/index edits, staging, commits, gameplay RAM injection or restores.

NEW productionFIX: D_8009D1C4 was another host-only scalar. Captured guestword
remained0 whileD280=A8003248, causingoriginal3F684loop toexitafteroneframe.
Nowguestmacroinpsx_compat.h, definitionremovedpe_globals.c, obsoleteexterns
removed1220C/3E680/native/route tests. Existing1220Cassignmentandbootreset
publishguestRAM; one-passnativefieldadapterunchanged. Original122CC..122DC
fourwordsSHA2566922f1d5ecd983d866c65a6fa40ea8c7e920f52cf5c1aeed17f54423450c9177;
sixboundaryinputsPASS. Existingbootresettestsseed/readguestword; existing
120frameopeningtestassertsactual1220CguestpublicationA8001048.
Release/DebugfullbuildPASS18808/86996.1397/1397native0failed/skipped;
10/10non-routeCTest63.52secPASS55495. Logs/tmp/pe-destination-ram-*.log.
EXEsbytecmpPASS,SHA1452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
[DESTINATION_SNAPSHOT_RAM.md](DESTINATION_SNAPSHOT_RAM.md).

Freshcoldboot58123terminalexit1existingF434boundary60271,historical50/57;
threevictoriesrepeat52111/53823/57791. Log/tmp/pe-m34-destination-connected.log.
Eleven2MBcaptures60260..60270 in
pc_port/build/day2-victory-evidence/m34-destination-window.
EachguestD1C4=D280=A8003248. Fullbytecomparisoneachold/newcapturechangesONLY
9D1C4,9D1C5,9D1C7 (snapshotword); nootherbytechanges.
60260SHA2569db65a73f20670c91ff8ac3477c5859aea9d060cabff0e4ef66ce4651865b36f.
60270SHA256c025a97acf805a40a59840d2bed061328b61d63c4aa00ed683429579e0ef23ab.

NEWchecked-inpe_m34_frame_device_probe.py executesoriginalprefix/tail/continuous
loopwithseparateMMIO4096bytes, explicitDMA6OTCstores, GPU-DMAcompletion,
GP1readystatus,andVBlank956AC/timer1F801110incrementsevery1024instructions.
Originalgame/SDKinstructionsnotreplaced. BIOSA0memcpy2Acontractaddedlocally
viacheckedinspecthook;allBIOScallslogged, memorywriteswatched. NO BIOSinternal
stack/IRQ/raster/timingproof!InitialCPUstack/GTE/scratchsupplied; capturesonly
isolateddiagnostics. Sourcecheckeveryexecutedinstruction+delayandcaptured
followingwordagainstoriginalEXE/M34. ExactSWL/SWRbytemasks.

142oldwindowprefix/tailprobesPASS69207,no6wordwrites. Prefix325/343PCsminSP
801FEF80;tail1548/1565/1585PCsminSP801FEF50includesactual70E54presentation.
Reproducepe_m34_window_stack_probe.py DIRECTORY --device-fragments.
Oldwindowreportindependent-device-frame-writers.jsonSHA256
66d2cc6ef1be1802a75612085b4f725c4c00249a3724ba8bb9b74ed388e1c83b.
Fresh22probesPASS14571,reportSHA256
38f82ac7f795fb2ed5e9516748ab83473d91b19017a49e5e61e1a9c010e7e676.

KEYcontinuousfresh60260capture --mode continuous (NO --caller-snapshot)
reachesF434after11frames,10891PCs,2271911instructions. CPU/GTE/scratch/RAM
carriedthroughoriginal3F684backedge,nointermediatecapturesloaded. 34192PASS.
/tmp/pe-m34-fresh-continuous-device-probe.jsonSHA256
93170da0c506a9574c056f5138fad0e37c9a26819c43d7b15ea377f8daab1bde.
EntrySP801FEEE8,lastwriterssameaspriorindependentwindow:
-76volume87at6E184frameindex0; -72body800A5D5Cat6DED8index0;
-68actor800BF490at6DEE0index0; -36VMargtable80010690at6916Cindex10;
-32/-28zeroextraargsatF0C8/F0D0index10.
Oldcapturevariant --caller-snapshot executesoriginal122CC..122DCfirst;
periods256/1024/2048allreachsame11frames/tuple10895PCs.24538/84815/13455PASS.
Logs/tmp/pe-m34-continuous-snapshot-{fast,slow}-device-probe.json anddefault
/tmp/pe-m34-continuous-snapshot-device-probe.json.
[M34_FRAME_STACK_PRESERVATION.md](M34_FRAME_STACK_PRESERVATION.md) fullscope.

NEXT: nativebindingstillNOTimplemented,F434unwired. Newcontinuousevidence
supportsoriginal-callpreservationunderexplicitdevice/BIOScontracts; cannot
generalizetupletoallprojectiles/interrupts. Trackcomputedspatialvolumeplus
savedbody/actorinactualscript64contextwithoutduplicatingsoundcall; preserve
M34commandargs5/6andVMargsavedS1; invalidateunverifiedwriters/contexts.
All970M34effectwordsalreadytranslated,prior256initializer/303childrenproof
unchanged. No externalimpasse; continuegoal. Priorwriter/APIlocationsbelow.

## PREVIOUS 2026-09-13: guest frame counter repaired; all six M34 stack writers identified

Goal ACTIVE. This turn made verified production, connected-state and source
writer-discovery progress. FullDay2/whole-route fidelity still UNPROVED.
All handles terminal, no pending questions. Preserve huge dirty tree; no
matching src/asm/YAML/index edits, staging, commits or gameplay RAM injection.

Production FIX: D_8009D250 was a separate host counter while captured guest
9D250 stayed0. Now PE_GUEST_U32 macro in psx_compat.h, host definition removed
from pe_globals.c, obsolete externs removed from pe_port_compat.h/3E680/tests.
Original six-word increment3F4D0..3F4E8 SHA256
f2ec3d82628422ef69623f94644a1d2b1037a5d8d6374f27655f45765a915b99.
Six original boundary inputs including wrapping PASS; native actual field
tick regression starts guestFFFFFFFF ->0 ->1, skipped-update branch preserves1;
boot reset also asserts guest word0.1397/1397 native,0failed/skipped;
10/10non-routeCTestPASS62.76sec(native60.78).95879/58249 terminalPASS.
Logs /tmp/pe-counter-ram-{release-build,debug-build,ctest,lasttest}.log.
No production changes after final passing checks. Subsequent optional route
capture logger also builds in Release/Debug full builds:
/tmp/pe-counter-window-final-{release,debug}-build.log.
[FIELD_FRAME_COUNTER_RAM.md](FIELD_FRAME_COUNTER_RAM.md) proof.

Fresh coldboot27752 terminalframe60250/frame-limit, three victories repeat
52111/53823/57791. Capture pe-m34-pistol-counter-connected.bin in
pc_port/build/day2-victory-evidence, SHA256
0a089c25504a18693bda7abe8a66892a930c84d9b09ff4d6af6e2fdabaafaa98;
guestcounterEB54. Pistol children alreadyretired; source35558probe40866
6381PCs, noneofsixwritten. /tmp/pe-m34-pistol-{counter-connected.log,writers.json}.

Secondcoldboot25934 terminalframe60271 existingF434unresolved boundary;
three victories repeat, optionalroute50/57. Read-only present-hook RAM window
60200..60270,71complete2MBfiles, directory
pc_port/build/day2-victory-evidence/m34-pistol-window.
New env PE_ROUTE_RAM_WINDOW +_BEGIN/_END in test_route_boot_day2.c onlyreads
RAM; capture phase insidepresenthook BEFORE remainingframe-tail. No restore.
Log /tmp/pe-m34-pistol-window-connected.log. All71 original independent
counter/mailbox/field probes PASS sourcechecks (56889 terminal), using
pe_m34_effect_stack_probe.py --frame --counter-prefix --watch-entry-sp801FEEE8.
Reproducer pe_m34_window_stack_probe.py DIRECTORY. Reportinwindowfolder
independent-field-writers.json SHA256
cd14f9e26c57c832fc1e8561e23f9d30ac7a4946d4613bd020fcae4bf5a971be.
Log /tmp/pe-m34-window-writers.log. Independent copied states, supplied initial
CPUstack/GTE, omittedinput/outerdraw/presentation; NOT continuousretailhistory.

KEY NEW WRITER: originalfieldcallfromcapture60260 executes
184EC ->2FAF8 ->6DCE4 ->6DED4 ->6DFA8. Allthreepreviouslyunknownwords:
F434entrySP-76=volume87, writer6E184 at801FEE9C;
-72=savedcallerS0=800A5D5C bossbody, writer6DED8 at801FEEA0;
-68=savedcallerS1=800BF490 bossactor, writer6DEE0 at801FEEA4.
6DED4entrySP801FEEB0,RA6DD28; args package8018EFE8,sound4D7,group0,
XYZ(-2004,505,-217). Native spatial_sound in func_8006DE80_port.c computes
volume already; do NOT substitute87 or duplicate sound request. Detailed
6690PC register trace /tmp/pe-m34-spatial-stack-registers.json; capture60260SHA
202b8af8828eff8a508189dcf6f9c51c58409759f47f6fab18758d250c407942.
Independent60261..269calls writenone;60270reachesF434 andonlywriteslastthree:
-36=80010690 savedVM S1 by6916C; -32/-28=0 commandfifth/sixthF0C8/F0D0.
CurrentM34commandnativeaccepts4args, needsfaithfulextraargumenttracking.
[M34_RETAINED_STACK_WRITERS.md](M34_RETAINED_STACK_WRITERS.md) fullscope.

Earlierwindow60200..237 first3writers8CBAC/8CBD8/8CBD0 (sounddispatcher saves
S1=0,S2=128,RA86684). Casinglateronlywriteslasttwo. SyntheticsourceC9FD8at
weapon drawdepth(root35558SP-192) showsmuzzlescale78D0C/78D4C canwrite-72/-68,
but theseareNOTlatestwindowwriters. /tmp/pe-m34-synthetic-{muzzle,casing}-stack.json
1006/704PCs. Originalouterdraw3F4F8..3F590old60270capture:588PCs,no6wordwrites;
/tmp/pe-m34-outer-draw-stack.json; excludespresentation,oldcounterincomplete.
Priorcounter-prefix100callprobe10796 terminaldidnotreachF434 (11789PCs), not
proof oforiginalhistory. /tmp/pe-m34-counter-history-error.log.

NEXT bindobservedscript64 spatialcuecontext(volume, savedbody/actor) andall
sixretainedvalues, verifyinterveningcalls preserve/overwrite, invalidateunknown
contexts. Do not guess tuple orclaimindependentframeprobesprovefullhistory.
F434 staysUNWIRED; 970effectwords translated and303childrencomparisons remain
validpriorproof. Laterprojectilescouldhaveotherwriters. No externalimpasse;
keepgoalactive. EXEsremainbyteidentical SHA1452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

## PREVIOUS 2026-09-13: M34 movement/drawing/collision verified; stack binding next

Goal ACTIVE. Previous turn and this turn made verified production/evidence
progress. Full Day2/whole-route fidelity remain UNPROVED. All handles terminal,
no pending questions. Preserve dirty tree; no matching src/asm/YAML/index edits,
staging, commits, checkpoint restores or gameplay RAM injection.

New production: m34_boss_effect_port.c now translates F830..FC54 (265 words),
FC54..FDD4 (96), FDE4..FEE0 (63), registered in PE_M34BossEffectChild with
existing overlay guard. All M34 effect words F00C..FF34 (970) now translated;
F434 STILL UNWIRED because retained stack history is unresolved. New public
PE_EffectPointQuadC61A8 in func_800CEB8C_port.c translates complete C61A8
(77 words) plus C653C (18), using existing triangle helper twice. F830 keeps
five original quads, first-image XYZ-minus-VX behavior, later Y overrides,
hit flags and owner classification. FC54 retains signed-byte timer modulo,
both polygon calls, trail allocation, wrapping motion/fade and retirement.
FDE4 uses full existing C42A4 billboard renderer. No effect callback is skipped.

pe_m34_children_oracle.py --build-dir /tmp/pe-day2-release --write-header:
303 original/native cases PASS,1540 uniquePCs, full RAM<1FE000 +1024 scratchpad
bytes compared, original stack excluded. Original executed/following words
verified; all original compared-RAM changes inside regression hash ranges.
132 movement,64 trail draw,107 main draw cases;31 hits; main packets0/4/5.
No callee mocks. Not all GTE register/flag/timing or rasterized pixel proof.
Generated retail_m34_children_cases.h 17177 lines (run-compressed common words,
sparse patches); test_m34_children.h included/called by native suite.
Initial movement/trail oracle196cases917PCsPASS; first standalone harness
compile omitted game_port.h, fixed include before comparisons. No production
correction needed after comparisons began. Final oracle52508 terminalPASS.
Full evidence [M34_PROJECTILE_CHILDREN.md](M34_PROJECTILE_CHILDREN.md).

Release/Debug full builds PASS.1397/1397 native,0failed/skipped;10/10non-route
CTest PASS61.11sec (native59.08).68132 Release/CTest and91476 Debug terminalPASS.
Logs /tmp/pe-m34-allchildren-{oracle,tests-build,debug-build,ctest,lasttest}.log.
Earlier build52865,12929 and movement oracle38766 also terminal. No production
changes after final checks. git diff --check and Python syntax PASS.
Retail/candidate EXE SHA1 unchanged452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

No new connected replay: source-bound F434 dispatch remains preceding stop at
60271. Need recover first three retained words at F434 entrySP offsets
-76/-72/-68 and bind all six using faithful context. Prior full-frame35558
trace from60270 proved only same-frame -36 saved VM S1=80010690 and -32/-28
command fifth/sixth arguments. Do NOT substitute guessed zeros. Earlier
3F404..3F4F8 history probes hit graphics DMA wait (interpreter peripheral limit),
and100 bare35558 calls from60197 did not reach F434. Do not repeat unchanged
histories or mock hardware silently. Details/captures in next historical entry
and M34_PROJECTILE_INITIALIZER.md. Existing M0023I writer-tracking precedent:
func_800D413C_port.c PE_EffectStack* and M0023I_FLASH_STACK.md, but those six
bytes have a different stack depth and cannot be reused as M34 values.

## PREVIOUS 2026-09-13: full F434 explicit-stack translation verified; all processes terminal

Goal ACTIVE; verified production/evidence progress. FullDay2/fidelity incomplete.
PE_M34BossProjectileInit(data,retained6) in m34_boss_effect_port.c translates
complete255wordsF434..F830. It is NOT YET WIRED TO CONNECTED DISPATCH because
sixcaller-stacktranslations mustbe suppliedfaithfully. Nozeroassumptions.
252original/nativecases+4capturedcontext variantsPASS406PCs, fullRAM<1FE000
(originalstackexcluded), originalsourcewords/delaywords checked, finalGTE
RT/TR/IR/MAC checked. Generatedcompactheader +newnative testincluded.
1396/1396native;10/10non-routeCTestPASS62.44sec, Release/DebugbuildsPASS.
Finalhandles13397oracle,86056CTest,4956Debug allterminalPASS. Logs
/tmp/pe-m34-projectile-{oracle,tests-build,debug-build,ctest,lasttest}.log.
[M34_PROJECTILE_INITIALIZER.md](M34_PROJECTILE_INITIALIZER.md) currentproof.

Freshbeforeeffectcoldboot94294 terminalframe60270 frame-limit. Full2MBcapture
pc_port/build/day2-victory-evidence/pe-m34-before-effect.bin SHA256
6e2e09c6310e60999a9ce81bea5f51e901a6d44ec3c66348694e7aba189dc584.
pe_m34_effect_stack_probe.py --frame onthiscaptureexecutesfulloriginal35558
untilF434:6674prefixPCs;entrySP801FEEE8,data801863A0. Writertracereveals
retainedoffset-36=80010690writtenby6916C(saveS1),-32/-28=0writtenF0C8/F0D0
(commandstackfifth/sixthargs). Firstthreeoffsets-76/-72/-68notwritteninframe.
Output/tmp/pe-m34-fullframe-stack.json (sourcechecks allprefix +7F434variants).
Earlierconstructor-onlyprobezeros omitted actualsameframewrites. Sourceframe
stillhas suppliedinitialstack/GTE, notcompleteoriginalboothistoryproof.

Pre-shot49736 terminalframe60190/frame-limit, firstcommandqueuedbutmenu mode1.
Capturepe-m34-before-shot.bin SHA2564a9d0ac7ccd6621247bc4bb98510855a4a995c8cd4117661d0950fad459edf48.
Original repeated35558 probe73251 hit instruction budget. Prefix3F404..3F4F8
with normal digital pad replies also hit budget (61862). Diagnostic52529
identified phase0 VSync polling:73A44 repeated49990times, lastPCs76408..77410.
Later probe46970 established graphics DMA wait, not menu-only diagnosis:
VSync caller RA800773E0, DPCR address1F8010F0. Interpreter lacks peripheral
completion/timer advancement. No production regression or replacement callee.

Committed-shot coldboot62899 TERMINAL at frame60197/frame-limit, mode0,
queue2, AT0, HP35, loaded4. Log /tmp/pe-m34-shot-committed.log, full2MB
pc_port/build/day2-victory-evidence/pe-m34-shot-committed.bin SHA256
b2e4be2c7cc78f7ece78993b5528a80140748faa2dea1f96ab4bd382413fad41.
Prefix probe30429 also TERMINAL budget failure in same graphics wait;
/tmp/pe-m34-committed-prefix-error.log. Bare35558 single call returns normally
from both60190/60197 captures (diagnostic46970). Committed inner-history34333
TERMINAL: completed100 calls without F434, then reachability assertion failed;
/tmp/pe-m34-committed-field-error.log. It omits outer frame preparation/input/
presentation; no complete original history or source-comparison claim.
Probe now reports a descriptive reachability error instead of bare assert.
No live sessions, no pending questions. No production edits after passing
1396/10checks. S1 at6916C isVM'sjtbl80010690 (17020/17024), notS0.

NEXT: recover earlier writers for retained offsets-76/-72/-68 with relevant
caller history; three same-frame writers are known but not yet bound in native
dispatch. Independently translate remaining F830 draw/hit, FC54 movement/trail,
FDE4 trail draw. FC54 calls existing1CAB0 andC2B90, FDE4 uses existingC42A4.
Original disassembly /tmp/pe-m34-effect.s. Do not keep repeating unchanged
inner-field histories or silently mock graphics/DMA to claim retail proof.
README and M34 effect evidence now link current initializer proof. Preserve
dirty tree and active goal; no external block, full Day2 still incomplete.


## FINAL 2026-09-13: M34 effect lifecycle verified; projectile initializer next

Goal ACTIVE; this goalturn made verified production/control-flow progress and
new original-code dependency evidence. FullDay2/whole-routefidelity unproved.
All handles terminal, no pending questions. Preserve dirtytree; no matching
source/asm/split/index edits, staging, commits or gameplay RAM injection.
Retail/candidateEXESHA1 unchanged452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

New m34_boss_effect_port.c:291words translated overF00C..F434,
FDD4..FDE4,FEE0..FF34. Lifecycle/commands/posecopies/timers/cleanup; 6F39C,
6F6D4,6FC18,effect/weaponcallbacks wired withM34sourceguard. 177 original/native
cases+capturedconstructorPASS534PCs; fullRAM<1FE000 anddefinedreturns, original
stackexcluded, instructions/followingwords/source andwriterangeschecked.
F3C4 allocation arm translated butnotfullycompared; leadsuntranslatedF434.
RemainingF434,F830,FC54,FDE4 explicitlyunported. Do notclaimwholeeffectcomplete.
Finalproof [M34_BOSS_EFFECT.md](M34_BOSS_EFFECT.md).

FinalRelease/DebugfullbuildsPASS,1395/1395native,10/10non-routeCTestPASS61.36sec
(native59.36). Logs/tmp/pe-m34-core-verified2-oracle.log and
/tmp/pe-m34-core-verified-{build,debug-build,ctest,lasttest}.log.
Finalhandles40977oracle,8371ReleaseCTest,90983Debug allterminalPASS.
Firstnativefixtureomittedreadonly80190020terminator, ownstuckchild2608277
SIGTERM;55409CTestfailed. FixedRANGES190020/70 whilekeepingnoiseseed190028/68;
regenerated177cases andpassedfinalsuite. Priorfixturefailuresarehistory.
No productionchangesafterfinalpassingchecks.

Connected54166 terminal60271 sameframebutnewnestedboundary:
[EFFECT] Unported weapon callback8018F434 slot801861A0 record80186226.
All3victories52111/53823/57791, M33entry58538,M34entry59996 repeat. HP35,
story6C/arrival21/tokenA8003248. NoM34pilotchanges. Optionalroute50/57.
Capturepc_port/build/day2-victory-evidence/pe-m34-core-connected.bin (full2MB)
SHA256 fe8b6a9ba4b02dc3cbb61d35faf4287a06f950acd20d13e698dc653b691c106c.
Log/tmp/pe-m34-core-connected.log. Constructor boundarypassed; F434next.

NEXT importantfidelitydependency: F434 full255words usesRotMatrix79754,which
writesrotationonly, thenreads6unwrittenstacktranslationwords atentrySP offsets
-76,-72,-68,-36,-32,-28. pe_m34_effect_stack_probe.py original-onlyconstructor
andupdatefromD1capturestopsatF434:entrySP801FEF70,data801863A0,noneof6written
bytracedsequence. SevenisolatedoriginalF434runs,911PCs,sourceschecked, each
individual1000perturbationchangespersistentdata+38matrixtranslation.
Baseline(-1280,993,-1868),offset-76->(-2373,993,-1762),offset-36->(-280,993,-1868).
Output/tmp/pe-m34-effect-stack-probe.json. This isNOTactualretailstackproof.
Do NOT substitutezeros andcallitfaithful. Need translatecompleteinitializer,
remainingchildren,andpreserveoriginalborrowedstackstate fromactualcallerhistory.
A coldbootcaptureoneframebeforeeffect60270 mayhelp originalcall-history probes;
newcaptureisjustifiedfornewquestion, notunchangedreplayexpectingprogress.
SourceoriginalM34C2=/tmp/pe-m34-c2-original.bin LBA16597/100sectors/base8018EFE8
SHA2560eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a.
FullF24C..FF34disassembly /tmp/pe-m34-effect.s. ExistinghelpersC22F8/C2414/
C251C/C2758/C2B90 available; missingcollisionC61A8 callsC653C (two existing
C62DCtriangles). HostPeEffectMatrix/PE_EffectQuadC42A4 available fordrawing.
Keepgoalactive; noexternalblock (stillconcretemissingtranslation/stackwork).

## CURRENT 2026-09-13: M34 effect core verified; corrected fixture regression pending

Goal ACTIVE; prior turn and this turn make verified production/control-flow
progress. FullDay2/fidelity incomplete. Source/newtests inM34_BOSS_EFFECT.md.
177original/native cases+capturedconstructorPASS534PCs. Core291sourcewords
translated; F3C4 allocation arm not completelyverified and F434/F830/FC54/FDE4
remainuntranslated. Connected54166 terminal60271 nowstops PE_WeaponCallback
8018F434 slot801861A0 rec80186226, ratherthanconstructor; sameframeprogress.
Capturepc_port/build/day2-victory-evidence/pe-m34-core-connected.bin SHA256
fe8b6a9ba4b02dc3cbb61d35faf4287a06f950acd20d13e698dc653b691c106c.

Original-only stackprobe completed911PCs: F434 has sixunwrittenmatrixtranslation
words atentrySP-76,-72,-68,-36,-32,-28; eachchangespersistentoutputwhenvaried.
No writers inisolated originalconstructor+update; notactualretailstackproof.
See pe_m34_effect_stack_probe.py and/tmp/pe-m34-effect-stack-probe.json.
Do NOT zero native matrices andcallitfaithful. F434 notyettranslated.

Initialoracle36344 terminalfailed fixtureprogramat+80; fixedto+78.
Correctedoracle69405PASS. InitialnativeCTest55409 child2608277 gotstuck because
generatedfixtureomittedreadonlyprogramterminator80190020; terminatedonlythat
ownchildSIGTERM. Debug9377 terminalPASS butoldfixture. Sourceproductionunchanged.
FixtureRANGESnow190020/70 includesreadonlyterminator. 21737 failed because an
initial broad replacement also filled that program with fixture noise; fixed
the noise range back to190028/68. Final40977 oraclePASS177+capture534PCs,
log/tmp/pe-m34-core-verified2-oracle.log. FirstCTest55409 reapedexit8 (terminated
test), notpass. LIVE8371 Release rebuild+CTest /tmp/pe-m34-core-verified-build.log
and/tmp/pe-m34-core-verified-ctest.log; LIVE90983 Debug finalrebuild
/tmp/pe-m34-core-verified-debug-build.log. Poll thesehandles; others terminal.

## FINAL 2026-09-13: D1 verified; M34 battle reaches effect 8 constructor

Goal ACTIVE. This turn produced verified production and connected progress.
Full Day 2 and whole-route fidelity remain unproved. All handles terminal;
no pending questions or running work. D1/80019D84 full13words translated and
VM dispatch wired; existing375E0 unchanged. 160 original/native cases plus
captured D1 PASS,91 PCs, complete callee/no mocks. Original stack excluded;
native temporary120F20 preseededFFFF and included in comparedRAM<1FE000.
[Full evidence and next work](MESSAGE_WINDOW_D1.md).

Release/Debug builds PASS;1394/1394native,10/10non-routeCTest PASS68.99sec.
Logs /tmp/pe-d1-{oracle,tests-build,debug-build,ctest,lasttest}.log.
79922 and88690 terminalPASS. Retail/candidateSHA1 both unchanged
452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching/source/split/index edits,
staging, commits, gameplay RAM injection or connected checkpoint restoration.

Connected29027 terminal exit1 at60271, stop=unresolved-boundary inM34.
D1 message finishes, mode0at60111; normalattack60188/60196, loaded4->2.
Stop func_8006F39C_constructor; story6C/arrival21/tokenA8003248. Threevictories
repeat52111/53823/57791; M33entry58538/M34entry59996. Same inputpilot, noM34
adaptation. Optional historical supply milestones50/57, not complete route.
Log /tmp/pe-d1-connected.log. Complete2097152bytecapture
pc_port/build/day2-victory-evidence/pe-d1-connected.bin SHA256
c0917383343f485315fd135ea17797fecb0e0bd74ffced88c9bf4b387cc935e9.

NEXT concrete missing: effectcode8, slot0at801861A0, userdata boss800BF490,
descriptor8018FF7C via table80094188. Constructor8018F00C..8018F0B8 (43words)
calls existingfunc800C22F8, assigns8018FF98table, initializes overlaydata80190054..8019006B.
Descriptor7words:8018F244,8018F00C,8018F0B8,8018F0E4,8018F12C,8018F1B8,8018F23C.
Fullcallbackfamily NOT EDITED. Need native translation and fullsource oracle.
Authoritative M34 C2 Disc1LBA16597,100sectors,base8018EFE8; SHA256
0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a.
Original extracted /tmp/pe-m34-c2-original.bin. Constructor43words SHA256
3fb78435b95216a969a5cbfbaff4155cce08c8fe67a5126b8b556fb44cd89b8c,
all matchcapture. Existing 6F39C has explicit knownconstructor dispatch; add
source-guarded M34 handling, then effectupdate/command dispatch as required.
Do not rerun unchanged route expecting progress. PriorLIVEentriesbelowhistory.

## CURRENT 2026-09-13: D1 translated; differential PASS, replay running

Goal ACTIVE; full Day 2 and whole-route fidelity remain unproved.
D1/80019D84 full 13 words implemented and dispatched in17018_port.c; existing
375E0 callee unchanged. 160 synthetic original/native cases plus captured D1
PASS,91 instruction PCs; `/tmp/pe-d1-oracle.log`. Generated fixture and native
test wired. [MESSAGE_WINDOW_D1.md](MESSAGE_WINDOW_D1.md) records proof bounds.
Release full initial build89824 terminalPASS; oracle19917 terminalPASS.
LIVE 79922 Release test rebuild then10non-routeCTest, logs
/tmp/pe-d1-tests-build.log and/tmp/pe-d1-ctest.log.
LIVE 88690 Debug fullbuild /tmp/pe-d1-debug-build.log.
LIVE 29027 fresh68000frame coldboot /tmp/pe-d1-connected.log, capture destination
pc_port/build/day2-victory-evidence/pe-d1-connected.bin. Same prior input pilot,
no M34 battle additions yet. No gameplay RAM injections/checkpoint restores.
Poll these existing handles; do not duplicate runs without concrete reason.

## FINAL 2026-09-13: M34 boss encounter reached; next native boundary D1

Previous/current goalturn is verified production and connected progress.
Goal ACTIVE; full Day2 and whole-route fidelity remain unproved. All handles
from this turn are terminal; no pending questions or live processes.
Source taskpause18300/18364 full58words:518cases+capturePASS143PCs.
Release/Debug builds,1393native and10/10non-routeCTestPASS71.72sec.
No production edits after these passing checks; M33controller changed only.
Original/candidateEXESHA1 unchanged452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

Final connected31229 terminal at60096, actualunresolvedboundary inM34:
`[VM] unported script opcode fn=0x80019D84 at pc=0x801B5E0C actor=0x800BF490`.
Story6C, arrival21, tokenA8003248, battlealreadyinitialized. All3victories
repeat52111/53823/57791, M33entry58538, scene starts aftermovingwestpast-530.
Log /tmp/pe-m33-forward-connected.log. Full2MBcapture
pc_port/build/day2-victory-evidence/pe-m33-forward-connected.bin SHA256
2d654c4dd8f000bbdc9b3cbc51f9fa02245229c86f968f25e736e0e4dbc33e70.
No gameplayRAM injection or checkpointrestore; capturesonlyisolatedcomparisons.
Oldoptional57milestonecheckstill50/57, notcompleteDay2regression.

NEXT: opcodeD1 wrapper19D84..19DB8(13words), afterD0 at801B5DF4 setsgeometry.
Originalasm: signedlh *arg0; stacklist{-1}; jal375E0(id,1,list);return1.
ReachedID8 atscript801B5E14. Existing375E0 native full161words in
func_800375E0_port.c; existingopcode0D func17410 mode0wrapper in17018_port.c
usesGA_375E0_LIST(80120F20). D1 NOT YET EDITED/VERIFIED. Use original fullcallee
comparison andcapturedcontext; thennextordinarycoldbootpastmessage/boss.
D1source dump command addresses19D84..19DB8, existingasm8B00.s is unrelated
(taskpause); locateD1originalviaEXE orasmA4*. No needredo passedtaskpause work.
[SCRIPT_TASK_PAUSE.md](SCRIPT_TASK_PAUSE.md) contains currentproof andlinks.

## CURRENT: task pause/resume verified; M33 movement replay running

Goal ACTIVE. This turn has verified production and connected progress.
Full original18300/18364 (58words),518original/native cases plus captured
suspend/resume pass;143PCs,fullcomparedRAM<1FE000,originalstackexcluded,
source/delaywords andalloriginalwrites checked. Firstcapturelookupfailure
fixed (active task ischainA8). Finaloracle43205 terminalPASS.
Release/Debugfullbuilds pass;1393native and10/10non-routeCTestPASS71.72sec.
CTest49441 terminal; Debug95181 terminal. Stablelogs
/tmp/pe-script-pause-{final-oracle,ctest,lasttest}.log.
Details [SCRIPT_TASK_PAUSE.md](SCRIPT_TASK_PAUSE.md).

Post-port replay11490 terminal at66000, no unresolvednativecallback, M33
openingdialoguefinished; Ayaentrance becauseoldpilotCrossonly. Capture
pc_port/build/day2-victory-evidence/pe-script-pause-connected.bin SHA256
c5ce742fe02ba8cf91a8a06a9256e04138f2ded44e356e2af9cc2695c3922348.
Controller nowM33target(-1000,0) plusCross; originalmodule3trigger
x[-4000,-530],z[-900,900] sendsAya24 at801CE904, leadingmessage15 and
actor1mailbox11,13,15,27 flow towardM34. M33exit fartherwest leadsM359,
so stopping at-1000 seeks the storytrigger without running tothat exit.
**ONLY LIVE session31229**, fresh68000run /tmp/pe-m33-forward-connected.log,
RAMto pc_port/build/day2-victory-evidence/pe-m33-forward-connected.bin.
Poll samehandle; do notduplicate/restartuntilterminalorconcretefault.
LatestDebugroutebuild /tmp/pe-m33-forward-debug-build.log.
No productionchanges sincepassedtests; onlyM33normalinputcontrollerchanged.

## SESSION 2026-09-13: M33 reached; original task pause/resume translation

Gate control works via southern approach: M33 entry58538; actual stop58549,
opcode5F/function80018300 at801CDAFC (explicit VM unresolved-boundary).
Capture `pc_port/build/day2-victory-evidence/pe-m32-control-connected.bin`
SHA256 f79b6d7b0f4d5b78b9bf1d6cb3ec01b447d46c37fafc28f1f230cfc0b140a5f9.
Gate persist22=7. Previous initial inspection called it a wait; actual log
contains explicit `[VM] unported` plus stop=unresolved-boundary.

Translated full original18300(25words),18364(33words) in17018_port.c, dispatch
and header wired. Pause preserves currenttask and flagsothers40 acrossall3
chains; resume clears40 even savedPC0, otherwise restoresPC/delay1/clears20.
PhysicalRAM accesses supported. Original58wordSHA256
605f5b21d57821f8f56477f5bdf4338132ab895d9345735a23e9b2dcb4527ca0.
No matching source/YAML/index changes. Production Release route build passes.

New `pe_script_task_pause_oracle.py` compares allRAMbelow1FE000 andreturns,
checks executed/following instructionbytes and coverageofalloriginalwrites.
518syntheticcases include fullVMfollowingwait; first run passedthese but
capturefixtureassumedwronghead(A0; actualA8). Corrected lookupwalksallchains.
**LIVE oracle session43205** with corrected capture lookup.
Final log `/tmp/pe-script-pause-final-oracle.log`, writesretail_task_pause_cases.h.
New test_task_pause.h included/calledtest_native; fullbuild/tests pendingheader.
**LIVE connected session11490**, coldboot66000: /tmp/pe-script-pause-connected.log,
RAMto pc_port/build/day2-victory-evidence/pe-script-pause-connected.bin.
Poll both existing handles; no duplicate runs. Continue actual M33 frontier.

## SESSION 2026-09-13: original M32 floor comparison and control approach

Previous turn was verified production and connected progress; goal remains
ACTIVE. Full Day2 and whole-route fidelity remain unproved. No agents,
matching-source/YAML/index edits, staging, commits or gameplay RAM injection.
Original/candidate EXE authority remains SHA1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

`pe_live_floor_compare.py` on the full gate capture passes all16 original/native
steps (eight directions, retained/cleared cached wall), all RAM below1FE000,
original stack excluded; executed and following instruction words verified.
Log `/tmp/pe-m32-gate-floor-compare.log`. No native floor change needed.
Mesh8019FC20 has90 2D vertices and104 triangles. The captured triangle91
shares a sloped corner at vertices59(-16928,3578),58(-16831,3698); Aya radius65
accounts for stopping outside the control area. Triangle90 to the south/west
is open. An isolated original-only 1AE40 path reaches(-16965,3468) via south3300
thenwest-16970 thennorth3500, inside the script control rectangle. This is
an isolated geometry probe, never a connected checkpoint restore.

Pilot now crosses west atz3300 (stage threshold3260), then approaches north
atx-16970 towardz3520. All battle inputs remain unchanged. **LIVE session33718**
coldboot64000: `/tmp/pe-m32-control-connected.log`, RAM destination
`pc_port/build/day2-victory-evidence/pe-m32-control-connected.bin`.
Release build passed. Poll this same live handle; do not duplicate/restart
without a terminal result or concrete fault. Next inspect control/gate/M33.

## SESSION 2026-09-13: victory handler restored; all three sewer encounters won

FINAL connected result: all processes terminal, no pending questions. Third
victory explicitly observed57791 HP35, two type6 bodies spawned then retired,
mode9andfieldflags. Full2MBcapture at
pc_port/build/day2-victory-evidence/pe-victory-gate-connected.bin SHA256
c9c4612dfd6125e4c2b1ad5fa1e1fda6abf13ac73f237b7a02d3e423debe18cc.
At64000:m32,HP35/max64,XP24,internallevel3(display4),BP28,reserve36,loaded4,
pistolslot0,flags4080,mode9,focus0; onlyAya hasbodyinactorlist. No unresolved
nativeboundary,50/57oldoptionalsupplymilestones. Gate remainsclosed:
persist22=6,2B=2, Aya(-16866,2097,3548),heading1536. Pilotstage1 westward
motion collidesjustoutside controlrectx[-17050,-16900],z[3400,3650]. Need
derive valid approach from originalfloor/control script, then followm33.
Do NOT rerununchangedpilot expectingnewresult. Rotationdiagnostic nowreads
correctAya+3A (thecompletedgate log readunused+64 and printed0); production
andcontrollerinputsunchanged by that final logging fix.

Previous goalturn was verified implementation/connected progress. GoalACTIVE;
fullDay2/whole-route fidelity unproved. No matching source/YAML/index edits,
staging,commits,restores or gameplayRAM injection. Preserve dirtytree.
2B0E8 now calls703F4 before4B70C,67CBC after4B70C, andFULL295E4 inphase3.
Phase2 halfwordcomparison no longertruncates; readsactualRAM flags; postcallee
phaseincrements reloadoriginalbyte. PhysicalRAM actoraccess restored and
295E4 actor+194store normalized. Existingdeath-tailhelper remainsothercalls.
109wordhandler fulloriginal/native comparison:83cases+isolatedcapturedphase,
1470executedPCs,allcomparedRAM<1FE000,source/delaybytesandwriteranges checked.
Generatedretail_battle_victory_cases.h/test_battle_victory.h includedinnative.
Log /tmp/pe-victory-final-oracle.log. [Evidence](BATTLE_VICTORY_HANDLER.md).

Finalregression:1392/1392nativePASS,10/10non-routeCTestPASS69.82sec
(native67.47). Logs /tmp/pe-victory-verified-{tests-build,ctest,lasttest}.log.
Oldmode2fixturesneededinitializedempty effectpools; reward_seed_resources
nowprovides2pools (no gameplayinjection). InitialCTestabortedBTL103phase0
becausepoolsuninitialized; correctedBTL10/BTL11filteredDebug43+31passed,
thenfinalReleasebroadPASS. Debug /tmp/pe-victory-debug-test-build.log passed.

Fresh66000run /tmp/pe-victory-forward-connected.{log,bin} repeatsbothvictories,
m31entry54520,50/57oldmilestones,no unresolvedboundary. M31HP33/maxHP53,
XP16,level2,BP9,reserve36. CaptureSHA256
f2729282269ffcca53938116d3996ccaa6b0b817e4a7229a349c7d65de0c17a4.
Navigationwaitsunchangedat(170,1048,-950). Secondcenteringrun80650alsofinished
samewait /tmp/pe-victory-center-connected.{log,bin}. Diagnosisfromoriginal
m31script:actor2waitsmessage10 at8019E600, thenmessage11 beforecontrol eventFF.
Pilotwithneutral/Crosssuppressed onm31couldnotadvance; notafloorcodeboundary.

Dialoguehandle41878 terminal: message10/11finish, receivesFF, movesx35 but
controllerdeadzone40 conflictedwithcenteringthreshold30. Navigationhandle93237
also terminal: reaches x18,z967; z980 threshold conflicts with40-unit arrival
tolerance. Latest explicit M31 stages consistently use40-unit tolerance.
Waypoint handle55742 terminal: M32entry55479, firstencounter55831, normal
pistol equip56021, Heal1 raisesHP21->51 at56336 and11->41 at56863. Aya dies
57189; restarts57398. No unresolvednativeboundary;50/57oldoptional milestones.
Capture /tmp/pe-victory-waypoints-connected.bin SHA256
c04f4b1efb78eaa4a1a7de9eedafff7a4a003ec0f8614f478e01681e497a750f.
Ranged handle9312 terminal: wider spacing without wall avoidance loses56943,
at(-16280,3060); two type6 enemiesHP16/34. Diagnostics now select actual living
bodies from9E000 rather than unrelated type2 script actors, reporting all M32
enemy positions/HP every60frames. /tmp/pe-victory-ranged-connected.{log,bin}.
Nearest handle9907 terminal: living Aya25HP, two enemies34HP, battlepaused
mode1/queue0 from56199 through62000. Capture focus800A2640 is Items sublist3,
parent list1, parent mainlist0 row0. Healcontroller blindlyconfirmed unexpected
focus instead of selecting PE. Capture /tmp/pe-victory-nearest-connected.bin
SHA2565fca56497e4bcc2b101123d0bda76447782680482a785bea32fe85cb99689589.
Heal-menu handle53833 terminal: recovers from Items3->Items1->root0 row1->PE8,
then waits atPE confirmation41 from56291. Pilot now confirms41 only when
originalconfirmationcallback CFA8==80046DBC, selectsfirstcolumnYes.
Heal-confirm handle80522 terminal: log shows both enemies defeated57631,
mode2victory57658, fieldmovementby57840 with35HP. Reaches(-16362,3590),
but gateclosed. RAM capturefailed (zero-bytefile) due/tmpquota, logalsohasgaps;
do NOT use thatcapture orclaim fullendpointproof. Eight earlier complete
victorycaptures copied/hashverified to pc_port/build/day2-victory-evidence,
then /tmp paths replacedwithsymlinks (16MBfreed, evidencepreserved).
Original M32script /tmp/pe-victory-m32-script.txt: module3 left control rect
x[-17050,-16900],z[3400,3650], Crossedge100 and facing1500..2600, message13
choice0 sendsactor0 event0; its async8019BB14 path sets persist22bit1 and
activatesoriginalgatepolygons. Door module5 rectx[-16600,-16100],z[3333,3636]
can publishmessage12 ifnotopened. Module4forwardexit remainsz>=3650.
Gate handle47333 terminal; its fresh64000run keeps nearest-enemy steering,
retreat800/circle950 and wall avoidance x[-16000,-14000],z[1500,2700]. M32heal
now selects mainrow1 thenPElist8 row0/col0; cancels unrelated Items/equipment
submenus viaCircle, neutralwhileothertransientfocus. LogsHEAL_MENU id/row/pad.
/tmp/pe-victory-gate-connected.log; RAM destination is
pc_port/build/day2-victory-evidence/pe-victory-gate-connected.bin.
Adds read-only thirdvictory observer: peak2 type6 bodies, bothretired,livingHP,
mode9andfieldflags; retains firsttwo via bitmask. Postvictoryifpersist22bit1
clear, goes(-16400,3500),(-16970,3500),thenfacesnorth+Crossat(-16970,3600).
Aftergatebit resumesoriginalforwardexit. Source only changes controller
and read-only diagnostics since83-case proof/full1392+10/10regression.
Weaponmenunowmoveseitherdirectiontowardtargetrow(pistol0/baton1), respecting
rememberedselection. M31centersx0 thenz1000 thenwest-1500. M32west-16400thenz3800;
battlepilotselectspistolwhilebulletsremain,950-distancecircle/attackandHeal1.
M33nowCrosspulsesforinitialdialogue, butnotreached. Allhandles terminal.
FinalRelease/Debugroutebuilds pass; /tmp/pe-victory-final-route-builds.log.
No productionchanges since83-case/full1392+10/10regression.

## SESSION 2026-09-13: level animation restored; both sewer victories and junction

Goal ACTIVE; full Day2 and whole-route retail fidelity remain unproved.
Six complete original level-draw bodies /1022 words now run natively:
4BF40,437B4,5BA78,6062C..61044,4FFF8,50D20. Callback dispatcher/header updated.
183 original/native cases pass,2526 executed PCs. A captured80-frame full-draw
sequence and confirmation passes,3488 PCs, internallevel2/maxHP53/BP9.
Source/delay bytes and full compared RAM verified; stack excluded.
Generatedretail_battle_level_cases.h +test_battle_level.h included in native.
Release/Debug pass;10/10 CTest excludingoldroute pass70.46sec;1391 native pass.
Original/candidateEXESHA1 unchanged452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
Details: [BATTLE_LEVEL_ANIMATION.md](BATTLE_LEVEL_ANIMATION.md).

Fresh coldboot /tmp/pe-level-connected.{log,bin}: field-control victory
**52111**, HP27; reserve0->18 by52080. Framecap57000, m27,Aya(-774,773,-1826),
mode9,XP8,level2,maxHP53,BP9,reserve18. Oldtimedsuffix openedmenu;46/57milestones,
no unresolvedboundary. CaptureSHA256
1a5dd130bd66f894d510590900c106d2077fc41c1d8a91b9c252ffe979d7a5f9.

Controller nowinrepo tests/route_reward_sewer_pilot.h, optin
PE_ROUTE_REWARD_PILOT=1. Normalinputs only; no gameplayRAM writes/restores.
Updatedtofightbothm27/m28 andwalk originalm27exit(0,-1250),m28exit(0,450),
closingmenus viaCircle. M31 neutraluntilnextnavigation. Session99243 terminal:
**secondsewervictory53823 HP33; m31junctionentry54520**. Atframecap60000,
no unresolvedboundary,50/57oldmilestones,story68,persist1=1C,tokenA80030C8.
Aya(170,1048,-950),HP33/maxHP53,XP16,level2,BP9,reserve36,clubslot2,
flags4000,mode9,focus0. /tmp/pe-level-forward-connected.{log,bin}, SHA256
f31641d79758f7b262f5a28bdff8d6ccca3e5eb01506de890698f8541e73c40e.
Pilot sourceSHA256bf20be01d5af60b522a19c76cfa2cd39dabf1fcc6c5810e3dcfb4a978284c48a.
Alloracles/builds/CTest/replays terminal. Debuglatest /tmp/pe-level-debug-final-build.log.
No pendingquestions orliveprocesses. Nextcontinuecorrectedm31/forwardroute.

Read-onlyaudit:2B0E8 phase0 stillsilentlyomits703F4(effectcleanup) BEFORE4B70C
and67CBC AFTER4B70C in func_8002AA98_port.c:431. Bothcalleesalreadytranslated;
mustrestorewith independentfullhandlercomparison. Originalspan2B0E8..2B29C,
109words, dumped /tmp/pe-victory-original.txt. Phase2 alsoincorrectlynarrows
aya+1A halfwordtobyte. Thisturn didnotalterthatcode. Do notclaimwholevictory
handler/runtimecomplete. Original703F4 cleanupcode in func_80020F18_port.c;
existing scripted-exit oraclecoversitscallees and effect-pool failures.

## SESSION 2026-09-13: loot restored and first sewer enemy defeat verified

Goal ACTIVE; full Day 2 and whole-route fidelity remain unproved. Preserve the
large dirty/staged tree; no matching source, YAML or index changes this turn.
Loot details and exact evidence: [BATTLE_LOOT_SCREEN.md](BATTLE_LOOT_SCREEN.md).
Ten original complete bodies /758 words restore commands, renderers, manual
swaps, Take All and equipment finalization. The D_8009D03C host/guest split is
fixed with a guest-RAM lvalue; real cold boot now collects item1/ammo6, XP2.
198 original/native cases,4145 executed PCs, and corrected capture sequence
3437 PCs pass full compared RAM. Release/Debug and10/10 non-route CTest pass;
1390 native tests. Logs /tmp/pe-loot-verified-{ctest,lasttest,debug-build}.log.

Old fixed route loses first sewer battle and later hits a restart menu at61593;
this is not the story frontier. Adaptive normal controller inputs in
/tmp/pe-loot-equip-pilot-route.c switch from empty pistol to baton, defeat
enemies at51980, and stop at **actual level draw8004BF40 frame52005**, m0027i,
story68,HP27,XP8, level0->2,maxHP45->53,BP0->9,pendingthreeID1items.
Capture /tmp/pe-loot-equip-pilot-connected.bin SHA256
47556529f1ea97f14129e476eac5f1f61d33cb694ea4b2b50191c6eb1bdeb073.
No connected RAM injection or checkpoint restores. Level drawing implementation
is now in progress; next evidence must independently compare it, then cold boot
the adaptive controller again. New full bar disassembly /tmp/pe-level-bar-original.txt
and /tmp/pe-level-bar-tail.txt:6062C..61044 (646 words).

## SESSION 2026-09-12: reward omission fixed; earlier loot frontier exposed

Goal ACTIVE, full Day2 and whole-route fidelity unproved. This turn is verified
implementation progress despite the earlier connected stop: the old reward
handler silently skipped XP and items. No agents, matching src/YAML/index
changes, gameplay RAM injection, commits, staging or restores. Preserve the
large dirty tree. No questions pending. No live build/replay processes remain.

**Critical finding:** Previous winning captures (M28, M31, supplies) all have
C0E00 XP=0 and C0E0A level=0. `func_800299CC_port.c` held explicit reward cuts:
fake window8010B000, ignored BP/items, no XP store at4BC54, no level/loot branch.
Do not preserve these bugs to keep the old route green. The effect on later
combat must be established from a corrected connected replay, not assumed.

**Production changes:** new `pc_port/game/boot/battle_reward_port.c`, 17 complete
new/replaced entry bodies,854 original words:4B70C/B90C/B970/BB80/BC80/BCB4,
4C1E0/C4B4/C520,51DF8,52764,5382C,55668,57ECC,5B8A8,63D78,48654.
Restores realwindows, scoretick, XPpublication, bonusaccumulation, levelcommit
(NINE halfwords despite SEVEN setup fields), PEunlocknotification, free-slot
search and lootwindowconstruction. Uses existing HostOut adapter for stacktemps.
Existing4BE4C/BF08 are reused. Oldthree rewardcuts removed;62CB8/62CC4 preserved.
CMake,header,input/drawcallbackdispatch registered for implementedcallbacks.
**Not complete:** leveldraw4BF40 +437B4/5BA78/6062C graph; lootrenderers
4FD68/50BE8,4FCF8/50B94; input48838/58030/58454/58670 and dependencies.
The existing dispatcher explicitlystops these callbacks. No replacementnoops.
2B0E8 itself still documents deferred703F4/67CBC; audit that separately too.

**Oracle:** `pc_port/tools/pe_battle_reward_oracle.py --compare-native
--build-dir /tmp/pe-day2-release --write-header`:240 cases,19entries (includes
existing4BE4C/BF08),2500 originalexecutedPCs,PASS. Sourceinstruction/delayslot
bytes checked; fullRAM below1FE000 compared (originalstackexcluded); ranges
coveralloriginalwrites; returnvalueschecked. No patchedcalleecontracts.
FixturesvaryXP/overflow/BP/loot/scoreconfirm/levelcommit/unlocks/inventoryruns/
cursorbounds. Generated`retail_battle_reward_cases.h`, wrapper
`test_battle_reward.h`. OldBTL103/112/113/114 phase0tests seededwithreal
resources; fakeaddressassertions replacedwithid20/callbacks; XPassertadded.
Log `/tmp/pe-reward-oracle.log`.

**Build/check result:** Release and Debug PASS. Afterupdatedtestbinaryrebuilt,
10/10CTest EXCLUDING route PASS62.61sec;1389/1389nativePASS60.25sec.
Logs `/tmp/pe-reward-{tests-build,debug-build,ctest,lasttest}.log`.
An earlier prematureCTest invocation used staleoldtests whilecompilewaslive;
its invalidresult was superseded by the completedbuild and full10checkrerun.
Do notclaimall11green: routefailsasdescribedbelow. Pythoncompile/diffcheckpass.
Original andcandidateEXE SHA1 both452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

**Current connected frontier (authoritative):** unmodified62000/3158pad route
stops **frame19301, m0013i/A80011C8, story40,persist1D, missingdraw8004FD68**,
27/57milestones (nextreturnstory48). Input4BB80 has committed **XP2**; level0,
maxHP45,BP0, pendingitemcountD078=1. Realid12/13/14windows plusid19help exist.
FocusA2370,id12list. D304=2,CFE8=CFEC=2,CF60=CF6C=0.
Capture `/tmp/pe-reward-connected.bin` SHA256
c5c44cfcb0c96e9ed03064da664733587927fdacc6c9cf8c2f68665dce674da6.
Read-onlycapture replaylog `/tmp/pe-reward-captured-connected.log`;
initialsamefrontierlog `/tmp/pe-reward-connected.log`. Bothterminalexit1
becauseexplicitunresolvedboundary, notgoalblocked.
PendingremappedA1FD4[0]=519 (0x207); originalA7FF0 halfwords
[1,FFFF,0,FFFF,0,FFFF,0,FFFF]. Do notassumeFFFFquantityisabug: traceoriginal
lootprocessingfromthiscapture. Currentfixtureitems6/7 exercisegenericitems;
actualcapturedremapcaseisnextusefuldifferentialcontext.

**Next action:** implement reached4FD68 /50BE8 andotherlootrenderers, then
actual48838/58030/58454/58670 selection/transfer/endgraph. Extend original/native
oraclewith capturedlootcontext. Complete leveldrawing beforeclaimingthatgraph.
Reruncontroller-onlyroute; revise fixedinputs/outcomes from real progression.
Oldm32difficulty cannotbe treated as gameplay-only while rewardgraph incomplete.

**Read-only original dumps ready:** `/tmp/pe-reward-original.txt`
4B70C..4BCB4; `/tmp/pe-reward-level-original.txt`4BE4C..4C34C;
`/tmp/pe-reward-helpers-original.txt`;`/tmp/pe-reward-more-original.txt`;
`/tmp/pe-reward-draw-original.txt` (some6062C andother tails incomplete).
`/tmp/pe-loot-original.txt` ranges48838..48920,4FCF8..4FDE8,
50B94..50C08,57ECC..58720. Readbefore re-disassembling.
SourceEXEverified; objdumpenv/commandsinolderhandoff.

**Earlier exploration finished:** circuit69295 andescape70231 terminalexit1,
72000framesbothendstory9/A8000148afterdeath. Circuitdeath63891; escapeuses
normalcommand409 (queued63317 and63573) andstillendsrestart.
Do notcall eitherlive or successful. `/tmp/pe-m32-{circuit,escape}-connected.*`.
No productionchange camefromthosefailedpilots. Currentrewardfixsupersedes
usingtheirunchangedXPstateasproof. Read-onlypostM32scriptdecode prepared
`/tmp/pe-post-m32-rooms.txt` m0033i,m0034i,m0035i,m0036i; decodeddoesnotmeannative.

**Docs current:** `BATTLE_REWARD_PROGRESSION.md` has854wordspans/hashes,
240caseproof,routefrontier/checklimits. `pc_port/README.md` and
`DAY1_SEWER_MOVEMENT.md` nowexplicitlyretire theolder62krouteascurrentproof.
Itsinputsanddetailedhistoricalisolatedevidenceremainintreebelow.

## SESSION 2026-09-12: A9 chest selection verified; supplies route extended

Goal active. Previous turn classified as verified progress (M32/parent poses,
supplies). This turn implements an actual missing runtime path and advances
connected proof. Full Day2 remains unproved. No matching source/YAML/index
changes, no agents, no gameplay RAM injection, no commits/staging/restores.

**A9 complete native graph:** `field_item_pickup_port.c` now has
- 19540..19618,54words: opcodeA9, taskbit20, first/retry/wait/delay, selected
  result3..386 and signed-1 cancellation. RewindsCE00 by16 whilewaiting.
- 4C34C..4C4B4,90words: id1 selectionwindow construction, original callbacks,
  savedcursor restoration, existingwindow return, storagefilter and itemmode.
- 55E14..55FB4,104words: storageeligibility bitset, excludes equippedslots,
  57654restrictions and itemflagsE0. Existing lookup/callback implementations.
VM19540 dispatch/headerdeclarations added. RoutineSHA256 respectively:
a5f391afa21ec7167e98fbd012445eea2c9f95483deb314b1dd9e1f03e2d3899,
a55d2fd773a65f13f93e2edee052fb4a774e786ec676e47ee986bf940230f545,
5f4347c079d6f4ce59ccddb0dd4700dfba3c4e68f4a8b31533d10d1d6d8592b3.

**Oracle:** `pe_field_pickup_oracle.py` extends74legacycases with110new
A9/builder/filter cases, total184. `--compare-native --build-dir ...` checks
all110new returns and fullRAMbelow1FE000, originalstackexcluded. Original
instruction+delayslotbytes verified; generatedrangescovereveryoriginalwrite.
`retail_field_pickup_cases.h` regenerated, `test_field_pickup.h` dispatches
entries10/11/12. AllPASS `/tmp/pe-a9-oracle.log`.
`--capture-only --capture /tmp/pe-m334-chest-connected.bin --compare-native`
reconstructs original A9 call operands/context from actual stoppedchest and
compares both firstwait/windowcreation phases, fullRAMPASS:
`/tmp/pe-a9-capture-oracle.log`. Capturesneverinjectedintoconnectedroutes.

**Builds/tests before new canonical extension below:** Release/Debug complete;
all11CTest +1388/1388native PASS150.48s (native67.36s).
`/tmp/pe-a9-{release-build,debug-build,ctest}.log`. No productioncodechange
since exceptheadercomment. Extendedcanonicalroute subsequentlypassed157.22s (below).
MatchingEXE unchangedSHA1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
gitdiffcheck/Pythoncompilepass.

**Connected A9:** `/tmp/pe-a9-chest-connected.{log,bin}`58500frames passed old
57733stop, menuopen. Sourceoldchestpilot keepsholdingdirection, sooriginal
menuinputinitial-release latch neverarms (D0E8=0). This was an input issue;
no gameplay/input code changed to force it. CaptureSHA256
f291fd6f49a60e87db9c24598cd4d2e4f1898d1e501cbda39cf16d3f70acd6f5.
`/tmp/pe-a9-selection-route[.c]` releasesallbuttonsfrom57733, Cross57780..57782,
thenneutral. `/tmp/pe-a9-selection-connected.{log,bin}` completes58500 with
**inventoryslot0=0, persist71=256, chestflag80cleared, CF00selectionmode0**.
Original A9 successfullystoresunequippedgun in chest. OnlyfourHOST_ADAPTED
movie/opening skips. Diagnosticold45milestone/m27assertionfailsbydesign.
Session10897 finished. CaptureSHA256
59735adda7e1807e6f7848f0508151a3117fb392c361e0c0e1ada38023d50ab5.

**Supply/menus:** firstreturnattempt `/tmp/pe-supply-menu-connected.*`62000
stuck(70,269): westtoX0 runsintofirstchest. Correctedstage3 goeswesttoX375,
southZ-700 thenreturn(0,-1100). `/tmp/pe-supply-return-route[.c]` completes
62000 inM31, HP45, reserve15, equippedgun0, item7consumed, story68,
persist1=14E, flagsD1A04000, statuscopy45, keysC8/C9 retained. Normalmenu
inputs61000..61242 consumeitem7 (HP12->45 at~61150);61500..61832equipgun0.
No statewrites. CaptureSHA256
eccd1f5bcabde763875f76389dceb4d4bd0e7d94c880d2232d67175069dd4e2f.
FrontierM31pc8019E8B8, Aya(1020,1048,2615), actorflags408.

**New canonical regression PASS (session4866 complete):**62000frames/57milestones,
3158pads. Added `route_sewer_supplies_pads.h`1610rawpairs covering
[54500,62000); suffixSHA256864ac392b0e256c1b250571c805d3331343ed1f6368d92cc6417f6f7d7567fde
(comma-separatedwithoutleadingcomma/newline). Existing1548unchanged.
Capacity4096, optionalPE_ROUTE_SUPPLY_PAD_BEGIN/END. Read-only supplies
observations pinammo15, switch2Bbit4, item7pickup, consumption/HP45 andgun0.
Previous two victories stillrequirethreebodiesretiredinoriginalrooms; final
M31 types7/8arechests soabsencecheckonlyappliestoM28. NewendpointM31/E8B8,
persist14E/HP45/status45/reserve15/gun0/keys/chest/switchflagsasserted.
Log `/tmp/pe-supply-fixed-ctest.log`, build `/tmp/pe-supply-fixed-build.log`.
`/tmp/pe-supply-fixed-lasttest.log` retains fullroute output. Passed157.22s.
Observations57706ammo,58188switch,58778item7,61153heal45,61683gun0.
Debug rebuilt afternewroute `/tmp/pe-supply-debug-build.log`; Release ready.
README/DAY1_SEWER_MOVEMENT updated withnewcanonical and A9 results.

**Supplied M32 attempts completed (no third victory):**
- `/tmp/pe-supplied-m32-route[.c]`, logs/capture
  `/tmp/pe-supplied-m32-fixed.{log,bin}`,71000frames, session11689finished.
  Same965initialpairs from`/tmp/pe-m28-battle-pad.txt`, supplies/menus above.
  From62000M31centerX0, southZ1000, westX-1500 ->M32entry~62703. M32west
 16400then north3800. Battle63058 with45HP/64PE/15reserve/pistol0. Controller
  firstlivingtarget, follows350distance, backsaway<200/circles200..350.
  Killsfirstenemy63503, thenHeal1 from10HP. Dies64146 withsecondenemy17HP,
  9reserve. Ends71000/restartstory9. SHA256
 71c5b644448d83e8c2e13efd22d115d6cbd88657df4ea0540e6dbb213bb1e2ae.
  Earlier9254run aborted62703 becauseTEMPpilotreadAya0duringM32load; fixed
  actorpresentguard inM32navigation. Runtimewasnotfaulty. Earlierlog
 `/tmp/pe-supplied-m32-connected.log` yieldedcanonical54500..62000pads.
- `/tmp/pe-m32-distance-route[.c]`, `/tmp/pe-m32-distance-connected.{log,bin}`,
 72000frames,session60506finished. Follows800distance, backsaway<450,
  clamps awayfromx[-16100,-13800]/z[1450,2800]edges. Stillstuckneareast/north
  edge; dies63588 withfirstenemy3HP,second34HP. SHA256
 3b9b38ae9acde92c3e327a69e3aba7eee646ec0e07e104711ab7654a0a3f6a7e.
Both losses are pilotoutcomes; no HP/damage/inputbehavioroverrides made.

**ONLY LIVE process: session69295** `/tmp/pe-m32-circuit-route[.c]`,
`/tmp/pe-m32-circuit-connected.{log,bin}`,72000frames. Same verifiedsupplies
andmenus. M32battle movement nowcycleswaypoints(-16000,1600),(-16000,2700),
(-14300,2700),(-14300,1600); advancecornerwithin100units or240frames.
NormalCross/Heal1 controllerunchanged. LogsM32_CIRCUIT andSUPPLY_FINAL_PAD.
No RAMwrites. Inspectactualresult; do not assume victory or runtimeblocker.
Newnative/A9andcanonicalregressionwork is complete andverified; nextwork
must stillcarrythirdfight and onwardtoactualDay2. Goalremainsactive.

## SESSION 2026-09-12: M32 movement and actor-frame omissions verified

Goal active. Full Day 2 and whole-route retail fidelity remain unproved.
This turn made verified native progress. No matching source/YAML/index changes,
no staging/commits/restores, no agents, no gameplay RAM injections.

**Production:** `m28_movement_port.c` now shares init/command/update bodies
with explicit unsigned relocation: M28=0, M32=FFFFFFF0 (-16). Cleanup and EXE
arithmetic unchanged. Signature-guarded constructor6F39C, command6F6D4,
D413C update/noops/cleanup, and 6FC18 destruction dispatch m32's original
addresses. Full 764-word comparison verifies 21 internal J/JAL targets and
five stored callback references relocate -16; all other words are identical.
Original M32 C2 LBA15937/48, SHA256
cad2f1feb66352c54806de4291c6a7fb28fd610039860a6631aebc93047fffa8.
Descriptor80192558: 80191504,8019150C,8019159C,801917E4,801917EC,
80192090,801920EC. `pe_m28_movement_oracle.py --room m0032i` verifies the
relocation directly and passes **991 original/native cases**. Default M28
passes **1003**. M32 excludes only 12 M28 room-command cases. New native
header/test included. Logs `/tmp/pe-m32-movement-oracle.log`,
`/tmp/pe-m28-relocated-oracle.log`.

`actor_contact_port.c` now implements all38 original words of3601C..360B4:
one ordered list pass copies position/rotation when flag400000 and parent
are present, preserving sequential memory access and overlapping poses.
35558 calls at original356F8, after optional1A9F8 and before66268, including
paused frames. Lifecycle oracle adds60parent/count/flags/order +6overlap
cases to467retirement; onecapturedretirement +16contact comparisons retained.
**550 original/native cases PASS**, `/tmp/pe-parent-sync-oracle.log`.
Generated533syntheticcases carrykind0retire or2parent, dispatched in test.
35558 also restores original35B34 ->661CC after6C5BC/69594; this existingleaf
resets projectioncenter160,112 even when69594's update gate is clear.

**Final builds/tests:** all11CTest + **1388/1388native PASS**,145.87s
(route145.87/native63.20). Release and Debug complete. Logs:
`/tmp/pe-frame-complete-{build,ctest}.log`,
`/tmp/pe-frame-complete-debug-build.log`. Earlier parent-only suite also
all11passed142.58s `/tmp/pe-parent-sync-all-{ctest,lasttest}.log`.
Canonical route unchanged:54500frames/49milestones/1548pads, two sewer
victories51167HP30 and54039HP12. README and DAY1_SEWER_MOVEMENT updated.
Matching EXE SHA1 remains452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
plan1145=796C/347asm/2rodata,b50a30290d4d,geometry1EE000. Plan/status,
Python compile/diffcheck passed. Earlier tracked-source verifier issue remains.

**Connected outcomes (all input-only cold boots):**
- `/tmp/pe-m32-movement-connected.{log,bin}`: old west approach passes the
  missing callback57424, Heal1 heals8->38 at57626, death57796/restart58005.
- `/tmp/pe-m32-north-connected.{log,bin}`: north first hits wallz3602,
  Aya(-12629,2097,3602),65000frames,no battle/unknowncallback.
- `/tmp/pe-m32-upper-connected.{log,bin}`: north to3400 thenwest still
  startsbattle57514, death58044. Lower polygons are enemy AI regions;
  they do not prove an encounter bypass. Includesparent, predates661CCcall.
- `/tmp/pe-m32-ground-connected.{log,bin}`: latest library,68000frames.
  Controller attacks firstlivingenemy only within200XZ/150Y; otherwise
  perpendicular dodges while airborne. Heal1 through normalmenus heals
 12->42 at57615; death57788 thenrestart. Both type6enemies start34HP.
  No thirdvictory. Source/binary `/tmp/pe-m32-ground-route[.c]`.

M32entry56619: Aya(-12345,2097,2585),12HP/64PE/noammo. West triggers
battle57351 nearx-13970. Original exit8019C888 ->m33/A80031C8 at
x[-16680,-16080],z[3650,4000],persist1=20. Scriptbase8019AC98 SHA256
8da1775b2d07e7083d786fbbbdcdf5e45e3c178429089b16f8d736c0d3cac1cc.
No inference from a losing pilot justifies changing gameplay values.

**Supply-room probe completed:** session32966 source/binary
`/tmp/pe-m334-chest-route[.c]`, log/capture
`/tmp/pe-m334-chest-connected.{log,bin}`. It opens firstchest, persist84=80,
**reserve ammunition increases0->15** (virtualitem512+A at800A1E6E).
Inventoryslot4 stays0 because ammunition merges into existing virtualslot3.
Repeated Cross then reopens the chest for item selection and stops57733 at
unported **opcodeA9/func80019540**, pc8019292C/actor800BF490. This is an
actual unresolved boundary, not a completed63000-frame replay. A user-facing
initial claim of no callback stop was corrected after reading full log.

A9 original19540..19618 (54words): taskD300 flags+8bit20; firstcallsetsbit,
67CBC, rewindsCE00 by16, sets task+16=1/returns0. Subsequently D2A4 in3..386
writes value-3 to operand1; signed-1 writes-1; eitherclearstaskbit/returns1.
Otherwise calls4C34C(operand0), rewinds/delays again. **Not implemented.**
4C34C..4C4B4 (90words) creates existing itemlist callbacks44444/F8D0/57C54/
50260/447F0 plus55760/52F70/647D0/4C594/55E14/5B890; routine itself notported.

**Supply continuation completed:** session36323 source
`/tmp/pe-m334-supplies-route.c`, log/capture
`/tmp/pe-m334-supplies-connected.{log,bin}` also stops57733 at the sameA9.
Straight movement toward switch left Aya against chestcollision at(0,245),
so Crossreopened it. No new runtime code changed; this is knownboundary.

**Corrected navigation completed, no live processes:** session61494,
`/tmp/pe-m334-waypoints-route[.c]`,
`/tmp/pe-m334-waypoints-connected.{log,bin}`,65000frames, frame-limit with
only four existing HOST_ADAPTED movie/opening skips. No unknowncallback.
Same965initialpads and coldboot. Afterchestflag80, stepssideways to(400,245),
then(400,1900), then switch(64,2148). Crossgatedtonear intendedswitch/chest,
but stillpulsed whenD1A0&4 to dismisspickupwindows. Switchpersist2B=4 by58200,
chest7flag200 by58800. **Inventoryslot4=7, reserve15, HP12**, chests84=280,
switch2B=4, m334A8063248/story68/persist1=1F, D1A04000. Originalitem07 has
heal90 property; not yet used. CapturedSHA256
765af3be311bd78607573bd5ca066e2e73810fc02d682e4203ab1cc5127b1410.
Probe's old45milestone/m27endpointassertion fails by design at this extended
endpoint; it is a diagnostic, not a new passingcanonicalregression.

**Immediate next work:** returnpath diagonally toward(0,-1100) getsstuck at
(685,244) from58920onward. Change stage3 towalkwest toX0 first, then south;
useordinarymenu to consume item7 and equipcarriedgun0 with15reserveammo,
thenreturnm31/m32 for thirdfight. All normalcontrollerinputs, noRAMrestores.
No change made yet. Initial6593run was intentionallyterminatedearly to
correctpickupwindowCross before completed61494restart. OptionalA9 boundary
remains unported; movingaway fromopenedchestavoids requestingthat menu.

Original M334 script `/tmp/pe-sewer-forward-rooms.txt`, base80191B90,
SHA256fb8e97859ff9d92198c759655aacc95e360e4580733de8a8c10d23bba8a28711.
Room entry(0,474,-640). Firstchest module4 at(-50,474,513), RNGitem0D/02;
actualentrycapture chooses02. Module5(-411,474,1274) item22/31, chooses31;
module6(589,474,2058)item1D/1E, chooses1D. Module7—not8—has fixeditem07 at
(837,474,13), onlyspawned after persist[2B]&4 (switch nearx25..111,z2100..2198).
Module8 is ambience. Return rectangle x[-676,777],z[-1300,-1000] ->m31,
persist1=14E. Original item table in capturedRAM baseAB608: item02 kind10hex
(16decimal), quantity15 at+10; item07 kind0A/heal90 at+12. Item06heal45.
No item-name string decoding done, so refer to IDs/record properties.

## SESSION 2026-09-12: both sewer victories; m0031i selected audio ramp

Goal active; full Day 2 and whole-route retail fidelity remain unproved.
This is verified progress through Day 1 prerequisites. No matching C/YAML or
index changes; no staging, commits, restores, agents, or gameplay RAM cheats.

**Production fixes since the movement/projectile section below:**
- `func_80020F18_port.c`: guarded801920A0 movement destruction callback.
  Movement oracle now1003cases PASS; native suite1386/1386 passed afterward.
- `actor_contact_port.c`: complete original360B4..361F4 actor retirement,
  replacing the flag-only cut in35558. Releases effects6FE14, clears Aya/input
  state as needed, releases first model allocation363F4, unlinks active list,
  pushes free list, decrements count. Original3D82C is return-one/no RAM effects.
  This fixes the defeated type8 actor's255-unit collision body remaining in
  the live list and blocking Aya nearz-1917. No collision bypass was added.
- Effect pool addressing now aliases physical RAM. Cold boot retirement calls
  6FE14 before pools initialize; retail reads physical8, which previously
  aborted the native host. Nine new physical/uninitialized pool fixtures pass.
- `pe_stream_commands.c`: full96-word **8008B780..8008B900**,
  commandA1 selected effect-volume ramp. Group overlap takes precedence over
  handle equality; duration default/full-word zero test, signed-low-halfword
  division and fault prefix preserved. This resolves the m31 boundary below.
  Routine SHA25629e7f2ce0e7fc181da4e8f3051148e981e0eec846274ec987110fee5721a13f8.

**Retirement evidence:** `pe_actor_retirement_oracle.py` / new native fixture
`test_actor_retirement.h`:467 synthetic cases +1 captured retirement +16
captured contact comparisons = **484 original/native cases PASS**. All RAM
below1FE000 identical, original scratch stack excluded, executed instruction
and delay-slot source bytes checked, generated ranges cover every original
write. Moving south5units at the old stuck state is rolled back before
retirement and accepted afterward in both original and native. Logs:
`/tmp/pe-retirement-{pool-oracle,contact-oracle}.log`.
Native regression includes467synthetic cases; latest pre-A1 full native suite
**1387/1387 PASS60.30s**, `/tmp/pe-retirement-verified-native.log`. Debug complete.
Audio oracle now964normal FIFO cases (483A1) plus4A1/2A9fault prefixes;
`/tmp/pe-m31-a1-oracle.log`. Full-RAM captured A1 native/original PASS below
1FE000: `/tmp/pe-live-a1.log` (temporary script's label still says retirement).

**Connected victory:** `/tmp/pe-retirement-pool-connected.{log,bin}` uses
old input-only `/tmp/pe-m28-battle-route.c` (no sidestep),965initialpad pairs
`/tmp/pe-m28-battle-pad.txt`. Reaches62000 without unresolved boundary,
m28A8002448/story68/persist1=1B, AyaHP12, mode9, D1A04080, menu0,
overlay40000040/musicF2=0. Both type7 and type8 enemies removed. Status copy
C0E08=22, clubslot2, keysC8/C9 retained. Capture SHA256
3484bcb8fa9e8c6921df8ff7ca849933f7e13dddf03098c2132f9757e9a7d3c7.
The earlier `/tmp/pe-m28-destroy-connected.log` navigator LOST (HP0 at54484,
restart54752). Its new-room ending was not story progress. That loss predates
complete retirement; do not confuse it with the verified victory above.

**Canonical regression extended (currently under full-suite validation):**
`test_route_boot_day2.c` now54500frames/49milestones/1548pads, endpointm28.
New `route_second_sewer_pads.h` adds587recorded pairs; previous961unchanged.
Suffix SHA2561d191a3c3caed7f4bfe121e7de6709c307f3878c9bf558f2bc923758fcaad896
(comma-separated suffix without leadingcomma/newline). Capacity2048. Raw pad
intervals[42713,45041),[50500,51200),[52344,54500). New env overrides
PE_ROUTE_SECOND_SEWER_PAD_BEGIN/END. Read-only per-frame battle observations
require3spawned enemy bodies, no bodies at exit, living Aya, mode9/control.
First fixed replay `/tmp/pe-second-sewer-fixed.{log,bin}` observed victories
**51167HP30 /54039HP12**, all49milestones, endpointtaskPC801A5A78. Its old
status-copy assertion36 failed; actual22 already corrected before full suite.
No pad change was needed. This first replay alone was not a passing test.

**Forward continuation:** `/tmp/pe-m28-forward-route.c` uses old battlepilot
then, from54500 in m28 with control, centersX then walks toZ450. Same965input
prefix. `/tmp/pe-m28-forward-connected.{log,bin}` reached **m31A80030C8 at55387**,
persist1=1C/story68, then stopped55612 at audioA1 callback8008B780. Capture
SHA2564867f80a60d736d49faed92f8e81bc2489ba0aef308e3ee77508f99c8706bc9f.
The native A1 implementation above has now replaced that missing callback.

**Live processes:**
-24926 `/tmp/pe-m31-a1-connected.{log,bin}` retries the same forward controller
  after A1,62000frames. Binary `/tmp/pe-m31-a1-route`.
-79270 all11CTest after A1 and new canonical endpoint:
  `/tmp/pe-second-sewer-all-ctest.log` (parallel2).
-91712 Debug rebuild after A1/header/newroute:
  `/tmp/pe-second-sewer-debug-build.log`.
Release build complete `/tmp/pe-m31-a1-build.log`. Check actual results next.
README and `DAY1_SEWER_MOVEMENT.md` still need final current verification updates.

Matching EXE unchanged, candidate/original SHA1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b; plan1145spans796C/347asm/2rodata,
b50a30290d4d, geometry1EE000. Plan/status, Python compile, gitdiffcheck pass.
Full repo verifier's earlier tracked-source metadata issue remains. Known
additional omission: original3601C parent pose synchronization is absent from
35558; inspected but not changed or tied to the current next boundary.

Latest update: all11CTest and1387/1387native PASS163.72s; route163.72s,
native72.17s. Final logs `/tmp/pe-second-sewer-all-{ctest,lasttest}.log`.
Debug complete `/tmp/pe-second-sewer-debug-build.log`. Fixed regression is
now validated; README and DAY1_SEWER_MOVEMENT have been updated accordingly.
24926 completed62000 with no unresolved boundary in m31: Aya(170,1048,-950),
HP12,flags408,D1A04000,mode9/menu0; captureSHA256
eeb2900eb601d8f8fbd6f421aa7895a4725126f3e22848bcbfa63fe9b8be595c.

**Only live replay50504:** `/tmp/pe-m31-forward-connected.{log,bin}`,
source/binary `/tmp/pe-m31-forward-route[.c]`. Same965initialpairs and m28
battlepilot/forward exit. From56000 in m31 with control, centersX then walks
north toZ2900, then east towardX1500. LogsM31_WALK every120frames. No gameplay
RAM writes. Frame limit62000. m31's original forward rectangle isx[1300,1800],
z[2600,3300], transfer8019EB40 -> m0334i/A8063248. Side exitx[-1600,-1400],
z[800,1250] -> m32/A8003148; backx[-740,740],z[-1150,-1050] ->m28.
Original script base8019DBE8/SHA2566dcc77b3c1e9378433f2522aee5fdbcc8f42972142b67cd3de8765f7d318d555,
`/tmp/pe-m0031i-probe.txt` regenerated without the invalid --room option.
Older live-process entries in this section are completed/historical.

**FINAL CHECKPOINT: no live processes. Goal active, verified progress.**
All11CTest/1387native and both builds remain green; no production change
followed the passing suite. Latest required frontier is now the third sewer
encounter in m0032i, not m31's audio command.

`/tmp/pe-m32-entry-connected.{log,bin}` enters m32/A8003148 at56619 and
completes62000 without an unresolved stop. Aya(-12345,2097,2585),HP12,
flags408,D1A04000,story68/persist1=1F. CaptureSHA256
1cb02041323f60c440654209284240b3289a6af7da06cc566ca475fbf6f75a24.
Its source/binary `/tmp/pe-m32-entry-route[.c]` follows the original m31 left
exit: from56000, centerX, north toZ1000, west toX-1500. The earlier m334
probe completed62000 in that optional side room, entry56922, no unresolved
boundary; `/tmp/pe-m31-forward-connected.*`, captureSHA256
d8dfea7fe920c9d35dc02b1e3117c61f073fda1d88c45952db385467035f0b9a.
Original m334 has only a return transfer to m31; main path is m32 -> m33.

**Latest `/tmp/pe-m33-approach-connected.{log,bin}`** (completed44402):
input-only source/binary `/tmp/pe-m33-approach-route[.c]`, same965initialpads.
Adds m32 navigation from57000: west toX-16400, then north toZ3800, and
reuses the M28 battle/Heal1 pilot when m32 battle flag2 is set. No guest
positions/HP/story/effect state is supplied. Battle starts57351; replay stops
**57424 unresolved-boundary**: constructor8019150C is unported, then
small-pool draw callback**801917E4 mode2** requests stop. AyaHP12,
(-14586,2097,1828),flags420,D1A04082. CaptureSHA256
7b0dd263d430b821d885163566ce957394a0e83ede553294f093118553579fb2.
Main-path m32 exit original8019C888 ->m33/A80031C8 atx[-16680,-16080],
z[3650,4000]; arrival writespersist1=20. Current controller stops after room
transfer unless battle continues. M32 script base8019AC98/SHA256
8da1775b2d07e7083d786fbbbdcdf5e45e3c178429089b16f8d736c0d3cac1cc.
`/tmp/pe-sewer-forward-rooms.txt` contains m334 and m32 original scripts.

**Read-only next implementation evidence:** M32 C2 is Disc1LBA15937,
48sectors/98304bytes, SHA256cad2f1feb66352c54806de4291c6a7fb28fd610039860a6631aebc93047fffa8,
`/tmp/pe-m32-c2.bin`. Effect49 descriptor80192558 has functions
[80191504,8019150C,8019159C,801917E4,801917EC,80192090,801920EC].
The complete764-word movement graph m28[80191514,80192104) vs
m32[80191504,801920F4) is identical except**21internal J/JAL targets and
5stored callback addresses**, all relocated by-16. Exact check file
`/tmp/pe-m32-movement-relocation.json`; original CD bytes also match all
764words in the actual57424capture. Five callback-reference instructions are
oldPC191550/191714/191758/191884/191C24; new immediates18 84/18 14/1D 08/
18 84/1D 00 (spaces only for readability). Existing native M28 movement is
therefore reusable with explicit relocated callback addresses after tests.
No M32 movement code has been changed yet; do not claim this frontier fixed.
Likely next: parameterize the existing init/command/update's code addresses
and add guarded M32 wrappers/dispatch; shared cleanup has no relocated data.
Cover constructor6F39C, command6F6D4, D413C draw/update/cleanup and6FC18destroy.
Reuse original/native histories with the M32 overlay/descriptor and preserve
M28's existing1003-case regression. Full arithmetic/helpers are otherwise
identical. Known parent-sync3601C omission below still untouched.

## SESSION 2026-09-12: m0028i movement and projectile effects connected

Goal active; full Day 2 and whole-route retail fidelity remain unproved.
This turn is verified progress. Current work is still Day 1 prerequisites.
See `docs/ai_context/DAY1_SEWER_MOVEMENT.md` for full evidence and scope.

**Production changes:**
- New `game/boot/m28_movement_port.c`: complete original effect49 constructor,
  commands, animation gate, first/resumed movement arcs, apex pause, cleanup,
  and EXE helpers DFB20/DFB78/DFF80/DFFB8. Uses original C2 overlay authority.
- `func_8006F39C_port.c`: dispatch m28 constructor and fix small-pool return
  index to i+11, matching existing original C. This was an actual defect.
- `func_80017018_port.c`: opcode6C query wrapper passes output addresses.
- `func_8006F6D4_port.c`: dispatch movement commands and preserve return.
- `func_80069594_port.c`: original69660 small-pool draw/update loop;
  `func_80035558_port.c` calls it at original356D0 before floor resolution.
- `func_800D413C_port.c`: overlay-aware movement, command and projectile
  callbacks. Command80193148 stores three arguments to193288..193290 and
  returns80193288. No unknown callback was replaced with a no-op.
- `m0013i_effect_port.c`: share the existing projectile routine through
  three explicit address parameters. M13 keeps its original wrapper; new
  M28 wrapper uses F1CC/F1D4/1924F8. Particle arithmetic is unchanged.
- CMake/header register the new native file/functions; no matching source
  or YAML change this turn. No staging, commits, restores, or agents.

**Verification completed:**
- `pe_m28_movement_oracle.py`: **947 original/native calls/history frames PASS**.
  Whole RAM below1FE000 compared (VM compatibility frame explicitly excluded),
  executed code/delay-slot words checked against original EXE/overlay. Original
  scratch stack/scratchpad/GTE state/timing outside RAM excluded. Generated
  fixture hashes also assert coverage of all original RAM writes belowcutoff.
  Native regression `test_m28_movement.h` PASS. Header18844+ lines.
- `pe_m28_projectile_oracle.py`: complete **788-word relocation proof**:
  M13 8018F004..8018FC54 vs M28 801924F8..80193148 differ only by18internal
  jumps and3data references. **120 original/native projectile cases PASS**,
  including both GPU packet banks and sound/collision/particles. Uses the
  existing M13 fixture family and its explicit packet-padding exclusions.
- Current **1386/1386 native tests and all11CTest checks PASS**, total185.87s:
  `/tmp/pe-m28-final-ctest.log`, `/tmp/pe-m28-final-lasttest.log`.
  Canonical52k/45milestone/961pad regression PASS129.03s, native54.90s.
  Release and Debug complete (`/tmp/pe-m28-final-{build,debug-build}.log`).
- Previous movement-only suite:1385native/all11PASS193.16s (route124.92s),
  `/tmp/pe-movement-all-{ctest,lasttest}.log`. Historical before projectile.
- Other logs: `/tmp/pe-movement-oracle-final.log`, `/tmp/pe-movement-native.log`,
  `/tmp/pe-m28-projectile-{oracle,native}.log`.
- Candidate/original SHA1 remains452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
  plan b50a30290d4d =1145spans796C/347asm/2rodata, geometry1EE000.
  Generated status check, Python compile and gitdiffcheck PASS. Full repository
  verifier still has the earlier untracked-source metadata issue (two files).

**Connected results:** input-only `/tmp/pe-m28-battle-route.c`, initial965pad
pairs `/tmp/pe-m28-battle-pad.txt`, 62000frame limit. First build stopped53241
at command80193148 (`/tmp/pe-m28-movement-connected.*`), next at main80192700
mode0 (`/tmp/pe-m28-command-connected.*`). Both now translated.
Latest `/tmp/pe-m28-projectile-connected.{log,bin}` completes **62000 frames
without an unresolved boundary**. m28A8002448/story68/persist1=1B; AyaHP38,
PE16, battle active. Type8actor800C0390 is defeated (body0/deathflags6001510).
Two type7 enemies remain atHP12 each:800C0110/800BFE90. Thus no victory yet.
Capture SHA256143baf259cc91f2e40d820533d46a4ed837f383e4fccfb1c8402ae73fcc3678b.
Aya=(8.1629,1153,-1917.1394); remaining enemies near(-656,-3306)/(254,-2877).
She keeps attacking out of range and becomes stuck on her approach. Battle
trace's old enemyhp=0 is not actual HP for these types; read body+16.

**Live next replay3436:** `/tmp/pe-m28-navigate-connected.{log,bin}`,
executable/source `/tmp/pe-m28-navigate-route[.c]`. Same965initialpads/62k.
Improved input-only controller holds Cross until within200units (except menus),
keeps Heal1 via normal menus, and sidesteps55frames if moving less than3units
for45free frames while target is far. Side alternates. LogsM28_ESCAPE and
M28_FINAL_PAD/M28_HEAL as before. No gameplayRAM writes. All productiontests
finished; only this exploratory replay is running. Inspect its actual result.

**Read-only floor audit:** temporary `/tmp/pe-live-m28-floor.py` adapts existing
`pe_live_floor_compare.py` to complete1A9F8 rather than1AE40. Uses latestcapture
and eight5unit directions, cacheon/off. **16/16 PASS**, every RAM byte below
1FE000 matches original. `/tmp/pe-m28-battle-floor.log`. This does not prove
all movement/collision; do not infer the entire frame is correct from it.
Arena polygon801AB604,4points=(1059,-3905),(1227,-1424),(-1306,-1424),
(-1122,-3905). Aya radius65/scale4096. Deadtype8 at(-48.39,-2149.77), body0.

Next: finish3436 and try normal input to win remaining fight, then towardm31.
Earlier frontier details below are historical; effect49/op6C/main now work.

## SESSION 2026-09-12: m0028i opcode DD and next movement-effect boundary

The goal remains active. Full Day 2 and whole-route retail fidelity are
unproved. The canonical regression still covers the first sewer victory:
52,000 frames, 45 milestones, 961 pad changes, m0027i / A80023C8, Aya HP30.
Its latest run passes in 143.74 seconds (part of the current CTest suite).

**New matching C:** `src/func_8001A214.c`, opcode DD, 55 words / 220 bytes.
Registered at YAML AA14; default era `-O2 -G0`. Leaf link and strong size
checks pass. All **796 matching C spans pass the full strong sweep**.
`scripts/build_us.sh` rebuilds the complete candidate byte-identically to
retail, SHA1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
`disc1_verify.exact_checks` also passes all 796 packed C spans. Plan:
`b50a30290d4df4e0b10a2798d38c7fbc10b1da3d20f9d8f97e77b134634be131`,
1145 spans = 796 C + 347 asm + 2 rodata, geometry 1EE000.
Logs: `/tmp/pe-polar-{match,strong-check,matching-sweep,retail-build,packed-check}.log`.
The full `scripts/verify_us.sh` stops at its tracked-source metadata gate:
new `src/func_8001A214.c` and preexisting `src/func_800374E8.c` remain
untracked. No staging or index changes were made. This is not a byte mismatch.
Generated status was refreshed to 796 through `disc1_plan.py --write-status`.
Generated, ignored AA14.s was removed with the plan cleanup tool; its original
copy is `/tmp/pe-retail-AA14.s`. No other matching source was changed here.

**Native DD:** guest adaptation in `func_8001A15C_port.c`, dispatch in
`func_80017018_port.c`, declaration in `pe_port_compat.h`. Signed-halfword
angle reduction and original store/load order preserved. New
`pe_polar_vm_oracle.py`, `retail_polar_vm_cases.h`, `test_polar_vm.h`:
**561 original/native full-VM cases pass**, including all operand modes,
quadrants, wrapping and aliasing, followed by a wait. All original writes
below 1FE000 are in compared ranges; stack/native VM frame adaptation and
hardware/timing outside RAM excluded. Filtered native test passes.
Release and Debug builds complete. All **11 CTest checks pass**, including
**1384/1384 native tests** (55.87s) and the fixed route (143.74s); total
201.87s, `/tmp/pe-polar-all-ctest.log`. Native LastTest copied to
`/tmp/pe-polar-all-lasttest.log`. All sessions from this update completed.

**Connected replay completed:** `/tmp/pe-m28-polar-connected.{log,bin}`,
executable `/tmp/pe-m28-polar-route`, session 50829. Same 965 initial pad pairs
in `/tmp/pe-m28-battle-pad.txt`, input-only battle controller source
`/tmp/pe-m28-battle-route.c`, requested 62,000 frames. No gameplay RAM writes.
Reaches m28 at 52344, battle at 53073, fade completed 53140. DD passes.
**Stops 53141**, actor 800C0390 / type8, at two untranslated paths in the
same VM invocation: effect 49 constructor and opcode **6C / 80018818** at
script **801A9450**. Aya HP30. The old m27 endpoint assertion also fails
because this probe advanced into m28; do not interpret it as a regression.
Capture SHA256 `0e5496adcdd0a514d113e1cf204673bfe0020f0809907e2dda820c512344ada3`.
Earlier completed `/tmp/pe-m28-flags-connected.*` stopped 53140 at DD
(801A91D8); op13 itself had passed. No current connected replay is running.

**Next: decompile movement effect 49 and opcode 6C.** The effect controls
actor movement and completion notification (earlier progress wording called
it a projectile path before the graph was inspected). Capture slot 8018D024,
small-pool index 0, code49, userdata800C0390. **Additional proven bug to fix:**
`func_8006F39C_port.c` returns the local index0 for the small pool, but existing
matching `src/func_8006F39C.c` returns i+11 for codes46..54. The capture
confirms local23=0. Correct this along with the constructor; otherwise6F6D4
resolves the wrong pool even after initialization is translated.
Descriptor 801931DC: init8019151C, command801915AC, noop801917F4,
update801917FC, cleanup801920A0. Constructor sets callback80191894;
command10 changes it to animation gate80191824. Query command25/mode1 stores
an output pointer at slot+0x10 and writes 1 through it. VM opcode6C passes
(*arg0,1,*arg1,arg2,arg3,arg4) to existing6F6D4, preserving pointers.
`/tmp/func_80018818.c` is only an unmatched experiment; do not put in src/.
Its era check differs in instruction scheduling (`/tmp/pe-event-query-match.log`).
No production change to opcode6C or effect49 has been made.

Original room C2 overlay extracted read-only: Disc 1 LBA15016, 69 sectors,
141312 bytes, guest base8018EFE8, `/tmp/pe-m28-c2.bin`, SHA256
`c15d03313a578f7c7ba07f255345df8e17734e38dd807162426d0bc3a7fd7e94`.
Code80191514..801931DC is byte-identical between this original data and the
capture. Disassemblies: `/tmp/pe-m28-projectile-dis.txt` (constructor/commands),
`/tmp/pe-m28-projectile-update-dis.txt` (updates plus subsequent unrelated
functions). Effect49 graph is 80191514..801920FC; later code is another effect.
Main callbacks: animation gate1824..1894; movement1894..1D10; noop1D10;
second movement1D18..20A0; cleanup20A0..20FC. Unported EXE callees
800DFF80/800DFFB8/800DFB20/800DFB78 are in C5060.s.
Command jump table at8018F164, entries0..25. Implement from original overlay,
not inferred gameplay. Existing6F6D4 ignores unknown callbacks; connecting
only the constructor will be insufficient. Field effect pump also needs
exact overlay-aware update dispatch. Keep explicit unresolved boundaries.

Evidence: `docs/evidence/func-8001A214/REPORT.md` and
`docs/ai_context/DAY1_SEWER_FADES.md`. No agents, commits, staging or restores.

# ACTIVE HANDOFF

Single source of truth for current working state. Read this first; update after
every meaningful change. Prefer shortening over accruing.

## SESSION 2026-09-12: first sewer battle fade and music reload

Goal active; still Day1 prerequisites, fullDay2/whole fidelity unproved.
New evidence [DAY1_SEWER_FADES.md](DAY1_SEWER_FADES.md).

Production changes this turn:
- New `game/boot/func_8003C638_port.c`: fade-in/out, packet modes, model
  color restoration and height recoloring. `3AF14` now runs its original
  fade/restore/color dispatch. `3B97C` skips untagged triangle's second NCCT.
- `func_80029810_port.c`: ambient reload3E/33, state40 redispatch, state32
  original timer/ramp. Original6D60C provider-control comparison426casesPASS.
- `pe_model_fade_oracle.py`:125scenarios/381frames nativePASS. Compact2528-line
  header before removing unused table; current regenerated header is smaller.
- `pe_live_model_compare.py`:195 captured ticks (3sewer enemies,65each) PASS,
  allRAM below1FE000identical; scratchstack/scratchpad/GTEstate excluded.
  `/tmp/pe-live-model-final.log`.

Connected probe `/tmp/pe-sewer-fade-connected.{log,bin}` wins all3enemies,
AyaHP30, stops51158 at explicitly deferred ambient reload (overlay3E).
That loader path is now translated. **Completed session1592**,58000frame replay,
`/tmp/pe-sewer-ambient-connected.{log,bin}`; same758prefix/controller. No boundary:
AyaHP30 at(119.3301,773,-3067.3823), flags408, D1A04080,battlemode9inactive,
overlay40000040,F2=0,story68/persist1=1A. First sewer fight/control restored.

Current verification:
- **1382/1382native tests PASS**, CTest56.55s:
  `/tmp/pe-sewer-native-verified-ctest.log`,
  `/tmp/pe-sewer-native-final-lasttest.log`.
- All381model frames/195captured ticks/426ambient casesPASS.
- **52000frame /45milestone /961fixedpad route PASS134.33s**,
  `/tmp/pe-sewer-fixed-final-ctest.log`, `/tmp/pe-sewer-fixed-final.bin`.
  Other9CTestchecks passed in`/tmp/pe-fade-music-ctest.log`. All11checks green
  across final targeted reruns. No further tests needed absent new changes.
- Debug final build complete,`/tmp/pe-sewer-debug-final-build.log`;
  Release current. Native prefix test now covers fade-out and fade-in through
  field loop with initially visible model; stops before later animation
  stages requiring pose data absent from this isolated fixture.
- New`tests/route_sewer_pads.h`, second exact raw interval[50500,51200),
  overridesPE_ROUTE_SEWER_PAD_BEGIN/END. kRehearsalRoutePads renamedkDay1RoutePads;
  suffix concatenated in rehearsal header. Recorded205battlechanges +5
  travelpairs. Full`/tmp/pe-sewer-victory-fixed-pad.txt` SHA256(no newline)
  99149fb604b73e56f514f113393f742714111e2779610f6b99a4b4303895b37b.
  Endpointm27PC80199734,story68/persist1=1A, HP30live/36status-copy,
  keys/club, all3type3enemybodies/tasksreleased+deathflag1000,
  D1A0&6clear,overlayF2=0/flags&44=40,battlemode9inactive.
  SeparateC0E08statuscopy is36evenafterroomtransfer (notliveHP); item/equipment
  synchronization51510/516B4writes it explicitly. Do not force it to30.
- Original/candidateSHA1 and disc1plan rechecked unchanged; tracked/new file
  whitespace and three newPython scriptscompilePASS.

**m28 connected** at52344 via961fixedpads+52000:FFEF,52400:FFFF:
`/tmp/pe-sewer-next-pad.txt`, `/tmp/pe-sewer-next-connected.{log,bin}`.
Completed57000frames, no boundary. Story68/persist1=1B, Aya(0,1153,-3455),
liveHP30/status-copy36,flags408/task0,D1A04000,mode9inactive,cameraidentity.
Oldm27endpointassertion mismatchisexpected.
Originalm28`/tmp/pe-m0028i-probe.txt`, base801A4968, SHA256
c151687a3b08229226eb255dc576bf8fe44b5d1eb22da5b997b8212bbee65df3.
m28backm27z[-4100,-3939]; sidem29doorsx[950,1110]or[-1110,-950],
z[-600,-500]; forwardm31x[-300,600],z[300,600]. Multipleenemytriggers.
Probe tool`pe_m0004i_mod4_probe.py m0028i`.

m28firstbattle probe38141 stopped53078 at **op13 /801A8C8C**, type8actor
800C0390. Other enemies:type7 actors800BFE90/800C0110 waitop92. Battle starts
53073,mode7at53076;HP30. `/tmp/pe-m28-battle-connected.{log,bin}`.
Rawprefix965pairs`/tmp/pe-m28-battle-pad.txt`, requested62000frames.
Temporary`/tmp/pe-m28-battle-route.c` copiescurrentcanonicalharness and
reuses input-only controller for m28: approach first living target from
zero-terminated9E000list, periodicCross, Heal1ifHP<=25/ATready/PE>=60.
PrintsM28_FINAL_PADchanges; no gameplayRAMwrites. Honorsbothrawpadintervals.

**Newproductionchange after lastgreenchecks:** op13 tableentry800176B8
alreadyexistsinmatching`src/func_800176B8.c` (10words): actor+98 OR **args,
return1. Added native VM dispatch branch in`func_80017018_port.c`.
No matchingfileedited. `pe_actor_flag_vm_oracle.py`75originalfullVMcasesPASS,
allfiveoperandmodes,high/fullbitmasks,followingwait,neighborpreservation.
Newtest`test_actor_flag_vm.h`,smallgeneratedheader. Release build and all75
native comparisonsPASS,`/tmp/pe-actor-flag-{build,native}.log`.
**Completed28691** same965pad input-only replay withop13connected; stopped53140 at opcode DD:
`/tmp/pe-m28-flags-connected.{log,bin}`, executable`/tmp/pe-m28-flags-route`.
Full **1383/1383native suite PASS57.51s**,`/tmp/pe-m28-flags-native-ctest.log`,
`/tmp/pe-m28-flags-native-lasttest.log`. Original/candidateSHA1andplanunchanged
recheckedafterop13. **Completed7006** Debugbuild`/tmp/pe-m28-flags-debug-build.log`.
Capturedm28modelcomparison **195/195PASS**,3actors65ticks each,
`/tmp/pe-live-m28-model.log`. Togetherwithm27,390capturedticksPASS.
That replay reached DD at801A91D8; the newer session above supplies its translation. Historical nativecount1383.
FullDay2unproved. Preserve preexisting dirty/untracked tree.

## SESSION 2026-09-12: rehearsal victory and connected sewer entrance

Goal remains active: full Day2 and whole-game retail fidelity are unproved.
Current work is still Day1. **m0319i is the rehearsal room**, C9 opens its
hallway door; actual sewer entrance is m0026i. No gameplay RAM, positions,
resources, story or command queues are seeded in connected runs.

**Canonical regression passes: 46000 frames /751 fixed pad pairs /41 milestones.**
Endpoint m0319i A80614C8, module6 PC801A355C, story60, persist1=17,
Aya HP36 (live/saved), club carried slot2 equipped, both keys retained,
Medicine consumed, cabinet opened, no battle/menu pause, Aya flags8/task0.
`route_rehearsal_pads.h` records86 supply/entry pairs +665 final battle pads.
Automatic Cross is suppressed only during[42713,45041), controlled by
PE_ROUTE_EXACT_PAD_BEGIN/END. Pad capacity1024. Full751-pair sequence
`/tmp/pe-victory-full-pad.txt`, SHA256 without trailing newline:
72d06612ba7360fb8f08e37203ee7ee938cac738d3fd7c7114c9252c2e1d8431.

Validation after all club/runtime changes: Release route CTest PASS121.54s
(`/tmp/pe-victory-fixed-ctest.log`, capture`/tmp/pe-victory-fixed.bin`).
Remaining9CTestchecks PASS61.05s, including **1381/1381 native tests**
(`/tmp/pe-club-final-ctest.log`, `/tmp/pe-club-final-lasttest.log`).
All10CTestchecks verified in2commands; both Release and Debug builds complete.
Original/candidate EXEs unchanged, SHA1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
plan1145spans,795C/348asm/2rodata, geometry1EE000, plan3a0f11a72cde rechecked.
Four existing HOST_ADAPTED movie/menu skips remain. No need to repeat these
checks absent new production changes.

New production work:
- Empty C9B90/CD8F0/CE16C weapon updates preserve flash stack bytes. Original
  CE934 writes Z at801FEF5C; shallow wrappers and shared VM do not touch
  borrowed801FEF58..5D. Unknown constructors/callbacks still invalidate tracking.
  Traces`/tmp/pe-stack-writers.log`, `/tmp/pe-club-stack-writers.log`.
- Club constructor CE084, update CE16C, origin CE1FC, spark CE2B4 and draw
  CE3B4 translated in`game/boot/func_800CE084_port.c`. Reuses generated matching
  CE144/CE464/CE470 and shared VM/rendering. Constructor6F39C, parameter6F6D4,
  callbackC2414, draw69594 and updateD413C dispatch connected.
- `pe_club_effect_oracle.py`: **61 original/native cases PASS**, compact16k-line
  fixture header/test. Logs`/tmp/pe-club-oracle-compact.log`, `/tmp/pe-club-native2.log`.
- `pe_m0023i_pump_oracle.py`: **70 histories /980 original/native frames PASS**,
  both banks/orders, empty pistol/hit/club updates, delay/loop/stop/pause.
  Logs`/tmp/pe-club-pump-{oracle,native}.log`. Fresh-generation unknown-Z stop
  still tested. No guessed vector or extra packet mask introduced.

Connected victory `/tmp/pe-club-stack-replay.{log,bin}` (52000frames):
ordinary Medicine before battle gives45HP; cabinet gives6 reserve +5 loaded;
Heal1 queued43669; club switch407 queued44629; club hit44752 changes Eve
1000006->999990 with AyaHP36; mode8 at44753; story5E/m0367 at45041;
return m0319/story5F at45525; story60/control restored45653. m0367 returns
m0319 directly here; earlier predicted m0023 return was wrong. Temporary
`/tmp/pe-club-route.c` produced pad input only; fixed regression captures it.
Previous boundaries44942Flash,44733constructor,44744Flash are resolved.
Detailed evidence/history: [DAY2_REHEARSAL_ROUTE.md](DAY2_REHEARSAL_ROUTE.md).

**Connected sewer entry proved** (`/tmp/pe-sewer-door-connected.{log,bin}`):
append46500:FF7F,46684:FFEF,47244:FFFF to canonical751 pairs; full754pairs in
`/tmp/pe-sewer-door-full-pad.txt`. m0026i A8002348 /story68 at47348. At52000,
Aya(20,1160,-6700), flags408, D1A04000; module0 BF water-sound loop is
intentional, not a ladder stall. Module2 PC80193DEC waits for exit rectangle
x[-1600,1600],z[-5600,-5200], then persist1=1A /m0027. No unresolved boundary;
old rehearsal endpoint mismatch is expected. Original m26 script:
`/tmp/pe-m0026i-probe.txt`, SHA256
984abe77e1e423ebdf331e726e9edc0bbb725540180f6fe8eac58d0b48bf70cc.

**First sewer hallway connected:** session32466 completed55000frames,
`/tmp/pe-sewer-first-connected.{log,bin}`. m0027i A80023C8 at49176,
story68/persist1=1A; Aya(0,773,-4335), HP36, cameraidentity, D1A04000.
No unresolvedboundary. Full756pairs`/tmp/pe-sewer-first-full-pad.txt` add
49000:FFEF,49250:FFFF to sewer-door inputs. M27 script
`/tmp/pe-m0027i-probe.txt`, SHA256
919c839e96aad94fc521036f01f84b834c229a364436f605dfb070d0fe117ae3.
Return m26 rectangle z[-5400,-5000], exit m28 rectangle z[-1400,-1100];
first enemy trigger around z[-3791,-3648] (module4).

Historical probe68336 completed58000frames; first sewer fight waited on all
three opcode92 fade-in timers. That missing3C638 path is fixed above.
`/tmp/pe-sewer-battle-connected.{log,bin}` and758pad prefix remain the
read-only captured evidence. Temporary controller in`/tmp/pe-sewer-battle-route.c`
uses normal movement/Cross and Heal1; no guest state writes.

## SESSION 2026-09-12: both theater keys; route reaches completed diary

Previous goal turn was verified progress (room cleanup, 29-milestone route).
Current input-only cold-boot traversal **collects C8 and C9** and finishes
m0018i's diary with player control restored. Evidence:
[DAY2_THEATER_KEYS.md](DAY2_THEATER_KEYS.md). No production runtime code
changed this turn; collision and interaction already follow the tested
original graphs. No positions, inventory, persistence or commands seeded.

Observed: first NPC dialogue flag at **22805**, C8 flag at **24758**;
return m0012i at **28035**, open m0018i door at **30293**, enter **m0018i
A8001448 at 30356**. Diary story **54 at 33565**, C9/rehearsal-key flag at
**33596**. At **35000**: story 54, persist[1]=12, persist[24]=010C0220,
C8/C9 in carried slots 5/6 (800C0E52/54), Aya 800BED10=(-159.9,226.9),
flags 8, task A8=0, D1A0=4080, module 3 PC 8019E374, matrix BD000 identity.
`/tmp/pe-both-keys.{log,bin}` is the successful connected capture; its old
compiled m0020i assertion alone reports exit 1.

The apparent diary animation stall was repeated Cross reopening it. New
input-only **PE_ROUTE_PULSE_END=33620** stops pulses after pickup. The
harness default now **35000 frames**, **50 pad pairs**, **34 milestones**;
requires both keys, their flags, story 54/persist[1]=12, live Aya, cleared
battle/menu pause and completed diary task. Optional diagnostics include
m0012i/m0020i/m0018i and persist24 changes; input capacity is 256 pairs.
CTest timeout scales to 600s for the longer Debug route. Both binaries built.
Final **35000-frame Release CTest PASS (91.16s)**,
`/tmp/pe-keys-final2-ctest.log`.
No full native rerun: production and native tests unchanged since prior
1380/1380 and 10/10 suite. Full Day 2/fidelity remains unproved.

New reusable **pe_live_floor_compare.py** compares native 1AE40 with original
MIPS on captured room geometry, checking EXE SHA1 and executed words plus
branch delay slots. **80 cases PASS**, RAM below 1FE000 identical (top 8 KiB
scratch stack excluded), `/tmp/pe-live-floor-verified.log`. Command in the
key evidence doc. Temporary contact 36448 comparisons also match three
captures. Do not persist captures as repository assets.

m0020i requires two approaches: first dialogue moves animated NPC collision
center (237,-300,260) → (386,-40,40), while actor origin stays (179,241).
Second contact must reach the new collision center and face within 45° of
the origin. `/tmp/pe-key-waypoints.log` records the intermediate positions;
`/tmp/pe-key-second-contact.bin` has C8. Floor-only path search scripts in
/tmp are proposal generators; only connected inputs establish traversal.

The 39000-frame rehearsal-door probe completed: `/tmp/pe-rehearsal-entry.{log,bin}`, requested
39000 frames, log/capture as below. It used the current default sequence plus
`35000:FFDF,35008:FFBF,35088:FFDF,35270:FFFF,36000:FFDF,36090:FFEF,36610:FFFF`.
It exits m0018i and returns to m0012i at **35296**, then reaches the rehearsal
door at position (0,-6035), Aya module 0 PC 801A2A18 opcode 22: a real dialogue
confirmation wait because Cross is off. C9/bit 80000 is retained. No crash.

The **42000-frame connected probe completed**: resume Cross at 39000,
rehearsal-door flag 40 at **39005**, enter **m0319i A80614C8 at 39101**.
Log/capture `/tmp/pe-sewer-resume.{log,bin}`. At 42000: Aya 800BED10=(-120,2855),
story 54, persist[1]=17, persist[24]=010C0260, D1A0=4080; module 6 PC 801A355C,
module 5 PC 801A3470, module 7 PC 801A3650. Aya's own task is done.
Matrix BD000=(-4085,0,301; 0,4096,0; -301,0,-4085), so FFEF walks mostly
world -z. Probe exit 1 only reflects the deliberately older m0018i assertion.
No live processes remain. Latest binaries include **PE_ROUTE_PULSE_RESUME**
(default INT_MAX) and were rebuilt; final post-option 35k CTest passed.

Next input-only proposal: append **42000:FFEF,42400:FFFF**, retain
**PE_ROUTE_PULSE_RESUME=39000**, extend to 46000. This should approach
m0319i module 6 rectangle at 801A34B0: x[-1271,1065],z[1265,1349]. At story 54,
it sends Aya payload 1 at 801A3534, starting the required m0023i encounter.
That proposal is not yet executed. `/tmp/pe-probe-next.py` extracts current
default C strings; include the seven sewer-approach pairs listed above
before these two new pairs. No RAM replay in connected traversal.

Original m0319i script is `/tmp/pe-m0319i-probe.txt`, base 801A202C, SHA256
33bdbfec502cbf3e6f6d4261548fec3aac746b3f6be571a1a94a85f50bb6bf85.
At story 54 it spawns Aya (-120,2855), and story gates require the m0023i
encounter at story 5B before the later m0026i exit (story 68). Do not use the
static shortest-path m0319i→m0026i edge to bypass that encounter. Additional
dumps `/tmp/pe-m0023i-probe.txt`, `/tmp/pe-m0026i-probe.txt`.

## SESSION 2026-09-12: room-entry cleanup; connected route reaches m0020i

Live `func_8003F074_dest_ready_cut` now calls the existing complete
`func_8003F074_after_poll_cut` continuation instead of its partial manual
copy. Restores E0060 effect cleanup, CDA4/942EC resets, display sync and
D1A0/D2E8/overlay flag resets. Original `3F23C..3F2F8` is the authority;
other loader cuts (6BD68/3F758) remain. See
[DAY2_ROOM_ENTRY_CLEANUP.md](DAY2_ROOM_ENTRY_CLEANUP.md).
New `pe_room_entry_cleanup_oracle.py --check`: **30 original region pairs
PASS**, including full E0060 but excluding loader/spawn/display-sync calls.
Native RCLEAN hashes and live BTL90 reset assertions pass. Full CTest
**10/10**, native **1380/1380**, zero skips; full suite used the prior
17500-frame route, which passed in 167.65s. Log
`/tmp/pe-room-cleanup-ctest.log`, details `/tmp/pe-room-cleanup-full-lasttest.log`.
Retail candidate cmp/SHA1 remains exact **452fb033f2eaa4b18aa20a5bca60b8125af3a37b**;
no matching inputs changed.

Input-only continuation centers Aya before m0011i's exit:
`17500:FF7F,17580:FFEF` enters **m0012i at 17809**, story 39 at 17810,
**m0013i at 18882** (story 40), back to m0012i at 19366, story 48 at 19457.
At 20000 Aya is (-52,-4169); room matrix is negative X/Z identity.
`20000:FF7F,20110:FFEF,20350:FF7F,20400:FFFF` enters **m0020i at 20434**.
At 22500 token A8002048, story 48, persist1=D, Aya 800BED10 (537,-19), matrix
identity; module 4 PC 801A1E98. No state/position/inventory seeding or replay.
The harness defaults now 22500 frames, **29 milestones**, m0020i module 4
frontier, story 48, persist1=D and battle cleared. Both binaries rebuilt;
new **22500-frame Release route CTest PASS** (76.85s), log
`/tmp/pe-m0020-route-ctest.log`.
Four HOST_ADAPTED movie/menu skip classes remain (skip_movie invoked 3 times).
**Still Day 1; full start-to-end Day 2 and full fidelity unproved.**

Next: m0020i first key is item C8. Original Aya payload 3 at 801A0AE0 gives
item via A7/E8 and sets persist24 bit 20 at 801A0BE8..801A0C00, unlocking
hallway side doors. NPC type 2 at (179,241), radius 100, first contact sets
bit 200 via dialogue; subsequent Cross with valid heading sends payload 3.
Main sewer door in m0012i requires persist24 bit 80000, not this first key.
The direct left/up approach is blocked by furniture: at 24500 Aya (445,81),
flags 24=01000000, no key. A down/left/up approach also failed (356,-72).
Upper approach (22500FFEF,22545FF7F,22800FFFF) also stops short at (478,206);
`/tmp/pe-m0020-key3.{log,bin}`. Lower probe
`22500:FFBF,22540:FF7F,22630:FFEF,22800:FFFF` ends at (435,66), still no
key (`/tmp/pe-m0020-key4.{log,bin}`). All four attempts complete normally;
their positions show the approach is unresolved. Next use read-only
`PE_ROUTE_AYA_DUMP=1 PE_ROUTE_AYA_EVERY=20` to inspect each intermediate
waypoint, and compare collision with original code if behavior diverges.
Do not infer the cause solely from the final position. Baseline m0020i capture is
`/tmp/pe-m0020-approach.bin`. Original script dumps
`/tmp/pe-m0012i-probe.txt` and `/tmp/pe-m0020i-probe.txt`; obtain again with
`python3 pc_port/tools/pe_m0004i_mod4_probe.py m0020i`.
Room m0020i mesh header 801A295C, vertices 801A29F8 (47), triangles 801A2AB4
(54, stride 22). Above obstruction, corridor runs from (474,257) through
(388,118)/(412,257) toward (190,212)/(109,240). Do not bypass collision.

Static next-key evidence: **m0018i** gives item **C9** at `8019D8E8`
and sets persist[24] bit **80000** at `8019D8F4..8019D90C`; its preceding
scene advances story to **54** at `8019D834`. Thus the intended key route
is m0020i first key → m0012i → m0018i second key → m0012i → m0319i.
This is script evidence, not yet connected traversal. m0018i dump
`/tmp/pe-m0018i-probe.txt`, base8019CF04, SHA256
6d45b760f7820cda0e20bd66e55b2b5546009544db8e4c82b7540b88c07d8088.

## SESSION 2026-09-12: retail frame gate; connected route reaches m0011i

Restored original `800355C8 -> 80035C04` overlay-bit-200 gate in
`func_80035558_walk_cut`. Actor callbacks run first; battle/floor/render/
resource/animation updates are skipped; final contact/task cleanup retains
its pause gate. Evidence: [DAY2_FRAME_GATE.md](DAY2_FRAME_GATE.md).
`pe_frame_gate_oracle.py --check`: **32 original full graphs PASS**, no mocks;
new FGATE native guest-byte test passes. Full CTest **10/10**, native
**1379/1379**, zero skips. Logs `/tmp/pe-frame-gate-{ctest,native-full}.log`.
The full run used the preceding 11000-frame route. The final **17500-frame
Release route CTest PASS** (46.87s), log `/tmp/pe-m0011-route-ctest.log`;
both Debug and Release route binaries have been rebuilt. Matching inputs unchanged; cmp/SHA-1 still exact original
**452fb033f2eaa4b18aa20a5bca60b8125af3a37b**.

Input-only breakthrough: release movement at **10000** and retain periodic
Cross. Enemy HP crosses the original script's 1000000 threshold at **11432**,
Aya HP 28; modes 6/7/8 then exit battle. Enter **m0367i at 11913** (story 26),
return to m0005i at **12205** (story 27), story 28 at **12417**. Hold FF7F at
**14000**, enter **m0009i at 14386**, release to avoid walking back through
the doorway. FFDF at **15000** reaches the hole interaction; periodic Cross
accepts the first option. Story 30 at **15373**, **m0011i at 15434**,
**story 38 at 16039**. At **17500**, token **A80010C8**, module 3 PC
**801A097C**, persist[1]=9, Aya **800BED10**, position **01970000/0/007D0000**
(x407,z125), battle flags4040/D244=0. No positions, story or commands seeded.

Harness default now **17500 frames**, seven continuation pairs:
`8990:FFDF,9140:FFEF,9600:FF7F,10000:FFFF,14000:FF7F,14386:FFFF,15000:FFDF`.
It requires **23 observed milestones**, current m0011i/module-3 PC, Aya and
cleared battle state. The four HOST_ADAPTED movie/menu skips remain.
**Still Day 1. Full start-to-end Day 2 and whole-game fidelity unproved.**

Next: m0011i module 3 polygon at **801A0800**, x -442..412,z1111..2959,
transfers to **m0012i A8001148 at 801A0890**. Current x407,z125 is near the
right wall; raw **FFEF** (held8) moves world +z because BD000 is identity.
The next connected probe can append `17500:FFEF`; determine whether the
walkmesh permits this approach. Script dump `/tmp/pe-m0011i-probe.txt`:
base8019FE3C, module offsets18/6E4/818/994, SHA256
70156a1575181e93ce5d4c8caab04fa109bf294f7dca9555486680eb6043fbd3.

Temporary diagnostics: `/tmp/pe-battle-stats.c` is a copy of the route harness
with a read-only present hook logging battle mode/queue/HP/AT/positions. It
links the Release runtime and is outside production. `/tmp/pe-m0009-hole.log`
and `.bin` are the connected 17500-frame run; `/tmp/pe-m0009-idle.bin` is the
15000-frame m0009i state. Exploratory logs exit 1 solely because their old
compiled assertion expects the active battle. No RAM replay in the route.
The intermediate 15000-frame m0009i Debug CTest passed (128.50s), log
`/tmp/pe-m0009-route-ctest.log`. Toolchain: `source /tmp/pe-tools/env.sh`;
approval never, unrestricted; omit sandbox_permissions. Preserve the large
existing dirty worktree.

## SESSION 2026-09-12: battle initialization and polygon boundary restored

`func_80029810_cut` now calls the complete retail `func_800293F4(0)` instead
of only its HP prefix. `func_8001A9F8_floor_cut` now executes the omitted
D2E8-bit-4 polygon collision graph (`1D170 -> 1CE88/1CBA0`). It uses the
existing original integer normalizer through a shared host-vector helper.
Evidence: [DAY2_BATTLE_BOUNDARY.md](DAY2_BATTLE_BOUNDARY.md).

Original oracles: **72 battle-entry graphs + 960 polygon graphs PASS**;
new native BENTRY/POLY guest-byte checks pass. No original callees mocked.
Full native CTest **10/10 PASS**, **1378/1378 native tests**, zero skips;
after extending the harness, the updated 11000-frame route CTest also passes
(96.98s). Logs `/tmp/pe-polygon-{ctest,route-ctest}.log`. The full run's route
used the preceding 9600-frame assertion; its replacement was checked separately.
Synthetic polygons and mathematically generated lookup tables; no captured
room/RAM assets added. Existing matching candidate rechecked with cmp/SHA-1:
**452fb033f2eaa4b18aa20a5bca60b8125af3a37b**, exact original. `src/` and
manifest inputs unchanged; no new matching-C claim.

The previous null-Aya diagnosis is resolved: after starting m0005i's battle,
the absent polygon collision allowed Aya to enter **m0009i during battle**.
Its loader clears Aya, and its script waits a frame before recreating her;
the still-active battle queue then wrote to 00000068. Initialization alone
did not prevent this. Restoring the original boundary keeps Aya in m0005i
and the same input completes 11000 frames without the transfer/crash.

Default route now uses `8990:FFDF,9140:FFEF,9600:FF7F` and 11000 frames.
It enters m0005i at **9300**, starts story **28** at **9766**, and ends at
**module 6 PC 801B284C**, token **A80002C8**, persist[1]=4, battle active.
Regression requires 16 observed milestones, Aya, battle bit 2 and the active
six-point script boundary. Movies/opening menu still have four HOST_ADAPTED
skips. **This remains Day 1; start-to-end Day 2 is still active/unproved.**

Next: finish this encounter through real controller input and original-code
comparison. Module-6 loop 801B284C reads type-2 position/stat through 5E/8B;
local[2] at 11000 is F4265. Default periodic Cross plus held FF7F has not yet
completed the battle. This section is historical; the subsequent frame-gate correction and battle
completion are recorded above.

Diagnostics: `PE_ROUTE_PAD_SEQUENCE` supports up to 32 increasing
frame:hex-mask pairs; empty disables continuation. `PE_ROUTE_RAM_DUMP`
captures final RAM only, never restores it. `/tmp/pe-polygon-route-end.bin`
is the natural 11000-frame state; `/tmp/pe-entry-seed.bin` is the pre-battle
state. Temporary guest-RAM tracing localized the room-clear call; no debug
instrumentation entered production files. Toolchain: `source /tmp/pe-tools/env.sh`.
Permission profile unrestricted, approval never; omit sandbox_permissions.

Previous floor collision work remains verified: cached-wall extent/NCLIP,
unbounded slide clearance, and physical-zero triangle record handling.
288 original graphs PASS; evidence [DAY2_FLOOR_COLLISION.md](DAY2_FLOOR_COLLISION.md).

## SESSION 2026-09-12: restore six shared Day 2 audio commands

`pc_port/game/boot/func_80015DAC_port.c` now executes EA301/303/401/402/405/409,
which previously returned success without performing their retail effects.
These restore effect stop/fade, first/second music-bank preload with the exact
VM retry/output contract, alternate animation-sound selection, and CD stereo
gain application. Reuses matched `80086948`/`80080AC4` via generated ports and
the existing `866A4`, `6D2B8`, and `7B964` graphs. No `src/` or manifest edits.

Evidence: [DAY2_AUDIO_DISPATCH.md](DAY2_AUDIO_DISPATCH.md). New
`pe_day2_audio_dispatch_oracle.py --check`: **118 original graphs PASS**;
native compares guest bytes/returns and actual CD gain latches. Full CTest:
**10/10 PASS**, native **1375/1375**, zero skips. Strong leaf verification:
**3/3 PASS** (`80086948`, `80080AC4`, `800866A4`). Tool env:
`source /tmp/pe-tools/env.sh`; legacy compiler needs the approved unsandboxed
execution because sandboxed 32-bit cpp exits SIGSYS. Logs `/tmp/pe-day2-audio-*`.
Full `scripts/build_us.sh` also completed: **795 registered C leaves**, 1145
spans, **EXACT MATCH**; original and packed candidate SHA-1 both
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. This includes remaining assembly
and is not proof of native Day 2 completion.

Connected route remains **m0004i module 4, pc=801B6CC8**, 14 milestones.
Input-only probes switching to FFCF at frames 8900/9000 do not cross the left
door. The former repeatedly collides near x=-1936,z=3376; the latter reaches
x=-2017,z=2174 just outside the polygon. Next route work: compare retail
collision/movement and find a valid doorway approach; do not force mailbox,
position, or story flags. Full Day 2, unskipped playback and retail acceptance
are still unproved. Previous goal turn classification: progress (existing
worktree contains the route and decomp additions); this turn adds verified
native behavior, rather than redefining the goal around passing leaf tests.

## SESSION 2026-09-11: native route frontier `m0377i` → `m0004i` (harness-only)

`pc_port/tests/test_route_boot_day2.c` gained a fourth pad stage
(`PE_ROUTE_SWITCH3`, default frame 6989, hold `0xFFAF`), replicating the
`m0378i` diagnostic-probe idiom; `0xFFAF = 0xFF9F & 0xFFEF` walks Aya into
`m0377i` module-1's op-77 rectangle at `0x801953C4`, so the module stops looping
at `0x8019544C`, writes `persist[1]=0x179`, bounces `m0377i -> m0378i -> m0004i`.
Command: `cmake --build pc_port/build -j && ./pc_port/build/pe-route-boot-day2-tests`.

```
route: frames=8000 stop=frame-limit story=0x00000018 persist1=0x0000017A token=0xA8000248
route: 14/14 ordered milestones reached
PASS: boot -> m0377i bounce -> m0004i route (14 milestones, frontier=m0004i mod4 pc=0x801B6CC8)
```

New frontier: `m0004i` module 4, `pc=0x801B6CC8` (frame-limit; next input-gated
op-77 volumes at `0x801B6940`/`0x801B6A08`/`0x801B6B74`). No matching C leaf,
`configs/`, or `src/` change. Native `ctest`: 10/10 passed. See
`docs/evidence/boot-day2-route-harness/REPORT.md` and
`docs/ai_context/ROUTE_COVERAGE.md`.

## SESSION 2026-09-12 (cont. 24): 9 leaves (785 → 794) + a manifest-loss incident

New count: **794 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `1144 spans (794 c, 348 asm, 2
rodata)`, geometry `0x1EE000`, plan `9b93313e87d7`).

`EXACT_REBUILD_GATE=PASS`: split + build produced
`orig=452fb033… cand=452fb033… RESULT=EXACT_MATCH`, then
`scripts/verify_us.sh` → `VERIFY_US=PASS` (7/7), `matching-C count: 794`.
`profile_necessity`: `630/630 era leaves clean`. Deep preflight: PASS.
Native suite: `1374 run, 1374 passed`.

**Matched (all `LINK_EXACT`):**

- **The five `D_8009D124`/`D_8009D128` gp-counter wrappers**
  `func_80060528`/`8006055C`/`80060590`/`800605C4`/`800605F8` (13w each,
  file `0x50D28.s`, era `-O2 -G8`). Identical shape, N = 6/5/4/3/2:
  `func_800602D0(a, N); D_8009D124 += N; D_8009D128 = D_8009D128;`

**New durable lever — a volatile read-modify-write materializes a
zero-delta store.** `D_8009D128 = D_8009D128;` is dead-code-eliminated by cc1
even at `-O1`, and a bare `extern volatile int` global folds the load/store
into the accumulator's own schedule. Only a local
`volatile int *p = (volatile int *)&D_8009D128; *p = *p;` reproduces retail's
**pre-accumulator `lw $v1,0x3B8(gp)` + trailing `sw $v1,0x3B8(gp)`** pair.
Two more sub-levers: the frozen callee `func_800602D0` must be declared with
**both** parameters (one-param declaration puts the literal in `$a0`, not
`$a1`, matching its `slt $s0,$s2` loop bound), and these five are **carved
contiguously** (five separate objects, not one C body).

**Parked:** `func_80071964`/`func_80071994` (`docs/evidence/func-80071964/`,
`func-80071994/`): 12-word conditional offset getters whose retail form keeps
two `nop`s that cc1's fill removes (8 mismatches; invariant across
`-O2/-O1`, `-G0/-G8`, `-fno-delayed-branch`, and local/`if`-`else` variants).

**INCIDENT — `git checkout` on an uncommitted manifest.** A careless
`git checkout configs/USA/disc1_build_profiles.json` (to undo a botched edit)
reverted the **uncommitted** build-profile manifest to `HEAD`, silently
dropping the `era_o2_g8_fill_epilogue_delay_slot` profile and the assignments
for `func_80062F9C` (patch 3), `func_800631DC`, `func_8005E4E4`,
`func_8005E518`. Those leaves then fell back to `era_o2_g0` and the deep
preflight failed on real syntax deltas (12 mismatches for the `lw $v0,0x378(gp)`
gp-relative slot at `D_8009D0E8`). **Restored and re-verified**: the manifest
now holds 27 profiles / 291 assignments, deep preflight PASS,
`profile_necessity 630/630 clean`. **Never `git checkout` a manifest file that
may hold uncommitted in-flight records** — edit surgically instead.

**Also fixed:** the five new source files were not `git add`-ed, which
`verify_us.sh` gate 3 correctly flags as a staging gap (`YAML C sources are
not tracked`) even after a successful `EXACT_MATCH`. Staged them, ran
`disc1_plan.py --write-status`, then `VERIFY_US=PASS`.

## SESSION 2026-09-11 (cont. 23): 33 small leaves (752 → 785)

Goal unchanged. New count: **785 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `1133 spans (785 c, 346 asm, 2
rodata)`, geometry `0x1EE000`).

This session carved 29 small `LINK_EXACT` leaves from former pure-`asm`
residue on the boot → end-of-Day-2 route. Two new profile assignments were
added: `func_80021054`, `func_80050308`, `func_800501C8`, `func_800509A8`,
`func_8004F7D8` → `era_o2_g8` (all read a `$gp` slot); `func_8007DD74` and
`func_800828F4` → `era_o2_g0_fill_epilogue_delay_slot` (maspsx patch 3).

**Durable levers discovered:**

1. **Source statement order for independent stores is load-bearing.**
   `func_80019CEC` writes three promoted halfwords; only the order
   `w28, w2C, w30` reproduces retail's `lh`/`sw` interleave. The natural
   `w28, w30, w2C` order produces 4 word mismatches. This is a plain
   permutation of independent stores with no aliasing.
2. **Branch polarity for an `if/else` call dispatch.**
   `func_80050308` matches only when the *nonzero* case is written first
   (`if (D_8009CF18) f(a0+0x7C); else f(a0+0x7F);`); the `== 0`-first form
   flips the `beqz` target and produces 3 mismatches.
3. **`-G8` is required when a leaf reads a `$gp` slot** even if the rest of the
   leaf uses absolute addressing. The gp base is `0x8009CD70`, so
   `off 0x184($gp)` = `D_8009CEF4` and `off 0x1A4($gp)` = `D_8009CF14`.
   Writing the wrong-looking symbol name silently emits the right-shaped
   instruction with the wrong displacement (1 word mismatch) — always convert
   `gp_offset + 0x8009CD70` to confirm the symbol.
4. **Maspsx patch 3 (`era_o2_g0_fill_epilogue_delay_slot`)** closes two more
   leaves (`func_8007DD74`, `func_800828F4`) whose `addiu $sp` teardown is
   scheduled into the `jr` delay slot.
5. **Unsigned comparison selects `sltu` over `slt`** (`func_8008E7F4`; the
   swapped `a1 < v` operand order is also load-bearing).
6. **Element type of a doubly-dereferenced parameter controls `lw` vs `lbu`**
   (`func_80019450`: `unsigned int **a0` gives retail's `lw`, `unsigned char *`
   folds to `lbu`).
7. **A permutation of two independent `$gp` stores is load-bearing**
   (`func_8005022C`: storing `a0` first reproduces retail; the value-first
   order is 2-12 mismatches).

**The 29 leaves:**

- `func_80019CEC` (14w), `func_80017E68` (13w), `func_80021054` (11w),
  `func_800192DC` (12w), `func_8004BC80` (13w), `func_80017DE4` (15w),
  `func_800198C4` (16w), `func_8008F178` (14w), `func_8007DD74` (13w, patch 3),
  `func_800828F4` (14w, patch 3)
- the pointer-wrapper family: `func_80015AB8` (14w, word+2 halfwords),
  `func_80018B30` (14w, 3 halfwords), `func_800193D8` (14w, 3 words)
- the slot-handler registration family: `func_8004EC3C`, `func_8004EC78`,
  `func_8004FD68`, `func_800501C8`, `func_800509A8` (all 14w)
- `func_80080F64` (13w), `func_80050308` (13w), `func_80019260` (14w),
  `func_8001A43C` (13w), `func_8001A474` (13w)
- the sequenced-init cluster: `func_8007DFE0` (12w), `func_8007E0C0` (14w),
  `func_8007DE40` (14w), `func_8004BCB4` (13w), `func_8004F7D8` (12w),
  `func_800504BC` (13w)
- `func_8008E7F4` (19w, sltu pair), `func_80019450` (13w, `int **` element),
  `func_80083790` (14w, packed record offsets), `func_8005022C` (13w, `-O2 -G8`
  ordered twin `$gp` stores)

New function-pointer wrappers declared **argument-less** (`void (*f)(void)`)
where retail clears no argument registers.

**pc_port propagation (same session):** `gen_decomp_ports.py` now emits **270**
decomp-derived TUs (up from 269): `func_8008C70C` was previously skipped by an
over-broad "pointer parameter stored into guest RAM" guard that flagged
`dst->value = arg0[1];` — an indexed *load* through the parameter, not a host
pointer stored into guest RAM. The guard now requires the parameter expression
to *not* be subscripted, so genuine pointer stores (`dst->source = source;`)
stay rejected while indexed loads port. Native suite **1374/1374 PASS**; the
generated glob keeps the CMake manifest honest (0 drifted, 0 stale TUs).
`tools/progress/native_metrics.py` was also corrected: it counted `TEST(` cases
only in `test_native.c` (1154) while the binary runs 1374, because ~220 cases
live in the `test_*.h` headers that file includes. It now walks the quoted
include closure, so the "expected run count" cross-check matches the binary
(1330 artifact-independent + 44 Disc-1-required = 1374).

### Gates (verbatim)

```
disc1_preflight: PASS (deep, 785 c / 346 asm / 2 rodata)
disc1_plan: 1133 spans (785 c, 346 asm, 2 rodata), geometry=0x1EE000, plan=3efbc8547764
profile-necessity: 621/621 era leaves clean; 0 hard defect(s); 0 redundant
EXACT_REBUILD_GATE=PASS plan=3efbc8547764af7c46f412503bfeb6e13e16823bb4724947a060824707e1ee6a yaml=43f755fb04328e00ce2a80be6088ab3fac875a8f8294abcbea10db101171f75c spans=[785 c, 346 asm, 2 rodata] sha1_orig=452fb033f2eaa4b18aa20a5bca60b8125af3a37b sha1_cand=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VERIFY_SWEEP=PASS leaves=785 plan=3efbc8547764af7c46f412503bfeb6e13e16823bb4724947a060824707e1ee6a
native: 1374 run / 1374 passed / 0 failed; 317 linked TUs (270 decomp-derived)
```

## SESSION 2026-09-11 (cont. 22): `func_80067CBC` matched (751 → 752) + three parks

Goal unchanged. New count: **752 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `1085 spans (752 c, 331 asm, 2
rodata)`, geometry `0x1EE000`).

**New leaf `func_80067CBC`** (`docs/evidence/func-80067CBC/REPORT.md`): a
23-word `D_800BCF88` status-word flag setter (fan-in 11), carved from the
former `0x58374` asm span (`0x584BC`..`0x58518`; a new `[0x58518, asm]`
resumes before `func_80067D18`). Default `-O2 -G0`, `LINK_EXACT`.
**New durable lever — dual pinned pointer bases:** retail keeps the head base
in `$a1` and *rematerializes a fresh base* in `$v1` for the tail block. One
pointer local gives the right shape but a shared base register (7 mismatches);
two distinct pointer locals pinned to the retail registers
(`register unsigned int *p asm("$5"); register unsigned int *q asm("$3");`)
close it to 0. Mismatch ladder: bare symbol 23 → single pointer 14 → single
pinned `$5` 7 → **two pinned `$5`/`$3` 0**. (This is the first matched leaf the
port generator rejects as `asm` because `ASM_RE` rejects all inline `asm()` —
a known generator limitation to revisit.)

**Three honest parks** (each with the full lever list and mismatch counts):
- `func_8003708C` (7 words, fan-in 16) — 64-bit multiply-extraction codegen:
  signed `mult` reproduces, but cc1 always emits `mfhi` before `mflo` (retail
  does `mflo` first) and allocates `$2`/`$3`/`$4`/`$5`/`$6`/`$7` instead of
  retail's `$v0`/`$v1`. `word mismatches=6`; invariant across ~15 expression
  forms, 7 flag rungs, and register pins (which add a dead `srl`).
- `func_80073A44` (94 words, fan-in 17) — VSync read. **Durable lever:
  pointer-to-volatile defeats the read-loop CSE** (`extern volatile unsigned int
  *` gives retail's two independent loads; a plain pointer CSEs them to one).
  Residual is prologue scheduling (retail hoists two pointer loads *above*
  `addiu $sp`) + a stack spill of the loop temp (`0x28` frame vs cc1's `0x20`).
- `func_800374E8` (24 words, fan-in 7) — 4-slot clear, 56-byte stride. Retail
  rematerializes `lui $at` per access in the symbol-relative indexed form; cc1
  hoists the loop-invariant base instead. `word mismatches=20`; fails for flat
  arrays, aggregate elements, `volatile`, per-slot pointers, and `-O1`.

### Gates (verbatim)

```
disc1_preflight: PASS (deep, 752 c / 331 asm / 2 rodata)
disc1_plan: 1085 spans (752 c, 331 asm, 2 rodata), geometry=0x1EE000, plan=83de048c6a46
PUBLIC_VERIFY=PASS  matching-C count: 752 (from YAML)
profile-necessity: 588/588 era leaves clean; 0 hard defect(s); 0 redundant
VERIFY_SWEEP=PASS leaves=752 plan=83de048c6a46c9719902549f6f68b6acd0c07799d0303a4f08d27f5f86a017d7
VERIFY_US=PASS: candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b == retail; all 752 packed C spans equal retail
ctest: 10/10 passed (native-tests 1374 assertions, route-boot-day2 12 milestones)
```

`pc_port/game/boot/func_80062D2C_port.c`'s `func_80067CBC` was realigned to the
now-proven **three-store** sequence (the hand version had folded retail's three
observable writes into one).

## SESSION 2026-09-11 (cont. 21): eleven leaves (739 → 751; incl. `func_8005DB44` park reopened)

Goal unchanged. This session carved **twelve spans from former pure-`asm`
residue** into eleven new `LINK_EXACT` `c` leaves and one corrected span
(`func_8005257C`), all on the boot → end-of-Day-2 route. Three new profiles
were added. Current count: **751 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `1083 spans (751 c, 330 asm, 2
rodata)`, geometry `0x1EE000`).

The session's marquee result is the **reopened `func_8005DB44` park**
(`docs/evidence/func-8005DB44/REPORT.md`): the old "reassociation residual"
(cc1 folds any `+` chain whose base is an *address constant* to
"constants-first") is defeated by making the base a **local pointer that is
decremented in place** (`p = &D_800A8038; … p -= 4`) plus a **block-local
`int sh = a0 << 5;`** assigned before the decrement — that yields retail's
`addiu $v1,$v1,-0x10` on the symbol register and `sll $v0,$a0,5` in the `beq`
delay slot. `-O1 -G0` (`era_o1_g0`, load-bearing; `-O2` flips the order back).
The adjacent `func_8005DAFC` twin is a genuine `$v0`/`$v1` allocation residual
and is honestly left in `asm`.

### Gates (verbatim)

```
disc1_preflight: PASS (deep, 751 c / 330 asm / 2 rodata)
disc1_plan: 1083 spans (751 c, 330 asm, 2 rodata), geometry=0x1EE000, plan=74acf19a28ff
PUBLIC_VERIFY=PASS  matching-C count: 751 (from YAML)
profile-necessity: 587/587 era leaves clean; 0 hard defect(s); 0 redundant
python3 tools/build/test_disc1_plan.py → Ran 8 tests … OK
EXACT_REBUILD_GATE=PASS plan=74acf19a28ffd98c4018b9cd3cc9eee70441ad3054eaccc27b1281b19491c096 yaml=a1a1b23b60b3583c188fde621da45856f7ec37ed5d11e89cdc72008687cc911b spans=[751 c, 330 asm, 2 rodata] sha1_orig=452fb033f2eaa4b18aa20a5bca60b8125af3a37b sha1_cand=452fb033f2eaa4b18aa20a5bca60b8125af3a37b sha1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
ROUTE_COVERAGE=plan=74acf19a28ffd98c4018b9cd3cc9eee70441ad3054eaccc27b1281b19491c096 funcs=349/979 c_words=6130 nonc_funcs=76 nonc_words=5414 asm_funcs=554 asm_words=66149 asm_words_unknown=0 nonmatchable=present tierA=734 tierB=248 tierB_unresolved=0
VERIFY_SWEEP=PASS leaves=751 plan=74acf19a28ffd98c4018b9cd3cc9eee70441ad3054eaccc27b1281b19491c096 yaml=a1a1b23b60b3583c188fde621da45856f7ec37ed5d11e89cdc72008687cc911b
```

Route honesty: route reports **349/979 whole on-path functions** matched
(`c_words=6130`) against `asm_funcs=554` / `asm_words=66149` still real
residue, plus 76 non-C-matchable / 5414 words. Keep reporting whole-function
share (~36%) *and* word share (~8%) — both are far from 100%.

### New leaves

| leaf | file span | size | era / profile |
|---|---|---:|---|
| `func_8005DB44` | `0x4E344` | `0x48` | `era_o1_g0` |
| `func_80078C94` | `0x69494` | `0x24` | `era_o1_g0_no_delayed_branch` (new) |
| `func_80038CE4` | `0x294E4` | `0x28` | default `-O2 -G0` |
| `func_800515C0` | `0x41DC0` | `0x38` | default `-O2 -G0` |
| `func_800528C4` | `0x430C4` | `0x2C` | `era_o2_g0_symbol_at_temp` (patch 5) |
| `func_80042EDC` | `0x336DC` | `0x44` | `era_o2_g8_force_d800bd024_absolute` (new) |
| `func_800524D0` | `0x42CD0` | `0x44` | default `-O2 -G0` |
| `func_80057ED8` | `0x486D8` | `0x3C` | `era_o2_g8_symbol_at_temp` (patch 5) |
| `func_8008594C` | `0x7614C` | `0x34` | default `-O2 -G0` |
| `func_80019410` | `0x9C10` | `0x40` | `era_o2_g8_force_d800bcfee_absolute` (new) |
| `func_800858B0` | `0x760B0` | `0x38` | default `-O2 -G0` |

All `check_leaf.sh` verified (`LINK_EXACT` + deep span-size exact), all
`profile_necessity.py` clean. Per-leaf reports in
`docs/evidence/func-80078C94/` … `docs/evidence/func-800858B0/`.

Every span was carved by proving the retail `jr $ra` boundary (exactly one
terminal return in the span) and matching the compiled `.text`; none was
derived by copying the next span's start. `func_8005257C`, whose declared
`0x214` span contained **7 interior `jr $ra`**, was trimmed back to its true
`0x18` with `- [0x42D94, asm]` restored — this was the `deep-interior-return`
defect class and it had been latent since before this session.

### Durable levers

1. **`-fno-delayed-branch` for an unfilled return slot** (`func_80078C94`):
   retail ends `<instr>; jr $31; nop` with the return `addu` **before** the
   `jr`. Plain `-O1` fills the slot with the return move; the new
   `era_o1_g0_no_delayed_branch` profile (`-O1 -G0 -fno-delayed-branch`)
   reproduces retail exactly. Same flag class as the pre-existing
   `func_80051E48`.
2. **Out-of-range `.data` byte with in-range gp words** (`func_80042EDC`,
   `func_80019410`): a leaf whose word destinations belong in small data but
   whose byte source sits beyond ±32K of `$gp` links only with
   `-G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=<byte>`; `-G0` loses every
   gp-relative word store and `-G8` alone fails to link
   (`relocation truncated to fit: R_MIPS_GPREL16`).
3. **Separate `int u = a0 & 0xFFFF;` local for a bounded index**
   (`func_8008594C`/`func_800858B0`): folding the mask into the condition
   yields a signed `slti`; the `int` local gives retail's unsigned `slti`.
4. **Pin the index/base pair when retail inverts cc1's choice**
   (`func_800858B0`): retail keeps the index in `$v1` and the loaded table
   base in `$v0` (7 mismatches without `register … asm("$3")`/`asm("$2")`
   pins, 0 with them). The setter twin `func_8008594C` does **not** need pins.
5. **An explicit inner-pointer local defeats double-load CSE**
   (`func_800515C0`): the nested-if form must reload `*D_8009D254` into a
   fresh local or cc1 merges the two `lui`/`lw` pairs.
6. **`deep-interior-return` matters**: any `c` span with more than one
   `jr $ra` is swallowing a function even if its size happens to match.
7. **Make a symbol pointer a *local that is decremented*, not an address
   constant copy** (`func_8005DB44`): cc1 reassociates a `+` chain whose base is
   an address constant (`&D_800A8038 - 0x10`) back to constants-first, losing
   retail's in-place `addiu $v1,$v1,-0x10`. `int *p = &D_800A8038; p -= 4;`
   keeps the decrement on the symbol register; adding a block-local
   `int sh = a0 << 5;` before the decrement puts the `sll` in the branch delay
   slot retail wants.

## SESSION 2026-09-11 (cont. 20): D_800BCD80 setter family (695 → 739)

Goal unchanged. This session matched the **`D_800BCD80` state-setter family** —
40 new `c` leaves — plus `func_80080950`/`80080998` (guarded byte-copy twins),
`func_8005E850` (signed-byte bias forwarder) and `func_80056C14` (bound-checked
halfword getter, patch 5): **44 new leaves, all `LINK_EXACT`, all on the
boot → end-of-Day-2 route**. Current count: **739 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `1069 spans (739 c, 328 asm, 2
rodata)`, geometry `0x1EE000`, plan
`818ab96a9b4db35a54337ecc3dce2c805d526a0d9945e9d98c8b6bb857852d7a`).

Setter-family per-leaf table and commands:
`docs/evidence/D_800BCD80_SETTERS/REPORT.md`.

### The setter batch (40 leaves)

Shape: store a command byte into `D_800BCD80`, 0–4 arguments into
`D_800BCD84`/`88`/`8C`/`90`, then `jal func_8008CBA8`. All era **`-O2 -G0`
(default)**; no maspsx patch, no explicit `assignments` entry
(`profile_necessity.py` proves the default reproduces all 40).
Left `asm`: none — `func_80086608` (`0x9C`, guarded variant) was matched and
registered this session (exact store order: `D_800BCD80 = 0x24`,
`84 = a0 + 4`, `88 = a1 & 0xFFFFFF`, `8C = a2 & 0xFF`, `90 = a3 & 0x7F`).

### Durable levers

1. **A real `switch` beats an if/else chain** for a command-byte selector
   (`func_8008682C`/`86728`/`867E4`). Retail emits `li $v0,1` / `beq $a0,$v0` /
   `li $v0,2` / `beq $a0,$v0` with the **default constant in the `j` delay
   slot**; hand-written `if`/`else` folds the chain or loses the delay slot.
2. **`int *p = &D_800BCD80;` for a two-call setter** (`func_800864F8`): keeps
   the base in callee-saved `$s1` (via `lui/addiu`) across the first `jal`
   instead of re-materializing `lui $at` per store. Same form as sibling
   `func_80086FF8`.
3. **Argument-store order may invert from source order** (`func_80086CA4`):
   retail masks `a2`/`a3` right after the command byte (before `sw $ra`) and
   stores the `a0` argument to `D_800BCD90` last.
4. **Call-poll `do`-while** (`func_80087090`): the `1` is materialized once into
   `$s2` before the loop; the `a0` argument is re-set in the branch delay slot.
5. **`do { } while (i < N)` with the post-increments in the body** reproduces
   the guarded byte-copy order (`func_80080950`/`80080998`), and the null-test
   order is `a1` then `a0` with the `a1 == 0` arm storing a NUL.
6. **Early-return polarity** applies to the bound-checked getter
   `func_80056C14`: writing `if (a0 >= 3) return 0; return D_800A1E6E[a0*16];`
   gives retail's compare→branch-over-fallthrough shape; patch 5
   (`MASPSX_SYMBOL_AT_TEMP=1`) supplies the 3-word `$at` load form.

### Deep-size guard caught a real gap mid-batch

The first carve declared `func_80086464` with span `0x68`
(`0x76C64 → 0x76CCC`) but its compiled `.text` is `0x40`. The extra `0x28` was
the real, previously unmatched `func_80086498` (`0x11`/`a0` setter).
`disc1_preflight.py --deep` flagged it by name/size and its remedy line named
the correct end; that function was then matched and carved. Same defect class
as the withdrawn `func_800906B4` — the guard works.

### Gates (verbatim)

```
disc1_preflight: PASS (deep, 739 c / 328 asm / 2 rodata)
PUBLIC_VERIFY=PASS  matching-C count: 739 (from YAML)
disc1_plan: 1069 spans (739 c, 328 asm, 2 rodata), geometry=0x1EE000
EXACT_REBUILD_GATE=PASS plan=818ab96a9b4db35a54337ecc3dce2c805d526a0d9945e9d98c8b6bb857852d7a yaml=b93dfa2ec80947f7c2e324c24ad8707eece7ef77a36f06ea098396da65a561a2 spans=[739 c, 328 asm, 2 rodata] sha1_orig=452fb033f2eaa4b18aa20a5bca60b8125af3a37b sha1_cand=452fb033f2eaa4b18aa20a5bca60b8125af3a37b sha1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
ROUTE_COVERAGE=plan=818ab96a9b4db35a54337ecc3dce2c805d526a0d9945e9d98c8b6bb857852d7a funcs=345/979 c_words=6048 nonc_funcs=76 nonc_words=5414 asm_funcs=558 asm_words=66231 asm_words_unknown=0 nonmatchable=present tierA=734 tierB=248 tierB_unresolved=0
profile-necessity: 575/575 era leaves clean; 0 hard defect(s); 0 redundant
python3 tools/build/test_disc1_plan.py → Ran 8 tests … OK
```

### Masked-match re-audit (mandated)

Structural audit of every `c` span against the retail image: **every `c` span
ends exactly on a `jr $ra` boundary, and no `c` span contains more than one
`jr $ra`** (a swallowed function always contributes its own `jr $ra`). Zero
additional instances beyond the three already repaired. The full
`disc1_preflight.py --deep` pass confirms no span exceeds its compiled `.text`.

## SESSION 2026-09-11 (cont. 19): deep-size defect repair (693 → 695)

Goal unchanged. This session **fixed three deep-mode span-size defects** (all in
`c` spans that declared more bytes than their C compiles to, silently
swallowing the next real function), rewrote one masked-false-match leaf, and
carved two newly-visible functions as exact leaves. Current count: **695
matching C leaves** (`python3 tools/build/disc1_plan.py --check` → `1025 spans
(695 c, 328 asm, 2 rodata)`, geometry `0x1EE000`, plan
`8a90f1e110e7f9390d1ccc2d82712c4ab07c0848e6359166068e164b77649c46`).

### Deep preflight output (the mode that catches sizes)

`python3 tools/build/disc1_preflight.py --deep` before the fixes:

```
disc1_preflight: FAIL 3 finding(s) — fix before running the expensive split/build
disc1_preflight: FAIL [deep-size] func_80076B58: declared span 0x88 (0x67358->0x673E0) exceeds compiled .text 0x40 by 0x48; trim_elf_section_pad.py will abort with `target size 0x88 > current 0x40`
  remedy: end the span at 0x67398 (move the following span/edge up 0x48)
disc1_preflight: FAIL [deep-size] func_80077D30: declared span 0x94 (0x68530->0x685C4) exceeds compiled .text 0x90 by 0x4; trim_elf_section_pad.py will abort with `target size 0x94 > current 0x90`
  remedy: end the span at 0x685C0 (move the following span/edge up 0x4)
disc1_preflight: FAIL [deep-size] func_800906B4: declared span 0x68 (0x80EB4->0x80F1C) exceeds compiled .text 0x30 by 0x38; trim_elf_section_pad.py will abort with `target size 0x68 > current 0x30`
  remedy: end the span at 0x80EE4 (move the following span/edge up 0x38)
```

After the fixes: `disc1_preflight: PASS (deep, 695 c / 328 asm / 2 rodata)`.

### The three defects and their resolution

| defect | declared | true (retail) | fix |
|---|---:|---:|---|
| `func_80076B58` | `0x88` | `0x40` | span → `0x40`; swallowed `func_80076B98` (`0x48`) carved as a new `LINK_EXACT` leaf at `0x67398` |
| `func_80077D30` | `0x94` | `0x90` | span → `0x90`; 4-byte layout pad at `0x685C0` now its own `- [0x685C0, asm]` span |
| `func_800906B4` | `0x68` | `0x30` | span → `0x30`; swallowed `func_800906E4` (`0x38`) carved as a new `LINK_EXACT` leaf at `0x80EE4` |

Retail evidence: `func_80076B58` ends `jr $ra`/`addu $v0,$zero,$zero` at
`0x80076B90`/`0x80076B94`, `func_80076B98` runs through `jr $ra` at
`0x80076BD8`; `func_80077D30` ends at `0x80077DBC` with a lone `nop` at
`0x80077DC0` before `func_80077DC4`; `func_800906B4` ends
`jr $ra`/`sh` at `0x800906DC`/`0x800906E0`, `func_800906E4` runs to
`jr $ra` at `0x80090714`.

**Masked false match withdrawn:** the oversized `func_800906B4` span had been
hiding a register-allocation mismatch (its C emitted `$a1`/`$v1` where retail
wants `$v0`/`$v1`). Its C was rewritten to a temp-local form and now compiles
`LINK_EXACT` at the true `0x30`. `func_80076B58` and `func_80077D30` were
already exact at their true sizes (`LINK_EXACT` re-verified).

### New leaves from the repair (693 → 695)

| leaf | span | size | era / profile |
|---|---|---:|---|
| `func_80076B98` | `0x67398` | `0x48` | `-O2 -G0` (default) |
| `func_800906E4` | `0x80EE4` | `0x38` | `era_o2_g0_symbol_at_temp` (patch 5) |

Both `LINK_EXACT`. `func_800906E4` needs `MASPSX_SYMBOL_AT_TEMP=1` (its
indexed `D_800B290C[(*(u16 *)(a0+0x5A)) << 6]` byte load is retail's 3-word
`$at` form with the `%lo` kept; the 4-word fallback makes `.text` `0x40` and
re-triggers `deep-pad`). Reports: `docs/evidence/func-80076B98/REPORT.md`,
`docs/evidence/func-800906E4/REPORT.md`.

### Durable guards added

1. **`tools/analysis/check_leaf.sh <func> <vram> <size> [flags]`** — runs
   `era_link_check.py` *and* `disc1_preflight.py --deep --only <func>`. The
   deep `--only` mode is the authoritative size gate; the fast preflight does
   not compile and cannot catch this class.
2. **Run the full `disc1_preflight.py --deep` before declaring any batch done**
   (it is now part of the documented routine).
3. **Env-leak hazard (real, cost one 4-minute build):** exporting a maspsx knob
   (`MASPSX_SYMBOL_AT_TEMP=1`, `MASPSX_SYMBOL_LOAD_DEST_TEMP=1`, …) in the
   persistent shell leaks into `exact_rebuild.sh` and poisons every leaf that
   does not want it (first `exact_rebuild.sh` run failed at `func_80076B20`
   purely because of this). Always scope knobs to a subshell.

### Gates (all on the live tree)

`disc1_preflight.py --deep` → PASS (695 c / 328 asm / 2 rodata);
`test_disc1_plan.py` → 8 tests OK; `disc1_plan.py --check` → 1025 spans;
`verify_us.sh --public` → PUBLIC_VERIFY=PASS, matching-C 695;
`profile_necessity.py` → 531/531 era leaves clean, 0 redundant;
`exact_rebuild.sh` → **EXACT_REBUILD_GATE=PASS** with
`sha1_orig=sha1_cand=452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

### Honest totals after the repair

Whole image: matched C **695**; non-C-matchable **184 funcs / 20252 words**
(`handwritten-gte-wrapper` 107/19958, `jr-t2` 53/163, `cop` 22/123,
`syscall` 2/8) + **33** alignment spans; `gte-inline` 5/192 informational.
Route view (`route_coverage.py`): matched C **332 / 5842**, provably non-C
76 / 5414, **real remaining asm 571 / 66437**.

### Next targets

`func_8008CBA8` (fan-in 12, 242w `D_800BCD80` dispatcher — dumped, large),
`func_80062D2C` (8, 124w pool allocator), `func_8005DB44` (9 — closest
near-miss: 2 words over, retail's `v0=a0-v1` differs from cc1's `v1=v0-a0`),
`func_800659F8` (frame residual), `func_80071964`/`80071994`, `func_80078C94`,
`func_80087798`, `func_80067CBC`.

## SESSION 2026-09-11 (cont. 18): 4 leaves (689 → 693) + cluster sweep

Goal unchanged. This session delivered **4 new `LINK_EXACT` leaves** (two of them
contiguous in the `0x561C8` row-accessor cluster) and re-verified the whole gate
set. Current count: **693 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `1022 spans (693 c, 327 asm,
2 rodata)`, geometry `0x1EE000`, plan
`f5bfe9ab8bba49880a16b62051248489fc41fc54fe42b5dc16f708ceca942bbb`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`tools/build/disc1_preflight.py` → `PASS (fast, 693 c / 327 asm / 2 rodata)`;
`tools/analysis/profile_necessity.py` → `529/529 era leaves clean; 0 redundant`.

| leaf | file span | size | era / profile |
|---|---|---:|---|
| `func_800659C8` | `0x561C8` | `0x30` | `-O2 -G0` (default) |
| `func_80065A60` | `0x56260` | `0x3C` | `-O2 -G0` (default) |
| `func_80065B70` | `0x56370` | `0xC8` | `-O2 -G0` (default) |
| `func_8005DB8C` | `0x4E38C` | `0x20` | `-O2 -G0` (default) |

All `LINK_EXACT` at the retail VMA. The `0x561C8` cluster is now three carved
C spans (`659C8`, `65A60`, `65A9C`) around asm islands; the two new entries
extend the row-accessor family already proven by `func_8006599C`/`80065A9C`.
`func_8005DB8C` (fan-in 9, global word + record-base helper) is the fourth.
Reports: `docs/evidence/func-<name>/REPORT.md`.

### Durable levers from these leaves

1. **`func_80065B70` is a tail-call target with no prologue.** Its only
   reference is a `li $t2,0x70 / jr $t2` tail jump from `func_80065B44`.
   cc1 emits retail's frameless straight-line `li`/`lui $at`/`sw|sb|sh`
   sequence (22 stores, every address absolute via `$at`) when the C body is a
   leaf with `return 0;` last; interleaved word/short/char declarations are
   load-bearing because the store width picks the opcode.
2. **Fold a scaled index into the base argument to fill the delay slot.**
   `func_80065A60`: `a1 = (a1 << 1) + (unsigned int)q; *(unsigned char *)(a1 + 1) = a2;`
   gives retail's `sb $a2,0x1($a1)` scheduled into the `jr` delay slot. The
   natural `*(q + (a1 << 1) + 1)` keeps the sum in `$v0` and lands the store
   before the epilogue (`SIZE_MISMATCH`).
3. **A split negative-offset local keeps the base register free.**
   `func_8005DB8C`: `q = base - 0x10` gives retail's `addiu v1,v0,-16`, and
   declaring the shift `off = a0 << 9` *before* `q` supplies `$a0` first so
   `$v0` stays free for the `lw`. Folding `-0x10` into the later `addu` (or
   reusing one pointer) reloads `$v1` (`MISMATCHES=4`). The source global is a
   **data symbol** (`&D_800A8038`), so the base is `lui`+`addiu`, not a folded
   `lui`+`ori` constant.
4. **`func_800659F8` (same cluster, `0x68`) is NOT carved.** It carries an
   unexplained 8-byte frame; every C phrasing was either 8 bytes short (no
   frame) or 8 bytes long (16-byte frame) versus the 56-byte body. Left as asm.

### `cop-inline` accounting — confirmed closed

The `gte-inline` bucket stays **5 spans / 192 words** (informational) and
`handwritten-gte-wrapper` **107 / 19958** (counted non-C). The top frontier
entries the mandate named (`func_800661A4`, `func_800661CC`, `func_8006698C`,
`func_8003B97C`) are already classified `handwritten-gte-wrapper`, so they no
longer inflate the liftable frontier. Honest whole-image totals: matched C
**693**; non-C-matchable **184 funcs / 20252 words** + 31 alignment spans;
`gte-inline` 5/192 informational. Route view: matched C **330 / 5834**,
provably non-C 76 / 5414, **real remaining asm 565 / 66207**. Manifest contract
unchanged for `tools/analysis/route_coverage.py` (counted classes under `spans`,
`gte-inline` out of `spans` and in the retained `matchable_cop_inline` key);
regenerating the manifest from the tool is byte-identical to the on-disk file.

### Park list audit

All 14 `PARK.md` files were re-checked: every parked function is still an asm
span (no stale park). No new parks this session — `func_80062D2C` (the fan-in-8
pool allocator) was attempted and remains unmatched (its list-relink store
ordering plus a `$s0`-frame shape resisted the `-O2 -G8` phrasing tried); it is
left as asm rather than parked, since the residual is not yet proven
source-invariant.

## SESSION 2026-09-11 (cont. 17): 15 leaves (674 → 689) + `cop-inline` retightened

Goal unchanged. This session delivered **15 new `LINK_EXACT` leaves** and
corrected the `cop-inline` split so that a lone GTE **command** op is no longer
miscounted as liftable. Current count: **689 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `1016 spans (689 c, 325 asm,
2 rodata)`, geometry `0x1EE000`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`tools/build/disc1_preflight.py` → `PASS (fast, 689 c / 325 asm / 2 rodata)`.

| leaf | file span | size | era / profile |
|---|---|---:|---|
| `func_80089F08` | `0x7A708` | `0x1C` | `-O2 -G0` (default) |
| `func_800C8C80` | `0xB9480` | `0x3C` | `-O2 -G0` (default) |
| `func_800C8CBC` | `0xB94BC` | `0x3C` | `-O2 -G0` (default) |
| `func_800C8CF8` | `0xB94F8` | `0x3C` | `-O2 -G0` (default) |
| `func_800CBBF0` | `0xBC3F0` | `0x3C` | `-O2 -G0` (default) |
| `func_800CBC2C` | `0xBC42C` | `0x3C` | `-O2 -G0` (default) |
| `func_800CBC68` | `0xBC468` | `0x3C` | `-O2 -G0` (default) |
| `func_800CCA40` | `0xBD240` | `0x38` | `-O2 -G0` (default) |
| `func_800CCA78` | `0xBD278` | `0x38` | `-O2 -G0` (default) |
| `func_800CCB6C` | `0xBD36C` | `0x3C` | `-O2 -G0` (default) |
| `func_800C9A34` | `0xBA234` | `0x3C` | `-O2 -G0` (default) |
| `func_800CD5B0` | `0xBDDB0` | `0x3C` | `-O2 -G0` (default) |
| `func_8008C6D0` | `0x7CED0` | `0x3C` | `-O2 -G0` (default) |
| `func_80016FE0` | `0x77E0` | `0x38` | `-O2 -G0` (default) |
| `func_8001784C` | `0x804C` | `0x30` | `era_o2_g8` |
| `func_80018718` | `0x8F18` | `0x3C` | `-O2 -G0` (default) |

All `LINK_EXACT` at the retail VMA. Priority fan-in cleared this session:
`func_80067CBC` (11) was analysed but is **not yet matched**; the eight
countdown-timer twins were a bulk sweep of a repeated overlay pattern.
Reports: `docs/evidence/func-<name>/REPORT.md` for each.

### Durable levers from these leaves

1. **A fold-in base keeps the displacement on the base register.**
   `func_80089F08`: `a0 = (a0 << 4) + (unsigned int)D_8009B3FC;` then
   `*(unsigned short *)(a0 + 0xC)` gives retail's `lhu v0,0xC(a0)`. Writing the
   whole address as one expression folds the `+0xC` into the address register.
2. **A second, independent `short` reload is load-bearing.** The whole
   countdown-timer family (`func_800C8C80`/`CBC`/`CF8`, `CBBF0`/`CBC2C`/`CBC68`,
   `CCA40`/`CCA78`, `CCB6C`, `C9A34`, `CD5B0`) has the shape
   `*(unsigned short *)(a2+4) -= 8; *(unsigned short *)(a2+6) += STEP;
   if (*(short *)(a2+4) < 0x14) { *(unsigned short *)(a2+4) = 0; a1[1] = 2; }`.
   The `if` must re-read `a2+4` as `short`; folding the compare into the value
   already in hand changes the register home. The byte-field variants
   (`CCA40`/`CCA78`, `CCB6C`) step `a2+3` with `lbu` and re-read it as
   `signed char` (`lb`).
3. **`if/else` beats early return for the 1/0 gate writers.**
   `func_80016FE0` (`D_8009D2E8` bit 0) and `func_80018718` (`D_800A76C4`
   bit 2) match only with `if (flag) *out = X; else *out = Y;`; the early-return
   spelling reverses the branch and duplicates the `li v0,1`.
4. **`unsigned int` counter → `sltiu`.** `func_8008C6D0` needs an `unsigned int`
   `do/while` counter to emit retail's `sltiu 0x18`; a signed `int` gives `slt`.
5. **`-G8` for a `0x590($gp)` pointer global.** `func_8001784C` reads
   `D_8009D300` (`extern unsigned int *`) at `0x590($gp)`; only `era_o2_g8`
   keeps it in the small-data section (assigned in
   `disc1_build_profiles.json`).

### `cop-inline` accounting correction

`nonmatchable_spans.py` previously treated an isolated COP **command** op
(e.g. one `rtps`) inside branch/call-bearing code as `gte-inline` (liftable).
A single `rtps` is a literal Psy-Q `gte_rtps` macro and has no C spelling, so
the rule is now: **any** GTE command op, **or** a contiguous COP run >= 3,
**or** no conditional branch and no call, is `handwritten-gte-wrapper`
(non-C). Only an isolated COP2 *register-transfer* pair (`mtc2`/`mfc2`/`swc2`/
`lwc2`) inside branch/call-bearing integer code stays `gte-inline`
(informational).

Result: `handwritten-gte-wrapper` **103 spans / 19008 words → 107 / 19958**;
`gte-inline` **9 spans / 1142 words → 5 / 192** (the four `rtps` spans
`func_800D2B58`, `func_800DB25C`, `func_800D1DEC`, `func_800D2104` moved into
the counted class). The remaining 5 informational spans are `func_8003EAC8`,
`func_800130B4`, `func_80077F7C`, `func_80078004`, `func_80078094`.

Honest totals now (whole image): matched C **689**; non-C-matchable **184 funcs
/ 20252 words** (`handwritten-gte-wrapper` 107/19958, `handwritten-jr-t2`
53/163, `handwritten-cop` 22/123, `handwritten-syscall` 2/8) + 31 alignment
spans; `gte-inline` 5/192 informational. Route view: `route_coverage.py` →
matched C 328 / 5776 words, provably non-C 76 / 5414, **real remaining asm
567 / 66265**. Manifest contract unchanged for the sibling consumer (counted
classes under `spans`, `gte-inline` out of `spans` and in the retained
`matchable_cop_inline` key).

**YAML syntax repair:** a hard-wrapped comment continuation at the
`func_8008F430` block (a bare ` cursor by` line) broke strict YAML parsing and
was caught by the sibling's `tools/build/disc1_preflight.py`; it is now a single
well-formed comment. `disc1_preflight.py` is clean.

Next targets unchanged: `func_8008CBA8` (fan-in 12, 242w), `func_80062D2C`
(8, 124w), `func_80067CBC` (11, 23w), then the small no-call sweep continues.

## SESSION 2026-09-11 (cont. 16): 4 leaves (670 → 674) + the `cop-inline` split

Goal unchanged. This session delivered **4 new `LINK_EXACT` leaves** and split
the old 117-span / 20196-word `cop-inline` bucket into a counted
`handwritten-gte-wrapper` class and a 9-span informational `gte-inline` class.
Current count: **674 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `997 spans (674 c, 321 asm,
2 rodata)`, geometry `0x1EE000`, plan SHA-256
`830424c3b7e2c31ee1179e09605492426189ae8e2c14fe81cf10629390527c6f`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`;
`tools/analysis/profile_necessity.py` → `510/510 era leaves clean`.

| leaf | file span | size | era / profile | fan-in |
|---|---|---:|---|---|
| `func_80038910` | `0x29110` | `0x30` | `era_o2_g8` | 2 |
| `func_8007DBC8` | `0x6E3C8` | `0x3C` | `-O2 -G0` (default) | 1 |
| `func_80087050` | `0x77850` | `0x40` | `-O2 -G0` (default) | 1 |
| `func_8007E594` | `0x6ED94` | `0x30` | `-O2 -G0` (default) | 1 |

All `LINK_EXACT` at the retail VMA (`tools/analysis/era_link_check.py`).
Reports: `docs/evidence/func-80038910|func-8007DBC8|func-80087050|
func-8007E594/REPORT.md`.

### `cop-inline` accounting fix (non-C taxonomy)

`tools/analysis/nonmatchable_spans.py` now emits two classes where it used to
emit one `cop-inline`:

- **`handwritten-gte-wrapper` — 103 spans / 19008 words, COUNTED as non-C.**
  A span with a contiguous run (>=3) of raw GTE ops (`ctc2`/`mtc2`/`mfc2`/
  `cfc2`/`lwc2`/`swc2` **and** the libgte command ops `mvmva`/`rtps`/`rtpt`/
  `nclip`/`avsz3`/`avsz4`/`sqr`/`op`/`gpf`/`gpl`/`intpl`/…), or with no
  conditional branch and no call at all while containing a COP op. 1810 of the
  2419 COP transfer ops in the bucket are flagged `/* handwritten instruction */`
  by the disassembler; the GTE command ops have no C spelling at all; and **0 of
  the 674 matched C leaves contains a single COP op** — empirical proof `cc1`
  never synthesises one.
- **`gte-inline` — 9 spans / 1142 words, informational only, NOT counted.**
  A COP op appears only as an isolated pair (run < 3) inside clean
  branch/call-bearing integer code: `func_800D2B58`, `func_800DB25C`,
  `func_800D1DEC`, `func_800D2104`, `func_800130B4`, `func_80078094`,
  `func_80078004`, `func_80077F7C`, `func_8003EAC8`. This is the genuine
  next-pass frontier.

`handwritten-cop` grew 17 → 22 spans as the `_max_gte_run`/`all-COP` boundary
was tightened. Manifest contract unchanged for the sibling consumer: counted
classes live under `spans`; `gte-inline` stays out of `spans` and is emitted in
the retained `matchable_cop_inline` key (`class: "gte-inline"`), so
`route_coverage.py` keeps reading it as informational without edits.

Honest totals now (whole image): matched C **674**; non-C-matchable **180 funcs
/ 19302 words** (`handwritten-gte-wrapper` 103/19008, `handwritten-jr-t2`
53/163, `handwritten-cop` 22/123, `handwritten-syscall` 2/8) + 31 alignment
spans; `gte-inline` 9/1142 informational. Route view:
`route_coverage.py` → matched C 326 / 5750 words, provably non-C 76 / 5414,
**real remaining asm 569 / 66291**.

### Durable levers from these leaves

1. **A narrow local type controls the destination register.** `func_8007DBC8`
   needs `unsigned short v = D_8009B3FC[a0];` so the value homes in `$a0`
   (`lhu a0,0(a0)` / `sllv v0,a0,v0` / `move v0,a0`); an `unsigned int` local
   lands in `$v1` plus a redundant `move`.
2. **`volatile` on a polled global stops load hoisting.** `func_80087050`
   needs `extern volatile unsigned int D_8009D2E0`; without it cc1 hoists the
   second load out of the loop.
3. **Arm order picks the loop's address.** `func_80087050` only matches with
   `if (a0 == 0) { while(...); return 0; } return D_8009D2E0 & 1;` — the
   mirrored spelling puts the `a0 != 0` arm first.
4. **A parallel pointer keeps a down-counting byte walk at ROM size.** In
   `func_8007E594`, `q = a0 + 3` + `q[5]`/`q--` reproduces retail; indexing `a0`
   directly makes cc1 emit `addu v0,v1,a0` and hoist the store, and an ascending
   byte loop grows the frame past `0x30`.
5. **`-G8` keeps gp-relative store bases.** `func_80038910` matches only on
   `era_o2_g8`; under `-G0` cc1 splits each of the seven stores into an
   absolute `lui $at` sequence (9 mismatches, object 0x50 vs ROM 0x30).

`func_8007E4E0` was attempted and **not** matched (retail's `li v0,0xDF80`
after the two `%hi/%lo` pairs is frontend-insensitive to source order/loop form);
no source was left behind. Next targets unchanged: `func_8008CBA8` (fan-in 12,
242w), `func_80062D2C` (8, 124w), then the small no-call sweep continues.

## SESSION 2026-09-11 (cont. 15): 23 leaves (647 → 670) + a GTE-macro ruling

Goal unchanged. This session delivered **23 new `LINK_EXACT` leaves** (all
small no-call leaves found by a systematic sweep of `asm/disc1/`), removed two
stale PARK files, PARKED one true frontend divergence, and settled the
`cop-inline` frontier's nature. Current count: **670 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `991 spans (670 c, 319 asm,
2 rodata)`, geometry `0x1EE000`, plan SHA-256
`dd084b9176ba5c9be09780c1be4e26ad3d8789e0059ec0ad16d00789a74dd573`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`;
`tools/analysis/profile_necessity.py` → `506/506 era leaves clean`.

| leaf | file span | size | era / profile | fan-in |
|---|---|---:|---|---|
| `func_80073D24` | `0x64524` | `0x34` | `-O2 -G0` (default) | 4 |
| `func_80073D58` | `0x64558` | `0x30` | `-O2 -G0` (default) | 2 |
| `func_80073D88` | `0x64588` | `0x30` | `-O2 -G0` (default) | — |
| `func_80073DB8` | `0x645B8` | `0x30` | `-O2 -G0` (default) | — |
| `func_80051684` | `0x41E84` | `0x30` | `-O2 -G0` (default) | 3 |
| `func_80067B40` | `0x58340` | `0x34` | `-O2 -G0` (default) | 2 |
| `func_80076B58` | `0x67358` | `0x40` | `-O2 -G0` (default) | 2 |
| `func_80076BE0` | `0x673E0` | `0x30` | `-O2 -G0` (default) | 2 |
| `func_80087864` | `0x78064` | `0x28` | `-O2 -G0` (default) | 2 |
| `func_8008F430` | `0x7FC30` | `0x40` | `-O2 -G0` (default) | 1 |
| `func_8008F784` | `0x7FF84` | `0x38` | `-O2 -G0` (default) | 1 |
| `func_80065A9C` | `0x5629C` | `0x38` | `-O2 -G0` (default) | 1 |
| `func_80018718` | `0x8F18` | `0x3C` | `-O2 -G0` (default) | 2 |
| `func_8009071C` | `0x80F1C` | `0x38` | `-O2 -G0` (default) | 2 |
| `func_80036DF8` | `0x275F8` | `0x3C` | `-O2 -G0` (default) | 2 |
| `func_8008A02C` | `0x7A82C` | `0x3C` | `-O2 -G0` (default) | 2 |
| `func_8007A400` | `0x6AC00` | `0x34` | `era_o2_g0_symbol_at_temp` (patch 5) | 2 |
| `func_8007A434` | `0x6AC34` | `0x34` | `era_o2_g0_symbol_at_temp` (patch 5) | 2 |
| `func_800858E8` | `0x760E8` | `0x30` | `era_o2_g0_symbol_at_temp` (patch 5) | 2 |
| `func_80085918` | `0x76118` | `0x34` | `era_o2_g0_symbol_load_dest_temp` (patch 4) | 2 |
| `func_800428D4` | `0x330D4` | `0x3C` | `era_o2_g0_symbol_at_temp` (patch 5) | 2 |
| `func_800556E8` | `0x45EE8` | `0x3C` | **new** `era_o2_g8_symbol_at_temp` (patch 5) | 2 |
| `func_80058E08` | `0x49608` | `0x3C` | **new** `era_o2_g8_symbol_at_temp` (patch 5) | 2 |

Durable levers from this session:

1. **Patch 4 vs patch 5 is decided by whether the indexed symbolic load's
   address temp is the *destination* register** — `func_80085918` has a `nor`
   immediately consuming the loaded value, and retail keeps the dest-register
   temp with the `%lo` displacement (patch 4, profile
   `era_o2_g0_symbol_load_dest_temp`); `func_8007A400`/`7A434`/`858E8`/`428D4`
   use the `$at` form (patch 5, `era_o2_g0_symbol_at_temp`). Both are
   load-bearing: the wrong knob costs 9–10 words.
2. **The patch-5 knob composes with `-G8`** — `func_800556E8`/`58E08` need the
   new profile `era_o2_g8_symbol_at_temp` (`-O2 -G8` **plus**
   `MASPSX_SYMBOL_AT_TEMP=1`). Removing the env drops 9 words. Never assign a
   patch-5 leaf to plain `era_o2_g8`.
3. **Early-return polarity for a bound-checked table getter** — the
   `if (i < N) return table[i]; return fallback;` spelling emits a
   `bnez`-into-fallthrough; retail's `if (i >= N) return fallback;` order
   (`beqz` to the exit arm) is required (`func_8007A400`).
4. **A signed `int` index gives `slti`; `unsigned` gives `sltiu`** — one word,
   at the exact site (`func_800858E8`).
5. **Two-guard signed range getters** (`func_800556E8`/`58E08`) reproduce retail's
   double `addu $v0,$zero,$zero` zero-init only as two separate early returns
   (`if (a0 < 0) return 0; if (a0 >= limit) return 0;`).
6. **Loop/sentinel spellings**: `c = a1 - 1` with `while (--c != -1)`
   reproduces retail's preheader/sentinel pair (`func_80076B58`); the
   `do { ... } while (a2)` form with a pre-loop `d = a1 - *a0` and a parallel
   `p = a0 + 1` pointer gives the `0x40`-stride pair incrementer
   (`func_8008A02C`).
7. **Named address temporaries**: a `q = p + 2` local keeps the second `sw`
   before the `hi` load (`func_8008F430`); a named `unsigned int t` for the
   second computed address fixes operand order (`func_8009071C`); split nested
   `if (p)` blocks keep the double-null check (`func_80051684`); one `$2` base
   pin plus absolute `lui $at` byte stores for the adjacent pair
   (`func_80067B40`).
8. **`volatile` keeps deliberate duplicate stores** — the five-word state seed
   `D_800A76A8 = 0; D_800A76A8 = 0x1499700; …` (`func_80036DF8`) and the paired
   `D_800B1624` base loads (`func_80065A9C`) both vanish without it.
9. **`ctc2`-only "functions" are PSY-Q GTE macro wrappers, not C** — the
   `cop-inline` bucket's top fan-in entries (`func_800661A4`/`661CC`,
   `func_8006698C`, `func_8003B97C`) are literally
   `lw`/`sll`/`ctc2 $t4,$N` sequences. No LLVM-built cc1 2.7.2 emits `ctc2`
   from any C construct, so these are macro-expansion wrappers rather than
   liftable C; documented here so the frontier is not miscounted. (Non-COP
   neighbours such as `func_800669B0`/`66A00`/`674C0` remain valid targets.)
10. **A `register long long p asm("$2")` pin cannot fix DImode half-read order
    or a dead `sra` tail** — `func_8003708C` (fan-in 16) is PARKED
    (`docs/evidence/func-8003708C/PARK.md`): the pin recovers the register home
    (7 → 5 mismatched words) but cc1 still reads `mfhi` before `mflo` and keeps
    the dead `sra $3,$3,16` after the `or`.

PARK files removed this session (stale — those leaves have since matched):
`docs/evidence/func-8006E6D4/PARK.md`, `docs/evidence/func-8007FBF0/PARK.md`
(both leaves are `c` spans with `REPORT.md`).

New PARK: `docs/evidence/func-8003708C/PARK.md` (fan-in 16).

Honest three-way coverage after this session (route 971 fns; `route_coverage.py`
now reads 670 matched C):

| bucket | functions | words |
|---|---:|---:|
| matched C (YAML `c` spans) | 670 | 5738 |
| non-C-matchable, on-route | 47 | 147 |
| **real remaining asm** | **599** | **71570** |

Next targets (ranked): `func_80079FB4` (fan-in 19, PARKED), `func_80073A44`
(17, PARKED), `func_8006DE80` family (15, PARKED), `func_8008CBA8` (12, 242w —
large `D_800BCD80` state dispatcher, no COP), `func_80067CBC` (11, PARKED,
4-word scheduling residual), `func_8005DB44` (9, PARKED, reassociation),
`func_80062D2C` (8, 124w — `D_800****` pool allocator). Also the many small
no-call leaves in `asm/disc1/` that this session's scan enumerated
(`func_8007E4E0`, `func_8007E594`, `func_80038910`, `func_8007DBC8`,
`func_80087050`, `func_8008F430`-family neighbours, `func_800599xx`).

## SESSION 2026-09-11 (cont. 14): 15 leaves (647 → 662) + stale parks cleaned

Goal unchanged. This session delivered **15 new `LINK_EXACT` leaves** and
cleaned two stale PARK files. Current count: **662 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `981 spans (662 c, 317 asm,
2 rodata)`, geometry `0x1EE000`, plan SHA-256
`e584072c97f93e9619769d84b4f667bcdf0ff9032c0e0ae60263db542441e41d`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`;
`tools/analysis/profile_necessity.py` → `498/498 era leaves clean`.

| leaf | file span | size | era / profile | fan-in |
|---|---|---:|---|---|
| `func_80073D24` | `0x64524` | `0x34` | `-O2 -G0` (default) | 4 |
| `func_80073D58` | `0x64558` | `0x30` | `-O2 -G0` (default) | 2 |
| `func_80073D88` | `0x64588` | `0x30` | `-O2 -G0` (default) | — |
| `func_80073DB8` | `0x645B8` | `0x30` | `-O2 -G0` (default) | — |
| `func_80051684` | `0x41E84` | `0x30` | `-O2 -G0` (default) | 3 |
| `func_80067B40` | `0x58340` | `0x34` | `-O2 -G0` (default) | 2 |
| `func_80076B58` | `0x67358` | `0x40` | `-O2 -G0` (default) | 2 |
| `func_80076BE0` | `0x673E0` | `0x30` | `-O2 -G0` (default) | 2 |
| `func_80087864` | `0x78064` | `0x28` | `-O2 -G0` (default) | 2 |
| `func_8008F430` | `0x7FC30` | `0x40` | `-O2 -G0` (default) | 1 |
| `func_8008F784` | `0x7FF84` | `0x38` | `-O2 -G0` (default) | 1 |
| `func_8007A400` | `0x6AC00` | `0x34` | `era_o2_g0_symbol_at_temp` (patch 5) | 2 |
| `func_8007A434` | `0x6AC34` | `0x34` | `era_o2_g0_symbol_at_temp` (patch 5) | 2 |
| `func_800858E8` | `0x760E8` | `0x30` | `era_o2_g0_symbol_at_temp` (patch 5) | 2 |
| `func_80085918` | `0x76118` | `0x34` | `era_o2_g0_symbol_load_dest_temp` (patch 4) | 2 |

Durable levers from this session:

1. **Patch 4 vs patch 5 is decided by whether the indexed symbolic load's
   address temp is the *destination* register** — `func_80085918` has a `nor`
   immediately consuming the loaded value, and retail keeps the dest-register
   temp with the `%lo` displacement (patch 4, profile
   `era_o2_g0_symbol_load_dest_temp`); `func_8007A400`/`7A434`/`858E8` use the
   `$at` form (patch 5, `era_o2_g0_symbol_at_temp`). Both are load-bearing:
   the wrong knob costs 9–10 words.
2. **Early-return polarity for a bound-checked table getter** — the
   `if (i < N) return table[i]; return fallback;` spelling emits a
   `bnez`-into-fallthrough; retail's `if (i >= N) return fallback;` order
   (`beqz` to the exit arm) is required (`func_8007A400`). The `i < N` form
   leaves 9 mismatched words.
3. **A signed `int` index gives `slti`; `unsigned` gives `sltiu`** — one word,
   at the exact site (`func_800858E8`).
4. **Loop-sentinel spelling**: a `do { ... } while (--c != -1);` with
   `c = a1 - 1` reproduces retail's `addiu $a2,$a1,-1` preheader and
   `addiu $a1,$zero,-1` sentinel (`func_80076B58`).
5. **Named cursor locals materialize the right store order** — a `q = p + 2`
   local keeps the second `sw` before the `hi` load (`func_8008F430`);
   splitting the guard into two nested `if (p)` blocks keeps the double-null
   check (`func_80051684`); one `$2` base pin plus absolute `lui $at` byte
   stores for the adjacent pair (`func_80067B40`).
6. **`ctc2`-only "functions" are PSY-Q GTE macro wrappers, not C** — the
   `cop-inline` bucket's top fan-in entries (`func_800661A4`/`661CC`,
   `func_8006698C`, `func_8003B97C`) are literally
   `lw`/`sll`/`ctc2 $t4,$N` sequences. No LLVM-derived cc1 2.7.2 emits `ctc2`
   from any C construct, so these are macro-expansion wrappers rather than
   liftable C; documented here so the frontier is not miscounted. (Non-COP
   neighbours such as `func_800669B0`/`66A00`/`674C0` remain valid targets.)
7. **A `$2` pin cannot fix a DImode result register** — `func_8003708C` (fan-in
   16) is a true frontend divergence: retail reads `mflo $v0` before
   `mfhi $v1` and has **no dead `sra`** after the `or`. A
   `register long long p asm("$2")` pin fixes the register home (recovers 2
   words) but cc1 still emits `mfhi` first and keeps the dead
   `sra $3,$3,16`. PARKED precisely (`docs/evidence/func-8003708C/PARK.md`).

PARK files removed this session (stale — those leaves have since matched):
`docs/evidence/func-8006E6D4/PARK.md`, `docs/evidence/func-8007FBF0/PARK.md`
(both leaves are `c` spans with `REPORT.md`).

New PARK: `docs/evidence/func-8003708C/PARK.md` (fan-in 16, DImode half-read
order + dead-`sra` tail).

Honest three-way coverage after this session (route 979 fns; `route_coverage.py`
now reads 662 matched C):

| bucket | functions | words |
|---|---:|---:|
| matched C (YAML `c` spans) | 662 | 5693 |
| non-C-matchable, on-route | 47 | 147 |
| **real remaining asm** | **602** | **71615** |

Next targets (ranked): `func_80079FB4` (fan-in 19, PARKED), `func_80073A44`
(17, PARKED), `func_8006DE80` family (15, PARKED), `func_8008CBA8` (12, 242w —
large `D_800BCD80` state dispatcher, no COP), `func_80067CBC` (11, PARKED,
4-word scheduling residual), `func_8005DB44` (9, PARKED, reassociation),
`func_80062D2C` (8, 124w — `D_800****` pool allocator). Also the many small
no-call leaves in `asm/disc1/` that this session's scan enumerated
(`func_8007E4E0`, `func_800858E8`-family neighbours, `func_8007E594`,
`func_80038910`, `func_800599xx`, the `0x8007Exxx` GTE-adjacent block).

## SESSION 2026-09-11 (cont. 13): 11 leaves (636 → 647) + a PARK resolved

Goal unchanged. This session delivered **11 new `LINK_EXACT` leaves**, one of
them a **resolution of the `func_80026FD0` PARK** (the earlier park was wrong),
and a **conservative refinement of the non-C-matchable taxonomy** (the new
`cop-inline` bucket). Current count: **647 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `965 spans (647 c, 316 asm,
2 rodata)`, geometry `0x1EE000`, plan SHA-256
`8a2306c96ad85908e82ee4ad78a799c42a8da547441dbbcf0dcfe39d9a423026`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`;
`tools/analysis/profile_necessity.py` → `483/483 era leaves clean`.

| leaf | file | size | era / profile | fan-in |
|---|---|---:|---|---|
| `func_80026FD0` | `0x177D0` | `0x28` | `-O2 -G8` (`era_o2_g8`) — **was PARKED** | 4 |
| `func_8008FFC0` | `0x807C0` | `0x24` | `-O2 -G0` (default) | 4 |
| `func_80090054` | `0x80854` | `0x24` | `-O2 -G0` (default) | 2 |
| `func_800900E4` | `0x808E4` | `0x24` | `-O2 -G0` (default) | 2 |
| `func_80090178` | `0x80978` | `0x24` | `-O2 -G0` (default) | 2 |
| `func_8007CE80` | `0x6D680` | `0x2C` | `-O2 -G0` (default) | 1 |
| `func_8008F4E8` | `0x7FCE8` | `0x2C` | `-O2 -G0` (default) | 2 |
| `func_8008FBFC` | `0x803FC` | `0x2C` | `-O2 -G0` (default) | 3 |
| `func_8008FCE4` | `0x804E4` | `0x2C` | `-O2 -G0` (default) | 2 |
| `func_8008F6B0` | `0x7FEB0` | `0x44` | `-O2 -G0` (default) | 2 |
| `func_8008FC28` | `0x80428` | `0x50` | `-O2 -G0` (default) | 5 |

Ten of the eleven are the **serial-cursor-reader family** in the
`0x7F000`–`0x81000` stream block: `lw *(unsigned char **)a0` cursor, `+1`,
store back, then a fetched byte transformed into a halfword field. Fan-in
cleared this session: 11 leaves on-path, top being `func_8008FC28` (5) and
`func_80026FD0`/`func_8008FFC0` (4 each).

Evidence: `docs/evidence/func-80026FD0/REPORT.md` (replaces its PARK),
`func-8008FFC0`, `func-80090054`, `func-800900E4`, `func-80090178`,
`func-8007CE80`, `func-8008F4E8`, `func-8008FBFC`, `func-8008FCE4`,
`func-8008F6B0`, `func-8008FC28`.

Durable levers from this session:

1. **`lb` comes from `signed char`, not `char`** — and a `signed char[16]`
   **array** declaration forces an **absolute** base (`lui`/`lb`) while keeping
   the sign-extending load, which is how `func_80026FD0`'s gate matches under
   `-G8`; the two store values in that leaf have **different signedness**
   (`unsigned char = 0x80` → `li $v0,0x80`; `signed char = -8` → `li $v0,-8`).
   The earlier PARK was a wrong-typing artefact, not a frontend divergence.
2. **A named loop-local `unsigned int v = *a1;`** (rather than `*a0 = *a1`)
   preserves retail's `lw` / `addiu a1` / `addiu i` / `sw a0` order
   (`func_8007CE80`).
3. **A named value local between the pointer-advance and the store** keeps the
   `lbu` late (`func_8008F4E8`); reading `*p` at the store reschedules it.
4. **Two `(unsigned char **)` casts, not a struct**, give the `lw+addiu+sw`
   then `lbu+sll+sh` pair (`func_8008FFC0`).
5. **Fresh pointer locals per cursor walk** are required — one expression
   reading `*(unsigned char **)a0` twice makes cc1 CSE the loads
   (`func_8008FC28`).
6. **Flag teardown masks are bit-exact**: `&= ~2` clears only bit 1
   (`0xFFFD`); `&= ~3` (`0xFFFC`) is a one-word miss (`func_80090054`).

PARKs added/updated this session:

- `docs/evidence/func-80067CBC/PARK.md` — re-attacked per mandate; the residual
  is now precisely 4 words (retail materialises the second RMW's `~0xC000` mask
  **before** reloading the base; cc1 emits the mask after the load). Tried two
  pointer-local forms, `volatile`, a ternary, a carried `v`, and four rungs — no
  change. Constant-materialisation-vs-load ordering class; no new lever.
- `func_8005DB44` — **RESOLVED this session (no longer parked)**. See
  `docs/evidence/func-8005DB44/REPORT.md`: the reassociation residual is
  defeated by making the base a **local pointer decremented in place**
  (`p = &D_800A8038; … p -= 4`) plus a **block-local `int sh = a0 << 5;`**
  assigned before the decrement; era `-O1 -G0`, `LINK_EXACT`. Its adjacent twin
  `func_8005DAFC` is a genuine `$v0`/`$v1` allocation residual and stays `asm`.
- `docs/evidence/func-800739C4/PARK.md` — retry log added: an early-`return`
  per arm and a `v0`-carried compare ladder both make it *worse*
  (`word mismatches=18` vs 12). Confirms the tail-duplication is downstream of
  C control flow.

### Taxonomy refinement (conservative)

`tools/analysis/nonmatchable_spans.py` now reports a **`cop-inline`** bucket
(**117 spans / 20196 words**) for spans that merely *contain* a COP2/GTE op but
are otherwise ordinary integer code. These are **not** counted as
non-C-matchable (a C front end cannot emit the raw COP2 op, but the bulk of each
body is liftable), so the honest total is unchanged at **72 functions /
248 words** (`handwritten-jr-t2` 53/163, `handwritten-cop` 17/77,
`handwritten-syscall` 2/8; 31 alignment-filler spans). `cop-inline` is the
natural next-pass frontier and is documented in
`docs/evidence/non-c-matchable/REPORT.md`; `MANIFEST.json` regenerated.

Honest three-way coverage after this session (route 979 fns):

| bucket | functions | words |
|---|---:|---:|
| matched C (YAML `c` spans) | 647 | 5593 |
| non-C-matchable, on-route | 47 | 147 |
| **real remaining asm** | **616** | **71954** |

Next targets (ranked, unchanged): `func_80079FB4` (fan-in 19, PARKED),
`func_80073A44` (17, PARKED), `func_8003708C` (16 — a `long long` mult whose
`mfhi`/`mflo` register home and dead-`sra $3` tail are frontend-determined),
`func_8006DE80` family (15, PARKED), `func_8008CBA8` (12, 242w — a large
`D_800BCD80` state dispatcher), `func_80062D2C` (8, 124w — a `D_800****` pool
allocator with the `-0x8($a1)` rolling store), `func_8006698C` (7, 117w).

## SESSION 2026-09-11 (cont. 12): honest non-C-matchable coverage + 5 leaves (631 → 636)

Goal unchanged. This session delivered (a) the **honest remaining-asm
classification**: provably non-C-matchable split-asm spans are now separated
from real remaining work by `tools/analysis/nonmatchable_spans.py`, and (b)
**5 new `LINK_EXACT` leaves**. Current count:
**636 matching C leaves** (`python3 tools/build/disc1_plan.py --check` →
`947 spans (636 c, 309 asm, 2 rodata)`, geometry `0x1EE000`, plan SHA-256
`8bd0fa89e3ec4c1a...`); `python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`;
`tools/analysis/profile_necessity.py` → `472/472 era leaves clean`.

| leaf | file | size | era / profile | fan-in |
|---|---|---:|---|---|
| `func_80073CF4` | `0x644F4` | `0x30` | `-O2 -G0` (default) | 4 |
| `func_8007F72C` | `0x6FF2C` | `0x4C` | `-O2 -G0` + patch 3 (`era_o2_g0_fill_epilogue_delay_slot`) | 5 |
| `func_800866A4` | `0x76EA4` | `0x4C` | `-O2 -G0` (default) | 7 |
| `func_8005DC4C` | `0x4E44C` | `0x50` | `-O2 -G0` (default) | 4 |
| `func_800718D0` | `0x620D0` | `0x74` | `-O2 -G0` (default) | 4 |
| `func_80073A44` (`0x178`) + `func_800739C4` (`0x64`) + `func_80026FD0` (`0x28`) + `func_80062F3C` (`0x60`) + `func_80085EB4` (`0x5C`) **PARKED** | | | | |
Evidence: `docs/evidence/func-80073CF4/REPORT.md`, `func-8007F72C`,
`func-800866A4`, `func-8005DC4C`, `func-800718D0`; parks
`func-80073A44/PARK.md` (redundant spilled-temp poll + pre-frame global loads),
`func-800739C4/PARK.md` (retail tail-duplicates the `jal` into both compare
arms; era cc1 shares one), `func-80026FD0/PARK.md` (era cc1 emits `lbu` for a
signed-char truthiness gate; retail `lb` — isolated probe included),
`func-80062F3C/PARK.md` (gp-relative head load + loop-head scheduling),
`func-80085EB4/PARK.md` (`sllv` amount register home + symbol-base CSE).

Durable levers from this session: (1) a **`&D_800A8028 + D_800A802C` byte-base
plus `unsigned char *` walker** reproduces retail's `la`+`addu` stream base for
pooled records (`func_8005DC4C`); (2) **early-return polarity beats `?:`** — the
`if (a0 >= n) return 0;` form gives retail's `sltu`/`beqz` + fallthrough tail
where the if-else/shared-exit form adds a `j`+`move` (96 vs 80 bytes); (3) the
walker siblings `func_800718D0` need the **live `s0` pointer** (not a bool) to
gate the second call and the tail kept in `$s1`.

### Honest non-C-matchable coverage (new tool)

`tools/analysis/nonmatchable_spans.py` classifies provably non-C-matchable
spans with positive per-span instruction evidence and reports honest counts.
`docs/evidence/non-c-matchable/REPORT.md` is the full write-up; the
machine-readable inventory is `MANIFEST.json`.

Counts after this session (route 979 fns: 315 matched C / 664 asm):

| bucket | functions | words |
|---|---:|---:|
| matched C (YAML `c` spans) | 636 | 5583 |
| non-C-matchable, on-route | 47 | 147 |
| non-C-matchable, off-path | 25 | 101 |
| **non-C-matchable total** | **72** | **248** |
| alignment filler (not functions) | 31 spans | 31 |
| real remaining asm | 617 | 71964 |

Classes: `handwritten-jr-t2` 53 / 163w (BIOS `addiu $t2`/`jr $t2`/
`addiu $t1` triples in `621E4.s`, `64B54.s`, `6E538.s`, `6E6C0.s`, `75F44.s`;
C's indirect call emits `jal $31,$at` with the argument in `$a0` — proven
differentially), `handwritten-cop` 17 / 77w (pure COP2/GTE `lwc2`/`mtc2`/
`mfc2`/`ctc2` primitives in `68664.s`; no C naming path), `handwritten-syscall`
2 / 8w (`func_80072714`/`724`). `alignment-filler` (31 lone `nop` spans) is
layout padding, not functions. Deliberately NOT reclassified:
`func_8001F814` (already `NONMATCHING_C`) and the 24 ACCEPTED-RESIDUAL leaves
(compiler-decision residuals, distinct taxonomy). Large real-C functions that
merely embed a BIOS call (`func_8003BCE0`, `func_80067E1C`, `func_8007E334`)
are not classified. Repository-wide there are 1879 `/* handwritten
instruction */`-flagged instructions, but most sit inside otherwise-C-matchable
bodies; only the 72 pure spans above are whole-function non-C.

### Prior session (2026-09-11 cont. 11): profile-necessity gate + 4 leaves (627 → 631)

Two things: a **durable loud check for the silent-profile-fallback hazard**,
and **4 new `LINK_EXACT` leaves** (one, `func_80077DC4`, on-path fan-in 17).

| leaf | file | size | era / profile | fan-in |
|---|---|---:|---|---|
| `func_80077CF4` | `0x684F4` | `0x3C` | `-O2 -G0` (default) | 5 |
| `func_80077D30` | `0x68530` | `0x90` | `-O2 -G0` + patch 5 (`era_o2_g0_symbol_at_temp`) | 3 |
| `func_80077DC4` | `0x685C4` | `0xA0` | `-O2 -G0` + patch 5 (`era_o2_g0_symbol_at_temp`) | **17** |
| `func_80052E30` | `0x43630` | `0x80` | `-O2 -G8` + `era_o2_g8_force_descriptor_absolute` | 7 |

Evidence: `docs/evidence/func-80077CF4/REPORT.md`, `func-80077D30`,
`func-80077DC4`, `func-80052E30`.

### The profile-necessity gate (new tool)

`tools/analysis/profile_necessity.py` (with a small fix to
`tools/analysis/era_link_check.py` so it honors `ERA_ASPSX_VER` and
`MASPSX_EXPAND_DIV`, matching `disc1_build.py`). For every **era** leaf it
compiles at its retail VMA under the default profile and under the assigned
profile and asserts:

- default exact ⇒ assignment must be absent (else **REDUNDANT**);
- default not exact ⇒ assignment must be present (**MISSING**) and exact
  (else **WRONG**).

Non-era (modern-toolchain) leaves are reported as unverifiable (164 of them);
`era_link_check.py` only reproduces era codegen. Results are cached in
`build/profile_necessity_cache.json` keyed by leaf+profile+VMA+size, so reruns
are cheap. Exit is non-zero on any hard defect; `--allow-redundant` downgrades
REDUNDANT.

Run: `python3 tools/analysis/profile_necessity.py --jobs 8` →
`467/467 era leaves clean; 0 hard defect(s); 0 redundant` →
`PROFILE_NECESSITY=PASS`.

The tool immediately paid for itself:

- **Real record defects removed.** Seven assignments were provably not
  load-bearing and were deleted: `func_8001735C` and `func_800176B8`
  (`era_o2_g8_force_d8009d2f0_absolute`), `func_80017410`/`func_800197D0`
  (`era_o2_g8`), `func_8003E680`, `func_8006A5BC` (`era_o1_g0`) and
  `func_80052BCC` (`era_o1_g0_sched2`). `-O2 -G0`, `-O1 -G0`, `-O1 -G0
  -fschedule-insns2` all reproduce them. `-G8` vs `-G0` is a no-op for leaves
  with no small data, which is why the `-G8` ones looked load-bearing.
  `func_8004E94C` etc. were **re-verified per-leaf** and kept (the tool tests
  each span, so no bulk false-revert).
- **A real carving regression caught.** `func_8008FBD4` had been carved as
  `0x38`, taking its span to the next `asm` edge at `0x8040C` to absorb the C
  object's 16-byte gas pad. But retail's `0x803FC` is the distinct asm function
  `func_8008FBFC` (body `lw/nop/addiu/sw`, then `func_8008FC0C` at
  `0x8008FC0C`). The carve swallowed it. Span is now the true `0x28`, asm
  resumes at `0x803FC`. (`func_8008FCBC` is the same 4-word shape but its next
  word is part of its own `jr`-delay-slot tail, so `0x28` there needs no
  change.) Evidence updated.

`⚠` **Honesty note on recovery.** A `git checkout` mistake briefly reverted the
manifest to HEAD and dropped the uncommitted patch-4/patch-5 profile
*definitions*; they were restored explicitly
(`era_o2_g0_symbol_at_temp` = `MASPSX_SYMBOL_AT_TEMP=1`,
`era_o2_g0_symbol_load_dest_temp` = `MASPSX_SYMBOL_LOAD_DEST_TEMP=1`) and all
seven prior-session assignments were re-added and re-verified by the tool.
Nothing was committed (`git commit` was not requested).

New levers pinned this session:

- **Patch 5 is the lever for *every* indexed symbolic `lh`/`lw`-with-`$at`
  table read**, not just the `D_800A0ED4[a0*1048]` shape. `func_80077D30` and
  `func_80077DC4` both go 31–33 object mismatches → 11 → `LINK_EXACT` at link
  level with `MASPSX_SYMBOL_AT_TEMP=1`.
- **A `-G8` leaf can need *both* gp-relative and absolute globals.**
  `func_80052E30`'s six config words are `sw 0x2D8..0x2F4($gp)` but its three
  descriptor addresses abort the link with `small-data section too large`; the
  fix is `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS` for just the descriptors
  (new profile `era_o2_g8_force_descriptor_absolute`).
- **`func_80079FB4` is PARKED**, not churned: `docs/evidence/func-80079FB4/PARK.md`.
  It is the top remaining fan-in (19) but retail materializes the same `slt`
  comparison **twice** before the zero-guard branch, and expands both `div`
  sites with full ASPSX guard ladders inline; no flag rung, operand order, or
  `MASPSX_EXPAND_DIV`/patch-4/patch-5 combination reproduced either.

Route queue note: the 0xC BIOS thunks (`func_80071A54`, `func_80071A74`,
`func_800726C4`, `func_80073C5C`, …) are `addiu $t2,0xA0` / `jr $t2` with an
`addiu $t1,<n>` delay slot — handwritten, not C-matchable. `func_80072714`/
`func_80072724` are raw `syscall 0` BIOS calls (also handwritten). The next
real C targets from `tools/analysis/route_coverage.py` are `func_8006DE80`
(15, parked twin `func_8006DCE4`) and `func_80077CF4`-style small leaves.

## SESSION 2026-09-11 (cont. 10): patch-5 registration + 11 leaves (616 → 627)

Goal unchanged. This session registered the previously-staged **maspsx patch 5**
(`MASPSX_SYMBOL_AT_TEMP=1`; `era_o2_g0_symbol_at_temp`) and matched/registered
**11 new leaves**, all `LINK_EXACT` at the retail VMA. Current count:
**627 matching C leaves** (`python3 tools/build/disc1_plan.py --check` →
`936 spans (627 c, 307 asm, 2 rodata)`, geometry `0x1EE000`, plan SHA-256
`183a1946ea9e8d61e273d9b6f216bb29395fbe39ee49ab4f778c0e794da2767e`,
confirmed by `scripts/verify_us.sh --public` `[4/4]`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`; maspsx suite OK.
On-path fan-in cleared this session: `func_8007FBF0` 11 sites,
`func_80064C54` 5, `func_80073C94`/`func_80073CC4` 4 each, `func_80042770` 4,
`func_80075B84` 4, `func_8007DC84` 2, `func_8008783C`/`func_8006599C`/
`func_80042964`/`func_8007DC5C` 1 each (others are leaves with no direct
`jal` callers in disc1 text). No toolchain-hiding event occurred this session;
`tools/mipsel-host/` was present throughout.
On-path fan-in cleared this session: `func_8007FBF0` 11 sites,
`func_80064C54` 5, `func_80073C94`/`func_80073CC4` 4 each, `func_80042770` 4,
`func_80075B84` 4, `func_8007DC84` 2, `func_8008783C`/`func_8006599C`/
`func_80042964`/`func_8007DC5C` 1 each (others are leaves with no direct
`jal` callers in disc1 text). No toolchain-hiding event occurred this session;
`tools/mipsel-host/` was present throughout.

| leaf | file | size | era / knob | note |
|---|---|---:|---|---|
| `func_8008FED8` | `0x806D8` | `0x24` | `-O2 -G0` | stream rewind: clear bit 0 @+0x38, zero +0xE8, set 0x10 @+0xF4 |
| `func_80090948` | `0x81148` | `0x28` | `-O2 -G0` | cursor advance + byte broadcast into +0x56/58/D0, zero +0xD2 |
| `func_8008783C` | `0x7803C` | `0x28` | `-O2 -G0` | SPU voice pitch field insert at `0x1F801C08+(a0<<4)` |
| `func_8008FBD4` | `0x803D4` | `0x28` | `-O2 -G0` | cursor read + sign-extend into +0xDE |
| `func_8008FCBC` | `0x804BC` | `0x28` | `-O2 -G0` | twin of `8008FBD4` into +0xE0 |
| `func_80036DC8` | `0x275C8` | `0x30` | `-O2 -G0` | 3 sequential init calls in one 0x18 frame |
| `func_80073C94` | `0x64494` | `0x30` | `-O2 -G0` | `D_8009566C->f+0xC()` tail dispatch |
| `func_80073CC4` | `0x644C4` | `0x30` | `-O2 -G0` | `D_8009566C->f+0x8()` tail dispatch |
| `func_8006599C` | `0x5619C` | `0x2C` | `-O2 -G0` | row `lh` getter through double-loaded `D_800B1624` |
| `func_80064C54` | `0x55454` | `0x2C` | `-O2 -G8` | `func_8005F354(func_8005DC4C(), D_8009D164)` |
| `func_80085F44` | `0x76744` | `0x24` | `-O2 -G0` | guarded `D_8009B434` setter |

Also closed out the registration of the four prior-session leaves
(`func_80042770`/`func_80042964` under patch 5; `func_8007FBF0` and
`func_80075B84` under patch 4 / patch 3 respectively) — the profile
assignments were missing and are now recorded.

Levers / facts pinned this session:

- **Patch 5 registered + profile wired.** `era_o2_g0_symbol_at_temp`
  (`MASPSX_SYMBOL_AT_TEMP=1`) now covers `func_80042770`/`func_80042964`
  (`D_800A0ED4[a0*1048] & 1` / `D_800A0EDE[a0*1048]`). Retail uses the `$at`
  temp *with* the `%lo` displacement kept on the load — distinct from both the
  legacy path and patch 4. Patch-4 (`era_o2_g0_symbol_load_dest_temp`) now also
  covers `func_8007FBF0` (`D_8009B574[a0]`, dest-register temp).
- **Stale profile assignments silently fall back to the default.** `func_80075B84`,
  `func_8007FBF0`, `func_8005DB8C` and the 5DB8C-era leaves had no assignment
  entry, so `disc1_plan.py` used `era_o2_g0` and the *object* still matched
  (the patch only changes the pad/epilogue). `func_80075B84` is now under
  `era_o2_g0_fill_epilogue_delay_slot` (patch 3 is load-bearing:
  `word mismatches=2` without it, `0` with it).
- **Pointer-to-volatile defeats a load CSE.** `func_8006599C` needs
  `extern unsigned char *volatile D_800B1624` so cc1 keeps retail's two
  independent `lui`/`lw` pairs; a plain pointer global CSEs the second load
  (`MISMATCHES=10`).
- **Intermediate narrow type controls `lbu`+`sll`/`sra` vs `lb`.** In
  `func_8008FBD4`/`func_8008FCBC` the fetched byte must stay an
  `unsigned char c` cast only at the store, or cc1 folds `lbu`+`sll 24`/`sra 24`
  into a single `lb` (`MISMATCHES=4`).
- **`unsigned int` value avoids a dead `andi 0xFF`.** `func_80090948`'s
  broadcast value must be `unsigned int`; `unsigned char` inserts a redundant
  zero-extension before the `sh` stores.
- **Argument-less fn-ptr slots again.** `func_80073C94`/`func_80073CC4` need
  `unsigned int (*)()` (retail `jalr` delay slot is a bare `nop`).
- **gas 8-byte pad is not a mismatch.** Nine of the ten leaves show
  `SIZE_MISMATCH C=…+8..12` with `BYTE_EXACT (ignoring gas align pad)` at the
  object level; all are `word mismatches=0` at the retail VMA.

Parked this session (no C left in `src/`, exact residual recorded here):

- `func_80089F28` (`0x28`): needs the `%lo` displacement to land on the **last**
  `sh` with the added base in `$v0` (retail `addiu $v0,$v0,%lo`); cc1 keeps the
  displacement on the first store's base register under every form tried
  (`word mismatches=5`).
- `func_80038CE4` (`0x28`): retail computes `addu $a0,$v0,$a0` (base first)
  then `addu $v0,$v0,$v1`; cc1's canonical order is `dest = idx + base`
  (`word mismatches=2` in the two `addu` operands).
- `func_80083578` (`0x28`): retail uses `$v1` for the base pointer and
  `$v0` for the loaded halfword; cc1 wants `$v0`/`$v0`
  (`word mismatches=4`).
- `func_8003E0A4` (`0x2C`, select-code ternary), `func_8003708C` (`0x1C`,
  `mult` high/low split), `func_800661A4`/`800661CC` (`ctc2` COP2),
  `func_8006A2E8` (gp-relative stores), `func_80072714`/`80072724` (handwritten
  syscalls), `func_80087798` (`0x1F801C00` voice init) — all remain
  unregistered; residuals are shape/schedule, not new lever classes.

## SESSION 2026-09-11 (cont. 9): new maspsx patch 4 + 4 leaves (606 → 610)

Goal unchanged. This session added a genuinely new maspsx patch class
(**patch 4, `MASPSX_SYMBOL_LOAD_DEST_TEMP`**) and matched four leaves:
`func_80052F70` (the `0x32`-clamped accumulator in `43724.s`), `func_800762A0`
(the `0xE5` draw-mode packer in `66970.s`), and the `D_800A3348` byte-table
pair `func_80076B44`/`func_80076B20`. Current count: **610 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `910 spans (610 c, 298 asm,
2 rodata)`, geometry `0x1EE000`, plan SHA-256
`32b6fe6d9681fcdcf33c44d2bf5a157073ad30d1e1478b5a99b43b53ab46aa3a`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`; maspsx suite → 173
tests OK.

| leaf | file | size | era / knob | note |
|---|---|---:|---|---|
| `func_80052F70` | `0x43770` | `0x5C` | `-O2 -G0` | clamped byte accumulator; fan-in 61 |
| `func_800762A0` | `0x66AA0` | `0x1C` | `-O2 -G0` | `0xE5` draw-mode packer (operand order) |
| `func_80076B44` | `0x67344` | `0x14` | `-O2 -G0` + **patch 4** | `D_800A3348[a0]`; dest-register temp |
| `func_80076B20` | `0x67320` | `0x24` | `-O2 -G0` + **patch 4** | `*D_80095854 = a0` then `D_800A3348[a0>>24] = a0` |

All four are reloc-normalized object matches; linked at the retail VMA they
are `LINK_EXACT` (0 word mismatches). Evidence:
`docs/evidence/func-80052F70/REPORT.md`, `func-800762A0`, `func-80076B44`,
`func-80076B20`.

Levers / facts pinned this session:

- **New maspsx patch 4 — `symbol_load_dest_temp`**
  (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`). For an indexed symbolic **load**
  `op $d,SYM($b)`, emit the naive GNU-as dest-register address temp:
  `lui $d,%hi(SYM)` / `addu $d,$d,$b` / `op $d,%lo(SYM)($d)`. For an indexed
  symbolic **store** that immediately precedes a bare `j $31`, emit
  `lui $at,%hi(SYM)` / `addu $at,$at,$b` / `j $31` / `op $src,%lo(SYM)($at)`
  (store into the return delay slot). Neither pre-existing knob covers this:
  the legacy path and `MASPSX_THREE_WORD_SYMBOL_STORE` both use `$at` *and
  drop the `%lo` displacement* (`op $d,0x0($at)`). Patch 1's fill covers only
  the **absolute** `sw $r,SYM` macro, not the indexed form. Survey: 69
  dest-temp sites in disc1 text (54 `lw`, 7 `lbu`, 5 `lh`, 2 `lhu`, 1 `lb`).
  Durable test `tools/era/maspsx/tests/test_symbol_load_dest_temp.py`
  (6 tests); registered in `scripts/setup_era.sh` `MASPSX_TRACKED` and the
  `.gitignore` negation list; profile `era_o2_g0_symbol_load_dest_temp`.
  Flag-off is byte-identical (full suite 173 OK).
- **Operand order can beat a flat OR.** `func_800762A0` only matches when the
  return is computed as `y = (a1 & 0x7FF) << 11; x = (a0 & 0x7FF) | 0xE5000000;
  return y | x;` — retail materializes the `a1` half first and the `a0` base
  word last. A flat `(a1...) | (a0...) | 0xE5000000` materializes the
  constant first (`MISMATCHES=6`).
- **`unsigned int` for a `srl` index.** `func_80076B20` needs `a0 >> 24` to
  emit `srl` (`0x00041602`); signed `int` gives `sra` (`0x00041603`).
- **CC1/vendor divergence is not a link failure.** `func_80052F70` emits
  `sw $ra,0x14($sp)` where retail has `sw $v0,0x14($sp)` for the first-call
  spill, and `func_80076B20`'s object carries a trailing aligned pad; both
  resolve to 0 word mismatches at the retail VMA.

## SESSION 2026-09-11 (cont. 8): `654C8` display cluster continued (601 → 606)

Goal unchanged. This session finished five more `654C8`-cluster leaves in
`asm/disc1/655C0.s` using the established `D_80095744` pointer-global /
`D_80095748` gate typing, and parked the cluster's big gate state machine
`func_800755F0`. Current count: **606 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `903 spans (606 c, 295 asm,
2 rodata)`, geometry `0x1EE000`, plan SHA-256
`8f7e8b3f394b98e9027230412b23a9abaf4e30833ec9246b9c87e89ca892216f`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`.

| leaf | file | size | era / knob | note |
|---|---|---:|---|---|
| `func_800751E4` | `0x659E4` | `0xC8` | `-O2 -G0` + patch 3 | link-walk + `D_8009580C`/`D_800957F8` window seed |
| `func_800752AC` | `0x65AAC` | `0xAC` | `-O2 -G0` + patch 3 | `f(0x2C)` handler + same window seed |
| `func_80075358` | `0x65B58` | `0x5C` | `-O2 -G0` + patch 3 | `void`; `f(0x3C)(0)` then `f(0x14)(a0+4, a0[3])` |
| `func_80075424` | `0x65C24` | `0xC0` | `-O2 -G0` + patch 3 | `((int*)a0)[7] \|= 0xFFFFFF`, `f(0x8)`, 0x14 template |
| `func_800754E4` | `0x65CE4` | `0xD8` | `-O2 -G0` + patch 3 | twin: splice `a0 & 0xFFFFFF` into `*(a1+0x1C)` |

All five are reloc-normalized object matches; linked at the retail VMA they
are `LINK_EXACT` (0 word mismatches). Evidence:
`docs/evidence/func-800751E4/REPORT.md`, `func-800752AC`, `func-80075358`,
`func-80075424`, `func-800754E4`. On-path fan-in: `func_80075424` (3 `jal`
sites), `func_800752AC` (3), `func_80075358` (2).

Levers / facts pinned this session:

- **Element-form OR keeps the displacement on the base register.**
  `((int *)a0)[7] |= 0xFFFFFF;` (`func_80075424`) and
  `((int *)a1)[7] = (((int *)a1)[7] & 0xFF000000) | (a0 & 0xFFFFFF);`
  (`func_800754E4`) give retail's `lw v0,0x1C(base)` / `sw v0,0x1C(base)`
  and keep the `lui 0xFF` mask in `$a0`; the local-`s0`-base form splits the
  load/store and schedules the mask into `$v1`.
- **Loop-local named mask constants.** In `func_800751E4` the `lo`/`hi`
  masks must be declared *inside* the walk loop (`unsigned int lo = 0xFFFFFFu,
  hi = 0xFF000000u;`); hoisting them promotes `lo` to a callee-saved `$s2`.
- **`func_80075358` is `void`.** Retail's tail is the plain
  `lw ra / lw s1 / lw s0 / jr ra / addiu sp,sp,0x20`; the object reports
  `0x60` only because gas pads `.text` to 16 bytes (one `nop` past `0x5C`),
  and the link check trims it. `s0` stays live because the second push uses
  `a0 + 4`.
- **Flag base is `0x08000000`** in `func_800755F0`, not `0x80000000` — the
  seed `lui $s0,0x800` in the `bnez` delay slot is `0x08000000>>16`.
  `0x80000000` makes cc1 emit `lui $s0,0x8000` (wrong).
- **`func_80075424`/`func_800754E4` gate the log with 3 args** (the `addu
  a2,s1` after `addu a1,s0`), while `func_800751E4` logs 2.

PARKED this session:

- `func_800755F0` (`0x65DF0`, `0x4F8`, the cluster's big gate/state
  machine) — `docs/evidence/func-800755F0/PARK.md`. Residual: prologue
  scheduling (retail materializes the `D_8009574E` pointer into `$s2` before
  the `lbu`; cc1 issues the `lbu` first) and a shared-tail clamp ladder that
  a linear C transcription does not reconcile. 298/318 words differ; the
  first divergence is in the prologue. Not invented; left as `asm`.

## SESSION 2026-09-11 (cont. 7): boot-spine + display cluster (588 → 601)

Goal unchanged. This session worked the boot-spine tail (`func_80074D28`,
`func_80077AC4`), the `0x80074xxx`/`0x80075xxx` display cluster in `654C8.s`
(with several patch-3 epilogue fills), and the `0x80075xxx` display-list node
inits. Current count: **601 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `899 spans (601 c, 296 asm,
2 rodata)`, geometry `0x1EE000`, plan SHA-256 `9f77a5bb7b77`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`.

| leaf | file | size | era / knob | note |
|---|---|---:|---|---|
| `func_80077AC4` | `0x682C4` | `0x3C` | `-O2 -G0` | fan-in 16; masks pinned `asm("$6")`/`asm("$7")` |
| `func_80074D28` | `0x65528` | `0x98` | `-O2 -G0` + patch 3 | boot-spine trampoline; `s0`/`s1` pinned |
| `func_80074DC0` | `0x655C0` | `0x68` | `-O2 -G0` + patch 3 | same gate/log pair, `+0x3C` slot |
| `func_8007506C` | `0x6586C` | `0x60` | `-O2 -G0` + patch 3 | `D_80095744` pointer-global `v0[8]` push |
| `func_800750CC` | `0x658CC` | `0x60` | `-O2 -G0` + patch 3 | twin of 7506C, `v0[7]` |
| `func_800753B4` | `0x65BB4` | `0x70` | `-O2 -G0` + patch 3 | gated reset + `v0[6]` push |
| `func_80075B1C` | `0x6631C` | `0x30` | `-O2 -G0` | `D_80095744->f()` slot `+0x38`, no-arg prototype |
| `func_80075B4C` | `0x6634C` | `0x38` | `-O2 -G0` + patch 3 | display-list node init |
| `func_80075C04` | `0x66404` | `0x40` | `-O2 -G0` + patch 3 | display-list node init (signed `lh`) |
| `func_80075C6C` | `0x6646C` | `0x28` | `-O2 -G0` | rectangle flag-word setter |
| `func_80075C94` | `0x66494` | `0x54` | `-O2 -G0` + patch 3 | 5th stack arg + `a3 & 0xFFFF` |
| `func_80075AE8` | `0x662E8` | `0x34` | `-O2 -G0` + patch 3 | `D_800957B8` 0x14 template wrapper |
| `func_800755BC` | `0x65DBC` | `0x34` | `-O2 -G0` + patch 3 | `D_8009575C` 0x5C template wrapper |

All thirteen are reloc-normalized object matches; linked at the retail VMA they
are `LINK_EXACT` (0 word mismatches). Evidence:
`docs/evidence/func-80077AC4/REPORT.md`, `func-80074D28`, `func-80074DC0`,
`func-8007506C`, `func-800750CC`, `func-800753B4`, `func-80075B1C`,
`func-80075B4C`, `func-80075C04`, `func-80075C6C`, `func-80075C94`,
`func-80075AE8`, `func-800755BC`.

Levers pinned this session:

- **Data-symbol arguments beat address literals.** Writing
  `(char *)0x80011870` (or `0x8009CD9C + 0x6A`) makes cc1 fold the constant and
  emit a single `lui`; declaring `extern char D_80011870` and passing
  `&D_80011870` restores retail's `lui`/`addiu` HI16/LO16 relocs. Same for the
  `D_800957B8`/`D_8009575C` template addresses.
- **`D_80095744` is a pointer global, not a struct symbol.** The display
  cluster loads it once (`lui`/`lw`) and reaches both the handler
  (`+0x8`) and the argument word from that one register. Model it as
  `extern unsigned int *D_80095744;` plus a single `v0` base local; a struct
  pointer makes cc1 reload the global, and a `char`-address form folds the base
  into `lui/addiu` and picks the wrong slot offset.
- **Fixed-register pins** (`register T x asm("$16")`) reproduce retail's
  callee-saved allocation when C's natural order differs (`func_80074D28`
  s0/s1, `func_80077AC4` mask constants in `$6`/`$7`).
- **Function-pointer slots are declared with no arguments** when retail clears
  none (`unsigned int (*f)()`): a prototype with a parameter makes cc1
  materialize `$a0` in the `jalr` delay slot.
- **`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` (patch 3)** carries seven of these
  leaves; the display cluster has an unusually high density of filled
  epilogue slots. Profile `era_o2_g0_fill_epilogue_delay_slot` now lists
  twelve leaves.

PARKed this session: `func_80075C44` (`docs/evidence/func-80075C44/PARK.md`) —
retail keeps `(a2 != 0)` in `$v0` via `sltu` and ORs it into the branch-arm
constant register, while cc1 folds the boolean into a select and branches on
`a2` (one extra word). Same class as the `func_8006F9F0` rematerialization
residual; not resolvable from C with the current rungs.

## SESSION 2026-09-11 (cont. 6): small leaves (581 → 588)

Goal unchanged. This session worked the small-leaf tail: two CD-cluster byte
searches plus **five field-VM `D_800910A0` opcode handlers**, each with a
link-exact proof at its retail VMA (`era_link_check.py`). Current count:
**588 matching C leaves** (`python3 tools/build/disc1_plan.py --check` →
`878 spans (588 c, 288 asm, 2 rodata)`, geometry `0x1EE000`, plan SHA-256
`8af66b433ce5`); `python3 tools/build/test_disc1_plan.py` → 8 tests OK;
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`.

| leaf | file | size | era / knob | fan-in |
|---|---|---:|---|---|
| `func_8006DB9C` | `0x5E39C` | `0x44` | default `-O2 -G0` | CD cluster |
| `func_8006DBE0` | `0x5E3E0` | `0x38` | default `-O2 -G0` | CD cluster |
| `func_80017294` | `0x7A94` | `0x28` | `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0` | VM handler `0x294` |
| `func_8001731C` | `0x7B1C` | `0x40` | `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0` | VM handler `0x31C` |
| `func_8001735C` | `0x7B5C` | `0x98` | `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0` | VM handler `0x35C` |
| `func_80017410` | `0x7C10` | `0x34` | `-O2 -G8` | VM handler `0x410` |
| `func_800176B8` | `0x7EB8` | `0x28` | `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0` | VM handler `0x6B8` |

All seven are reloc-normalized object matches; linked at the retail VMA they
are `LINK_EXACT` (0 word mismatches). Evidence:
`docs/evidence/func-8006DB9C/REPORT.md`, `func-8006DBE0`, `func-80017294`,
`func-8001731C`, `func-8001735C`, `func-80017410`, `func-800176B8`.

New levers pinned this session:

- **Aggregate-member indexing keeps a displacement on the load** (`func_8006DB9C`/
  `DBE0`): writing the walk as a `signed char *p = &tag[0][0]` pointer folds the
  `0xDC` displacement into the base; indexing the real aggregate
  (`t->tag[i][0]`, with a `Tag *t = &D_800B0CD8` local) keeps
  `lb $2,220($3)`/`lb $2,221($3)` and reproduces retail's `$v1`-base /
  `$a1`-counter allocation. (Inverse of the usual "hoist the base" lever.)
- **`-G8` + absolute base / gp-relative store split** (VM handlers): the
  `D_8009D2F0` state object is loaded absolutely (`lui`/`lw`) while the VM cursor
  `D_8009CE00` is a small-data word at `0x90($gp)`. Under `-G8` cc1 gp-relocates
  *both*; the existing `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0` knob (extern
  stripped) keeps the base absolute while the cursor stays `%gp_rel`. Registered
  as profile `era_o2_g8_force_d8009d2f0_absolute`.
- **`buf` must be an array** (`func_80017410`): a scalar `short` only reserves
  one stack slot (`addiu sp,sp,-0x20`); `short buf[8]` gives retail's `0x28`
  frame with the store at `0x10` and the `sp+0x10` argument window.
- **`tools/analysis/era_link_check.py` now defines `_gp=0x8009CD70`** and honors
  `MASPSX_FORCE_ABSOLUTE_SYMBOLS` — without the former, every `-G8` leaf failed
  to link (`R_MIPS_GPREL16` truncated); with it the gp-relative store resolves to
  its retail target.

PARKED: `func_8006DE80` (`docs/evidence/func-8006DE80/PARK.md`) — 21-word
forwarding shim to `func_8006DED4`; 16-word residual is a cc1 argument-promotion/
schedule artifact (retail reads the outgoing stack arg with `lw` + explicit
`sll/sra`, and orders the sign-extension after the call setup). Twin
`func_8006DCE4` (`D_800B0E64`) has the same shape. `-O1 -G0` /
`-O1 -G0 -fschedule-insns2` do not move it.

Next targets: `func_80074D28` (`0x98`, boot-spine tail, fan-in 9; 22-word residual
not yet reduced), `func_80077AC4` (`0x3C`, fan-in 16; currently untested),
`func_800698D4` (disc-mount wait; `accepted-residual`), and the remaining small
VM handlers in `asm/disc1/8374.s`/`8E*.s`.

## SESSION 2026-09-11 (cont. 5): boot→Day-2 coverage queue (578 → 581)

Goal unchanged: boot → end of Day 2 at 1:1 retail (matching C + `pc_port/`).
This session started working the ranked gap list in
`docs/ai_context/BOOT_TO_DAY2_COVERAGE.md` §4 and matched **three** more leaves
on the default `era_o2_g0` (`-O2 -G0`) profile, each with a link-exact proof at
its retail VMA via the new `tools/analysis/era_link_check.py`. Current count:
**581 matching C leaves** (`python3 tools/build/disc1_plan.py --check` →
`872 spans (581 c, 289 asm, 2 rodata)`, geometry `0x1EE000`);
`python3 tools/build/test_disc1_plan.py` → 8 tests OK; plan SHA-256
`8e5aa8a6a601`.

| leaf | file | size | words | rank / fan-in | command |
|---|---|---:|---:|---|---|
| `func_8001A680` | `0xAE80` | `0x104` | 65 | Tier A∪B fan-in **27** (highest on path) | `era_leaf_match.sh src/func_8001A680.c 0x8001A680 0x104 -O2 -G0` |
| `func_8006DED4` | `0x5E6D4` | `0x7C` | 31 | CD/boot cluster (boot spine tail) | `era_leaf_match.sh src/func_8006DED4.c 0x8006DED4 0x7C -O2 -G0` |
| `func_8006DF50` | `0x5E750` | `0x58` | 22 | gap-list rank 12, fan-in 9 | `era_leaf_match.sh src/func_8006DF50.c 0x8006DF50 0x58 -O2 -G0` |

All three are reloc-normalized object matches whose remaining diffs are only
relocation fields; linked at the retail VMA they are `LINK_EXACT` (0 word
mismatches). Evidence: `docs/evidence/func-8001A680/REPORT.md`,
`docs/evidence/func-8006DED4/REPORT.md`,
`docs/evidence/func-8006DF50/REPORT.md`.

- **`func_8001A680`** is the field/script handler-surface activator: handler =
  `D_800B0E98[body->f0C].entries[(u16)id]` (0xC0-byte class rows, so a
  `HandlerClass` element type keeps cc1's `la`/index addressing), stores to
  body `+0xE/+0x14/+0x18/+0x1B0`, clears flag `0x200`, caches handler byte
  `[2]-1`, then recurses over the `D_8009D20C[0]` chain for entries whose
  `+0x18C` matches and `f98` bit `0x200000` is set.
- **`func_8006DED4`**/**`func_8006DF50`** are the CD-cluster packet builder and
  its record-lookup forwarder (see reports).

**New tool: `tools/analysis/era_link_check.py`.** There was no committed
link-level checker; the tool compiles one leaf, resolves every undefined
address-named symbol with `--defsym`, links with an explicit script
(`.text : SUBALIGN(4)`) so the leaf lands exactly at its retail VMA (a default
link's 16-byte section alignment shifts absolute `j`/`jal` targets by the pad),
slices the `.text` from the leaf symbol, and compares word-for-word against the
extracted EXE. Validated against `func_800702DC`/`func_80070064`/`func_8006ECEC`
(all `LINK_EXACT`).

New load-bearing levers pinned this session:

- **`HandlerClass` aggregate element for `D_800B0E98`** (`void *entries[0xC0/4]`):
  an explicit `HandlerClass *tbl = D_800B0E98;` local makes cc1 emit retail's
  exact arithmetic order for `tbl[classId].entries[idx]`. Writing the nested
  aggregate directly as `D_800B0E98[classId].entries[idx]` hoists the `la` base
  and the index mask (8 mismatched words); a byte-based
  `*(void **)((char *)base + classId*0xC0 + idx*4)` also mismatches. The handler
  byte must be re-read through `body->handler`, not a local copy.
- **`unsigned short` stack parameters change the load width**: `func_8006DED4`'s
  args 4/5 (both on the stack) must be `unsigned short` to emit `lhu 0x48/0x4C(sp)`
  rather than `lw`.

Parked this session (no C left in `src/`):

- **`func_80077AC4`** (`0x3C`, fan-in 16) — 7-word residual: retail assigns the
  `0xFF000000` mask to `$a2` and the `0x00FFFFFF` complement to `$a3`; cc1 2.7.2
  assigns the reverse. Every instruction matches, only the two mask temporaries
  are swapped. Exhausted: operand order, `~`/`<<8`/compound-literal derivations,
  `int`/`unsigned` masks, explicit temporaries, bitfield struct, and every era
  rung (`-O1`, `-G8`, `-fschedule-insns2` all worse).
- **`func_8006DE80`** (`0x54`, fan-in 15) — a 21-word argument-forwarding shim
  into `func_8006DED4`. Retail's first loop-invariant (`move $v1,$a0`) hoists
  ahead of the prologue's `sll $a3,$a3,0x10`; cc1 sinks it into the call block.
  Every era rung and argument-signature variant leaves 13–16 mismatched words.
  (Note: `func_8006DED4` — the **callee** — did match when written directly.)
- **`func_80073A44`** (boot spine, fan-in 16) — a VSync-family routine
  (`func_80073BBC` = VSync shim) with two `func_80073BBC` calls, a `0x400000`
  GPU-status gate, and `D_80094578`→`D_8009457C` / `D_800956AC`→`D_80094580`
  counter snapshots. Retail reloads `D_80094578` in the poll spin
  (`D_8009457C` is a **volatile** counter, `lw` twice) and reads `D_80094574`
  once at entry and again after the second call. Closest shape is
  `r2`-style (69 mismatches); the 0x178 span needs more structural work.
- **`func_8006E1C0`** (`0x110`, `+0x4/+8/+0xC/+0x10` bitfield packet builder,
  `func_8007506C` callee) — 9-word residual around the `rec[7]==0 ? 0x100 :
  rec[7]&0xFF` select and the delay-slot fill before the second `jal`.

Next targets (coverage-map order, still asm): `func_80073A44` and
`func_80077AC4` (both close — one schedule decision each), `func_8006DE80`,
`func_800698D4` (disc-mount wait), `func_80017018` (the task VM, still no `c`
span at all), then the §4.2/§4.3 handlers.

## SESSION 2026-09-11 (cont. 4): CD-stream + command-record teardown (571 → 578)

Goal unchanged: boot → end of Day 2 at 1:1 retail (matching C + `pc_port/`).
This session matched seven more boot-CD leaves on the default `era_o2_g0`
(`-O2 -G0`) profile, each with a link-exact proof at its retail VMA. Current
count: **578 matching C leaves** (`python3 tools/build/disc1_plan.py --check`
→ `867 spans (578 c, 287 asm, 2 rodata)`, geometry `0x1EE000`);
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`;
`python3 tools/build/test_disc1_plan.py` → 8 tests OK; plan SHA-256
`496c44e653238c99eb9c9057483d3b92aa2990774639d9d245c0c6f5d427fea7`.

| leaf | file | size | words | command |
|---|---|---:|---:|---|
| `func_8006ECEC` | `0x5F4EC` | `0x358` | 214 | `era_leaf_match.sh src/func_8006ECEC.c 0x8006ECEC 0x358 -O2 -G0` |
| `func_8006F044` | `0x5F844` | `0x1E0` | 120 | `era_leaf_match.sh src/func_8006F044.c 0x8006F044 0x1E0 -O2 -G0` |
| `func_8006F39C` | `0x5FB9C` | `0x338` | 206 | `era_leaf_match.sh src/func_8006F39C.c 0x8006F39C 0x338 -O2 -G0` |
| `func_8006FC18` | `0x60418` | `0x1FC` | 127 | `era_leaf_match.sh src/func_8006FC18.c 0x8006FC18 0x1FC -O2 -G0` |
| `func_8006FE14` | `0x60614` | `0x250` | 148 | `era_leaf_match.sh src/func_8006FE14.c 0x8006FE14 0x250 -O2 -G0` |
| `func_80070064` | `0x60864` | `0x150` | 84 | `era_leaf_match.sh src/func_80070064.c 0x80070064 0x150 -O2 -G0` |
| `func_800702DC` | `0x60ADC` | `0x118` | 70 | `era_leaf_match.sh src/func_800702DC.c 0x800702DC 0x118 -O2 -G0` |

All seven are reloc-normalized object matches whose remaining diffs are only
`R_MIPS_HI16`/`LO16`/`26` fields; linked at the retail VMA they are
`LINK_EXACT` (0 word mismatches). Evidence:
`docs/evidence/func-8006{ECEC,F044,F39C,FC18,FE14,70064,702DC}/REPORT.md`.

- `func_8006ECEC` is the **CD-stream orchestrator**: four `D_80093168`
  table-driven `func_8006E6D4`+`func_800811E4` read/poll stages (the six
  previously-thought-separate poll labels `8006ED88`/`EE5C`/`EEE8`/`EFB8`/
  `F0F0`/`F1A0` are its basic blocks), `func_800718D0` arena walks (3 then
  `0x106` entries), a `D_800A77FC & 0x2000` B/C split, and the
  `func_80072714`/`726C4`/`72724` tail.
- `func_8006F044` is the reset + two-stage stream loader.
- `func_8006F39C` is the record allocator: overlay prelude for ids
  `0x6C..0x72` (CD-read `D_80093162` → `D_80011618` when `D_800B0CD8` bit
  `0x10000` clear, install the seven `D_801F1B..` descriptors at
  `D_800E10A0[0..6]`), then clamp-id/`D_800942E0` lookup, inlined
  `func_8006F224` free-slot search, record format and handler dispatch.
- `func_8006FC18` invokes a record's offset-`0x14` handler and tears the
  record down (seven-slot clear + `0x10000` bit when byte 1 was `0x72`).
- `func_8006FE14` scans both arenas for records whose `+8` equals an int,
  calls `func_8006FC18`, then clears; `func_80070064` is the same sweep over
  the E4 arena filtered by byte class `{0..7} ∪ {0x55..0x72}` with
  `a1 = D_8009D254`; `func_800702DC` is the unfiltered E4 sweep with
  `a1 = 0`.

New load-bearing levers pinned this session:

- **`D_800942E0` must be `extern void **`** (pointer-to-handler-pointer-table),
  never `void *[]` — an array declaration makes cc1 take the *address*
  (`addiu at,at,%lo(D_800942E0)`) while retail loads the pointer first
  (`lw $v1,%lo($v1)` then `sll`/`addu`/`lw`). 3 mismatched words per lookup.
- **Overlay handler descriptors are data symbols, not integer constants**:
  reference `(unsigned int)D_801F1BD8` with `extern unsigned char
  D_801F1BD8[]` so cc1 emits `lui/addiu %hi/%lo` with HI16/LO16 relocs; the
  split asm defines them, so the link resolves exactly.
- **`D_800B0CD8` bit-set via a pointer local** (`unsigned int *flagsPtr =
  &D_800B0CD8; *flagsPtr |= 0x10000;`) reproduces retail's single
  `a0 = &D_800B0CD8` for both `lw` and `sw`; a bare `|=` gives absolute
  `lui/lw` + `lui $at/sw`. The `flagsPtr` assignment's source position
  (between the 6th and 7th `D_800E10A0` stores) reproduces the schedule.
- **`D_800B0DD8` is `int`, not a pointer** for the `func_8006E6A8`/`6E6D4`
  `base + offset` argument (`addiu $a0,$s1,%lo`), silencing the
  pointer-from-integer warning.
- **`func_8006F39C` uses `int slot = 0;` initialized at declaration** —
  a separate `slot = 0;` assignment birth-orderizes `$s0` differently.
- **`func_8006FC18`'s second arena lookup must use a distinct local (`q`)**,
  not reuse the first pointer — retail keeps the second pointer in `$a1`
  while the first lived in `$a0`; reuse costs 11 register mismatches.
- **The clear-loop base identity**: `D_800E10A0 == D_800E0EF0 + 0x1B0`
  (`0x1B0 = 0x6C*4`). `func_8006FE14`/`func_80070064`/`func_800702DC` emit
  `lui/addiu %hi/lo(D_800E0EF0)` then `addiu $v1,$s4,0x1B0`, so they index
  `D_800E0EF0[k]` for `k = 0x6C..0x72`, while `func_8006F39C`/`func_8006FC18`
  emit the absolute `D_800E10A0` symbol; both `dlabel`s exist in
  `asm/disc1/C5060.s`.
- **The sweep loops must use `i` as both counter and call argument**
  (`func_800702DC`, `func_80070064`); adding a separate `idx = i + 0xB` biv
  perturbs the callee-saved allocation order (see the PARK below).
- The `func_8006ECEC` poll reuses the `func_8006E834` `$v1`-backup/
  `$v0`-restore register pins; the `func_8006F39C` prelude poll
  (`func_8006E7E8`) uses the plain `-1`-then-`0` order without pins.

Parked this session: `func_800701B4` (E8-arena teardown sweep, `0x128`) — a
pure **callee-saved register ordering residual**: 9 of 74 words, all in the
prologue. Retail schedules the `idx` initialiser (`li $s0,11`) *after* the
loop-invariant table base and the two giv initialisers; cc1 2.7.2 schedules
it immediately after the loop-counter init. Loop body, tail, arena lookups,
clear loop, flag update and epilogue are byte-identical. See
`docs/evidence/func-800701B4/PARK.md` for the full residual and the list of
exhausted variants (comma order, `while`/`do`, declaration order, `-O1`,
`-fschedule-insns2`, `-fno-schedule-insns`).

Next targets (CD/boot cluster, file order): `func_80070064`'s siblings are
done; the `0x601F0..0x60BF4` asm block now holds only the PARKed
`func_8006F9F0` and `func_800701B4`. Continue at `func_800703F4` onward
(already C) / the next asm span after `0x60C1C`, plus retry
`func_800701B4`'s preheader ordering and `func_8006F9F0`'s delay-slot fill.

## SESSION 2026-09-11 (cont. 3): command-record family (565 → 571)

Goal unchanged: boot → end of Day 2 at 1:1 retail (matching C + `pc_port/`).
This session matched six command-record / CD-helper leaves on the boot CD
path, all on the default `era_o2_g0` (`-O2 -G0`) profile, each with a
link-exact proof at its retail VMA. Current count: **571 matching C leaves**
(`python3 tools/build/disc1_plan.py --check` → `861 spans (571 c, 288 asm,
2 rodata)`, geometry `0x1EE000`); `scripts/verify_us.sh --public` →
`PUBLIC_VERIFY=PASS`; `python3 tools/build/test_disc1_plan.py` → 8 tests OK;
plan SHA-256
`bf088f44cfc5efde33b59db80b1e86951eacb8d5dc126df09b1ad41e4aa8158f`.

| leaf | file | size | words | command |
|---|---|---:|---:|---|
| `func_8006EC84` | `0x5F484` | `0x68` | 26 | `era_leaf_match.sh src/func_8006EC84.c 0x8006EC84 0x68 -O2 -G0` |
| `func_8006F224` | `0x5FA24` | `0xA0` | 40 | `era_leaf_match.sh src/func_8006F224.c 0x8006F224 0xA0 -O2 -G0` |
| `func_8006F2C4` | `0x5FAC4` | `0xD8` | 54 | `era_leaf_match.sh src/func_8006F2C4.c 0x8006F2C4 0xD8 -O2 -G0` |
| `func_8006F6D4` | `0x5FED4` | `0x14C` | 83 | `era_leaf_match.sh src/func_8006F6D4.c 0x8006F6D4 0x14C -O2 -G0` |
| `func_8006F820` | `0x60020` | `0xCC` | 51 | `era_leaf_match.sh src/func_8006F820.c 0x8006F820 0xCC -O2 -G0` |
| `func_8006F8EC` | `0x600EC` | `0x104` | 65 | `era_leaf_match.sh src/func_8006F8EC.c 0x8006F8EC 0x104 -O2 -G0` |

All six are reloc-normalized object matches whose remaining diffs are only
`R_MIPS_HI16`/`LO16`/`26` fields; linked at the retail VMA they are
`LINK_EXACT` (0 word mismatches). Evidence:
`docs/evidence/func-8006EC84|F224|F2C4|F6D4|F820|F8EC/REPORT.md`.

Shared facts pinned this session:

- `D_800942E4` and `D_800942E8` are **pointer globals** (retail loads each
  base with `lui`+`lw`), not arrays: `extern unsigned char *`. `D_800942E4`
  stride `0xA0C`, ids `0x00..0x0A`; `D_800942E8` stride `0x10C`, ids
  `0x0B..0x15`. `D_800942E0` is a pointer to a table of handler pointers
  (`extern void **`). Handler-byte clamp is `0x55`; `>= 0xC0` is an error.
- Arena-selection source form is load-bearing: write
  `if ((unsigned)idx >= 0xB) <E8 arm> else <E4 arm>` (or the equivalent
  `idx >= 0xB ? E8 : E4` ternary) so cc1 lays out `bnez` → E4 arm as retail
  does; the mirrored `idx < 0xB ? E4 : E8` ternary inverts the block order.
- Branch polarity for handler-pointer guards selects layout: for the
  `handler[+8]`/`handler[+0xC]` calls the `== 0`-guard (body fallthrough)
  form matches; see per-leaf reports for the exact shape.
- `h` (handler byte) must be `int`, not `unsigned char` — a char local makes
  cc1 emit an `andi …0xff` re-mask before the `sltiu`/clamp chain.

Parked this session: `func_8006F9F0` (record-state dispatcher, `0x228`) — a
`reorg`/`sched2` delay-slot-fill residual (cc1 rematerializes `li v0,4` twice
instead of filling the first branch slot with the second `sltiu`'s `addiu`);
all other instructions agree. See
`docs/evidence/func-8006F9F0/PARK.md`.

Next targets (CD/boot cluster, file order): `func_8006F39C` (`0x338`),
`func_8006ECEC` (`0x358`, the CD-stream orchestrator whose poll labels
`8006ED88`/`EE5C`/`EEE8`/`EFB8`/`F0F0`/`F1A0` are the 6 "jal 0x800811E4"
call sites — the `func_8006ED88` etc. in the mandate are its basic blocks,
not separate functions), `func_8006F044` (`0x1E0`), then
`func_8006FC18`, `func_8006FE14`, `func_80070064`, `func_800701B4`,
`func_800702DC`.

## SESSION 2026-09-11 (cont.): CD issuer + CD read poll (563 → 565)

Goal remains boot → end of Day 2 at 1:1 retail (matching C + `pc_port/`).
This session closed the parked `func_8006E6D4` CD issuer and matched
`func_800811E4` CD read poll. Public YAML gate passed.

### Exact rebuild attempt (this session, evidenced)

The documented flow was run end to end up to the toolchain gate:

1. `scripts/split_us.sh` — **now exits 0**. It was previously aborting with
   `comm: input is not in sorted order` under `LANG=en_US.UTF-8`: `sort`
   honours `LC_COLLATE` while `comm` compares byte-wise, and `comm` exits 1,
   which `set -e` turned into a spurious failure. Fixed in
   `scripts/split_us.sh` (`LC_ALL=C comm` / `LC_ALL=C sort`). Split result:
   `853 spans (565 c, 286 asm, 2 rodata)`, all output git-ignored.
2. `scripts/build_us.sh` — validates `OK original EXE SHA-1
   452fb033f2eaa4b18aa20a5bca60b8125af3a37b` and `OK YAML plan: 853 spans,
   565 C leaves`, then stops at `=== Toolchain ===`:
   `ERROR: mipsel-linux-gnu toolchain not found; run inside pe-mipsel-img or
   install binutils-mipsel-linux-gnu and gcc-mipsel-linux-gnu`.

Precise blocker (host): no `mipsel-linux-gnu-gcc` on `PATH` (only binutils
exist, under git-ignored `tools/mipsel-host/`), and no `docker`/`podman`/
`distrobox` (so `pe-mipsel-img` cannot be entered). `uid=1000` with `sudo`
requiring interactive auth, and `apt-cache policy gcc-mipsel-linux-gnu` →
`Candidate: (none)` (the package is not in the configured apt sources at all),
so a local install is not possible non-interactively. 164 of the 565 leaves
are `modern`-toolchain spans that need this GCC; the other 401 are built with
`tools/era/gcc-2.7.2-psx` + maspsx. **Exact packed SHA-1 remains unverified
here** — run `scripts/split_us.sh && scripts/build_us.sh &&
scripts/verify_us.sh` inside `pe-mipsel-img`.

### Path meaning (repo evidence, not guesses)

- **Day 1 first-play prefix** (`docs/acceptance/PE_DAY1_ACCEPTANCE_CONTRACT.md`,
  `docs/ai_context/DAY1_ROUTE_AUDIT.md`): cold boot → opening FMV (identity
  still RESEARCH_REQUIRED) → curb/limo → `m0002i` Carnegie sidewalk →
  `m0003i` lobby → `m0372i` + FMV003 → `m0004i` → `m0378i` → `m0377i` →
  rehearsal/sewer chain `M0319I`/`M0023I`/`M0367I`/`M0026I`…`M0036I` →
  M0000I dispatcher with persist g74=`0x80` selecting **M0351I** (Day 1 exit).
  Day 1 boss / completion marker still RESEARCH_REQUIRED in SYS0; script
  audit pins g74=`0x80` → M0351I as the Day 1→Day 2 selector.
- **Day 2 entry** (`docs/ai_context/DAY2_ROUTE_AUDIT.md`): M0351I → M0042I
  (g74=`0x88`) station / shared rooms (M0041I/M0043I…) → Central Park scripts
  (M0191I/M0037I/M0374I and later park interiors). Day 2 terminal transition
  is still unproven.

Matching C is **565 YAML spans** (`python3 tools/build/disc1_plan.py --check`),
not the stale ~275 CLAUDE.md figure. `pc_port/` movie/CD autonomous stream
(DMA pointer seed + XA mode 0x50 bits) remains the native FMV frontier
(DAY1/DAY2-157). Native ports are not matching leaves.

### Matched this session (reloc-normalized object vs ROM; link-exact proof)

| leaf | file | words | era / profile | command |
|---|---|---|---:|---|---|
| `func_8006E6D4` | `0x5EED4` | 69 | `-O2 -G0` | `era_leaf_match.sh src/func_8006E6D4.c 0x8006E6D4 0x114 -O2 -G0` |
| `func_800811E4` | `0x719E4` | 28 | `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` | `era_leaf_match.sh src/func_800811E4.c 0x800811E4 0x70 -O2 -G0` |

`func_8006E6D4` PARK closed: the extra `lui $v1,0x100` rematerialization
disappears once `func_800719E4` (BIOS `B(38h)` = `exit`) is declared
`__attribute__((noreturn))` — `0x01000000` stays live across the call and the
`beq` delay slot reuses it. `func_800811E4` (CD read poll, 9 retail `jal`
callers on the boot load path) needed two levers: one retained `$a0` base
(`&D_8009B6B4.issue` + zero-code `asm` barrier) serving both adjacent
pending/issue words, and **vendored maspsx patch 3**
(`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`) that schedules `addiu $sp,sp,32` into
the `jr $31` delay slot (ASPSX reorder fill, class of patch 1's store fill).
`tools/era/maspsx/tests/` (167 tests OK) and `setup_era.sh`'s tracked-file
list cover it. Link-level check at `0x800811E4`: `LINK_EXACT` (0x70 bytes, 0
word mismatches after relocation resolution).

Retail EXE SHA-1 after `scripts/extract_us.sh 1`:
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Public:
`scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`, matching-C **565**.
Plan SHA-256 `e7a9ec2c0c4fae5098d7fe73b3b8afffe9231b54fa82973626cb50a89545ecb7`.
Evidence: `docs/evidence/func-8006E6D4/REPORT.md`,
`docs/evidence/func-800811E4/REPORT.md`.

### Next concrete step

1. Exact packed SHA-1: `scripts/split_us.sh` && `scripts/build_us.sh` &&
   `scripts/verify_us.sh` in `pe-mipsel-img` (needs docker/distrobox GCC).
2. Continue Day 1/2 matching leaves; next CD/boot-path candidates. Also
   `pc_port/` native FMV: seed retail CD/DMA pointer table provenance and XA
   ReadS bit6 so movie player `121C04` completes autonomous frames.
3. Do not claim boot→end-of-Day-2 complete: Day 1 completion marker, Day 2
   terminal transition, and most EXE functions on that graph remain asm.

No git commit this session.

## ACTIVE OBJECTIVE: decompile all Day 1 and Day 2; implement/fix Day 1

Current user goal (2026-09-08): "decompile all of day 1 and day 2 implement
and fix day 1 as it goes along and start porting into the native port again
fixing what may have been missing so far". Preserve BOTH days in the decompilation scope.
The Day 1 retail-accuracy and release requirements below remain unfinished.
Day 2 has an initial shared-entry script inventory in
`docs/ai_context/DAY2_ROUTE_AUDIT.md`; the complete script/scene/call-graph
inventory and decompilation acceptance evidence are still missing. The shared Day 1 exit transition is the present
implementation frontier, not a substitute for Day 2 coverage. After restoring
that graph, trace its original M0351I/M0042I branches into Day 2 and build the
full Day 2 inventory from source scripts, including optional/shared branches.

### Earlier Day 1 acceptance requirements

Goal wording: Complete all of Day 1 to verified 100% retail accuracy, including
mandatory and optional content and the transition out of Day 1. Decompile and
port missing original behavior, verify the window resize/maximize fix, validate
the finished Linux and Windows packages, and publish the final launcher update.

User expanded the goal on2026-09-05: "finish up all of day 1 to 100% retail
accuracy", then explicitly requested "update the goal for it". This SUPERSEDES
the narrower end-of-sewers objective. Continue in story order, decompile when
needed and port original behavior. Reaching Day1's end is necessary but not
sufficient: verify every Day1 scene/route, mandatory and optional interactions,
battles/enemies, items/equipment/PE, menus/save behavior, visuals/camera/effects,
audio/music, movies and the transition out of Day1 against original behavior.
Do not label100% until scope-wide evidence proves it. Existing partial systems
and silently skipped command paths must be resolved; happy-path tests alone
are insufficient. Preserve explicit boundaries for unported behavior.

On resume2026-09-06, the goal-control API returned no stored goal. The expanded
user objective above remains the working authority. Earlier automatic goal
text referred only to the endofsewers; that narrower wording is superseded.
Acceptance ledger: `docs/ai_context/DAY1_RETAIL_ACCURACY.md`.
No token budget was requested. The earlier requested launcher update is
published; the finished Day 1 build still requires final validation and
publication. Continue the expanded implementation and verification below.

## ACTIVE USER REQUEST: Banshee update 128 (2026-09-08)

Build PE-DAY2-128-38af4ca9fcc1 replaces the unpublished, broken 125 candidate.
The user explicitly authorized compilation and launcher publication and said
"retry" after interruption. Opening regression is fixed; no further approval
is needed. Linux and Windows Release builds completed; all 8 CTest tests pass
(ctest-day2-128.log). Both Release startup checks reach frame-limit at 120
frames; Windows checked under Wine, not a native Windows OS playthrough.
Stage: local/live/publish-day2-128. Both private and default launcher channels
now publish 128. Both full default-channel downloads and embedded metadata
verified (launcher-verification.json); previous channels backed up in stage.
Linux archive 618682359 bytes, SHA a8a6875829fa4f355d0dd10896b62330152e3b8c83012f07d570db82044be2ce.
Windows archive 618569463 bytes, SHA 4a2807f2b271ecfb80da9b801b8995fa90bc175e614f50182a0e389db8877de5.
Source commit 7821e7c3 with dirty working tree included. No blanket commit.
User additionally requested placement under launcher In Development. Updated
launcher classification, development caption and Windows availability while
retaining Install/Update/Play, runtime statuses and quick launch. Launcher
focused checks now pass 57/57 (launcher-focused-day2-128.log), including
ParasiteEveDistribution, StoreLibraryOwnership, ProjectChannelRefresh,
PreservationWorkbench and AdminInstallVisibility. Initial full launcher
suite: 827 pass/12 fail; corrected 5 affected expectations/behaviors; other
failures are outside this change (Universe repository hygiene, WoW, admin
menu, live service expectations and settings refresh). Do not claim full
launcher suite passes. Launcher 0.9.87 Linux and Windows built/published; both default client
channels and full download hashes verified (launcher-client-verification.json).
Both user requests are fulfilled: runtime 128 published and Parasite Eve
placed under In Development in launcher 0.9.87. Users must install the
launcher update to see the new placement. Full Day 1/Day 2 goal remains open. Launcher working-tree
changes existing before this task are retained in the build, including the
0.9.86 Windows-era changes; no reset or blanket commit.

## DAY1/DAY2-157: movie player 121C04 ported (2026-09-09)

Previous156 was verified updater validation plus the GPU DMA2 parallelism
fix, all jobs terminal. Ported the 271-word movie player 80121C04 in
movie_overlay_port.c: record selection (id>=0x2F returns 0), B0DBF/B0DBB/
227E4, both 21004 display envs, strcat filename (prefix table word at
120FF4/120FFC by id<0x15, then the record's word 0), 81414 DsSearchFile
behind readiness, 22414 location copy, slice geometry from record+0xA/+0xC
and the CDDC bank byte, BE3C(0) tables, C0D8(1214D4), A214(2434,0x40),
C304(1, record+6 LBA, -1, 0), Setloc/81314(0x1E0) behind readiness,
870F0(B0DBE), BD4C(2430) and the first-frame decode loop (121270 -> bank
flip -> C89C -> C394 -> 23F5=0/B0DBA++/B0DBC=1, return 1 via the bne delay
slot). The three retry loops keep the recorded boundaries with host-safety
cycle bounds (movie_player_search_wait, movie_player_start_wait,
movie_retry_wait). Oracle pe_movie_player_oracle.py PASS 20 original
graphs; bring-up fixed a real port bug (the strcat prefix must LOAD the
table word). Native DAY2_movie_player: early returns, the disabled-device
search boundary with the setup prefix checked, and the enabled-device path
where the real fixture search resolves, the start Setloc completes through
the stage155 queue, the streaming callbacks install (B8AB4=813E8, DMA3
slot=7C214), and the mode-0x1E0 ReadS reaches the recorded
CD_device_read_mode device boundary (0x50-bit read modes are the recorded
XA-mode frontier). Player 14E30 real-path integration and the autonomous
stream delivery remain the frontier.

Normal/ASan builds terminal0 warning-free. Focused DAY2 runs 49 PASS/1297
skip/1346 total in both builds (test-day2-157.log, test-asan-day2-157.log).
Full CTest 8/8 in140.74s, native1346/1346 PASS 0skip
(ctest-day2-157.log); full ASan binary 1346/1346. Oracles 192+20 PASS.
All stage157 jobs terminal.

Next: XA read-mode device coverage (mode 0xE0's 0x50 bits), then the
autonomous stream (records delivered to 121270 through the physical
chain), 14E30 real path, old 80191DC8 callback, libpress wait fidelity
(C03C/C078/C308/C39C/C430), XA filter/audio, hardware pixel validation,
and scope-wide opening→Day2 acceptance. Full user goal open; published
runtime 128 unchanged.

Autonomous-stream evidence (probed 2026-09-09, change NOT landed): with the
device's 0x50 read-mode guard narrowed to 0x10 (mode bit6 = CdlModeRT per
the Psy-Q SDK; 7C564's own software header/channel filtering proves the
drive delivers all raw sectors), the enabled-device player path starts the
mode-0x1E0 ReadS and reaches the real autonomous chain: CD device sector
delivery → 7F88C/CdDataCallback → 7C564. It then faults on the SDK DMA
register pointer table that retail seeds but no current fixture/startup
seeds: 9B34C (DMA1 CHCR pointer, StreamOutputChcr), then 9B32C/9B338/9B33C/
9B340 (CD bus control/DMA3). The proven physical seed set is in
test_cd_dma.h (9B32C=0x1F801800, 9B334=+2, 9B338=+3, 9B33C=0x1F801018,
9B340=mailbox, 9B344/48=0x1F8010F0/F4, 9B34C=0x1F801098, 9B35C=DMA3+8) —
next stage must first establish WHO writes those words in retail (SDK CD
driver init vs the movie path), then land the bit6 device support together
with that provenance and drive the chain end-to-end (device → DMA3 → 7C214
→ 7C564 → record status 2 → 121270 success → C89C real frame). The bit6
device change was reverted this session; the player test's recorded
CD_device_read_mode boundary stands and all suites are terminal.

## DAY1/DAY2-156: updater validated natively, GPU DMA2 parallelism (2026-09-09)

Previous155 was verified command-queue/poll progress, all jobs terminal. The
movie updater 122040, abort 22354, restore 121A00 and 6E60C were already
ported in movie_overlay_port.c with the game-loop branch (3F51C: 6EC08 low
byte → 122040; return 0 → 121A00 + 6E60C → loop tail; nonzero → 70E54 frame
tail) wired in func_8003F3C4_port.c. This stage validated them natively:
new test_movie_updater.h (DAY2_movie_updater) executes the real
SDK/MDEC/CD implementations — early return, full continue path (C89C bound
exit telemetry, MDEC upload/output graphs, mode-3 command edit 0x62000100,
B0DBC wrap, bank flip, 0x800000 countdown expiry arm with slice restore,
223F8 reconfigure through 21004), the device-disabled retry boundary
(movie_retry_wait after the recorded 2000-poll exhaustion with the 7C2A0
position write 0x11010200), the abort helper's device-disabled Pause stop
(CD_command_wait) with teardown effects, and enabled-device teardown where
the Pause completes through the real stage155 queue and the updater returns 0.
Oracle pe_movie_update_oracle.py --check PASS 192 original updater graphs.
Fixed a stale seed (limit must be at the record pool's +8, not 227EC) and a
24-bit-masked MADR expectation during bring-up.

The real-frame complete test (DAY2_movie_complete_frame, actual updater +
1214D4 slice callback over FMV001.STR frames 2-3) had been failing in VRAM:
GPU DMA2 movie-slice uploads completed only at explicit main-loop/DrawSync
checkpoints, so during the updater's wait the queued LoadImages drained
late and read slice buffers the MDEC had refilled (one frame column landed
13 slices late; proven by column-matching diagnostics: slice 5's VRAM held
column 18 byte-exact). Hardware advances GPU DMA and MDEC in parallel.
HostFB_VSync now runs PE_Port_ServiceDmaIrqCheckpoint before PE_MDEC_Service
only when a decode is in flight AND a GPU DMA2 transfer is pending; with no
pending transfer the tick ordering is unchanged (DAY2_mdec_dma granularity
preserved). After this, all 115200 pixels per real frame match in both
movie formats. Pixel rounding remains model output, not a hardware golden.
Details in DAY2_MOVIE_UPDATER.md (stage156 sections).

Normal build and ASan build terminal0 warning-free. Focused DAY2 runs 48
PASS/1297 skip/1345 total in both builds (test-day2-156.log,
test-asan-day2-156.log). Full CTest 8/8 in131s, native1345/1345 PASS 0skip
(log ctest-day2-156.log); full ASan binary separately 1345/1345. All stage
156 jobs terminal. Oracle/check and Python compilation PASS.

Next: retry-branch enabled-device completion needs the autonomous physical
stream (7C564/DMA3/mode 1E0 ReadS delivering records so 121270 eventually
succeeds), then player 121C04/14E30, old 80191DC8 callback, libpress wait
fidelity (C03C/C078/C308/C39C/C430), XA mode/filter/audio, hardware pixel
validation, and scope-wide opening→Day2 acceptance. Full user goal open;
published runtime 128 unchanged.

## DAY1/DAY2-155: command queue/poll ports (2026-09-08)

Previous154verifiedprogress/alljobs terminal. Ported7EE84(145words),7F418
(124words),7FC64forwarder,80998eightbytecopyand80DC4wrapper incd_stream_port.c;
newSDKdeclarations. Enabled-device80D5C delegatesidenticaloriginalqueue/poll
structureto80DC4; disabledlegacySetlocaccommodationretaineduntilruntimewiring.
Queueprefix via9B4BC[cmd&255]&&param insertsSetloc2, thenseparatecmdrecord.
Capacitychecksindependent;count7prefixfillsringthenactualcommandreturns0with
prefixretained. Sequence9B53C skips0. Preserveparamsnullstalebytes,sourceptr,
extra/callbackwordsonactualrecordonly;headreadydispatch7E8F4realnative.
Pollnonzerosequence→7FC64→7B010(1,0),whichclock/IRQservicesthedevice; forward
ringsearchandoriginalsignedsequencecomparison,thenreverselatestmatching
record;sequence0previousrecord. Missinghistory6,pending0. SnapshotA3500loads
first3wordsbeforestores,fourthafter;copy8bytesA3505→response,returnliveA3504
(aliasaffectsstatus). Blockinglowbytestatus2→1,othernonzero→0; noenableddevice
orpolls>100000hex→explicitCD_command_waitstop,nofalsecompletion. Approxclock,
notretailwatchdogtiming. DetailsDAY2_CD_COMMAND_QUEUE.md.

Originalpe_cd_command_queue_oracle.py --check PASS96enqueuegraphs(dispatch
7E8F4provider) and324pollgraphs(7FC64provider);originalallocator/copy/getters
execute. NativeRAM/resultcomparisonactualhelpers+queueinitgate0. Livefixture
publicSDKinit→80D5CSetlocnonnullresponse→80DC4Pausepassesrealdevice/IRQ/SDK
completion,8bytebound/retiredqueue/readychecked. Initialnormal35568/ASan53066
buildsterminal0warningfree; focused55013/59031terminal0each47PASS/1297skip/
1344total. AddedinvalidBCDSetlocfailureintegrationafterthispass: finalbuilds
86358/35224 terminal0,warningfree;normal/ASannewbinaryspecificgrouppass1/1343skip
(logstest[-asan]-day2-155-error.log). Actualerror3,10hexreturned,nativeStopclear,
queueretired. FullCTest36398 terminal0:8/8 in143.02s,1344/1344native0skip;
logctest-day2-155.log.Alljobs terminal.
Oraclecheck54716terminal0;Pythoncompile/whitespacePASS.Nootherlivejobs.

Nextactualmovieupdater122040/abort22354+game-loopbranch(3F3C4stillboundary).
Original3F520callsupdater;lowbytereturnnonzero→3F590frametail;zero→121A00
restore→6E60C(unported)→3F678,skippingusualafter_drawtail. Savedoriginalbranch
inlocal/live/movie-game-loop-155.asm.Do notfallthroughnormaldrawoncompletion.
Then
player121C04/14E30,old80191DC8,libpresswaitfidelity,XA filter/audio,hardware
pixelvalidation andscope-wideopening→Day2acceptance. Actualqueuepathrequires
enabledCDdevice; defaultdisabledaccommodationisnotfullcommandfidelity.
Fullusergoalopen; publishedruntime128unchanged.

## DAY1/DAY2-154: updater dependency ports (2026-09-08)

Previous153wasverifiedprogress,alljobs terminal. Traced197-word122040..22354
updater and21-word22354abort; detailed decomp inDAY2_MOVIE_UPDATER.md. Actual
updaternotyetnative; game-loop3F3C4boundarypreserved. ItsubmitscurrentRLEto
BFA0+currentbufferC01C,acquires/decodesnextframeintooppositeRLE,waits228FCwith
800000hex timeout(realforcecompletion/bankflip),thencontinuesorunregisters/
Pause. Acquisition2000attempttimeout→7C2A0nextlocation→ready/queueemptywait→
80D5CSetlocwithnonnullresponse→81314mode1E0→retry. Existing80D5Caccommodation
rejectsnonnullresponse,so cannotfaithfullywireupdateryet. 80DC4Pause hasexact
samequeueissue/pollstructureas80D5C:7EE84+7F418. Needfullbothwrappers,notshortcut.

Portedincd_stream_port.c/sdk.h:7A930signedLBA→BCD(wrapping+150,signeddivision,
frame/second/minutestoreorder,fourthbyteunchanged);7AA34inversearithmetic(noBCD
validation);7C2A0guardA8020nonzero→-1/nochange,elseA3490+onesector→out,readlive
A3494afterstores(aliasesimportant). 7A4BCexchange9AFB8;7A8EC→DMA3registration;
7A2A4EnterCritical→9AFD8==1high824F0/824C8else low7A8EC/7A4BCclear→write0through
9AF1Cthen9AF28→ExitCritical. PhysicalCDownerandRAMpointercases;unresolvedstops
propagate. No fakecallbacks/deviceresponses. BCDnegativeedgesnotreplacedbyold
unsignedIntToPosaccommodation.

pe_movie_stream_helpers_oracle.py --check PASS20signedconversion+8inverse+
120completealias/guardretry+8teardownoriginalgraphs. Teardowncritical+DMA3
registrationproviders explicit;originalothercalls/stores/orderexecute. Native
fixtureactualSDKcritical/registrationandphysicalbankclear,allPASS. Normalbuild
86066/ASan49657 terminal0,warningfree; focused86506/72498terminal0each46PASS/
1297skip/1343total. FullCTest89003 terminal0:8/8 in153.01s,1343/1343native0skip;
loglocal/live/ctest-day2-154.log. Pythoncompile/whitespacePASS.Alljobs terminal.

Nextcommanddependencycarves:local/live/movie-command-8007ee84-154.asm145words
through7F0C8,7F418-154.asm124words through7F608(plusnextprefix),80D5Cwrapper.
7EE84callsalreadyported7E6B0/80950/7FBF0/7E8F4;7F418calls7FC64(0),80998copy.
Trace/portthese next,thenactualupdater+game-loopbranch,fullplayer121C04/14E30,
old80191DC8,libpresswaitfidelity,XAmode/filter/audio,hardwarepixelvalidation
andopening→Day2acceptance. Fullusergoalopen; runtime128stillpublished.

## DAY1/DAY2-153: complete opening STR frames (2026-09-08)

Added pe_movie_complete_frame_oracle.py and separate native/header fixtures;
existing pe_movie_frame_oracle.py remains display/acquisition verification.
Authenticated EXE/libpress; original BD4C69632-byte table + C89C instructions
on FMV001.STR frames1..3. Ordered2016-byte video chunks from rawoffset56;
subheader/type/metadata checked, interleaved audio excluded. First/last LBAs
189742/751,189752/761,189762/771;9videochunks each,320x240. Compressed2712/
3628/4356bytes, command+RLE7300/11140/12804bytes. Original reads exactlydeclared
compressedsize andnormalpadexit writesexactlydeclaredRLE,16bytecanaryintact.
COP0CBAC/CBBC providers explicit,read0/write20000hex; nooriginalcodepatch.
Native comments corrected bit17 vsincorrectIEc; CPUcache/statusunmodeled.

Native realdisc fixture compares fulltable/paddedinput/RLEhashes against
original graphs, checksoutputcanary, thenBE3C/BFA0/C01C+MDECService across300
macroblocks/frame inboth15/24bit formats. NoGPU/callbackinthisfixture; nopixel
hardwaregolden. Earlier syntheticDMA→callback→VRAMintegration remainsseparate.
Newfixtures containhashesonly,nooriginalframepayload. Details in
DAY2_MOVIE_COMPLETE_FRAMES.md. FirstnativefailurewaslegacyB54K FNVoffsetseed;
correctedtoexistingstandardB54K_MdecFnv1a64,expectedhashesunchanged.
A filenamecollision initiallyoverwroteoldframeoracle; restoredexactprior
creationcommandfromsessionrecord, --check passes140display+122acquisition.
Newcomplete-frameoracle --check PASS3graphs+tablebuilder; PythoncompilePASS.
Normal/ASan finalbuildsterminal0,warningfree; focusedtests terminal0each:
45PASS/1297skip/1342total,includingcompleteframefixture. FullCTest session52962 terminal0:8/8 in133.65s,1342/1342native0skip.
Loglocal/live/ctest-day2-153.log. Allstage153jobs terminal.
Onlysourcecommentcorrectionafterbuilds; nofunctionalproductionchange.

Next: connectliveframeassembly/playerbuffers;121C04/122040/14E30 andold
80191DC8 callback, command/statuswaitfidelity, XA mode/filter/audio, hardware
pixelvalidation andscope-wideopening→Day2acceptance. Realframes can nowbe
usedfor fullheight movie-slice→VRAMintegration; currentfixture requestsone
macroblockatime. Fullusergoalopen; runtime128remainspublished.

## DAY1/DAY2-152: libpress DMA and movie slices (2026-09-08)

Previous151turn verifiedpixel-modelprogress;alljobs terminal. BFA0 translated
commandbits27/25 edits andC1EC submission; C01C/C27C outputsetup viaMDECowner.
Inputcommand1nowpending,latchedcmd/sourcememoryconsumedatService; wordcountmust
fitroundedDMAtransferelseinput_shortstop. Submitdoesnotdecode/outputcallback.
ServicecompletesDMA0 (tablesalso),gatedbycontrolbit30/DPCR8, thenDMA1 gatedby
bit29/DPCR80,updatesMADR/BCR/CHCR,flags+CPU3bridge. Outputrequestsrounded32word
blocks;highBCR0means65536blocks,not0bytecompletion. OutputwaitservicespriorDMA
orstopifgatecannotprogress. HostVSyncalwaysservicesMDEC, invokesCPUscannerwhen
HasDecode, skipsrecursiveSDKIRQ. Resetclearsdecode/pendingtransfers;tabledata
retained. MdecPixelTablesfixture nowenablesrequestcontrolbeforeuploadwait.

DispatchDmaCallback recognizes1214D4; DMA0/1MADR readsroutedMDECowner. Movie
callbackcontinues afterC01C whenHasDecode, queuesnextoutputthenuploadsoldbuffer;
finalflag/bankbranchcorrectlyelse-only. No-inputcallbackretainsoldboundary and
argumentcontract. Corepixelroundingstillapprox/unverifiedhardware; nofalse
pixel-exact claim. Timingcheckpointapprox,notmeasuredMDECbus/fifo model.

Originalpe_mdec_io_oracle.py verifies24inputwrapper +16outputissuer graphs;
input/waitproviders explicit,registerwrites compared. Check/Python PASS.
Nativeintegration2macroblocks,realretailtables,SDKinput/output,DPCRandCPU3mask,
deferredcallback,nextslicequeue,final15/24bitVRAMtiles+untouchedneighbors.
No fakepixels/returns/callbackproviders. InitialtestfailedVRAMfirstpixel0 vs
4A52 despitecorrectRAM4A52/39CE. Cause:firstResetCallbackafterGPUSeed resets
DPCR33333333;fixtureinitializescallbacks beforeGPUsetupnow. Expectedpixels
unchanged. Firstbuild25413/21123 terminal0; diagnosticbuild50329 terminal0;
final91905/49761 terminal0 warning-free. FinalDMAonly1PASS,1340skip/1341total.
Finalfocusednormal63592/ASan3887 terminal0:44PASS,1297skip/1341total.
FirstfullCTest37215 terminal8:7/8pass, native1340pass/1fail. Onlyfailure was
B54KY_real_disc oldexpectation finaltableDMApermanentlyactive. Updatedassert
completed2/active0/MADRDB14/BCR20/CHCR201 afterhostpolls. Verifiedbuild9407 and
ASan52480 terminal0 warning-free. Real-disc-onlynormal+ASanPASS1group each,
1340skipped/1341total. FinalCTest63775 terminal0:8/8PASS120.88s,
native1341/1341PASS0skipped. Log local/live/ctest-day2-152-final.log. Allstage152
jobs terminal. Scopedwhitespace PASS.
See DAY2_MDEC_DMA_OUTPUT.md. No agents orpublication.

Next remaininglibpresswait/status (C03C/C078/C308/C39C/C430),
source local/live/libpress-waits-152.asm: C308 testsMDECstatusbit29 (commandbusy,
NOTjustDMA0active), C39C testsDMA1CHCRbit24; countdown100000 thenC448timeout
recovery. CurrentSubmitInputTable waitsDMA0active only; command-overlapwait
mustbecompletedbeforeclaimingfullC1EC equivalence. C430 readsstatuspointerDB50. thenmovie121C04/
122040/14E30 andbitstream-to-RLE pipeline. Opening/legacyoverlay registers
80191DC8 asDMA1callback (B54KYcase); thatidentity/body also remainsunported,
not coveredby1214D4dispatch. RealmovieCDmodeXA/filter/audio stillneedscoverage. CurrenttesthasrealDMA1callbackbut
syntheticRLEtwo-graymacroblocks,notphysicalmovieframeendtoend. Stillneedhardware
roundingreference,realframes,fullplayer+autonomousphysicalstreamcallback.
Deviceactivationstillopt-in. FullopeningthroughDay2 and100%Day1acceptance
unfinished. Runtime128 staypublished.

Additionalnext-actionevidence: func_8010C89C_port.c alreadycontainsresumableVLC
pureRAMdecoder, unitvectors+pe_mv1d_c89c_oracle.py; doNOTrewriteit. Production
801924F8 tailintentionallystopsatC89C becauseoldstreamreturnedpartialframe.
Nowphysicalsectorassemblyexists, validatefullrealSTRframe beforewiringthat
oldtail. B54KYrealdiscfileFMV001.STR startsLBA189742 length42584064. Existing
8010BD4C VLCtablebuilderalsoported. Newplayer271words/updater218words disasm
local/live/movie-player-152.asm andmovie-updater-152.asm; playerjalC89C at121F5C
with a0=s1stream,a1=selectedRLEarena,a2=[122430]. UpdaterjalC89C1221B0.

## DAY1/DAY2-151: streaming MDEC pixels (2026-09-08)

Previous150turn verifiedDMAprogress;alljobs terminal. pe_mdec nowretainsquant/
scale payloads whilekeepingexistingreset/uploadtelemetry. NewBeginDecode copies
completecommandRLE words(into131070halfwordownedbuffer); ReadPixels lazily
produces/drainsone768bytemacroblockbuffer. DecodedMacroblocks diagnosticcount.
RLEsign10/skip/zigzag,qscale0bypass,dequantclamp11bit,programmablescale2passIDCT,
signed9bitwrap,mono4/8,color24/15,signedoutput/bit15. Initcleartables;BeginReset
abortdecodebutretaintables. APIsnotyetcalledbylibpress compressedinput/output.
NoDMA1/completionirq implementedthisstage. No thirdpartydecoderimported.

IMPORTANT numericalfidelitynotproved: PSX-SPX lowlevelIDCT andcolorrounding
explicitlyuncertain. CurrentCusesdefinedfloor shifts,IDCT+4095>>13 afterscale>>3,
color359/-88/-183/454 with+128>>8. Exacthardware/retail-frame pixelsstillneeded;
doNOTclaimbitexactdecoder orfullmovieoutput. Localresearchignoredunder
local/live/mdec-research-151;FPGArepoalsoadmitsunfinishedprecisionvalidation.
See DAY2_MDEC_PIXELS.md forlimits andprimaryreference.

Native272cases:252analyticaldiagonalmatriximpulses(63positions,bypass/sign),
16depth/signed/bit15formatswithdistinguishablelumaquadrants/chroma,3realreset
retailtableDClevels,1unterminatedblockstopbeforepixelwrites. Splitread7+57.
pe_mdec_pixel_tables.py authenticates256bytesoriginaltablesviaexistingreset
hashes fromlibpress LBA1940/38, load8010BCF8. Fixtureextractionnotpixeloracle.
Check/Python PASS. Pixelonly1PASS1339skipped/1340total beforeandafterrealDCcases.
Build91578/ASan52690 terminal0;final96205/69748 terminal0 warning-free.
Finalfocusednormal59193 andASan28151 terminal0:43PASS,1297skipped/1340total.
FullCTest32634 terminal0:8/8PASS173.56s, native1340/1340PASS0skipped.
Log local/live/ctest-day2-151.log. Allstage151jobs terminal.

Next: originalBFA0 editscommandbits27(modebit0inverse)and25(modebit1), then
C1EC submit(command,lhuwords) ->currentlySubmitInputTable onlyrecords. Wire
compresseddecodeaftertable commandswithoutspoofingDMA0completion. C01Cwrapper
calls35-wordC27C..C308 outputworker: C39Cwait,DPCR|88,CHCR0,MADRdest,
BCR=((words>>5)<<16)|20,CHCR01000200. Rawdisassembly
local/live/libpress-output-151.asm SHA256
9b251c4da588b28ef1b1f26d4f73a06a56a451814e30e9391876a7ba265dea03. OriginalI/Odisassembly saved
local/live/libpress-io-151.asm; authenticatedtablepayloadsinfixture. Connect
DMA1output/completion/moviecallback1214D4, thenplayer121C04/updater122040/loader
14E30 and autonomousphysicalstreamcallback. Deviceactivationstillopt-in.
FullopeningthroughDay2 and100%Day1acceptanceunfinished. Runtime128 staypublished.

## DAY1/DAY2-150: DMA3 and physical stream assembly (2026-09-08)

Previous149turn verified sectorFIFO progress,alljobs terminal. Added channel3
7CEAC translated106-word idle/request-ready path. Non3/busy/noDRQ explicit
boundaries. DICRbyteenable preservesunrelatedW1Cflags,DPCR/MADR/BCR/CHCRorder.
pe_cdreg ServiceDMA3 consumesFIFOintoRAM,prevalidatesrange/count, masksMADR24,
supports11000000 and11400100,clearsstart/busy;choppedupdatesMADR/end andBCRlow0.
ManualunchangedMADR/BCR. ZeroBC65536,oversizeboundary. Synchronouscompletion
approx,nochoppingbusinterleave/arbitration. DPCRdisabled/noDRQ remainpending.
CompletionusesPE_GPU_LatchDMACompletionFlag3+existingIRQbridge. DispatchDmaCallback
recognizes7C214 andReadRepresentedDmaMadr routesCDowner. No secondDICRstate.

7C564 enabledphysicalbranchreads4location+8discardedFIFO,7CEAC8wordheader,
then504wordpayload; existingmemorypath stays. Disableddevice retainsoldstops.
pe_cd_dma_issue_oracle.py authenticatesEXE,executes16completeoriginalidle/
requestready graphs withMMIOproviders; nativecomparesregistercontractincl
sixtharglowbyte. Check/Python/scopedwhitespacePASS. Notoriginaltimingoracle.
Integrationtestmountsdisc,SDKstartup/Setloc/ReadSrealIRQ,validsectorheader,
manual7C564call,exactpayload/location bytes,bothDMAcontrols,IRQ3then7C214record2,
BE9E4/BE9981,B89F4clear. FIFOremaining280 then71wordDMArejectedbeforedata/RAMwrite.

Initialbuild61411/31636 terminal0. Firstprematuretestusedoldbinary(0pass/allskip,
notvalidation); newbinaryfirsttestfailedDMAstartupbecauseResetTestState detaches
disc. Reattachedfixtureinsideeachcase. Finalbuild64876/13323 terminal0 warning-free.
FinalDMAonlyPASS1,1338skipped/1339total. Finalfocusednormal75259 andASan78293
terminal0:42PASS,1297skipped/1339total. CTest50513 terminal0:8/8PASS126.97s,
native1339/1339PASS0skipped. Log local/live/ctest-day2-150.log. Allstage150jobs
terminal. Finalcomment-onlyupdates explainconnectedDMA scope. See DAY2_CD_DMA_TRANSFER.md. No agents/runtimepublication.

Next: MDECpixels/output andrealmovie121C04/updater122040/loader14E30 integration;
physicalstreamtransfernowconnected, butintegrationtestmanuallyenters7C564after
realSDKarrival andservicesDMAIRQafterreturn. Exerciseinstalledmoviecallback
chain/autonomousplayer next. DMA3completioncallbackB0CC8nonzero stillstops;
movieprefixsetszero. Deviceactivationstillopt-in. FullopeningthroughDay2 and
100%Day1acceptance remainunfinished. Runtime128 staypublished.

## DAY1/DAY2-149: mounted-disc sector FIFO (2026-09-08)

Previous turn verified publicstartup/hostVBlank progress; all148jobs terminal.
pe_disc exports bounds-checked ReadRawSector viaexistingrawowner. pe_cdreg
now supports2Setloc,6ReadN,9Pause,15hSeekL,16hSeekP,1BhReadS inexplicitdevice.
PackedBCD validation (invalidINT5/error10); pre-gap/out-of-image remainboundary.
ReadackINT3 thenINT1/status22 fromactualrawsector; BFRD exposesdataFIFO andDRQ,
byte reads consume distinctbytes. Modebit5:2340@raw12 vs2048@raw24; bit7 cadence
225792 vs451584 approximatecycles. Pauseackoldreadstatus thencompletion2;seeks
complete2. Onependingsector+independentrequestedFIFO; unreadpendingoverrun,
underflow,unsupportedreadmodes50h bits andmedia/readerrors explicitstops. No
hardwaremultisectorqueue/retry/mechanicaltiming claim; noDMA3implementation.

Tests in test_cd_sector_device.h coverrawbounds,consecutivesectors/everybyte,
bothmodes/speeds,delays,BFRD/DRQ,seek/pause,BCDerror,overflowboundary. Integration
starts7EC14 thenSDK7FB44Setloc/ReadS;hostVSync(-1) drivesCPU source2ack/data
callback, publishesA3520/A3524/A3525=1/1/22 thenconsumesactual2340bytes. PASS
sectoronly1group,1337skipped/1338total. Originaltableextractor adds26 command
jumps11D0C (now192tablewords+26jumps); check/Python PASS. No neworiginalcode
oracle. test_cd_device's unportedboundarynowPlay3, sinceReadS isimplemented.

Build99650/ASan7965 terminal0. Finalbuild32990/6264 terminal0 warning-free.
CDfocused77679 terminal0:13PASS,1325skipped/1338total (beforeSDKintegration).
Final DAY2normal54154 andASan42020 terminal0:41PASS,1297skipped/1338total.
FullCTest53487 terminal0:8/8 PASS135.89s, native1338/1338PASS0skipped.
Log local/live/ctest-day2-149.log. Allstage149jobs terminal.
Scopedtrackedwhitespace PASS. See DAY2_CD_SECTOR_DEVICE.md forlimitations.

Next immediateconnection: stream7C564 physicalprefix7C70C..7C754 reads4bytes
frompointerB334 intolocation then8bytesdiscarded; then7CEAC(channel3,record,
0,8,11000000,0,0) DMAheader, waitCHCRB35C, storeslocationatrecord+28. Physical
bodylater7CEAC(channel3,dest,0,1F8,control,last,0). Authenticateddisassembly
local/live/cd-physical-transfer-149.asm includesprefixand7CEAC..7D100.
7CEAC ends7D054 (106words); readsCHCRbusy,updatesDICRbyte+2 channelenable
fromsixtharg,setsDPCRchannelenable,writesMADR/BCR,waitsDRQ,startsCHCR. Seventh
arg unused. ExistingPE_GPU ownsDPCR/DICR andchannelcompletion; useitsAPI,not
asecondDICRshadow. ImplementDMA3owner/translated7CEAC andconnectheader/body,thenMDECpixels/output
andmovie121C04/updater122040/loader14E30. Deviceactivationstillopt-in; full
openingthroughDay2and100%Day1acceptanceunfinished. Runtime128 staypublished.

## DAY1/DAY2-148: public startup and host VBlank IRQs (2026-09-08)

Connected7EC14 to7F994 when PE_CdReg_DeviceEnabled; default device-off cold
path remains collapsed. Added11-word7F960 optionalB8AB8 notification callback
and known dispatcher entry; unknown target stops with low status/preserved
response. New pe_cd_startup_notify_oracle.py authenticates original EXE and
checks20 graphs (10 complete null,10 unknown-callback prefixes); generated
retail_cd_startup_notify_cases.h compared in test_cd_device.h. Oracle/check
and Python compilation PASS. No default runtime activation or publication.

Enabled-device HostFB_VSync now advances1024 cycles forqueries,564480 per
waiting frame, generating GPU VBlank edges/source0 and using existing CPU
IRQ service outside active SDK IRQ. Timer is approximate60Hz, not fullCPU/
scanline/PAL timing. Test usespublic7EC14, originaltables, mountedmemorydisc,
firstResetCallback;commands1/A/C then1/1/13/1 reachlane1/state11 fromordinary
HostFB_VSync waits. No manualcallbackdispatch or SDKcompletionfieldproviders.
Masked source0 test proves clock doesnotbypassIRQ gating. Existing transport
cases retained. See DAY2_CD_PUBLIC_STARTUP.md.

Build94305 terminal0 anddeviceonlyPASS. Finalbuild72876 andASanbuild51145
terminal0 warning-free. Normal19268 andASan6858 terminal0:40 DAY2 groups PASS,
1297 skipped,1337 total. Scoped whitespace PASS. FullCTest72908 terminal0:
8/8 PASS167.77s; native1337/1337 PASS0skipped. Log local/live/ctest-day2-148.log.
All stage148 jobs terminal.

Next: physical seek/read device commands and sectorFIFO/DMA3, then MDEC
pixels/output andmovie121C04/updater122040/loader14E30 integration. Existing
7FB44->7FCFC->7B558 queueissue chain is alreadytranslated; PE_Disc has2048
usersector reads but no public raw2352 read, needed for mode-dependent FIFO.
Defaultactivation remains gated untilsupported downstreambehavior verified.
Full opening-through-Day2 and100%Day1 acceptance remainunfinished.128 stays
published. All priorstage147 jobs terminal; no agents active.

## DAY1/DAY2-147: device-generated startup responses (2026-09-08)

Previous turn made verified low-level initializer progress, full CTest8/8.
pe_cdreg now has explicitly enabled mounted-disc command device for1/A/B/C/E/F/13.
It captures parameters, schedules response FIFO tags/bytes, honors CD mask,
and asserts CPU source2. Enabled-device HostFB_VSync service advances1024
approximate cycles and invokes existing IRQ service outside active SDK IRQ.
No SDK completion RAM is synthesized. Init3->2 responses serialize behind
acknowledgment; parameter-count errors useINT5/20; unported commands/overflow/
media changes/unsupported overlap stop. Device defaults off and public cold
path is still unactivated. See DAY2_CD_DEVICE_RESPONSES.md for timing limits.

Authenticated6 tables/192words via pe_cd_device_tables.py, check/Python PASS.
Native device-only PASS: real7F994 startup commands1/A/C, responses3/3/2/3,
then4 installed VBlank ticks issue1/1/13/1 and reach lane1/state11. Mounted
memory-disc fixture, first-time ResetCallback, original tables, no completion
or return providers. Transport tests verify mask/ack/queue/parameters/errors.
Initial builds failed missing pe_bootstrap.h; included it. Builds87597/70829
terminal0 warning-free. Normal69416/ASan76697 terminal0:40 DAY2 groups PASS,
1297 skipped,1337 total. Scoped whitespace PASS. Full CTest47939 terminal0:
8/8 PASS122.93s, native1337/1337 PASS0skipped. All stage147 jobs terminal.
Logs local/live/*day2-147*.log.

Next: public cold initializer wiring/activation and7F960 (13 words including
padding; actual through7F98C is11words): optionalB8AB8 callback, low status byte,
response preserved. Source local/live/cd-startup-completion-147.asm. Other
seek/read commands, sector FIFO/DMA, MDEC output and movie integration remain
unfinished, as does full opening-through-Day2 acceptance.128 stays published.

Clock follow-up: HostFB_VSync waiting modes call PE_GPU_VBlankStep (counter
only). PE_GPU_SetVBlank(level,generation) is the separate real edge API that
updates Timer1 and asserts source0; PE_GPU_SetHBlank updates Timer1. Do not
assume installing slot0 causes automatic updater ticks from the old counter
step. Public activation needs that timing/IRQ integration checked as well.

## DAY1/DAY2-146: connected low-level initializer (2026-09-08)

Previous turn made verified startup-helper progress, full CTest8/8.
Translated38-word7F994 in cd_stream_port.c: calls original-order helpers,
installs command/data callbacks, binds7FE24 native body and installs slot0
via73D58, then publishesAFD8/B554. Ordinary7BBFC failure is ignored exactly
as original; unresolved native stops unwind. Corrected7EC14 guard to return0
instead ofB554, including old native regression expectation. Public cold path
is still collapsed/unwired. See DAY2_CD_LOWLEVEL_INITIALIZER.md.

Original oracle/check/Python PASS32 complete low-level init graphs (16 include
the following original7440C VBlank dispatch) and8 public guard graphs. Only
console/VSync queries use providers. Fixtures force original Nop/Init parameter
rejection; they prove failure-path helper integration, not drive readiness.
Builds13727/23194 terminal0 warning-free. Normal54563/ASan18658 terminal0:
39 DAY2 groups PASS,1297 skipped,1336 total. Scoped whitespace PASS.
Full CTest63234 terminal0:8/8 PASS119.54s, native1336/1336 PASS0skipped.
All stage146 jobs terminal. Logs local/live/*day2-146.log.

Next: device command-response production and public cold-path wiring. Existing
controller issuer writes actual commands but does not produce device events.
Full physical sector/MDEC/movie delivery and opening-through-Day2 acceptance
remain unfinished.128 stays published.

Timing integration evidence: psx_compat.h func73A44 is inline HostFB_VSync;
host_framebuffer.c services SPU DMA on all calls, audio commands onlymode>=0,
and advances GPU VBlank only waiting modes. Negative query currently does not
service CD events. PE_RetailVSync remains a separately tested source kernel.
Do not infer device response delivery or CPU IRQ service from a query alone.

## DAY1/DAY2-145: audio setup and startup helpers (2026-09-08)

Previous turn made verified CD updater progress, full CTest8/8.
Translated7BAC0 audio/SPU setup,812F4 mode setter and73D58 canonical VBlank
slot wrapper. Updater tests now install via73D58. CD register owner retains
pending/applied ATV0..3 gains with bank3 ADPCTL bit5 application; existing
raw shadow behavior retained. Volume register state only, no sample mixing.
Hardware mapping verified against PSX-SPX; see DAY2_CD_STARTUP_HELPERS.md.

Original oracle/check/Python PASS18 audio setup+72 slot wrapper+8 mode setter
graphs. Native audio covers RAM and three SPU physical aliases, compares full
SPU region/register writes, and checks volume apply/reset/response isolation.
Builds19810/11155 terminal0 warning-free; scoped whitespace PASS.
Normal43991/ASan45829 terminal0:38 DAY2 groups PASS,1297 skipped,1335 total.
Full CTest41566 terminal0:8/8 PASS115.53s, native1335/1335 PASS0skipped.
All stage145 jobs terminal.
Logs local/live/*day2-145.log.

Next: translate/wire outer7F994 using the now-present helpers and bind7FE24
before73D58 installation. Original7F994 ignores7BBFC's ordinary failure return,
but native unresolved boundaries must still propagate. Real device command
responses remain missing, so full7EC14 startup replacement needs integration
evidence. Physical sector/MDEC/movie delivery and full opening-through-Day2
acceptance remain unfinished.128 stays published.

Public initializer original saved local/live/cd-init-public-145.asm. Important
existing mismatch to fix with public wiring:7EC14 guard B554!=0 returns0
(EC2C delay slot), while current collapsed native returnsB554. Cold entry
clears queues, calls7F994, installsA36A4/A8/AC/A0 callbacks,822BC abort check,
81E5C(0), returns1. Preserve its ordinary return semantics and stop propagation.

## DAY1/DAY2-144: CD VBlank updater and retries (2026-09-08)

Previous turn made verified initializer/state progress, full CTest8/8.
Translated7FE24,800F4 and known7F7E8 queue callback (249 original words)
in cd_stream_port.c. Countdown/retry, SDK startup/read/play phases, internal
command issue and idle polling retain original control flow.7FCFC now checks
callee stops after7B9EC and7B558 before suffix publication. Unknown no-arg
callbacks remain stops. SetMode scratch801FFE90..93 corresponds to original
1FEFF0..F3; only byte0 written, test initialword12345678. See
DAY2_CD_VBLANK_UPDATER.md.

Initial oracle3852 PASS2016 and native updater-only PASS1group. Added the
normal7F7E8 path rather than leaving it unresolved. Final oracle66487 terminal0
PASS3360 graphs/prefixes (2856 complete,504 unknown callbacks). Original reset,
issue/retry and queue issue execute; onlyVSync(-1) query0 is a provider. Native
enters through actual bound VBlank slot, counterFFFFFFFF->0; compares SDK,
pending/issuer and actual CD register state. Initial builds73438/72087 and
final builds52165/84673 terminal0; check33847 and Python compilation PASS.
Final builds warning-free; scoped whitespace PASS. Normal45618/ASan63567
terminal0:37 DAY2 groups PASS,1297 skipped,1334 total. Full CTest5961
terminal0:8/8 PASS169.82s, native1334/1334 PASS0skipped.
All stage144 jobs terminal.
Logs local/live/*day2-144*.log.

Outer7F994 installation remains unwired. Next7BAC0 audio/CD volume setup,
812F4 mode store and device command-response production; then connect full
initializer and updater installation. Physical sector FIFO/DMA, MDEC output,
movie player/updater/loader and full opening-through-Day2 acceptance remain
unfinished. Published runtime stays128.

Next hardware owner: pe_spu_dma.h PE_SpuRegister_LoadU16/StoreU16 uses
offsets0..1FE; pe_stream.c shows physical SPU base decode vs guest RAM.
7BAC0 uses pointerB290 base1F801C00 with offsets180/182,1B0/1B2,1AA,
reads1B8/1BA, then programs CD banks2/3 volumes.73D58 wrapper is still
absent; original saved local/live/cd-vblank-install-144.asm.

## DAY1/DAY2-143: controller initialization and SDK state (2026-09-08)

Previous turn made verified CD registration progress, full CTest8/8.
Translated7BBFC (120words) and7FA2C (54words) in cd_stream_port.c.7BBFC
performs reset/registration, pending-tag clearing and original Nop/Init/Demute
sequence with final status check and callee-stop propagation. BIOS diagnostic
calls use a fixed host log; no device completion is synthesized.7FA2C preserves
selective clearing/padding, lane2/state14/phase21/counter1 and fixed LBA0 BCD.
Outer collapsed7EC14/7F994 remains unwired pending other dependencies. See
DAY2_CD_CONTROLLER_INITIALIZATION.md.

Original oracle/check/Python PASS36 complete rejected-command startup graphs
and4 complete SDK state graphs. Startup oracle executes initialized-guard
ResetCallback, registration and real7B558 missing-parameter branch; only
console calls are providers. Native maps MMIO through actual device owners.
Tests deliberately alter Nop/Init parameter requirements to reach rejection;
this does not verify successful startup. Builds8970/16126 terminal0 warning-free.
Normal9631/ASan83287 terminal0:36 DAY2 groups PASS,1297 skipped,1333 total.
Scoped whitespace PASS. Full CTest27704 terminal0:8/8 PASS125.01s,
native1333/1333 PASS0skipped. All stage143 jobs terminal.
Logs local/live/*day2-143.log.

Next: real command response production,7BAC0 audio/CD volume setup,
812F4 mode store and VBlank7FE24 callback installation before outer7F994
can be wired. Pending-tag drain/repeated Nop/Init/Demute/final success and
first-time reset in this initializer still need integrated tests. Physical
sector/MDEC/movie delivery and opening-through-Day2 acceptance remain open.
Published runtime stays128.

Next updater source local/live/cd-vblank-update-143.asm:7FE24..800F4
decrements pending/delay counters, advances SDK state, issues7FCFC commands,
callsA36A0 underB554 guard, then polls idle/error lanes. Pending timer expiry
calls800F4..80164:7B9EC reset, incrementB59C, resetB598 deadline30/960
byB5A4[command], reissue7B558(command,B560,0,1). Neither updater is ported.

## DAY1/DAY2-142: real CD source registration (2026-09-08)

Previous turn made verified queue-completion progress, full CTest8/8.
Found earlier handoff assumption wrong:73CC4 exists but740D0 explicitly
rejected source2. Original82-word worker confirms source2 has no BIOS side
calls; enabled its existing table/mask path in pe_libetc.c.52 CD interrupt
tests now call real73CC4 registration and verify its table/mask effects before
source assertion. See DAY2_CD_SOURCE_REGISTRATION.md.

New original oracle270 complete installed-wrapper/worker cases verifies
guard, same/null/replacement handler and16-bit mask behavior. Regeneration/
check and Python compilation PASS. Builds62740/40716 terminal0 warning-free.
Normal97976/ASan74339 terminal0:35 DAY2 groups PASS,1297 skipped,1332 total.
Scoped whitespace PASS. Full CTest79283 terminal0:8/8 PASS118.29s,
native1332/1332 PASS0skipped. All stage142 jobs terminal.
Logs local/live/*day2-142.log.

Next:7BBFC startup still needs diagnostics, ResetCallback, actual source2
registration, tag clearing and Nop/Init/Demute sequence. Source saved at
local/live/cd-init-141.asm; worker local/live/cd-registration-142.asm.
Caller source local/live/cd-init-caller-142.asm:7F994 calls7BBFC,7BAC0,
clearsA36A8/A4/A0,7FA2C,812F4(0), installsAFB4/AFB8,73D58(0,7FE24),
thenAFD8/B554=1.7FA2C..7FB04 initializes SDK phase14/subphase21/lane2,
including80B44(0,B582). Diagnostics are BIOS string CD_init: and
71A74("addr=%08x\\n",8009B298), an address value (not a string dereference).
Full physical disc delivery, MDEC output, movie player/updater/loader and
opening-through-Day2 acceptance remain unfinished.128 stays published.

## DAY1/DAY2-141: queued completion, retry and removal (2026-09-08)

Previous turn made verified command-completion progress, full CTest8/8.
Translated7E964 and7E5C4 (196 original words) in cd_stream_port.c and wired
7E964 into checked CD callback dispatch. Completion/mailbox publication,
sequence-group advancement/removal, finite/unlimited retries, record/global
callback ordering and existing7FBF0/7FB44 restart path now follow original.
Unknown callbacks remain stops. See DAY2_CD_QUEUE_COMPLETION.md.

Original oracle97290 terminal0:5184 graphs/prefixes (3200 complete,1984
unknown callback prefixes),30 removals. Known callbacks execute813E8->7C564
MDEC-busy branch; restart tests execute original7FB44 with pending gate set.
Regeneration/check39612 and Python compilation PASS. Builds73369/26482
terminal0 warning-free. Scoped whitespace PASS. Normal27739/ASan33682
terminal0:34 DAY2 groups PASS,1297 skipped,1331 total. Full CTest61439
terminal0:8/8 PASS120.79s, native1331/1331 PASS0skipped.
All stage141 jobs terminal. Logs local/live/*day2-141.log.

Next: initialization7BBFC original source saved local/live/cd-init-141.asm.
ResetCallback/source2 registration must precede tag clearing and Nop/Init/
Demute commands.73C94/73CC4 already exist in pe_libetc.c. Diagnostic73C5C
is the BIOS B0/3F string-output wrapper (local/live/cd-init-diagnostic-141.asm);
71A74 startup formatting also needs proper handling. Current collapsed init
still lacks this sequence. Physical
disc responses/sector FIFO/DMA, MDEC output and movie player/updater/loader
plus full opening-through-Day2 acceptance remain unfinished.128 stays published.

## DAY1/DAY2-140: CD command-completion state machines (2026-09-08)

Previous turn made verified queue-recovery progress, full CTest8/8.
Translated80164/80220/80404/8068C (389 original words) in cd_stream_port.c,
registered80164 with checked CD dispatch. Public command jump table/location
copy, SetMode transitions, internal startup/retry phases, timer threshold301,
callback gates/order and final kind33 cleanup now follow original code.
Unknown nested callbacks and live jump targets stop. See
DAY2_CD_COMMAND_COMPLETION.md.

Original oracle70829 terminal0:3314 graphs/prefixes,2716 complete/598 unknown
callback prefixes. Data-ready52 now48 complete/4 prefixes. Regeneration/check
97169 and Python compilation PASS. Builds89130/84780 terminal0 warning-free.
Initial focused55127/36713 terminal1 found phase22 advancing to24 instead
of23; corrected from original80528 delay-slot value. Final builds42022/1937
terminal0 warning-free. Focused normal15030/ASan59393 terminal0:33 DAY2
groups PASS,1297 skipped,1330 total. Scoped whitespace PASS. Full CTest4141
terminal0:8/8 PASS87.54s, native1330/1330 PASS0skipped.
All stage140 jobs terminal. Logs local/live/*day2-140*.log.

Next source7E964 (local/live/cd-queue-completion-140.asm) handles queue
completion/retry/removal, callbacks and pending-command restart7FBF0/7FB44.
It remains a boundary.7FBF0/7FB44 already exist in pe_libcd.c;7E5C4 removal
(span7E5C4..7E6B0) still needs translation. It removes consecutive matching
sequence records, preserves bytes9..11, updates head/count/current index.
Source local/live/cd-queue-remove-140.asm. Initialization7BBFC reset/registration remains missing;
physical disc delivery, MDEC output and movie/player/loader integration plus
opening-through-Day2 acceptance remain unfinished.128 remains published.

## DAY1/DAY2-139: CD queued-command recovery (2026-09-08)

Translated7E704 cancellation and7EB88 completion publication in
cd_stream_port.c, wired7F88C error recovery. Queue is cleared before saved
callbacks; completion and callback deduplication retain distinct original
rules. Unknown callbacks remain stops. Corrupt positive count>8 has an
explicit native boundary. See DAY2_CD_QUEUE_RECOVERY.md.

Original oracle360 cancellation cases:328 complete,32 unknown-call prefixes;
18 direct completion writers. Known callbacks execute813E8->7C564 MDEC-busy
path, making callback counts observable. Data-ready oracle52 now44 complete,
8 prefixes. Fixture regeneration/check and Python compilation PASS.
Normal/ASan builds86600/42385 terminal0 warning-free. Focused normal14075
and ASan8110 terminal0:32 DAY2 groups PASS,1297 skipped,1329 total.
Logs local/live/*day2-139.log. Full CTest21265 terminal0:8/8 PASS90.21s,
native1329/1329 PASS0skipped. Scoped tracked/new-file whitespace PASS.
All stage139 jobs terminal.

Next: initialization7BBFC still needs original command-completion behavior
(80164/80220/80404/8068C and7E964) before correct reset/registration wiring.
Physical CD responses/sector FIFO/DMA, MDEC pixels/output/IRQ, movie player,
updater/loader and full opening-through-Day2 acceptance remain unfinished.
Published runtime remains128. Full Day1/Day2 goal stays active.

## DAY1/DAY2-138: CD data-ready dispatch (2026-09-08)

Previous turn made verified progress on acknowledgment; full CTest8/8 passed.
Native data-ready chain7C13C->80778->7F88C->813E8->7C564 is now translated,
including8080C status/response handling and80998 copy semantics. CPU IRQ
service dispatches known7C13C identity. Unknown callbacks and7E704 error
recovery still stop. No automatic CD registration, event producer or physical
sector/MDEC delivery added. See DAY2_CD_DATA_READY_DISPATCH.md.

Oracle PASS52 original graphs/prefixes:40 complete,12 unresolved-call
prefixes. Original executable authenticated; CD register/VSync providers,
real original callback chain/sector copying. Native enters through source2
CPU IRQ service with explicit registration/assertion and actual responseFIFO,
compares complete RAM pool/payload, cleanup and unknown callback arguments.
Original location stack word1FEF68/native801FFE88 explicitly seeded.
Builds52568/2131 terminal0 warning-free. Normal97204 terminal0:31 DAY2 groups
PASS1297skipped. ASan95149 terminal0:31groups PASS/1297skipped. Oracle
regeneration/check/Python and scoped whitespace PASS. Full CTest96342
terminal0:8/8 PASS87.78s, native1328/1328 PASS0skipped. Log
local/live/ctest-day2-138.log. All138 jobs terminal.
Next initialization source7BBFC calls ResetCallback then73CC4(2,7C13C)
at7BC64; current collapsed CD init does not perform this registration.

Full physical CD stream, MDEC pixels/output/IRQ, player/updater/loader and
opening-through-Day2 acceptance remain unfinished.128 stays published.

## DAY1/DAY2-137: CD interrupt acknowledgment (2026-09-08)

Previous turn made verified progress on stream assembly, full CTest8/8.
Replaced7AAB4 stub with343-word original control flow in pe_libcd.c. Real
response/status-byte publication, command-table gates, jump-table dispatch,
short-response padding and edge-counter behavior. Diagnostic/unknown-target
calls remain stops. pe_cdreg adds bounded16-byte response ingress and byte
FIFO/readiness/ack/mask behavior, not disc commands or CPU interrupt delivery.
7B010/7B558 inner loops use proper CD pointer decoding and stop propagation.
See DAY2_CD_INTERRUPT_ACKNOWLEDGMENT.md for evidence/limits.

Oracle80583 PASS505 graphs/prefixes (501 complete,4 boundaries), original
executable verified. Stream29507/check90630 PASS66+48, now58 complete stream
paths/8 prefixes; idle interrupt-poll executes original7AAB4. Native ack-only
PASS1group/1326skipped. First combined test75875 aborted because old stream
fixture omittedB288; fixture now seeds all4 physicalCD pointers, interpreter
redirects all4. No expected RAM change from pointer correction. Initial
builds31596/24030,26201/43328,34756/79426 all terminal0. Final3 builds
90163/42775 terminal0 warning-free (also propagates the initial7B010 stop
through7B558). Normal25406 and ASan4274 terminal0:30 DAY2 groups PASS,
1297skipped; logs test-*-day2-137-final.log. Oracle/check66722, Python and
scoped whitespace PASS. Full CTest96652 terminal0:8/8 PASS91.32s, native
1327/1327 PASS0skipped. Log local/live/ctest-day2-137.log. All137 jobs terminal.

Only response ingress models FIFO semantics; legacy shadow persists until
activated. No real disc event producer, CPU CD IRQ assertion, physical
sector FIFO/DMA, MDEC pixels/output, player/updater/loader completion or
opening-through-Day2 acceptance.128 stays published; no release this stage.

## DAY1/DAY2-136: stream record assembly (2026-09-08)

Previous turn made verified progress on124-word movie callback and8/8 CTest.
New cd_stream_port.c translates583-word7C564 record assembly,178-word7B290
status poll,8-word7A488 wrapper and11-word7CE80 word copy. Full RAM-backed
sector assembly now executes original header/payload copies, rejection,
frame limits, pool wrap and7C214 final-record publication. Physical FIFO,
DMA3 issue/wait,7AAB4 interrupt-poll and unknown callbacks remain stops.
pe_cdreg owns added bus-control018 and DMA3 register storage; it does not
execute DMA. Native movie callback calls7C564 and clearsB0CD0 only after
successful return. MDEC output10C01C remains a boundary. See
DAY2_STREAM_RECORD_ASSEMBLY.md for exact source spans and limitations.

Native scratch801FFE80..8B retains response/location locals. Original has
retained stack bytes whenA8020!=0; tests explicitly seed both locations.
No arbitrary stack-residue/asynchronous callback equivalence claim.
Oracle21571 PASS66 stream graphs/prefixes (56 complete,10 boundaries) and48
complete status polls, original executable verified. Callback15531 PASS360
with real stream active guard:180 complete/180 output-boundary prefixes.
Final normal24580 and ASan1116 builds terminal0 warning-free. Normal59936
and ASan40754 focused runs terminal0,29 DAY2 groups PASS/1297skipped. All
new cases pass on first run. CTest66270 terminal0:8/8 PASS90.66s, native
1326/1326 PASS0skipped. Oracle checks6005/99921, Python compilation and
scoped whitespace PASS. All136 jobs terminal. Logs local/live/*-day2-136.log.
Only fixture/generator comments were clarified after the tested build.

No package/publication;128 remains released. Full physical movie delivery,
MDEC pixel decode/output/IRQ, player121C04/updater122040/loader14E30 and
opening-through-Day2 acceptance remain unfinished. Do not mark goal complete.

## DAY1/DAY2-135: partial movie slice callback (2026-09-08)

Ports124-word1214D4 callback control flow in movie_overlay_port.c, declaration
in pe_port_compat.h. IMPORTANT: stream7C564 and output10C01C remain explicit
stops at original call sites. No fake decoded data, no IRQ/game-loop wiring.
Original continuation clearsB0CD0 only after7C564 returns; it remains absent
from the native stopped path. Final-slice buffer/bank/mode/rectangle changes
and real GPU LoadImage are implemented. See DAY2_MOVIE_SLICE_CALLBACK.md.

Oracle pe_movie_callback_oracle.py pins executable/overlay and checks360
complete provider graphs.84 contain no missing native callee;276 native paths
compare the exact prefix before a boundary. Native tests compare RAM, stop
identity/output arguments and every final-slice pixel. GPU is asynchronous:
fixture now waits through real DrawSync after RAM comparison. Initial test
incorrectly inspected pixels before DMA completion; production unchanged.
Initial compile corrected a nonexistent two-argument diagnostic helper to
existing four-argument API. First normal run failed only pixel timing check.
Final normal45969 and ASan63826 builds terminal0, warning-free. Focused
normal and ASan9691 both PASS1group/1324skipped, all360 cases. Logs use
-day2-135-final2. Oracle22279/check96000, Python compilation and scoped
whitespace PASS. Full CTest25201 terminal0:8/8 PASS146.12s, native1325/
1325 PASS0skipped. Log local/live/ctest-day2-135.log. All135 jobs terminal.

7C564 is583words ending7CE80, with7A488,7CE80,7CEAC,7C444,7C214 and
indirect callback calls. Raw inspection local/live/movie-stream-handler-135.asm
includes one following instruction; do not treat584-word span as function.
MDEC remains reset/table bookkeeping only. Consulted hardware docs but no
external decoder code imported; exact pixel rounding needs evidence.
Player121C04/updater122040/loader14E30/full Day1/Day2 acceptance remain open.
128 remains the published runtime; no release for this stage.

## DAY1/DAY2-134: movie display and frame acquisition (2026-09-08)

Previous turn made progress:133 movie init/restore/flag port verified, full
CTest8/8 passed.134 ports121004 display environments and121270 frame polling,
volume arithmetic, completion flag, dimension clear/cache/publication.
See DAY2_MOVIE_FRAME_ACQUISITION.md for original source and provider limits.
Oracle passes140 complete display graphs and122 frame graphs; includes
actual2000-poll timeout, frame regression/end and unchanged/signed dimensions.
Output cells use documented host scratch; changed-status BIOS path unverified.

Initial builds17484/58547 terminal0. Native17340 failed because the isolated
fixture left CD register pointers zero; volume7A88C reached invalid address0.
Fixture now seeds original four physicalCD register pointers. Production code
unchanged. Second fixture correction distinguishes width480 mono-rectangle
clears from width320 quick fills, retaining pixel checks. Builds54216/91064
terminal0, warning-free. Normal98701 and ASan22074 PASS1group/1323skipped,
all140 display+122 frame cases. Final full CTest8/8 PASS169.25s; native1324/
1324 passed,0skipped.58719 terminal0; local/live/ctest-day2-134.log. Final
build/test logs use-day2-134-final2. All134 jobs terminal. Oracle check42617,
Python and whitespace PASS. No publication;128 remains release.
Full player/updater/DMA callback and opening/Day1/Day2 acceptance still open.
3F3C4 at3F50C selects122040 when6EC08 low byte is nonzero; it remains an
explicit boundary. Updater122040 calls121004,10BFA0,10C01C and121270;
next resolve decoder input/output and callback1214D4 before wiring it.

## DAY1/DAY2-133: native movie buffers and display lifecycle (2026-09-08)

Previous turn made progress: native movie initializer/completion helper added,
normal focused comparison passed; sanitizer failed only due sandbox ptrace.
After permission change, same sanitizer focused run PASS1group/1322skipped.
Initializer-only full CTest8/8 PASS109.74s (59795 terminal0).
Now also implemented full121A00 restoration. movie_overlay_port.c includes
1216C4 init,121A00 restore,1223A8 completion flag; no movie-player121C04 or
14E30 wiring yet. Both movie overlay packages pinned/extracted; see
DAY2_MOVIE_INITIALIZATION.md. Oracle passes45 original init/restore pairs and
126 complete flag helpers. GPU/sync calls are explicit original providers;
native tests use real GPU, compare original RAM and every backup/restore pixel.

Restorer-inclusive builds43115/35937 terminal0, no warnings. Focused normal
and ASan/UBSan each PASS1group/1322skipped, including45 init/restore pairs,
all VRAM pixels and126 flag-helper cases. Final full normal CTest8/8 PASS
109.00s;78976 terminal0, local/live/ctest-day2-133-final.log. Other logs
build{,-asan}/test{,-asan}-day2-133-final.log and oracle-day2-133.log.
All133 jobs terminal. This result covers the restorer-inclusive binary.
No publication;128 remains current release.

Next full player121C04 setup, movie updater122040 and decoder/DMA callbacks.
Original player chooses20-byte records at122438, builds filename, issues
CD read1E0, primes a frame through121270/10C89C and sets active++.
Original movies8/9/10 refer to FMV007/008/009; IDs differ from filename suffixes.
Full opening/Day1/Day2 story, movies, audiovisual/native acceptance remain open.

## DAY1/DAY2-132: late park and movie-loader decompilation (2026-09-08)

Previous goal turn was progress:130/131 verified park predicates and endpoints.
132 pins19 scripts (including existing58/351/367 and candidate90/91),51
transfers,4030 original-handler cases PASS. Late park adjacency now extends
77..89.89 persist0 bit2 distinguishes direct140->92 from continued scene;
other endpoint writes11B->367, whose11C endpoint returns58, then120->41.
90 writes130 then134 without a yield;91 writes138->351->140/92 selector.
The connection from park return through90/91 is still unproved; these are
candidate ending paths, not a completed Day2 ending or membership proof.

Source/movie requests71:8,88:9,89:10 expose native14E30 unresolved boundary.
Decompiled full106-word loader call order, two package retry loops, overlay
stack descriptor and signed movie argument.42 original window cases PASS;
external disc/SDK/overlay calls remain explicit stops. No native playback
implementation or runtime edits. See DAY2_LATE_PARK_AND_MOVIES.md and tools
pe_day2_park_late_routes.py / pe_movie_loader_contract.py. Logs:
local/live/park-late-routes-132.log (58668 terminal0),
local/live/movie-loader-contract-132.log (42 PASS, terminal0).
Initial research failures corrected two88 false-branch stop addresses and
an omitted367 fixture pin; final gates use actual original destinations.
Python compilation and whitespace checks pass.128 remains last full native
regression/release. No132 native build or runtime job was started.

Next restore movie overlay1216C4/121C04/1223A8 and real playback; trace the
post120 station/native continuation into the actual day boundary. Preserve
opening/Day1 completion, all Day2 scripts, optional routes and live acceptance.
Do not count --skip-movie or closed-predicate counts toward100% completion.

## DAY1/DAY2-130/131: park passage and side branches (2026-09-08)

Previous goal turn was progress:130 passage audit exists and its fresh rerun
passes10819 original paths.131 adds pinned69/70/73/75/76 inventory,13 more
static transfers and original-handler checks of story writes/music gates.
See DAY2_PARK_PASSAGE_ROUTES.md for source addresses and explicit stops.
Original execution corrected an inclusive-F0 assumption in69 music selection:
its middle interval is strictly F0<story<F8; exactlyF0 selects none of the
three music calls. Local writes69:F0,72:F4,70:F8 are verified independently;
connected movement/dialogue/battle progression remains unproved.
131 verification passed6176 original paths (session75915 terminal0); log
local/live/park-branch-routes-131.log. Both Python compilation checks pass.
No native runtime changes or build jobs started;128 remains latest full
native regression/release evidence.
Next inspect71 and77/78/79; preserve optional park and both-day scope.
Do not treat the historical INV20 tail's build sessions as current: those
/tmp logs are absent and no current build was found during this resume.

## LAUNCHER: INSTALL/UPDATE progress vanished mid-transfer (2026-09-08, fix uncommitted)

User report: pressing INSTALL/UPDATE made the launcher "go empty" until the install
finished. Reproduced in `~/dev/banshee-realm-client` (Debug build, isolated XDG dirs,
3 MB/s CONNECT proxy so the 618 MB transfer lasts minutes). Two causes, both fixed in the
launcher working tree (NOT committed, NOT released; needs a 0.9.88 client build):

1. `SettingsService.MutateAsync` raises `SettingsChanged` after `ConfigureAwait(false)`
   awaits, i.e. on a thread-pool thread, and `ShellViewModel` re-initialized every page
   inline on that thread. Off the UI thread the rebuild raced the page: the installing
   row was replaced, the card showed DOWNLOAD PAUSED / RESUME / CLEAR DOWNLOAD, the bar
   said NO ACTIVE DOWNLOADS or 1 PAUSED, Home said no installs active, and a duplicate
   card appeared, all while the transfer kept running. `ShellViewModel` now marshals the
   re-init to the UI thread (captured SynchronizationContext, Dispatcher fallback) and
   exposes `SettingsRefresh` so callers observe it instead of racing it.
2. `EnsureManagedLibraryAsync` re-adds the configured library on every install, which
   always saved settings and fired `SettingsChanged`, so every INSTALL/UPDATE rebuilt all
   pages mid-transfer (page jumped to top). `MutateAsync` now skips save+notify when the
   resulting document is unchanged (JSON equality); regression test
   `Re_adding_the_configured_library_neither_saves_again_nor_notifies`.

Verified on the fixed build with window captures: fresh install (library created, so a
real settings change fires) keeps INSTALLING/DOWNLOADING with a live bar on the card,
bottom bar, Home Active Work and Downloads; UPDATE on an existing library fires no
reload at all. Targeted launcher tests: 117 pass; the 2 failures in
`SettingsRefreshDuplicationTests` fail identically on the unmodified baseline.
Full launcher suite with the fix: 839 pass / 6 fail; all 6 (the 2 settings-refresh tests,
`Initialised_shell_holds_no_operational_data`, `Linux_play_uses_umu_and_userdata_prefix_not_source_client`,
`Service_backed_regions_report_not_configured`, `The_right_click_menu_actually_opens_over_a_row`)
fail identically with the four touched files stashed, so none are from this change.

## DAY1/DAY2-129: park interior route expansion (2026-09-08)

Previous goal turn was progress: runtime128 and launcher0.9.87 published and
verified. Resumed full Day1/Day2 work, preserving the full objective.
New pe_day2_park_interior_routes.py pins62/63/64/65/66/67/358; verifies21
static transfers and absence of direct story assignments in their decoded
scripts. Three signed story gates have2316 original-handler execution cases,
all PASS (park-interior-routes-129.log). Python compilation/whitespace pass.
See DAY2_PARK_INTERIOR_ROUTES.md for ordered destinations, music gate
semantics, explicit asynchronous stops and decoder-data limitations.
No runtime code changed;128 remains latest native regression/build evidence.
Next route frontier:67 module0 scene-controlled transfers to68; inspect68
and follow progression/day-boundary callees. Interior battles and optional
62/64 branches still require full semantic and native execution coverage.
Boot investigation also confirmed85290 still omits original8CB54(4)/85A64(1)
at85614/8561C despite128 restoring those callees. Wiring them requires real
SPU/table initialization and updating isolated bring-up fixtures; not changed
this stage. Source:asm/disc1/75974.s; existing test85644 assumes old TSA1010.

## DAY1/DAY2-128: SPU modes and music consumers (2026-09-08)

Restored 8CB54 mode-switch graph, allocation checks, mode register setup and
clear transfers with explicit host DMA/event adaptation. Supports RAM-backed
and physical SPU register aliases. Original oracle covers 40 mode graphs;
native repeats with both register providers. Restored music start/restore
consumers for commands 10/12/19; 72 complete original consumer graphs agree.
Focused normal and ASan tests pass, including opening and music payload.
Full normal regression suite: 8/8 PASS in 201.28 seconds. Source docs:
DAY2_SPU_MODE_SWITCH.md and DAY2_MUSIC_START_CONSUMERS.md. This does not
complete score sequencing, audible synthesis, BIOS emulation or full Day 1/2.
Packaging supports optional PE_PACKAGE_LEVEL (default 19); this release uses
10 to reduce compression time without changing extracted runtime contents.

## DAY1/DAY2-127: SPU DMA event completion restored (2026-09-08)

Stage126 made verified progress. Stage127 pe_libetc retains the existing
SPU event classF0000009/spec20/mode2000/handler0 registration, enabled and
delivered state. APIs PE_Event_SpuDmaEnabled/DeliverSpuDma/ConsumeSpuDma.
Matching enabled handle consumes one delivered completion; issue does not
signal; reset clears registration/completion. pe_spu_dma Begin accepts
callback0 only with enabled registration. Service copies SPU data first,
then reads live9B434: zero dispatches event,85098 invokes old callback.
callback_order records either completion route. Unsupported/unavailable
completion remains an explicit boundary. This is host event adaptation;
not full BIOS/kernel/hardware emulation or a blocking WaitEvent yet.

pe_spu_dma_event_audit.py pins EXE and verifies8 original7D614 IRQ paths
through event/callback provider boundary. Confirms control bits~30 and
DeliverEvent argumentsF0000009/20 when callback0; no original BIOS run.
New test_spu_dma_event.h covers wrong/missing/disabled registration,
three1024-byte data transfers, pending visibility, correct/wrong handles,
single consumption and reset cancellation. B48A callback-lifetime test
now expects registered event after livecallback removal instead of the old
unimplemented-provider abort. Normal+ASan PASS newgroup1 andall8 B48Agroups;
total1320groups,1319/1312skipped respectively. Builds14903/50684 warning-free,
terminal0; originalaudit54170/Python/whitespace PASS. App/Release remain125.
Full CTest1898 TERMINAL:7/8,156.16s; native1319/1320 passed,0skipped.
Only failure is the known SKIP2 opening (Aya not allocated); no new regression
observed. All127 jobs terminal. Mode switch remains unwired and opening broken.
Docs DAY2_SPU_DMA_EVENTS.md; logs oracle/build/test/ctest-day2-127 variants.

NEXT implement physical SPU register provider and mode graph8CB54/8CF70,
85A64/85BB4 and8D610. Use126's8D140 helper. 8D610 prepares7D778 ops2(dest),
1(write-mode),3(source9C4C0,size<=400), then WaitEvent(9B384); afteractual
PE_SpuDma_Service consume registered event, never fabricate completion.
Provider now supports that real event path. Original IRQ additionallyclears
controlbits30; physicalregister handling remains required. Original mode
clear saves/zeros9B434 and restores it after transfers; don't substitute
85098 callback for its event path. Restore opening SKIP2 before rebuilding
and publishing the Banshee update already requested. Saved125 packages
are verified but known broken; no126/127 changes included and no uploads.
Full Day1/Day2 scope remains active.

## DAY1/DAY2-126: mode-register dependency translated (2026-09-08)

Previous turn made verified progress by compiling125 Release binaries and
starting real package jobs. Stage126 translates full8D140..8D610 in
platform/pe_stream.c with declaration inpe_sdk.h. Zero mask selects all32
halfword registers, otherwise bits select attributes+4..42 -> base+1C0..1FE;
base9B3FC reloaded per selected store. NOT yet wired into8CF70 and does not
resolve the opening blocker. New pe_spu_mode_register_oracle.py/numericheader/
test_spu_mode_register.h compares204 original graphs: zero/full/alternating,
32one-bit/32one-cleared-bit masks,3data patterns, explicit RAM register base.
Normal and ASan/UBSan PASS204 (1group,1318skipped,total1319). Full original
--check/Python/whitespace PASS, builds51016/31620 warning-free and terminal.
Oracle67020/check36775 terminal. Logs oracle/build/test-day2-126 variants.
No full CTest rerun: new leaf has no runtime caller yet;125's known SKIP2
failure still applies. Current app/Release packages remain125, not126.
Details DAY2_SPU_MODE_REGISTERS.md.

Next priority is the mode switch blocking opening, not another release of
silently omitted behavior. Native capturedstate125:9B3FC=1F801C00 physical
SPUregisterbase;9B384=100 registered event;9B434=0 callback;9B424=3 shift;
9B418=0;9B464=B6958 allocation table. Original7D614 IRQ sends event class
F0000009/spec20 when callback0; otherwise calls callback. Mode8D610 clears
SPU RAM in400-byte chunks with7D778 operations2/1/3, then WaitEvent through
8D7B0(BIOS B0/A). Existing PE_SpuDma_Begin rejects callback0; Service aborts
on callback0; PE_Event_Open/Enable only retain the audio timer registration,
not SPU DMA event state. Implement real registration/delivery/consumption for
this existing event, then mode clearing and physicalregister handling.
Do not fake completion or bypass the startup regression. Original IRQ and
DMA dispatcher disassembly saved local/live/spu-dma-event-126.txt. 8D140
physicalregister access will need a host SPUregister provider before wiring.
The full Day1/Day2 goal and Banshee publication request remain active.

## DAY1/DAY2-125: valid music payload restored (2026-09-08)

Stage124 made verified progress. Stage125 extends actual M40 selection1 from
message opening through confirmation,60-tick fade, EA314, story D8 and flag
updates, EA200 music14, previous-room999 and map token A8000048. Final task
PC801C9FA4. Four original graphs/264 checkpoints cover initial banks0/1,
confirmation delays0/4, with actual music cached in slot=bank and SPU mode4
already configured. pe_m0040i_transition_oracle.py uses real package metadata,
AKAO data and full original VM/render/fade calls. Music directory14/bank14
is1896 bytes atchunk440A0/guest801D3088; header songID11, mode4.
No uncached CD/SPU load, mode change or audible acceptance claim.

Initial native final-frame mismatch exposed missing valid music command
queueing in platform/pe_stream.c:8CBA8 commands10/12/19 only checked magic.
Now original8CC68..8CD04 reads songID+4/mode+8; if current stream-state+54
matches ID, returns0. Otherwise, for configured mode, queues payload+16 and
ID atentry+C, command12 alsoCD88 atentry+10, returnsID. Mode mismatch now
requests explicit unresolved boundary (previously silently succeeded).
New pe_music_payload_oracle.py verifies36 complete original graphs across
three commands, IDs0/FFFF, same/different current song and queue0/7/15.
Native also asserts an unported mode transition cannot publish playback.
Both new groups pass normal+ASan/UBSan;264 checkpoints+36 command graphs.
New numeric headers/helpers wired into test_native.c; total1318 groups.
Each focused run passes1, skips1317. Fixture --check/Python/whitespace pass.

Initial build missing declaration corrected by including game_port.h;
final normal/app build80793 and ASan95167 succeeded warning-free. Latest
runtime/app125. First full CTest26119 failed9.27s: BTL94 room-only fixture
bypassed streaming init and had null9D2C8; diagnostic87247 terminal.
Verified M367 music IDs34/35/36 all requestmode4; btl94_setup_m0367i now seeds
state pointerB6980/configuredmode4. Original persistence/actor assertions
kept. Fixture builds92412/8176 terminal; BTL94 normal+ASan pass.
Full regression rerun30801 TERMINAL FAILED:7/8 in118.28s; native1317/1318
passed, SKIP2_opening_script_spawns_aya failed (Aya not allocated). This is
an actual newly exposed boot boundary, not a fixture to bypass. The new
explicit SPU mode check stops opening before Aya allocation. Keep this
regression visible and fix it next. Do not call125 validated or release it.
GDB reproduced: command10, song2, requestedmode5, configuredmode0,
payload800F4CF8, task8009D310, scriptnextPC8018F3FC. Saved full native guest
RAM at this boundary in local/live/opening-music-mode-125.bin (ignored).
GDB commands/log gdb-music-mode-125.{txt,log}; both GDB runs terminal.
No125 process remains live. Logs local/live/{build,test,ctest,oracle}-day2-125
variants; payload logs oracle/test-music-payload-125 variants. Details
DAY2_M0040I_TRANSITION.md. Probe53232 and35885 failures, final oracle50667
andcheck64577 all terminal; original-only probe-m40-fade-125.py remains
ignored with its failed loader attempt, not acceptance evidence.

PRIORITY: fix the opening regression by implementing the SPU mode-change graph8CB54 -> 8D7C0/85A64/8CF70
and its event/hardware leaves, then remove the mode boundary after original
comparison. Unconfigured cached original probe reached BIOS B0 service0A
through8D7B0; original runner does not implement that event service. Do not
invent event completion to get green results. Existing disassembly files:
local/live/music-command-producer.txt, music-command-callees.txt,
music-mode-full.txt. 8CB54 compares9B3A0 withrequestedmode; matching mode
returns directly, otherwise85A64(0),8CF70(mode|100),85A64(1).
8CF70 invokes85BB4,8D140,8D610,7DAE0 and event/hardware behavior.
Full Day1/Day2 scenes/battles, uncached music, sequencing/synthesis, BIOS/card,
random battle freeze, resize, packages and release remain unfinished.
Latest published launcher DAY1-41. Keep the full goal active.

## DAY1/DAY2-124: actual M0040I dialogue choice verified (2026-09-08)

Stage123 restored the FB9 cursor packet in native37870. Stage124 verifies
that behavior with actual M0040I message1, VM opening/waiting, navigation,
confirmation, opcode43 result read and both outgoing branches. No new runtime
fix was needed. The numeric fixture and test are pe_m0040i_choice_oracle.py,
retail_m0040i_choice_cases.h and test_m0040i_choice.h (wired into test_native.c).
Eight original graphs cover selections0/1, initial banks0/1 and delays0/4;
all44 frame checkpoints match native and ASan/UBSan. Total1316 groups;
focused runs pass1 with1315 skipped. Original --check, Python compilation,
whitespace and warning checks pass. Both test builds succeeded.
Full CTest passes8/8 in95.77s (native93.47s), ctest-day2-124.log.
All124 jobs terminal: oracle86346/check87835, builds75670/93512, CTest47117.
Logs local/live/{oracle,build,test,ctest}-day2-124 variants.
Details DAY2_M0040I_CHOICE.md; original-only probe in local/live/probe-m40-choice-124.py.
Latest runtime/native app remains123; latest published launcher DAY1-41.

M40 module2 base801C8F0C starts at8FEC (message1), actual text801D0800.
After wait8FF8, opcode43 at9004 reads selection into local4. Selection0
executes the30-tick wait9078, nextPC9084. Selection1 executes EA305 at952C,
starts60-tick fade954C, nextPC9558. Hashes include message/cursor/glyph
packets, actor/task, audio queue and fade globals. Explicit actor/task/draw
fixture and captured original371B0 initialization; no scene construction or
prior story gate, subsequent branch execution, GPU or audible acceptance.

Next extend selection1 through fade completion, EA314 at9560 and subsequent
story/exit commands, or connect the remaining actual dialogue consumers.
Existing pe_fade_wait_oracle.py verifies original68E24 fade ticks and19410
script polling separately and provides a model for an integrated continuation.
Other choice sites: M59 message6@F1A8/43local5@F1C0; M60 message2@9473C/
43local6@94754. Full Day1/Day2 scenes/battles,367 music,239 producer/consumer,
374 entry/dialogue/E8 and remaining routes still needed. Live/BIOS/card,
random battle freeze, resize, packages and final release remain unfinished.

## DAY1/DAY2-112: shared CD->CE fade/dialogue/exit component (2026-09-08)

Previous111 made verified progress. Pinned367 and new
pe_m0367i_transition_oracle.py executes original VM fromA8AC after initial
EA200 throughRGBfade,30tickdelay,realtext94/render/confirmation,EA206
(no registered channel34),finalfade,CEwrite and239destination publication.
PASS4 original/native+ASan component runs,622 frame checkpoints across
startingbanks0/1 andconfirmationdelay0/3. Actor/task/order/input fixtures;
no initial musicload, GPU, audibleoutput, roomload orfullscene acceptance.
No runtime fix needed. New numeric header/test helper; total1304nativegroups;
focused1group passes1303skipped. Builds warning-free. FullCTest PASS8/8 in
81.43s(native79.39s). All112 jobs terminal;111 latest runtime implementation,
112 latest native regression verification.

New pe_day2_shared_transition_routes.py audits367 scene/actor selectors,
239 pretransition gate andfinalassignments. First research expectation wrongly
read ALU0E as<=; originalexecution rejectedCE. Correct predicate is !=CE.
Closed audit PASS2322; component oracle --check, Python andwhitespace PASS.
DetailsDAY2_SHARED_PARK_TRANSITION.md.
Logs local/live/oracle/build/test-day2-112 variants,ctest-day2-112.log;
JSONm0367i-transition-112.json/day2-shared-transition-routes-112.json.

Next restore initial367 music-load acceptance andfull239 producer/consumer,
59/61 battles,full374 entry/dialogue/E8,59 ED2401/2402 and40 EA305/314.
FullDay1/Day2/live/BIOS/card/freeze/resize/packages/release unfinished;
latestlauncherDAY1-41. Goal active.

## DAY1/DAY2-111: restore park dialogue background and scene flag (2026-09-08)

Previous110 made verified progress. Restored nativeED2300 and2101 in16910:
2300 booleanizes fullwordarg1 into9CED4 (original16ABC->375D0), enabling
37870's independent dialogue background;2101 ORs overlayB0CD8mask800. Both
were silently ignored, used repeatedly by239. No renderer code change needed.
pe_dialogue_background_oracle.py PASS80 original flag/empty-window packet
graphs; native normal+ASan PASS. First fullCTest PASS8/8 in83.27s.

Added real239 text coverage: pe_m0239i_dialogue_oracle.py original371B0,
375E0 messages7C/7D, ED2300 and37870 first pages ->state2. Textdirectoryid1
base801E465C from real field chunk. PASS8 cases original/native+ASan across
both packet banks and backgroundtoggle. No page confirmation/GPU/whole-scene
claim. Numeric headers/helpers for both tests; total1303nativegroups. New8
focused group passes with1302skipped; earlier80 group had1301skipped before
addition. Both oracle --check/Python PASS; builds warning-free, native app
rebuilt. Final fullCTest PASS8/8 in84.80s (native82.90s). Whitespace PASS;
all111 jobs terminal.111 latest runtime change/build. DetailsDAY2_PARK_DIALOGUE_FLAGS.md.
Artifacts local/live/m0239i-dialogue-111.json; oracle/build/test-day2-111
variants; finalctest-day2-111-final.log. Original exploratory RAM dump ignored.

Read-ahead found CEproducer:367 script SHA dc0d5b09ccc503f24ed981f17c6a1cb2329fdc4723e6b1b0a5a0ef4fe7a5676e
base801AA2FC,5modules; CDgate801AA864, dialogue94/fades, CEwrite801AA938
then239transfer801AA948. Decodedlocal/live/day2-367-111.txt; staticonly,
not yet a pinned execution audit. Sharedotherstories27/5F/11C/29F also occur.
Next 239 fullscene/dialogue confirmation and367 CD->CE chain,59/61 battles,full374
constructor/dialogue/E8 and remaining routes.59 ED2401/2402 and40 EA305/314
remain to audit/restore. FullDay1/Day2/live/BIOS/card/freeze/resize/packages/
release unfinished; latestlauncherDAY1-41. Goal active.

## DAY1/DAY2-110: six-script route expansion (2026-09-08)

Previous109 made verified progress. New pe_day2_extended_routes.py pins and
inventories59/60/61/230/239/40:37modules,8585commands, transfers/storywrites,
opcode counts, immediateEA/EDkeys and battle command addresses. PASS11604
original closed paths:59 return selector,239 CE gate,40 D0 gate,230 counter/
actor/message gates and final239/40 progression/masks. Python+whitespace PASS;
all110 jobs terminal. No native changes;109 latest native verification/build.
JSON/log local/live/day2-extended-routes-110.{json,log}; decoded originals
local/live/day2-{59,60,61,230,239,40}-110.txt. DetailsDAY2_EXTENDED_ROUTES.md.

59 returns374 for signedstory<120 else58; writesprev59. Otherexit60->61;
61 has63/65/358/60 edges. Static battle opcodes:59 nine,61 five; no fullbattle
proof.230 setscounter34=16; first actor/message requirescounter16 ANDstory<D0,
secondcounter17 ANDstory>=D0 (signed).239 CE gate finalblock writescounter17/
storyD0;40 D0 gate finalblock writesD8; bothmask prior|1E000 plus800000 if
persist0mask1. Intervening scenes/music/waits unexecuted; not day classification.
239/40 map exits writeprevious191/999 separately.230 exit40 writesprevious191.

Next full59/61 battle/scene routes,61->63/65/358,239 CE producer andscene,
full374 constructor/dialogue/E8, optional/shared branches. New keys include
59 ED2401/2402,239 ED2101/2300,40 EA305/314; audit their native behavior before
claiming these routes accepted. FullDay1/Day2/live/BIOS/card/freeze/resize/
packages/release unfinished; latestlauncherDAY1-41. Goal active.

## DAY1/DAY2-109: restore M0374I readiness and music reset (2026-09-08)

Previous108 made verified progress. M374 module1 ED3100@55DC was silently
ignored by native16910. Restored original6914C(1) call and busy result1→
PC-=40/taskdelay1/return0; ready returns1. Shared6914C native default state
incorrectly returned1; original69198/691B4 return0. Fixed that return.
Original pe_script_readiness_oracle.py PASS1016 graphs (254states x4variants),
excluding CD states34/35 and state0 initialization callback, empty directories
at36. Initial normal+ASan focused PASS1016cases. Full scene/BIOS unproven.

Also restored EA217 used at3746F18/7A08: original6D24C clears channelbytes
B0DB2..7 toFF, masks overlay flagsF0 and calls86FF8 audio commandF0. Native
15DAC previously ignored217. New pe_music_reset_oracle.py PASS48 original
reset/FIFO graphs. Native6D24C declared incompat and wiredEA217. New numeric
headers/tests for both oracles; total1301nativegroups. Final builds warning-free;
native app rebuilt. Normal+ASan each PASS1016 readiness and48 reset cases in
separate1group runs (1300skipped each). Both oracle --check, Python and
whitespace PASS. FullCTest PASS8/8 in84.63s (native82.70s). All109 jobs
terminal;109 latest runtime change/build. DetailsDAY2_M0374I_READINESS.md.
Logs local/live/oracle/build/test-day2-109 variants; final regression log
ctest-day2-109.log. No observed live freeze attributed to these fixes.

Next full374 constructor/scene entry/dialogue completion/E8/59 andpark195->230;
optional/shared branches and Day2 ending classification remain. FullDay1/Day2/
live/BIOS/card/freeze/resize/packages/release unfinished. LatestlauncherDAY1-41;
goal active.

## DAY1/DAY2-108: M0374I real model/clip and animation wait release (2026-09-08)

Previous107 made verified progress. New pe_m0374i_animation_oracle.py replaces
its recordless boundary with original type2 model8019690C, clipE801B88C0 and
real room mesh. Original complete3D050 (texture adjustment disabled),1A918,
actual sender/delivery/receiver/child VM,1A4AC ticks and full3D834 poses at
frames0/122 are compared with native. Clip has123frames; script target123
clamps to122. Four supplied speeds8000/10000/18000/20000 release wait5D94 in
244/122/82/61ticks, for both packet modes. Next VM executes placement5D9C;
real mesh raises requestedy0 to530000, selectsclip0 and waits at dialogue62
poll5DC0. No dialogue completion, GPU submission or preceding scene claim.
Actor/task/allocation/light/speed inputs remain fixtures; native publication
comes from original directory, not full room loader/constructor validation.

First8 original/native real-model cases PASS normal+ASan; initial fullCTest
PASS8/8 in80.02s before the following fix. Found missing sharedEA406/407:
original16794 appends animation sound event; native15DAC silently discarded it.
Restored bounded16-slot registration at944A8/B0CE9 with actor type/subtype,
command/frame bytes and sound halves (406duplicates,407separate bank).
Actual374 EA406 at5840/5860 now included in real-model fixture: command1,
frames0/18, sounds3F8/3F9. New original registration/playback oracle PASS80
synthetic-bank graphs with limits/truncation/zero-sound/bothbanks. New
retail_animation_sound_register_cases.h/test_animation_sound_register.h added.
Final builds warning-free (native app rebuilt); normal+ASan focused PASS:
512 existing sound cases+80 new registration cases in2groups (1297skipped),
8 real-model cases in1group (1298skipped). Total1299groups. Both oracle
--check and Python/whitespace PASS. Final fullCTest PASS8/8 in77.87s
(native75.96s). All108 jobs terminal;108 is latest runtime change/build.
New test_m0374i_animation.h/retail_m0374i_animation_cases.h wiredtest_native.c.
Artifacts local/live/m0374i-animation-108.json; oracle/build/test-day2-108 logs
(including -register, -fix, -asan variants); finalctest-day2-108-fix.log. DetailsDAY2_M0374I_ANIMATION.md.

Next verify374 full constructor/scene entry, dialogue completion and subsequent
E8 alternatives/59 transfer; Day2 ending classification still unproven. Park
195->230 and optional/shared branches remain. FullDay1/Day2/live/BIOS/card/
freeze/resize/packages/release unfinished; latestlauncherDAY1-41. Goal active.

## DAY1/DAY2-107: M0374I actual sender/receiver scene start (2026-09-08)

New pe_m0374i_delivery_oracle.py executes original handler install, module0
sender54B8, queue drain65400 and full receiver VM6354. Payload1 forks5D54;
mailbox flags4 have lowbits0, so child executes in the SAME traversal, writes
E6, opens dialogue62 and yields at animation wait5D94. Prior story is not a
condition inside this delivered-message path. The previous sender scene is
not executed. Recordless actor fixtures do not establish real animation/model
behavior or completion. No battle command identified in this script.

PASS64 original graphs across eight incoming stories, four serials including
wrap and optional inactive following task. Original numeric digests include
whole field chunk, actor/task/queue and selected scene/dialogue state. Native
normal and ASan/UBSan each PASS64 cases in1 test group (1296 skipped in focused
runs; total1297 groups). Builds warning-free; oracle --check and Python PASS.
Full CTest PASS8/8 in78.57s (native76.67s); all107 jobs finished.
Whitespace PASS. No runtime changes;102 remains latest runtime implementation.
New test_m0374i_delivery.h/retail_m0374i_delivery_cases.h wired in test_native.c.
Artifacts local/live/m0374i-delivery-107.json, build/test-day2-107[-asan].log,
oracle-day2-107-check.log and ctest-day2-107.log. DetailsDAY2_M0374I_DELIVERY.md.

Next restore/verify actual374 module2 model/mesh actor setup and wait5D94
continuation, E8 alternatives and59 transfer; no full scene/Day2 ending claim.
Park195->230 and optional/shared branches remain. FullDay1/Day2/live/BIOS/card/
freeze/resize/packages/release unfinished; latestlauncherDAY1-41. Goal active.

## DAY1/DAY2-106: station return DA/E0 and next scene E4 (2026-09-08)

Previous105 made verified progress. New pe_day2_station_return.py verifies
original46 D8->DA gate,47 DA return-scene gate,39 E0 scene gate (772cases each),
47 final E0 assignment,8 E0 station mask preparations and8 scene39 E4 final
blocks. PASS2333 closed cases plus16 original92030 selector prefixes.
Python/whitespace PASS; all106 jobs finished. JSONlocal/live/day2-station-return-106.json.
No runtime changes;102 remains latest native build/regression verification.

AtE0 station mapmask prior|3E000 (plus800000 ifpersist0mask1) selects39 atslot8.
39 final block writesE4/persist1=999 and preserves preparedmask; slot8 then
selects374. These are separate gates/final blocks, not scene-middle execution
or proof of the Day2 ending.47's preceding counter wait is still unverified.
Pinned39 and374 originals;374 static storywritesE6/E8/E8 andtargets0/0/59.
Next audit374's E4 progression/scene/battle and terminal classification; park
interiors195->230 and optional/shared branches also unfinished. Details
DAY2_STATION_RETURN.md. FullDay1/Day2/live/BIOS/card/freeze/resize/packages/release
unfinished; latestlauncherDAY1-41. Goal active.

## DAY1/DAY2-105: park secondary progression and return-scene endpoints (2026-09-08)

Previous104 made verified progress. New pe_day2_park_progression.py pins
37/192/195 and verifies original closed predicates/assignments. Added optional
extra_persist fixture input to station.run. PASS17042 paths:191 actor gate7720,
191 progression gate7720,191 return selector772,37 scene gate772,
195 counter floor50,37 final block8. Python/whitespace PASS; all105 jobs terminal.
103 and104 full research audits rerun PASS; normalized outputs exactly match
previous JSON (only new empty extra_persist result metadata ignored).
No runtime changes;102 remains latest native app/regression verification.

191 initial actor/progression gates require storyC0 AND signed persist34<=10.
The latter writesC8 and persist34=10; otherwise preserves both.195 initialization
checks signed persist34<13, then writesC0/persist34=13 regardless of inputstory.
191 return block writes178 only for168, elseD0.37 D0 scene gate and final block
verified separately: final writesD8, persist1=999, mapmask prior|1E000 plus800000
if persist0mask1; intervening scene not executed. Newscript counts37:5/722,
192:8/1148,195:9/929 modules/commands. Static neighbors include230,196,340 and
later271/367; not all classifiedDay2. JSONlocal/live/day2-park-progression-105.json.

Read-ahead finds46 D8->DA write801AB6B4 and47 DA branch. Next verify that return
progression and the ending, and expand195->230 park content with optional/shared
branches. Persist34 producer14/15 chains also remain. Details
DAY2_PARK_PROGRESSION.md. FullDay1/Day2/live/BIOS/card/freeze/resize/packages/
release unfinished; latestlauncherDAY1-41. Goal active.

## DAY1/DAY2-104: station flags through M0000I to M0038I/M0191I (2026-09-08)

Previous103 made verified progress. New pe_day2_world_map_routes.py composes
original46 mask preparation -> original96620 record enable bytes ->92030 exit
prefix, stopping at74D28. PASS108 station/constructor graphs,1080 exit prefixes,
772 M0038I story gates,8 M0038I final-block-to-selector graphs. Python and
whitespace PASS; all104 jobs finished. JSON local/live/day2-world-map-routes-104.json.
No runtime changes;102 remains latest native build and full regression result.

With zero prior mask and persist0 mask1 clear, story90..B7 sampled enables6/7
(M0024I/M0046I). AtB8, slot9 enables and selectsM0038I. Its storyB8 gate and
closed final block writeC0, resetmask2000, prepare1C000, setpersist1=999 and
returnM0000I; selector9 then reachesM0191I. The intervening scene is unverified.
AtD0, station sets2000; slot7 override selectsM0037I andslot9 selectsM0192I.
Prior availability bits may persist; persist0 mask1 also enables5/M0290I.

Pinned new38(5modules/651commands) and191(10/1634) scripts.191 static targets
include195,239 and0; writesC8/168/170/178/D0 are shared and not all classified
Day2. Next audit191 C0/persist34 gate and outgoing195/37/192 progression to
identify Day2 terminal scene, including optional/shared branches. Details in
DAY2_WORLD_MAP_ROUTES.md. FullDay1/Day2/live/BIOS/card/freeze/resize/packages/
release unfinished; latest launcherDAY1-41. Goal stays active.

## DAY1/DAY2-103: station-neighbor routes and world-map flags (2026-09-08)

Previous102 made verified progress. Added pe_day2_station_routes.py and
DAY2_STATION_ROUTES.md. Pins9 original shared scripts: prior41/42/43/351 plus
45/46/47/51/56. Inventory65 modules/6471 decoded commands/33 immediate
transfers/10 immediate story writes; no dynamic31 among these9. These are
candidate shared rooms, not proven complete Day2 membership.

Original closed-handler execution PASS5958 cases:43 door dispatcher258,
43 storyDA fork772,51 signed-story stair selector772,41 trigger gate772,
41 reminder3088,46 world-map mask296. Stops precede every asynchronous/scene
command. Argument binding and input state are fixtures. Python/whitespace
PASS; detailed JSON local/live/day2-station-routes-103.json; all103 jobs finished.
No native C changes/build/test rerun needed;102 native results remain latest.

41 reminder requires98<=signed story<A4 and persist45 mask10 clear.
51 stair arm writes persist1=51 then selects52 below signed188, else48.
46 closed map-mask block always ORsC000 into persist3, adds cumulative masks
at later story thresholds and controls mask2000 (cleared90..CF); persist0
mask1 adds800000.43 door selector maps local4 values1/0/4/5/6/7 to scene arms
whose static destinations are42/46/45/51/47/56. No intervening scene execution
or trigger/mailbox proof claimed. Details in DAY2_STATION_ROUTES.md.

Next follow46's prepared persist3 through original M0000I map selector to
classify Day2 destinations, then bind station triggers/delivery/asynchronous
arms. FullDay1/Day2 inventory/decomp/live/BIOS/card/freeze/resize/packages/release
unfinished. Latest launcherDAY1-41. Goal remains active.

## DAY1/DAY2-102: full native mode6 readiness restored (2026-09-08)

Previous101 made verified progress. Replaced2BC90_mode6_cut shortcut with a
compatibility wrapper around complete2BC90..2D1F0 in func_8002DC58_port.c.
Restored21D4C/374E8, AT/PE pulses, three Aya amount timers, actor command
settling/queued Aya resumption/task release and27D14 before6914C readiness.
An active amount or changed command forces another tick even when consumed.
Only ready actors plus complete media preparation publish7/reset colors/unlock.

384 original full-handler cases match through native dispatcher; normal and
ASan/UBSan PASS1group each (1295skipped,total1296). Initial comparison failed
case33 because shared exit_pulse used hostD250=0 while guestD250=1. GDB proves
that mismatch; corrected helper to read guestD250, affecting modes6 and8.
One historical mode6 test now supplies the required Aya stats pointer.
Builds warning-free; native app rebuilt; header reproduction/Python/whitespace
PASS. Full CTest PASS8/8 in81.43 sec; all102 jobs finished. Logsday1-102 include
initial failures and final -fix normal/sanitizer successes; gdb log retained.

No live scene/load/GPU acceptance claim; synthetic original-call comparisons
include queued animation state, task data, packets, pools and battle globals.
Next trace restored mode6 progression from real scene/VM entry, then complete
the shared exit intoDay2 and expand Day2 room inventory. WholeDay1/Day2 and
live/freeze/BIOS/card/resize/packages/release unfinished. LauncherDAY1-41 latest.
Details DAY2_STATION_DECOMP stage102.

## DAY1/DAY2-101: missing native mode7 HUD handler restored (2026-09-08)

Previous100 made verified progress. Restored complete original2D1F0..2DC58
and wired mode7 into299CC dispatcher. Original full-call-graph oracle240 cases
matches native dispatcher state and packet bytes: both banks, four pulse
phases, AT8999/9000/65535, timer0/1/29/30/31/255, signed amounts and colors,
hit states0/2000/4000/6000, empty lists and missing records. Normal and
ASan/UBSan PASS1group each (1294skipped,total1295). Header reproduction,
Python and whitespace PASS; builds warning-free; native app rebuilt.
Full CTest PASS8/8 in80.02 sec; all101 jobs finished.
See local/live/ctest-day1-101.log.

Mode7 pulses AT colors only at unsigned16 AT>=9000, snapshots projected
coordinates on timer30, submits Aya damage/heal and enemy damage/MISS packets,
decrements timers, calls27A08 for pending flashes and changes remaining2000
to4000. It does not publish a new mode. All four original callees execute in
these comparisons. Join is intentionally inactive in fixtures. No GPU/live
scene acceptance claim. Source confirms mode6 publishes7 at2CF28; native
pe_battle_ready_tail already calls that store, but mode6 itself remains a cut.
Next restore/audit full mode6/2BC90 and its actual script-driven progression;
its omitted calls include21D4C/374E8,32B0C,1A680,36254 and27D14 (all ported);
then follow the shared exit into Day2 and expand its inventory as required.
WholeDay1/Day2 and live/freeze/BIOS/card/resize/packages/release unfinished.
Latest published launcherDAY1-41. Details DAY2_STATION_DECOMP stage101.

## DAY1/DAY2-100: missing native mode5 restored (2026-09-08)

Previous99 made verified progress. Restored original2F0B0 three-phase mode5
cleanup and wired mode5 into299CC dispatcher.41 original complete cases match
through the actual native dispatcher, normal and ASan/UBSan PASS1group each
(1293skipped,total1294). Header reproduction/Python/whitespace PASS; builds
warning-free. Native app rebuilt. Full CTest PASS8/8 in82.25s; all100 jobs finished.

Phase0 excludes Aya, selects eligible record commands, starts30-frame fades and
sets model flags. Phase1 retires completed/flag40 actors, waits for other fades,
then703F4. Phase2 waits for6D60C, clears slots, selects Aya21, runs295E4, publishes
mode11 and calls293F4(0). Original19DE4.s2F0B0..2F300. Existing guards preserve
explicit callee-stop boundaries. No real scene/fade/music/live acceptance claim.

Details DAY2_STATION_DECOMP stage100; logs100 underlocal/live. Next remaining
mode7/2D1F0 dispatcher gap and real entry/progression of restored modes4/5.
WholeDay1/Day2 and live/freeze/BIOS/card/resize/packages/release unfinished.
Latest published launcherDAY1-41.

## DAY1/DAY2-99: mode4 restored; decoder and animation alias fixes (2026-09-08)

Previous98 made verified progress. M0034I mode-gate native/original comparison
covers112 two-pass cases including full script bytes, persistent state, task
cancellation, position checks, queued exits and fade waits. Native/ASan PASS.
Located missing original2B94C: mode4 was absent from native299CC dispatcher.
Restored full four-phase2B94C and wired mode4.41 original cases through native
mode dispatcher cover actor/effect cleanup, fade waits, music-busy gating and
mode10 publication. First native run aborted in1A680 on resource0+2; restored
local physical-RAM byte mapping in1A680/1A784 while retaining resource pointer.

Research decoder pe_pst0_scan.py incorrectly read every argument mode from
header word1. Corrected word2 handling after arg4;20 original VM binding checks
PASS. The earlier apparent immediate-script write was a decoder error; actual
77 writes local6. Existing metadata for >5-argument commands needs regeneration
when consumed; native VM bindings were already correct. Day2 route audit PASS.

Mode4 final41case normal/ASan PASS1group each (1292skipped,total1293). Native
app rebuilt with runtime fixes. All header reproduction/decoder/Python/whitespace
checks PASS; final builds warning-free. Full CTest PASS8/8 in80.28s. All99 jobs
finished. Details DAY2_STATION_DECOMP stage99; logs99 underlocal/live. No live
mode4/battle acceptance claim. Next connect mode4 entry and real fade/music
progression to script continuations; audit remaining omitted mode5/7 dispatcher
paths. Full Day1/Day2, live/freeze/BIOS/card/resize/packages/release unfinished.
Latest published launcherDAY1-41.

## DAY1/DAY2-98: native sender VM/subtask ordering matches (2026-09-08)

Previous97 made verified progress. Added64 original/native full sender46D4 VM
graphs:101 send,46F8 task fork, immediate-versus-later child traversal, actual Aya
readiness producer,112 exit handler, child resumption through4764 delay to4770.
Both readiness bits are script-produced. Flags0/1/2/3, next-link occupancy and
serial wrap covered; no new runtime fix needed. Normal and ASan/UBSan PASS1group
each,1290skipped(total1291). Header reproduction/Python/whitespace PASS; builds
warning-free. Full CTest PASS8/8 in96.11s; all98 processes finished. Logs98 andm0034i-fork.json underlocal/live.

Next4770 mode gate:10->47B4,9->4880,else one-frame loop. Execute those original/
native continuations and mode producers; scene construction/entry still pending.
Tests supply actor/root-task publication, traversal and common clip frame header;
no live scene/animation acceptance. Details DAY2_STATION_DECOMP stage98.
Full Day1/Day2 and live/freeze/BIOS/card/resize/packages/release unfinished.
Latest published launcherDAY1-41.

## DAY1/DAY2-97: native real-mesh readiness graph matches (2026-09-08)

Previous96 made verified progress. Added64 original/native graphs connecting
module5's112 wait, actual101 send/delivery, Aya's real M0034I walkmesh placement
and command4 selection, script-produced bit16, and controller exit requests.
Original1A918 publishes/rebases mesh801BA270; polygon801BA31A snapsY1197to1200.
No fixture bit16 write is used. Animation selection signals immediately; this
script does not wait for playback completion. Common clip4 frame-count header
is supplied (absent from room), so no real animation/pose/playthrough claim.

Normal and ASan/UBSan PASS1group each,1289skipped(total1290), all64cases. No new
runtime fix needed. Header reproduction/Python/whitespace PASS; builds warning-free.
Full CTest PASS8/8 in105.91s; all97 processes finished. Details DAY2_STATION_DECOMP
stage97; logs/artifact underlocal/live.

Next full module5 sender VM46D4,46F8 subtask spawn and470C wait, then scene
construction/entry. The test executes the101 send handler with actual arguments,
not its surrounding VM; task/actor publication remains supplied. Full Day1/Day2
and live/freeze/BIOS/card/resize/packages/release unfinished. LauncherDAY1-41.

## DAY1/DAY2-96: M0034I delivery/exit; Aya setter pointer fixed (2026-09-08)

Previous95 made verified progress. New original fullVM audit sends real451C
payload112, drains65400 into installed module5 mailbox4DEC, executes repeated
waits and the full recipient calculation through exit requests/task end.
352graphs plus1024 original Aya setter cases. Initial native/ASan comparison
FAIL case0 final stage: 2FF78 omitted actor-to-stats dereference at8002FF88.
Restored it with local physical RAM alias handling. Corrected old unit fixtures
that encoded the same wrong pointer layout. Fixed normal and ASan/UBSan PASS
1group each (1288skipped,total1289), covering352graphs+1024setter cases. Header
reproduction/Python/whitespace PASS; builds warning-free. Native app rebuilt.
Full CTest PASS8/8 in106.04s; all96 processes finished.

The earlier95 decoded-only wait polarity was wrong: wait while scratch0 bit16
is CLEAR, continue when SET. The new original executions prove this. Tests supply
that external flag change and republish current task for each VM pass; no claim
that scheduler or actual battle completion sets it. Details DAY2_STATION_DECOMP
stage96. Logs96 underlocal/live. Full Day1/Day2, live/freeze/BIOS/card/resize/
packages/release remain unfinished. Latest published launcher remainsDAY1-41.

## DAY1/DAY2-95: M0034I setup; script multiply overflow fixed (2026-09-08)

Previous94 made verified progress. New original fullVM audit executes actual
M0034I module4 afterattachment40E0 through firstyield4510 in672cases. Native
comparison matches RNG, slot allocation, tagged setters/getters and task state.
First sanitizer run exposed signed multiply overflow in ALU0F; native12850 now
uses unsigned low-word multiplication. Fixed normal and ASan/UBSan PASS1group
(each1287skipped,total1288). Header reproduction/Python/whitespace PASS, builds
warning-free. Full CTest PASS8/8 in131.41s; all stage95 processes finished.

Actual first random selection is34or50, not7: earlier assignment overwrites the
random value before a later comparison. Second selects33or49. Getter clamp then
million addition verified with signed/wrapping persist80 inputs. Occupancy0/3/6/7
covers first/middle/last/free-table failure preserving prior record; serialFFwrap.
Supplied attached actor/task/RNG/allocation; no actual construction or live scene
claim. Details DAY2_STATION_DECOMP stage95; all logs/artifact underlocal/live.

Next451C send(type5,id0,payload112) recipient path, then remaining M0034I scene
entry/construction. Full Day1/Day2 and live/freeze/BIOS/card/resize/packages/release
remain unfinished. Latest published launcher remainsDAY1-41.

## DAY1/DAY2-94: native real M0034I model/packets match (2026-09-08)

Previous93 made verified progress. Disc-backed test_DAY1_m0034i_model composes
native3D050 initialization and complete3D834 over realM0034I model/clips,84cases.
Original init+pose hashes generated by --native-header; --check-native-header
verifieswithoutrewriting. Header contains hashes/metadata only, nooriginalassets.
Native normal and ASan/UBSan PASS1group each,1286skipped(total1287); no runtimefix
needed. Full CTest PASS8/8 in80.06s. Header reproduction, Python and whitespace
checks PASS; builds warning-free. All stage94 processes finished.
SeeDAY2_STATION_DECOMP stage94.

Coverage: fullmodel/actor/storage/packet/chunk mutations,B1638,CDDC hashes;
14clips*3frames*2packetmodes. Commandpublication suppliedfromreal directory.
Noouterconstructor/textureadjust/real lighting/GPUsubmission/live scene claim.
Testsreadconfiguredoriginaldisc; skipifabsent, executedhere. Normal log named
-before isfirstsuccessfulcomparison, notfailed/pre-fix. No game/platformCchange.

Next M0034I module4 persistent80/scratch10 predicates/randomactions andactual
actorconstruction/sceneentry. WholeDay1/Day2 andDay1 live/freeze/BIOS/card/resize/
packages/release remainunfinished. Logs94 underlocal/live.

## DAY1/DAY2-93: NCCT overflow fixed; real packet updates audited (2026-09-08)

Previous92 made verified progress. Originalrunner now implementsNCCT0118043F
frompsx-spx documented arithmetic (not console captures). New512NCCTcases compare
nativeRGBFIFO/finalMAC/IR; old nativefailscase256. Fixed pe_gte_mul3 (NCCTonly)
44-bit per-sum wrap and signed32-bitMAC beforeIRclamp. Normal/sanitizerPASS1group,
1285skipped(total1286). NativeNCCT FLAGinterface remains unimplemented; noflagclaim.

M0034I resource audit --lighting runs fulloriginal3D834 including3B97C and both
3BCE0 packet buffers;84casesPASS,39distinctnonempty packet hashes. Supplied
lighting/view/list/task/allocation; actualmodel/clips/point/A4bytes. Artifact
local/live/m0034i-anchor-lighting.json. This removes priorNCCToracleboundary.
NoGPUsubmission/renderedimage/liveconstructor/complete scene claim. See stage93
inDAY2_STATION_DECOMP. FullCTestPASS8/8; Total Test time (real) = 118.11 sec. Bothartifactrechecks,
NCCTheaderrepro,Python/whitespacePASS. Warning-freebuilds. Allstage93processesfinished.

Next native complete real-resource graph comparison, remainingM0034I module4
persistent80/scratch10 predicates/randomactions, realparentconstruction/sceneentry.
Both complete days and Day1 live/freeze/BIOS/card/resize/packages/release remain
unfinished. Logs names93; old before-fix failure retained.

## DAY1/DAY2-92: real M0034I model/pose/attachment evidence (2026-09-08)

Previous91 made verified progress. New pe_m0034i_anchor_resources.py executes
original publication6B7B0..6B898, full3D050 on actual model3,1A680,3D834 pose
prefix BEFORE3B97C lighting, then real module4 VMprefix BEFORE12850.84 slices
(14clips*3frames*2packetmodes) PASS,35distinctjoint49poses. Paired packet modes
agree. Real52-joint model, point49=[0,-10,-174,245]; model80197250, script801B4B00
at suppliedchunkbase8018EFE8. ExactA4bytes at801B8BCC, following assignment runs.
Pinned chunkhash, seeDAY2_STATION_DECOMP92. Artifactlocal/live/m0034i-anchor-resources.json.
Memory placement/actorlist/task/view supplied; not outer loader/constructor/live.

Probe full packet pose/render call hits unsupported original GTE NCCT0118043F.
Finalaudit stops before lighting explicitly. No native C changed/no new native
suite claim. Initialprobes/CE00assertfailed; finalCE00script+4108. Next native
real-resource comparison, resolve lighting oracle gap, and remainingmodule4
persistent80/scratch10 predicates/randomactions. Both full days and Day1 live/
freeze/BIOS/card/resize/package/release acceptance remain unfinished.
Exact artifact reproduction, final strengthened assertions, Python compilation
and whitespace checks PASS. All stage92 processes finished.
Logs oracle-day1-92-resources*.log and probe-day1-92*.log inlocal/live.

## DAY1/DAY2-91: M0034I A4 anchor attachment restored (2026-09-08)

Previous90 made verified progress. NativeA4/15108 now performs3DF50 anchor bind,
3A6A8 transform,18C/position/2000 stores. Fixed existing3E188 omissions: world
point comes from parent18 selected record; view/joint compose and two projected
anchors64/5C were missing. Private view-aware helper, legacy public wrapper
usesB89F8,3A6A8passesview. Original640fullVM comparisons pass normal/sanitizer
1group,1284skipped(total1285); original header and640prior attachment regression
PASS, Python/whitespace PASS; warning-free final builds. Full CTest PASS8/8; Total Test time (real) =  83.61 sec. All stage91 processes finished.

Initial91comparison failed;gdb low-RAMdiff exposed originalrunner scratch1F800000
aliasing intoRAM0. Added OPTIONAL separate scratchpad buffer to instruction
load/store mapping (defaultunchanged; BIOS/fetch not covered). Newanchor oracle
usesit, preserveslowRAMcanaries, exercises nonzero camera. Source independently
showed native3E188's missingworldpoint/projection work. Final logs use-fixed;
all old91failedlogs retained. SeeDAY2_STATION_DECOMP stage91 for limits.

Next actualscene frontier: M0034I module4 beginsA4(3,0,49) at raw40CC, exactbytes
verified. Current tests use syntheticparent records/matrices and append20 to
complete controlledVMpass. Trace real parenttype3 resources/joint49 and module4
following persistent80/scratch10 predicates and randomactions. Module3init starts
9B(35,130,11),CA(3),9D(58982),ED(2601,96,0...), see source. FullDay1/Day2 and live/
freeze/BIOS/card/resize/package/release requirements remain unfinished.

## DAY1/DAY2-90: five attachment VM commands restored (2026-09-08)

Previous89 made verified progress. Native VM now handles C5/CC/D4/D5/D6,
including model3E0A4/3E0D0 leaf stores, actor lookup, animation flags, and D5's
original low-RAM98 tail after clearing18C. Local physical-address mapping only.
New pe_attachment_vm_oracle640 original full17018 VM passes through attachment
then20/end: original/native/sanitizer comparisons PASS1group,1283 skipped (total
1284). Repro/Python/whitespace PASS; warning-free builds. Full CTest PASS8/8; Total Test time (real) =  82.89 sec. All stage90 processes finished.
SeeDAY2_STATION_DECOMP stage90; logs names90 inlocal/live.

Actual next source site: new pe_attachment_script_inventory extracts21 inspected
Day1-route+initialDay2/shared packages; ONE attachment opcode: M0034I module4,
script-relative40CC, A4->15108. NoC5/CC/D4/D5/D6 in this inspected set, not full-day
absence proof. Exact artifact reproduction PASS (attachment-script-inventory.json).
A4 is still unported:15108 calls3DF50 (no native definition found), then3A6A8
(existing nativefunc_80015240_port.c), sets18C and copies signed254/256/258 to
fixed-point28/2C/30, sets2000. Trace real M0034I site and resource/model state next.
Latest640fixtures are synthetic model/list fullVM passes, no live scene/rendering
claim. Both full days and Day1 BIOS/card/freeze/resize/package/release remain open.

## DAY1/DAY2-89: linked animation restored (2026-09-08)

Previous88 made verified progress. Native1A4AC now calls1A784 with parent+E then
copies parent14 on flag200000, retaining earlier pause/target returns. Fixed
1A784's incorrect reuse of1A680: local command changes must not unconditionally
restart descendants before their own unchanged-command checks.576 original
comparison graphs cover restart/select/tick, three-level chain, siblings and
excluded actors. Old native fails case196; final normal/sanitizer PASS1group
1282 skipped (total1283). Repro/Python/whitespace PASS; warning-free builds.
Full CTest PASS8/8; Total Test time (real) =  85.97 sec. All stage89 processes finished. SeeDAY2_STATION_DECOMP stage89; logs names89.

Next source frontier: attachment writers C5/19170,CC/19260,D4/19F04,D5/19FE0,
D6/1A064 are absent from native VM; opcode table910A0 verified from pinned EXE.
D4 sets18C, parent100000 and current600000. D5 clears18C BEFORE reading it again
and can reach low-RAM98; preserve original order, don't invent a repaired parent
pointer. C5/CC call3E0A4/3E0D0 on actor1B4. Another attachment writer15200 is inside15108 (asm3420.s); needs
caller/command tracing. Inventory initial Day2/shared41/42/43/351 finds zero of
these five opcodes (ignored day2-linked-command-inventory.json); not a full-day
absence proof. Latest actor fixture has valid pointers/resources and no sound
records; no live attachment/model/scheduler/scene claim. Both full days and Day1
BIOS/card/freeze/resize/package/release acceptance remain unfinished.

## DAY1/DAY2-88: animation sound event scan restored (2026-09-08)

Previous87 made verified progress. Restored6A318 in func_80029810_port.c and
unconditional1A4AC call before previous-frame/flags stores. Aya-only four-record
scan, extra actor type/ID records, command matching, forward/reverse/wrap frame
intervals, sound-variant selection and signed positional6DCE4 calls are native.
SeeDAY2_STATION_DECOMP stage88. Paused/latched animation still processes events.

New pe_animation_sound_oracle512 original graphs including projection/FIFO,
half direct scan and half integrated ticker. Normal/sanitizer comparisons each
PASS1group with1281 skipped; original header reproduction, Python compilation,
whitespace PASS; builds warning-free. Synthetic package/camera, no audible or
live-scene claim. Full CTest PASS8/8; Total Test time (real) =  84.68 sec. All stage88 processes finished. Logs names88 in
local/live; generated header retail_animation_sound_cases.h; test helper
pc_port/tests/test_animation_sound.h. No processes from earlier stages pending.

Next fix/audit1A4AC flag200000: source1A550..574 calls1A784(actor,parent+E), then
reloads18C and copies parent14. Native currently returns without either action.
Existing1A784 and1A680 propagate through D20C for100000 actors; verify attached
actor recursion and model/resource binding, then join real outer scheduler and
station waits. Both days' complete decompilation and Day1 live/BIOS/card/freeze/
release acceptance remain unfinished.

Source inspection while tests ran: native1A784 reuses1A680 on command change,
which already propagates/restarts descendants; original1A784 performs its own
local reset then recursively calls1A784 only. This may reset an already matching
descendant unnecessarily. Include changed-parent/same-command-child cases in
the next linked-actor comparison.

## DAY1/DAY2-87: animation ticker arithmetic repair (2026-09-08)

Previous86 made verified progress. Original1A4AC exposed four native arithmetic
errors: missing pre-wrap target clamp, quotient instead of remainder, missing
reverse-wrap previous-frame reset, and strict rather than inclusive old-frame
comparison in the final clamp. Fixed infunc_80029810_port.c. New original-code
oracle3360 cases; native regression fails before fix, passes after in normal
and sanitizer builds (1group PASS,1280 skipped; total1281). Header reproduction,
Python compilation and whitespace checks PASS; final builds warning-free.

Station waits --advance mode now executes original1A4AC between original VM
passes:300graphs PASS, speed3frames/pass, no supplied target-frame shortcut.
Both station artifacts reproduce exactly. SeeDAY2_STATION_DECOMP stage87.
No live scheduler/clock/resource/scene completion claim.

Critical source correction:6A318 is unconditional sound-event processing, even
with no attachment; old BTL66 comment was wrong. Native call is still absent.
Next restore/audit6A318 (asm55C00.s:5726, through6A5BC; sound calls6DCE4) and
flag200000 linked-animation1A784/parent14 copy, then join actual resource and
scheduler paths. Arithmetic oracle uses non-Aya actor/no additional sound
records, so original6A318 executes without eligible events. Both full days and
Day1 live/BIOS/card/freeze/release scope remain open. Logs/artifactslocal/live
names87. Full CTest PASS8/8; Total Test time (real) =  75.52 sec.
All stage87 audit/build/test processes finished.

## DAY1/DAY2-86: station VM wait-to-signal timing (2026-09-08)

Previous85 made verified progress. New `pe_day2_station_waits.py` runs original
17018 VM/binder over five real station2F/30/signal tails.300 graphs PASS: target
clamping, exact integer-frame equality (not >=), fractional-bit independence,
always-yielding wait, PC rewind/advance, delay1 and deferred scratch assignment.
The final tail also executes20/task termination; other tails stop before the
next ALU handler. SeeDAY2_STATION_DECOMP stage86. These are original VM passes,
not manual per-handler binding. Supplied inter-pass animation state is explicit.

Exact artifact reproduction, Python compilation and whitespace checks PASS.
Native17B34/17B74 already match the inspected conditions; no C change/new native
suite claim. Next trace actual animation advancement and scene resource commands,
then join actor creation/delivery/outer scheduler with these VM slices. Both days'
full scope and Day1 model/BIOS/card/live/freeze/release acceptance remain open.
Artifact `local/live/day2-station-waits.json`; logs `oracle-day2-86-waits*.log`.
All audit processes finished.

## DAY1/DAY2-85: restore type0 initialization at zero offsets (2026-09-08)

Previous84 made verified progress. Found and fixed native2F76C's incorrect
A8038/A803C nonzero guard: these are relative offsets, and original code always
calls5218C/51980/51E64. Expanded equipment audit then exposed an invalid native
read at7 when weapon lookup returned0; original51980 reads physical low RAM.
Added local physical-to-canonical mapping for that weapon access, preserving
armor's original null branch. No skipped-call shortcut remains in2F76C.

New `pe_equipment_offsets_oracle.py` generates132 original complete equipment
cases (33 x4 offset combinations). EQP1 native comparison expanded accordingly;
normal/sanitizer focused checks PASS1group,1279skipped. Constructor audit now
includes8 type0 model-free paths, total120 PASS. Header verification, exact
constructor artifact reproduction, Python/whitespace checks PASS; warning-free
final builds. Full CTest PASS8/8 in74.62s (`local/live/ctest-day1-85.log`).
SeeDAY2_STATION_DECOMP stage85 for scope/correction of the earlier zero-offset
assumption. Initial focused tests aborted134; gdb trace inlocal/live
identified weapon lookup/low-RAM access; final focused builds pass.

Remaining: actual table/model publication, type0 model continuation, complete
pre14 initialization/scheduling, all Day2 rooms and Day1 live/BIOS/card/freeze/
release acceptance. Logs `oracle-day1-85-*`, `oracle-day2-85-*`,
`build-day1-85-*`, `build-asan-day1-85-*`, `ctest-day1-85.log` underlocal/live/.

## DAY1/DAY2-84: station constructor module binding (2026-09-08)

Previous83 made verified progress. Added `pe_day2_station_constructors.py`:
original12574 relocation,1266C task-pool setup,35038 type1..7 constructors and
isolated14 installs after returning constructors.112 paths PASS:56 model-free
returns and56 model-bearing prefixes stopping at362B8 with exact180/76-byte
requests. Verified module entry selection, initial task, IDs/serials/count wrap,
free-list/list links, handler initialization and model pointer before boundary.
SeeDAY2_STATION_DECOMP stage84. Native C unchanged; no new native-suite claim.
Exact artifact reproduction, Python compilation and whitespace checks PASS.

Actual scene-loader publication, type0's2F76C/resources, full model continuation,
pre14 scene initialization and scheduled execution remain. This establishes
original type-to-module binding for1..7 without substituting model-free actors
for the final game. Next join constructor/init/delivery/scheduler evidence and
restore missing native behavior; preserve both days' full scope and Day1's
BIOS/card/live/freeze/release work. Artifact `local/live/day2-station-constructors.json`;
logs `oracle-day2-84-constructors*.log`. All processes finished.

## DAY1/DAY2-83: station handler installation and delivery (2026-09-08)

Previous82 made verified progress. New `pe_day2_station_delivery.py` executes
all ten original station opcode14 installs, original script sends,65400 drain
and177AC poll in an explicit actor-list/free-task-pool fixture.32 graphs PASS,
376 tasks verified, including entry PCs, links, serial wrap, payload/sender,
flags/delay, disabled handlers, ID mismatches and multiple matching recipients.
Module2 mailbox entry is711C, distinct from67B0 main script; modules0/3/4/5
install the entries inDAY2_STATION_DECOMP stage83. Task+18/+1C are preserved.
Python compilation/whitespace and exact artifact reproduction PASS. No C changed.

Actual actor/module construction and pre14 initialization, scheduler execution
and asynchronous scene completion remain unproved. Next trace35038/module
binding and the scene commands; retain both days' full scope and all Day1
BIOS/card/live/freeze/release work. Raw artifact `local/live/day2-station-delivery.json`;
logs `oracle-day2-83-delivery.log` and `oracle-day2-83-delivery-check.log`.
All processes finished at this checkpoint; no native-suite pass is claimed.

## DAY1/DAY2-82: Day2 station mailbox/counter relationships (2026-09-08)

Previous81 made verified progress. Added `pe_day2_station_mailboxes.py` using
fresh pinned M0042I extraction and original handlers: receiver branches in
modules0/3/4, eight constant-ready stores and their source continuations,
module3/4 placement gates, and ten source sends through original17764/653B8.
815 receiver/placement paths,8 assignments and160 sends PASS. Shared station
fixture now binds task/payload and supports177AC; prior2696-case audit PASS.
Exact artifact reproduction, Python compilation and whitespace checks PASS.
All audit processes finished; no native build/test is claimed for this C-free change.
See `DAY2_STATION_DECOMP.md` stage82 for payload/address/sequence tables.

Key distinction: module2 sends7 to type0 at6F2C, matching module0's scratch20
producer. Module4's7 branch produces scratch18, but there is no immediate
(4,0,7) send in this script. Do not fabricate that link. Source continuations
are inspected, not executed past asynchronous boundaries. Actor initialization,
delivery, scheduling and scene completion still lack complete evidence.
No runtime C changed. Next trace the actual actor/module binding and scene
commands between receiving messages and setting signals, then remaining Day2
rooms. Preserve unfinished Day1 native/BIOS/card/live/freeze/release requirements.
Logs: `local/live/oracle-day2-82-*`; artifact `day2-station-mailboxes.json`.

## DAY1/DAY2-81: Day2 station predicate decompilation (2026-09-08)

Previous80 made verified progress. Returned to M0042I station semantics.
New `pe_day2_station_paths.py` extracts/pins the original script and executes
closed module1/2 regions through original ALU/branch/assignment handlers. It
proves module1's story88 scratch22 producer, module2's placement selector,
entry counter/completion/flag gate, component5 decision, seven synchronization
counter waits, and final persist44|=8/story90 assignments. No asynchronous opcode
is skipped or response fabricated. Source anchors for all nine constant-ready
writes to scratch18/19/20/22 are asserted in the freshly extracted script.
See `DAY2_STATION_DECOMP.md` for pseudocode, paths and explicit scope limits.

Audit PASS2696 original paths, including signed story boundaries and wrapped
counters; exact trace-artifact reproduction, Python compilation and whitespace
checks passed. All sessions finished. Output is ignored
`local/live/day2-station-paths.json`; log `oracle-day2-81-station.log`.
No runtime C changed. Next trace module0/1 actor setup and module3/4 producers,
then module2 asynchronous scene commands and remaining rooms. Day2's full room
set/ending and native passage remain unproved. Preserve all Day1 implementation,
BIOS/card/parent-context, random-battle freeze and final release work below.

## DAY1/DAY2-80: cleanup formatter context (2026-09-08)

Previous79 made verified progress. Added `PE_CardCleanupFrame`: original50h
prologue, override42228/handle72774/window4D5CC boundaries, and state9's two
formatter calls. Negative-handle state9 now formats filename then device+filename
into frame+18h, stopping at72314 string length with original partial output.
No BIOS completion or cleanup suffix is synthesized. Framed dispatcher invalid
flags now propagate original saved registers and42000 return into this adapter.
Context-free native entry retains its prior returning behavior/boundaries.
SeeCARD_OPERATION_CONTRACT.md stage80 for exact memory/register semantics.

Frame oracle expanded to1280 original comparisons (1024 dispatcher+256 direct
cleanup); generation and exact header check PASS. Normal/sanitizer focused card
checks PASS8groups,1272skipped; full CTest PASS8/8 in73.42s. Both builds warning-free. Logs under `local/live/` use
`oracle-day1-80-*`, `build-day1-80-*`, `build-asan-day1-80-*`, `ctest-day1-80.log`.
Next: propagate context through window/driver parents and establish BIOS string/
event/file authority. Also resume Day2 M0042I predicate/scene decompilation and
route expansion; neither day's complete inventory or acceptance is established.
Live stack/context, random-battle freeze and final release remain unfinished.

## DAY1/DAY2-79: card dispatcher formatter context (2026-09-08)

Previous78 made verified progress. Added `PE_CardOperationFrame` entry sharing
func_80041108 implementation. Supplied original SP/registers establish78h frame,
five saved-register writes, filename fifth argument and path-specific formatter
register spills/return address. States3-6/11 now format and stop at72734 with
original mode; state14 formats frame+18h and stops at72784. State2 supplies actual
frame+20h directory pointer. Stop guards preserve unresolved formatter behavior.
Context-free live entry keeps prior boundaries; framed invalid flags stop40F80
before its still-unbound cleanup frame. No BIOS result is synthesized.

New original1024-case framed dispatcher audit and exact header verification PASS.
Normal/sanitizer focused dispatcher tests PASS2groups,1278skipped; both builds
warning-free. Full CTest passed8/8 in73.43s (`local/live/ctest-day1-79.log`).
See `CARD_OPERATION_CONTRACT.md` stage79 for call/register table and parent SP
trace:5C49818h +425DC20h +4110878h +formatter250h =300h total depth.
This establishes relative frame identity, not live absolute SP or all prior
scratch/register effects. Next propagate context through parents, restore40F80
frame path, and establish BIOS string/event/file semantics. Full Day1/Day2,
live acceptance, random-battle freeze and final release remain unfinished.
Logs: `local/live/oracle-day1-79-*`, `build-day1-79-*`,
`build-asan-day1-79-*`, `ctest-day1-79.log`.

## DAY1/DAY2-78: explicit-frame native formatter core (2026-09-08)

Previous77 made verified progress. Added `PE_FormatterFrame` in
`func_80071A84_port.c`, implementing original parsing/numeric/pointer/character/
counted-string/count-store/padding behavior and live tables. Caller supplies
original guest SP and incoming s0..s7/ra. Argument spills, saved-register memory,
flags/width/precision, and backward temporary text remain in guest memory;
72334 copies preserve overlap. No fabricated guest frame or host printf.
Ordinary strings explicitly stop at72314/72324; unknown dispatch stops.
See `FORMATTER_CONTRACT.md` stage78 for exact scope and remaining adapter work.

New `pe_formatter_frame_oracle.py` executes880 source-pinned original cases and
compares output and complete exercised stack-region hashes. Includes three SPs,
nonzero defaults, counted-string/output overlap, output in temporary storage,
%n modifying a later argument, precision530 writing before frame, and BIOS stops.
Header regeneration/check and Python compilation passed. Normal and sanitizer
focused test passed1group,1278skipped; both builds warning-free. Initial build
needed the existing game_port.h stop declarations; corrected before validation.
Full CTest passed8/8 in71.87s (`local/live/ctest-day1-78.log`).

Card41108/40F80 callers retain71A84 boundaries: their original caller SP/register
context is not established. This core preserves memory/output, not full register
restoration after possibly overwritten saved slots. Next establish caller context
and BIOS length/search authority, then restore post-format card paths. The local
file inventory found no BIOS image underlocal/. Broader Day1/Day2, live testing,
random-battle freeze and final release remain unfinished; launcher stillDAY1-41.
Logs: `local/live/oracle-day1-78-*`, `build-day1-78-*`,
`build-asan-day1-78-*`, `ctest-day1-78.log`.

## DAY1/DAY2-77: formatter contract and copy dependency (2026-09-08)

Decompiled the full original71A84 formatter (545 words) and executed432
source-pinned cases covering numeric conversions, modifiers, padding, pointers,
characters, counted strings, count stores and card filename/device formats.
See `FORMATTER_CONTRACT.md`. This is original execution evidence; the formatter
itself remains an explicit native boundary, including card operation calls.

Added native72334 byte-copy helper (27 words), preserving signed nonpositive
count behavior, raw unsigned pointer direction and direction-dependent return.
Local access normalization handles physical/KSEG1 RAM aliases without changing
raw pointer comparisons or the global memory API. Compared2048 original cases
including overlap and alias ordering; generated fixture header check passed.
Normal and sanitizer focused copy test passed (1 group,1277 skipped).
Full CTest passed8/8 in72.62s; both builds had no warnings/errors.
Artifacts/logs: `local/live/formatter-contract.json`, `oracle-day1-77-*`,
`build-day1-77-*`, `build-asan-day1-77-*`, `ctest-day1-77.log`.
Next: native formatter with explicit guest argument/temporary-storage semantics,
then resume post-format card graph. Ordinary-string BIOS length/search and
actual card I/O remain unresolved. No live/full-day/release acceptance claim.

## DAY1/DAY2-76: cleanup callback and window restoration (2026-09-08)

Previous75 was progress. Added42228/4D5CC fullnative helpers and40F80 returning
paths through first BIOS/format boundary in func_80041108_port.c. Invalid-flag
41108 paths now call40F80. Callback9CFFC uses live value afterclose40; supports
42910,42928,5C488,62F9C, unknownstops before slotclear/suffix. Newstopguards,
originalinverse stride, conditionalwindow cleanup/selection restoration preserved.
SeeCARD_OPERATION_CONTRACT.md stage76. Post-BIOS40F80 remains unported.

Updated8192 operation and112driver original comparisons generated successfully;
operationfixtures now use validempty windowlist andexplicitA185C/callback/index
because execution goes deeper. Newpe_card_cleanup_oracle.py256case generation PASS after adding explicit
no-active-window cases and correcting a visited-PC assertion from delay slot
4D664 to owning call4D660. Native test_card_cleanup.h added; initial normal/sanitizer runs failed boundary
case8 because sparse fixture lacked92224=80010ED4, while RAM already matched.
Corrected seed, regenerated all expected data from original. Final focused
normal/sanitizer DAY1_card_ suites PASS7 groups each,1270 skipped. All three
header regeneration checks PASS. Finalbuilds no warnings/errors; Python/scoped
whitespace PASS. Full normal CTest8/8 PASS74.55s, including1277 native groups.
All process handles terminal. Logs local/live/
oracle-day1-76-card-{operation,driver,cleanup}{,-check}.log and
{build,build-asan}-day1-76-{build,card}.log; ctest-day1-76.log. Final acceptance still unproven; Changes local,launcherDAY1-41.

Next71A84 formatter (545words; local/live/card-formatter-calls.json and source
formats inCARD_OPERATION_CONTRACT.md), guest-stack binding, BIOS event/file semantics and
post-call41108/40F80 perCARD_OPERATION_CONTRACT.md. FullDay1/Day2 unfinished.

## DAY1/DAY2-75: operation dispatch and full source state contract (2026-09-08)

Previous74 was progress: status/driver verified. Read all41108..42020 (966words)
and40F80..41108(98words). Full source-derived decompilation is documented in
CARD_OPERATION_CONTRACT.md:16-state table, directory accounting, alternating
candidate search, signed transfer caps, retry/cleanup and parsed-header copy.
This document is not a claim that post-BIOS behavior has been implemented.

New func_80041108_port.c implements ALL dispatch entry paths through their first
unresolved callee or return. Includes state1/13 status gating, state2 directory
initialization, fullstate3 candidate search, state4 variant write, state7/8/9
signed sizes and invalid-flag cleanup routing. Stops at40F80,71A84,727B4,
72754,72764,72774. Boundary payload records known-register mask and fifth arg;
unknown frame+18/+20 addresses are not fabricated. 425DC now calls41108 and
can continue second slot after an actual returning operation. CMake/prototypes
updated. Post-call paths remain unported; no BIOS response is synthesized.

Newpe_card_operation_oracle.py8192 cases generated from original;updated
pe_card_driver_oracle.py112cases now run original41108 to deeper boundaries,
not stop at its entry. Native tests compare RAM, known arguments, fifth arg,
return/stop target. Original generations/regeneration checks PASS. Normal/sanitizer builds and
focused DAY1_card_ tests PASS6 groups each,1270 skipped. No warnings/errors;
Python/scoped whitespace PASS. Full normal CTest8/8 PASS71.43s, including1276
native groups. All process handles terminal. Logs local/live/
{build,build-asan}-day1-75-{build,card}.log andoracle-day1-75-card-
{operation,driver}{,-check}.log; ctest-day1-75.log. Changes local; launcherDAY1-41.

Next implement40F80 cleanup (first invalid-flag operation edge), its42228/
4D5CC dependencies, then filename/guest-stack binding and BIOS card events.
Full post-call41108 behavior remains to port perCARD_OPERATION_CONTRACT.md.
Keep bothdays/fullDay1/live/freeze acceptance unfinished.

## DAY1/DAY2-74: card status polling and frame driver (2026-09-08)

Previous73 was progress: confirmation/progress callbacks verified. New native
func_800405A4_port.c translates408 words:405A4,425DC,42928,4CDAC,4D4A0,4D9D8,
4DC84. Status machine five branches, original event priority/poll counter and
slot alternation, status dialogs and removal cleanup follow original. BIOS
726F4 and operation processor41108 remain explicit nonreturning boundaries.
5C498 now calls425DC for9D030>1, skips timer increment after it as original;
notice dispatch supports42928. Contracts/pins inCARD_RECORD_CONTRACT.md and
new pe_card_status_oracle.py (4096 original cases) /pe_card_driver_oracle.py
(96 driver frontiers+16 callback cases). Both original generation/regeneration checks PASS. Normal/sanitizer builds
and focused DAY1_card_ comparisons PASS5 groups each,1270 skipped. Includes
new4096 status and112 driver/callback cases plus prior card regressions.
No build warnings/errors; Python/scoped whitespace PASS. Full normal CTest8/8
PASS76.54s, including1275 native groups. All process handles terminal. Logs local/live/{build,build-asan}-day1-74-{build,card}.log,
oracle-day1-74-card-{status,driver}{,-check}.log; ctest-day1-74.log. No full acceptance claim.

Next restore full41108 operation processor (966words, SHA256 and call/state
inventory local/live/card-operation-calls.json) and original BIOS event/card semantics;
pe_libcard.c initialization collapses hardware bring-up, andpe_libetc.c event
storage retains only audio event state, discarding card registration state.
These gaps must be resolved for returning card BIOS behavior;
425DC now reaches these deeper boundaries instead of stopping before polling.
Do not infer completed card operations from state13/14 or synthesized BIOS results.
Changes local; launcherDAY1-41. Both days/full Day1/live/freeze remain unfinished.

## DAY1/DAY2-73: card confirmation and progress callbacks (2026-09-08)

Previous72 was progress: exit input/notice graph and5C1EC cleanup fix verified.
Now restored247 original words:42464,428D4,42B50,4CE28,4CFD4,4DA04,4DA9C,
50580 in func_80015AF0_port.c. Source spans/hashes/ordered semantics are in
CARD_RECORD_CONTRACT.md. Both confirm and cancel constructors restored;
progress display and two-line notice draw connected, always1 progress input
connected,44E98 dispatches50580,42B6C dispatches delayed428D4 state13 store.
State13 card I/O processing remains unported; callback is not I/O completion.

pe_card_confirmation_oracle.py generated128 original states,64 include three
42B6C polls and drawing. Native test_card_confirmation.h compares RAM and
requires no stop/stub; text/input/frame/progress fixtures are synthetic.
Original generation/header regeneration PASS128. Normal and sanitizer focused
tests PASS1group each (1272 skipped); full normal CTest8/8 PASS74.44s, including
1273 native groups. No build warnings/errors; Python/scoped whitespace PASS.
All process handles terminal. No live/full-day acceptance claim.
Logs local/live/{build,build-asan}-day1-73-{build,card-confirm}.log and
oracle-day1-73-card-confirm{,-check}.log; ctest-day1-73.log. Changes local; launcherDAY1-41.
Next:5C498 still stops at425DC for9D030>1. Restore405A4/41108 full state
machines and425DC driver; all in307CC.s. Original table10F6C maps state13
411E4 thenstate14 41270, whose71A84/72784 graph remains unresolved. Detailed
state13/14 semantics at end ofCARD_RECORD_CONTRACT.md. Keep original BIOS
boundaries and stop propagation; do not synthesize card-operation completion.

## DAY1/DAY2-72: exit-menu input and returning-cleanup fix (2026-09-08)

Restored full4D2DC..4D4A0 (113words) in func_80015AF0_port.c and wired
63E0C menu input dispatcher. Confirm/cancel priority, invalid selection sound,
card gates/enqueue, close37/38/36/19 and post-close reset follow original order.
New stop epochs suppress effects after unresolved callees. Added42910 notice
callback dispatch in4F910_port.c func4D030. See EXIT_MENU_CONTRACT.md.

pe_exit_menu_input_oracle.py checks328 original graphs (320menu input across
buttons/selections/card flags/busy/cleanup states +8notice input cases), with
72774 explicit stop and original visited-PC sound-path checks. Generated header
and test_exit_menu_input.h compare RAM, return/stop and boundary identity;
queued5E30C event verifies actual dispatch. Original/header check PASS328.
Initial native cancel case80 differed onlyB0CD8: shared5C1EC(0) omitted original
post-cleanup clearC000. Restored that suffix with stop guard; corrected old
B54KQ quiet-return test that required stale flags. Normal and sanitizer focused
tests now PASS1group each,1271 skipped. Python/scoped whitespace checks pass.
Full normal CTest8/8 PASS69.09s, including1272 native groups. Both build logs
contain no warnings/errors. All process handles terminal. Logs local/live/{build,build-asan}-day1-72-
{build,input}.log and ctest-day1-72.log. Changes local; launcher remainsDAY1-41.

Next restore modal50580..50618 and deeper callback graph. Original source:
40D80.s50580 constructswindow39 with4DA04/4DA9C, sets9D000=40, invokes42B50
with428D4; zero argument uses4CE28 and42910. 330D4.s428D4 writes state13 at
A0ED5+(A1860-1)*418 with unchecked wrapping arithmetic. 3E204.s4DA04 draws
text9D000 then, unless40, eight progress glyphs based on42464. Locate42B50,
4DA9C and4CE28 bodies before implementing. No card I/O, audio queue, live
presentation, whole Day1/Day2 or freeze acceptance established by these fixtures.

## DAY1/DAY2-71: modal gate/layout and canonical resource bank (2026-09-08)

Previous70 restoredcardhelpers/cleanupstop(progress). Restored42848..428C4
(31words) and4DAA4..4DC84(120words),151total, in func_80015AF0_port.c.
42848 testsrecordflag4, publishesindex+1 toA1860 onlyifzero thenmodalconstructor,
returnswhetherflagclear.4DAA4 fullmode9CF50 conditional: existing42/40 reuse,
mode1modal42/windowlistcallbacks/textconcat/minwidth120/centeredlayout,
mode0shared4CC50 notice and42910callback. Detailedorderedsemantics and source
SHA256 inCARD_RECORD_CONTRACT.md. No matching-source edits.

pe_card_modal_oracle.py112originalgraphs (96gate/modal+16directmissingtext),
fullallocator/text/resourcecalls; visitedPCchecks bothconstructionmodes,
missingtextandsecondmeasurement. Initialnativecase13 exposed resourcebank
host/gueststate split:52E30 updatedhost D048/D050/D058/D064 butRAMstale.
UnifiedD048,D04C,D050,D054,D058,D064 throughPE_GUEST_U32 inpsx_compat.h,
removedduplicatepc_port/src/pe_globals.c definitions. RetainsD018/D03C owners.
Afterfixnormal/sanitizer DAY1_card_modal PASS1group1270skipped. Newheader
regenerationcheck/Pythoncompile/scopedwhitespacePASS. Existing64draw+1024
predicateoriginalregressionPASS. Finalbuildlogs no warnings/errors. FullCTest
8/8 PASS70.12s; fullsanitizer1271/1271 PASS. Allprocesshandles terminal.
Updated13 olderresourceownershiptests tocanonicalRAMreset/footprints; corrected
validbankseedincanary51CC4 andexactoriginalfieldvalues. Finalscopedwhitespace
PASS; comment-only ownership cleanup followed successful functional tests.
Logs local/live/{oracle-day1-71-card-modal,oracle-day1-71-exit-draw,
native-day1-71-card-modal,native-asan-day1-71-card-modal,
native-day1-71-resource-regression,native-asan-day1-71-resource-regression,
ctest-day1-71}.log. StageLOCAL,launcherDAY1-41.

Next wire/restore4D2DC input (63E0C_port.c dispatcher), noticecallback42910
(4F910_port.c func4D030 currentlysupports62F9C/5C488 only), andmodal50580.
50580..50618 in40D80.s: arg1nonzero constructswindow39 ownedby62CC4,
installsdraw4DA04/input4DA9C,selectswindow,9D000=40,42B50(callback428D4).
Arg1zero=>4CE28(428C4()+47,4B),9CFFC=42910. Missingdeepercallback/IOroutines
need originalproof/nonreturnpropagation. Main4D2DC nowhasmostrecordgatehelpers;
remainingcallees512AC(9),5C1EC(0)/BIOSboundary needcare. Do notinferfullmenu
interactionfrommodalconstructor. BroaderDay1/Day2,live,raster/BIOS andfreeze
remainunfinished. Testsusesynthetictext,notretailvisualproof.

## DAY1/DAY2-70: card-record helpers and cleanup stop fix (2026-09-08)

Previous69 restored menu drawing/predicates(progress). Restored428C4,42910,
4298C,42A10,42AD8,42B28,62CD0 (122words) in func_80015AF0_port.c. Selection
wrapgetter/dualclear, busygetter, state3/8/10 predicate, saved-selection setter,
record0/12 enqueue(state1,+0B=2,+16half=10,selection0,A186C=mode), andcleanup
plusrecord/globalresets. Full source hashes pinned inpe_card_record_oracle.py.
42A10 shares original-identical42798 cleanup loop and gates reset suffix on
stop epoch. Found42798 previouslycontinuedafterunknown72774, stampinghandle-1
andstate12 thenprocessingnextrecord. Fixedto returnimmediatelyafterstop; EV1
expectations corrected to preserve callerstate atthis unresolvededge. No matching
source edits. See CARD_RECORD_CONTRACT.md for full semantics/limits.

New256originalrecordchains exerciseevery statebyte, alternatingrecords, mode,
selectionwrap and cleanup normalreturn vs stop_at72774. Newnative/sanitizer
DAY1_card_record PASS1group1269skipped; sanitizerEV1 PASS3groups1267skipped.
Headerregenerationcheck,Pythoncompile,scopedwhitespace PASS. Finalbuildlogs no
warnings/errors. FullCTest8/8 PASS68.46s,1270nativegroups. Allhandles terminal.
Logs local/live/{oracle-day1-70-card-record,native-day1-70-card-record,
native-asan-day1-70-card-record,native-asan-day1-70-cleanup,ctest-day1-70}.log.
StageLOCAL,
launcherDAY1-41. BIOS72774 remainsunresolved; don'tclaimclosedhandles/cardIO.

Decompiledordered4D2DC input inCARD_RECORD_CONTRACT.md, stillnotnative.
Confirm10000 priorityovercancel40; invalidselectionerrorsound526C4;
0/1 gates42848,42770,42B28 then42A10/4298C(index,1)/525EC;
2 closeswindows37,38,36,19 then5C1EC(0),512AC(9,0),9CFF8=0,525EC;
cancel sameclosechainbut52634. Everyreturn1onlyaftercalledpathsreturn.
Next restore42848+4DAA4 and remainingcallback/callees.42848statebit4set with
A1860zero publishesindex+1 thencalls4DAA4.4DAA4 in3E2A4.s spans4DAA4..4DC84:
mode9CF50 nonzero=>modal42 creation unlessalreadyexists, text/source helpers,
widthmeasure120minimum; installs44E14/44E98/4F950/50580; modezero=>window40
via4CC50 andcallback42910. Mosttext/allocatorhelpersalreadyported; inspect
allreturns before wiring. BroadDay1/Day2,prologue,live/raster/BIOS/random-battle
freeze remain unfinished. Stopfix doesnotcompleteinputorwholemenu.

## DAY1/DAY2-69: restore exit-menu drawing/predicate (2026-09-08)

Previous68 restored E7/constructor (progress). Added complete42770 (10words),
4FDA4(17),4FDE8(28),50C08(18) to func_80015AF0_port.c, total73words.42770
readsA0ED4+index*418hex bit0 with unchecked32-bitwrap;4FDA4 shortcircuitsindex2
true, otherwisecalls42770.50C08 offset(-2,-2), glyphresourceindex+84hex for
signedindex<2 else62hex.4FDE8 publishes9CEF4=list,638D8(list,50C08),style1,
reloadlist+38 count, emitresource68/offset(0,16) percount. Stopguardafterdraw
andresourcecall. Addeddrawdispatch4FDE8/50C08 andcellpredicate4FDA4 arms.
Original input4D2DC remains explicitboundary. No matching-source edits.

pe_exit_menu_draw_oracle.py pins73words:
42770..42798 d7402296e472db8a1ceaa0da190dad0b33ca01eb6d84484c9e125a2ffe2b29ab
4FDA4..4FE58 195a711c9047332870ac24e1be664e47713c67d50e9cb944d7244adcc6aad2be
50C08..50C50 1c30fa45849ee274fec64c80586df607dac8bc016043d3f217037222fb2d646c.
64fulloriginaldrawinggraphs plus1024predicates matchnative/sanitizer focused
tests (1passed1268skipped). Testscoverbuffers,blinkphase,recordflagcombinations,
selection,packetexhaustion,uncheckedindices andallbytevaluesforindices0/1.
Drawingfixtures reuseinventorysynthetictext/resources, originalconstructorthen
recordflags; no retailasset/GPUvisual/liveinputclaim. Headerregenerationcheck,
128originalE7/constructorregression,Pythoncompile/scopedwhitespacePASS.
Finalbuildlogs no compilerwarnings/errors. Details EXIT_MENU_CONTRACT.md.
ChangesLOCAL,launcherDAY1-41. FullCTest8/8 PASS68.34s,1269nativegroups.
All processhandles terminal. Logs local/live/{oracle-day1-69-exit-draw,
oracle-day1-69-exit-menu,native-day1-69-exit-draw,
native-asan-day1-69-exit-draw,ctest-day1-69}.log.

Next restore4D2DC input and calledcard/event graph. Source3DA98.s spans
4D2DC..4D4A0; confirm10000, cancel40.42770nowavailable.42848 in307CC.s tests
recordbit4; ifset andA1860zero writesindex+1 then4DAA4 (3E2A4.s); returns
whetherbit4wasclear.4D2DC confirms0/1 via42848,42770,42B28,42A10,4298C and
sound525EC. Choice2/cancel close36/37/helpmenus via62F1C,5C1EC(0),512AC,
sound. Decompile andverifyactualcalleegraph, keep stoppropagation. M0351I
prologue,menuinteraction,Day2inventory,raster/BIOS/liveacceptance,random-battle
freeze remain open. Callbackrestore doesnotfinishallDay1orDay2.

## DAY1/DAY2-68: restore E7 and menu36 constructor (2026-09-08)

Previous turn67 fixed fade arithmetic and verified waits (progress). Restored
missing native E7/15AF0 and4D18C in game/boot/func_80015AF0_port.c; added CMake,
prototypes and VM arm. Full47+60words, hashes:
15AF0..15BAC 2d4a817590127635949524c2af473ebc41aa77d79c7b3681fada37ee2657a1e7
4D18C..4D27C 80a6ad99960fd8029c6bbe703a9dcdfd67d0406957b7478983ae03e73feeca2a.
E7 scene1000 bypassreturns1; firsteligiblecall sets taskbit20/delay1/PC-8return0;
retry67CBC+constructor then reloadscene/task,scene|9000,input|4,cleartask20return1.
Constructor allocateswindow/list36, installs4D2DC/4FDE8/4FDA4, normalizescursor2
to0, selectslist, finds/createshelp19(+38=36),9CF50=1,5C1EC(1),42538,5DE88.
Newlyraisedstopguards E7suffix. Callback execution remains unresolved.

pe_exit_menu_oracle.py128cases execute original E7/fullconstructor including
allocator/list/reset callees, with BIOSA28 bzero memory contract added to the
test-only runner. Checks original M0351I E7at801910C0/noargs. Original/native
states match across initialcall/retry, existinghelp, savedcursor, sceneflags,
taskflags and renderenabled. VisitedPC asserts firstyield,constructor,selection2
reset,helpcreation. Native test_exit_menu.h alsochecks VM firstyield and bypass
continuation. Republishactor/taskbetween VM passes (walk_next exhaustscurrent).
Normal/sanitizer focusedtests PASS1group1267skipped. Original headercheck and
entryaudit772/96/256/1050/256 PASS. Pythoncompile/scopedwhitespace PASS; final
buildlogs no warnings/errors. Details EXIT_MENU_CONTRACT.md. StageLOCAL,
launcherDAY1-41. FinalfullCTest8/8 PASS68.01s,1268nativegroups. All process
handles terminal. Logs local/live/{oracle-day1-68-exit-menu,
oracle-day1-68-entry-chain,native-day1-68-exit-menu,
native-asan-day1-68-exit-menu,ctest-day1-68}.log.

Next restore menu36 callbacks (currentlyabsent native):4D2DC..4D4A0 in
asm/disc1/3DA98.s;4FDA4..4FDE8 and4FDE8..4FE58 in401A0.s.4FDA4 enablesindex2
or result42770.4FDE8 publishes9CEF4=list, calls638D8(list,50C08),5EB58(1),
then list+38 times5EB64(68)/5E8A4(0,16).4D2DC calls62A20/6346C forselection,
handles input10000 confirm and40 cancel; choices0/1 traverse card/event
42770/42848/42B28/42A10/4298C (missingnative); choice2/cancel closemenus via
62F1C then5C1EC(0)/512AC and sounds. Decompile this actual graph, retain
nonreturn boundaries, don't call constructor completion whole-menu acceptance.
Broader scene/prologue,Day2inventory,live/raster/BIOS,random-battlefreeze remain.

## DAY1/DAY2-67: fade tick arithmetic and wait sequence (2026-09-08)

Previous turn66 restored input-query behavior (progress). This turn traced
original85/86 starts,68E24 tick and9C wait. Fixed native68E24 signed-overflow
risk: original mult/mflo wraps32-bit product before signed division; C signed
multiply could overflow. Now unsigned multiply then signed interpretation via
int64 preserves original low32 bits. Removed impossible host divide fallbacks:
original denominator=max(u16duration-1,1), hence positive. No matching edits.

New pe_fade_wait_oracle.py pins full original spans18EB4..18F0C,19410..19450,
66B60..66BD8,66C7C..66CE8,68E24..6914C; checks M0351I86(60)atF5B8 and
85(60)/9C at910C8/910D4. Executes512 ticks (all lowstates/buffers, signedcolor,
duration/timer extremes) and16 start/wait chains,1056 RAM/return checkpoints.
Native test_fade_wait.h matches generated retail_fade_wait_cases.h; normal and
sanitizer focused groups PASS(1passed1266skipped). FullCTest8/8 PASS67.33s,
1267 nativegroups. Oracle header-regeneration check/Pythoncompile/scoped
whitespace checks PASS; builds no compiler warnings/errors. Logs local/live/
{oracle-day1-67-fade-wait,native-day1-67-fade-wait,
native-asan-day1-67-fade-wait,ctest-day1-67}.log. All handles terminal.

See FADE_WAIT_CONTRACT.md: duration60 requires60 updates, duration0/1 oneupdate.
9C whileactive rewindsnext-PC8,task+10=1,return0; inactive returns1. Explicit
argument/task/emptyOThead/alternatingbuffer fixtures; not complete field
scheduler, packet presentation or wall-clock proof. StageLOCAL; launcherDAY1-41.

Next full M0351I message/resource/prologue and then M0042I/Day2 inventory.
B4/B5 already inline in VM->375B4/375C4 (659F8_port.c, CED0=1/0).
43/17DE4 reads signedchoicebyte9CEA4 via37864. E7/15AF0 appears missing from
native dispatcher (inspect before adding): original47words15AF0..15BAC.
If B0CD8&1000 return1; else task+8 bit20 clear=>setbit,delay1,rewindPC8,return0;
secondcall67CBC then4D18C, B0CD8|9000,D1A0|4,task+8clear20,return1.
This command occurs801910C0 after choice0 sets persist1=998, beforefade85.
Audit callees/nonreturns and original pointer reloads; don't substitute a stub.
Full-day/live acceptance, raster/BIOS integration and random-battle freeze open.

## DAY1/DAY2-66: restore complete opcode11 held-counter query (2026-09-08)

Previous turn65 was progress: original entry-gate and connected-chain evidence.
This turn restored missing selector3 in game/boot/func_800130B4_port.c using
full77-word130B4..131E8 (same pinned SHA as65). Missing held bits write0 with no
GTE/mask mutation. Success writes GTE data30(LZCS), updates31(LZCR); special
mask80000000 retains argument and usesindex31; others overwrite mask withLZCR,
reload pointer/value, index31-value, reload output pointer then read counter
A7770+(index<<2) with32-bit wrap. Zero/FFFFFFFF readA776C. Preserved alias order.
Added PeGteState.lzcs/lzcr and PE_GTE_SetLZCS; arithmetic helper remains pure.
Original oracle runner adds optional initial_cop_data for preservation tests.
No matching-source or proprietary asset changes. Details INPUT_QUERY_CONTRACT.md.

New pe_input_query_oracle.py and retail_input_query_cases.h hold2560 original
RAM/GTE results. Native test_input_query.h matches all and executes a VM
selector3 immediate100 -> mask23/outputcounter8/next opcode. Native and
sanitizer focused runs PASS(1group,1265skipped); fullCTest8/8 PASS66.52s,
1266native groups. Original oracle regeneration check and all entry cases PASS:
772selector,96calculation,256timer,1050gates,256gatedtimer. Pythoncompile/scoped
whitespace checks PASS; no compiler warnings/errors in either build log.
Logs local/live/{oracle-day1-66-input-query,oracle-day1-66-entry-chain,
native-day1-66-input-query,native-asan-day1-66-input-query,ctest-day1-66}.log.
All process handles terminal. Changes LOCAL; latest launcher DAY1-41.

Next continue asynchronous M0351I prologue and message/fade execution into
final selector, then M0042I and broader Day2 inventory. Original opcode table:
EA15DAC,40=17D7C,AA19618,88=18F54,08=1735C,82=18E58,03=17C54,
86=18EE0,E9=15C7C,9C19410,B4=197D0,B5=197F0,43=17DE4,85=18EB4,
E7=15AF0 (all800 prefix). Prologue sequence EA(D9),40,AA,88,08,story
comparisons/82/03,wait5,86(60),wait40,E9(453/454/456/457),EA(300,451/452),
9C,wait1 then pollF694. Decoder modes/operands accessible through entrytool.
85/9C native bodies live inside func_80017018_port.c (around395/312), so inspect
those before assuming unported. Live timers/raster/BIOS integration, transition
scratch tail, random-battle freeze and whole-day acceptance remain unresolved.

## DAY1/DAY2-65: prove shared entry input/state gates (2026-09-08)

Expanded pe_day2_entry_paths.py with original opcode11/130B4 and explicit
pressed/persist1/persist8 fixture inputs. Pinned full77-word130B4..131E8 SHA256
41ee2a1659cb0f8d1905ad71fb9f8b99408d6647c1a55127e789122ae7e25bbd.
1,050 gate cases at8018F694 reach all six exits: absent pressed100->91174;
persist1==3E6->910C8; persist8==0->91054; signed persist8<0->90BC8;
otherwise story80->F770, other stories->FB88. Addresses share801 prefix.
Checks preserve persistent values and ordered predicates. Command11 selector1
reads newly pressed8009D1F4, NOT a mailbox; corrected DAY2_ENTRY_DECOMP.md.
256 additional original-handler chains join those eleven gate commands to
D2/D3 and calculation, matching prior chains' suffix traces/components/writes.

Validation: oracle-day1-65-entry-gates.log passes772 selector,96 calculation,
256 earlier timer chains,1050 gates,256 connected gate/timer chains. Python
compile and scoped whitespace checks pass. No native source changed this
stage; DAY1-64's1265 groups/full8-of-8 CTest and sanitizer remain latest native
validation. Changes LOCAL; latest launcher DAY1-41. No live/full-day claim.

Next: finish async M0351I prologue and fade/message/remaining story handlers;
trace into M0042I and expand Day2 inventory. Newly observed native gap:
func_800130B4_port.c silently returns for selector3. Original selector3 checks
held mask, writes GTE LZCS (data30), optionally stores LZCR (data31) back over
mask, then returns A7770 hold-counter entry. Special mask80000000 usesindex31;
zero mask leadsindex-1; negative masks need signed-leading-bit semantics.
Restore exact GTE/alias side effects with original-execution/native evidence,
not the stale source comment describing COP2 FLAG. Current gate usesselector1,
so selector3 gap is independent of the gate proof. Broader timer/raster/BIOS
integration and transition scratch-tail/live acceptance remain open.

## DAY1/DAY2-64: implement script timer conversion and extend entry proof (2026-09-08)

Previous turn made verified progress on two shared entry-script regions. Found
D3/19DF4 still unported in native VM. Restored complete D2/D3 span19DB8..19F04,
83 words, as game/boot/func_80019DB8_port.c. SHA256
f684e1f70afe44e8a2195316e020111fecf4c6ad3f99e2e1e3b0ad297662effe.
Moved existing D2 read into this shared owner, added D3 declarations/CMake and
live dispatcher arm. D2 retains unchecked wrapped index*12 addressing. D3 emits
signed truncation-toward-zero components x/216000,(x%216000)/3600,(x%3600)/60,
reloading input pointer/value and output pointers between writes. Aliases must
not be collapsed into one cached input. No runtime decoder or matching-source
changes. This fixes a real VM boundary in the shared Day1 exit script.

pe_script_timer_oracle.py executes2048 original D2/D3 cases with signed limits,
threshold values and aliases. Native state hashes match; a native VM case runs
D3 then its next opcode. Expanded pe_day2_entry_paths.py executes256 original
script chains8018F770..8018FB88, including slot2 read, copies, D3 conversion
and calculation. It retains the prior772 selector/96 calculation cases and
49-command dominance proof. Only the initial timer count is a fixture input
in the new chains; preceding asynchronous gates/live time remain unproven.
See DAY2_ENTRY_DECOMP.md for ordered semantics and limitations.

Original fixture65786, normal33445, chain/oracle38692 and sanitizer69124 ended0.
Normal and ASan/UBSan each pass one actual targeted group. Full CTest65729
ended0:8/8 PASS71.16s,1265/1265 native groups. Logs:
local/live/{oracle-day1-64-script-timer,oracle-day1-64-entry-chain,
native-day1-64-script-timer,native-asan-day1-64-script-timer,ctest-day1-64}.log.
No compiler/sanitizer warnings. py_compile and scoped whitespace checks pass.

Continue M0351I asynchronous gates and M0042I entry/progression; expand both
days' mandatory/optional inventories. Day1 raster/BIOS/VSync integration and
original scratchpad/9234C outer history remain open. Full-day/live/release
acceptance and random-battle freeze remain unfinished. LOCAL; launcher DAY1-41.
Goal remains active.

## DAY1/DAY2-63: decompile closed M0351I entry paths (2026-09-08)

Previous turn made verified progress on blanking/IRQ controls. Continued the
explicit both-day decompilation scope by tracing M0351I's shared entry script.
New DAY2_ENTRY_DECOMP.md gives ordered semantic decompilation of its final
selector and Day1 calculation block, with exact addresses and remaining gates.
At801910DC, signed persist74<=80 clears persist1, writes story88 and requests
M0042I; otherwise story138 writes140 and requestsM0092I; other stories wait.
Do not describe the first condition as equality80. Earlier asynchronous scene
conditions are not assumed satisfied outside the explicit region fixture.

The49-command8018F7C8..8018FB88 calculation block computes a time-based value,
clamps it, then unconditionally overwrites persist311 with800 at8018FB78.
New pe_day2_entry_paths.py executes original ALU/branch/assignment handlers
under explicit pointer-bank binding, refusing unknown opcodes.772 selector
cases and96 calculation cases pass. A forward-only graph retaining BOTH edges
of every branch proves8018FB78 dominates exit for all possible branch outcomes.
Do not silently optimize away intermediate original writes or interpret this
as an execution proof of preceding timer/D3/resource/message/fade calls.

Original script SHA2561cc11664e59100e6378107b99ade037704e1917702182264e459c7d1172b7203;
handler-span SHA256d77f3345b511d05f01bc227f60d3ad61af0e2e6c28361447459716ff26868b9a.
Original handlers run in the test-only MIPS runner; no production runtime decoder
or source/asset edits. Oracle sessions26238,55526 and final83820 ended0. Output:
local/live/day2-entry-paths.json; logoracle-day2-entry-paths.log. Existing route
audit also reran successfully (four rooms,26 modules,3636 decoded commands,
16 immediate transfers, six story writes). py_compile passes. No native code
changed this stage, so native suite was not rerun (latest DAY1-62:1264 groups).

Next decompilation: trace M0351I's asynchronous gates and M0042I entry/advancement,
expand reachable Day2 rooms and all mandatory/optional content. Day1 timing
integration remains open: raster signal generation, BIOS handler processing,
PeVSyncClock/public VSync, original scratchpad history and9234C integration.
Full-day/live/release acceptance and random-battle freeze remain unfinished.
LOCAL; latest launcher DAY1-41. Goal remains active.

## DAY1-62: blanking edges, critical IRQ gating and BIOS controls (2026-09-08)

Previous turn made verified progress on full PutDispEnv. New GPU signal inputs
PE_GPU_SetHBlank/SetVBlank track levels; rising HBlank ticks timer1 and rising
VBlank releases its synchronization gate and asserts CPU source0. Repeated
levels/stale generations are inert. These APIs do not call callbacks. Device
reset starts levels high; separate edge counts are diagnostic. The legacy
synthetic VSyncQuery counter remains unchanged by physical signal inputs.
CPU IRQ delivery now defers at nonzero critical-section depth, preserving
pending I_STAT across masking/nested locks; queued physical edges coalesce.

Exposed real73C74/73C84 BIOS control veneers. Six source words excluding padding
have SHA25688a578c1ecfaeee8cbbae622a3423ee1d4231699ad9ad4d96959f527588a3887.
Full flag values and old-value returns are retained for counters0..3; invalid
counters stop.82534's collapsed82CF0 now makes original73C84(3,0) inside its
critical section instead of skipping it. Registration uses the same controls.
BIOS handler processing/auto-ack remains unported; the setters do not pretend
that a PAD/timer handler has run or clear I_STAT themselves.

pe_vblank_bios_oracle.py executes64 original veneer forwarding cases to the
BIOS boundary (not a ROM policy oracle). Native DAY1_blank_edges covers32
edge/mask/lock scenarios plus BIOS controls/controller reset. Checks timer gate,
duplicate levels, stale-level retention, multiple masked-edge coalescing and
nested critical sections. Normal6638 and ASan/UBSan26575 pass one actual group
each. Full CTest61687 ended0:8/8 PASS70.16s,1264/1264 native groups. Logs:
local/live/{oracle-day1-62-bios,native-day1-62-blank,native-asan-day1-62-blank,
ctest-day1-62}.log. py_compile/scoped whitespace checks pass. Hardware/BIOS
source references and limits are in VSYNC_CONTRACT.md.

Next: derive timer/IRQ blanking signals from GPU raster timing, including
fractional/interlaced phase and programmed ranges. Documentation distinguishes
video-output blanking from timer signals; do not infer exact timing solely
from a nominal3413/3406-cycle constant. Implement BIOS handler processing and
the PeVSyncClock/public VSync adapter. HostFB_VSync remains its old shim; no
live-frame acceptance is implied. Then finish actual-scratchpad frame history,
9234C integration, shared Day2 branches and both full scope inventories.
Full-day/live/release acceptance and random-battle freeze remain unfinished.
LOCAL; latest launcher DAY1-41. Goal remains active.

## DAY1-61: complete PutDispEnv and GPU display registers (2026-09-08)

Previous turn made verified progress on timer1 initialization. Before adding a
GPU scheduler, inspection found PutDispEnv still skipped all original mode/range
commands. Restored318 words755F0..75AE8 as platform/pe_dispenv.c, source SHA256
c8dc390773e463aadf9158ed73f3cb4467a6c8b2af8f853c3950512ef0577ca1.
Public755F0 now returns its environment pointer and preserves all original
ordered GP1 words, input byte18 mutation,20-byte957B8 cache, signed resolution
thresholds and NTSC/PAL range clamps.76B20's command-byte cacheA3348+opcode is
also retained. Each writer reloads the live jump table; unknown print/writer
identities stop before later effects. Canonical debug printing is implemented.
Host presentation follows successful cache publication. No runtime decoder.

GPU authority accepts GP1(05..08), retains display start/ranges/mode and maps
mode bits into GPUSTAT14,16..22. Reset restores these registers' defaults.
Diagnostic first-four-command trace/count never feeds hardware behavior.
70E54 and80190660 now stop after display nonreturn before draw/buffer flip or
later loop/cleanup. Read DISPENV_CONTRACT.md for original and hardware sources.

pe_dispenv_oracle.py executes2048 full original software cases with real getter
and GP1 writer instructions, relocated bus pointer and explicit BIOS memcpy/
printf contracts. Native checks compare environment/cache RAM and exact ordered
commands. Also checks256 display-mode payloads, reset and two nonreturn paths.
Normal and ASan/UBSan targeted groups pass; final CTest8/8 PASS66.67s with1263
native groups. Oracle32292 and native35781 ended0. Initial build62237 failed on
one stale void declaration, corrected to the original pointer-return ABI.
Two full regression attempts (5325/43617) found fixture dispatch omissions and
new original cache writes outside old allowlists; fixed specific fixture setup
and exact ranges, preserving their unrelated canaries and assertions. Avoid
adding display setup to generic clear-only fixtures (6A9E4 needs its old scope).
Final full21633 and sanitizer38936 ended0. Oracle recheck51947 exited1 only
because its final warning-search had no matches; oracle log itself passes2048.
No compiler warnings; py_compile and scoped whitespace checks pass. Logs:
local/live/{oracle-day1-61-dispenv,native-day1-61-dispenv,
native-asan-day1-61-dispenv,ctest-day1-61-final}.log.

Next: GPU scanline/field scheduling from the now-represented GP1 ranges/mode,
source0 production respecting BIOS clear policy, then PeVSyncClock/public VSync.
HostFB_VSync still uses the old shim; scanout still uses the DISPENV host window.
Other GPU reset/command behavior remains partial. Finish actual-scratchpad frame
history/9234C integration, shared Day2 branches and both full scope inventories.
Full-day/live/release acceptance and random-battle freeze remain unfinished.
LOCAL; launcher remains DAY1-41. Goal remains active.

## DAY1-60: restore timer1 initialization and edge counter (2026-09-08)

Previous turn made verified progress on VBlank callback/IRQ dispatch. Original
743B4..7440C (22 words, SHA256
 a8d76ebfddf20244c3904501babf8e4409502f625eefcd21ea740d9fac6c76f2)
writes107 through956B0=1F801114 before clearing956AC/eight slots. ResetCallback
had omitted this hardware setup. It now initializes platform/pe_timer1.c there.
This is the original installed configuration: HBlank clock, pause until first
VBlank then free-run,16-bit wrap, no timer IRQ. Reads do not advance time.
Explicit HBlank/VBlank edges validate IRQ-generation tokens; SDK reset clears
the lane. Repeated guarded ResetCallback leaves the running counter untouched.
The mode read exposes/clears target-zero andFFFF flags. Only this configuration
is implemented; no generic MMIO bus or other timer modes are claimed.

pe_vblank_init_oracle.py executes32 original initializer prefixes with the real
clear callee, stopping at source0 registration. A relocated fixture-RAM mode
pointer observes the original107 write; this proves software setup, not device
timing. Native tests verify documented hardware edge/gating/wrap/reset behavior.
See VSYNC_CONTRACT.md for hardware reference and limits. Normal and ASan/UBSan
pass one targeted group each. Full CTest8/8 PASS66.25s;1262/1262 native groups.
Build/normal2288, sanitizer60357, CTest91326 all ended0. Logs:
local/live/{oracle-day1-60-timer1,native-day1-60-timer1,native-asan-day1-60-timer1,
ctest-day1-60}.log. No build warnings; py_compile/scoped whitespace checks pass.

Next: provide real GPU edge scheduling and source0 production respecting BIOS
clear policy, then bind a PeVSyncClock adapter/public VSync. HostFB_VSync remains
the old shim. General timer MMIO/cycle delays and other modes remain missing.
Then finish actual-scratchpad frame history and9234C integration, trace shared
transition branches into Day2, and complete both days' scope inventories.
Full-day/live/release acceptance and random-battle freeze remain unfinished.
LOCAL; latest published launcher DAY1-41. Goal remains active.

## DAY1-59: VBlank game callback and CPU IRQ routing (2026-09-08)

Restored3E91C RNG/timer callback and36F7C four-record timer update. Together
with7440C dispatch,105 original words have SHA256
8b623f46317d202830636347024c7c3fb20712bc247b0e6869521c6c5a7c019f.
The initialization callback binding now runs this port. Timer flags and busy
state preserve original early exits and unsigned increment/decrement wrap.
CPU source0 now recognizes7440C through checked callback dispatch, preserving
IRQ masking, generation checks and acknowledgement before callback entry.
Unknown callback identities and callback non-return stop later slots; normal
callbacks retain original live slot reads and increment956AC before dispatch.

pe_vblank_oracle.py executes512 complete original dispatcher cases with real
RNG, timer and busy callees. Native comparisons cover state hashes; four IRQ
scenarios also cover masked/stale delivery, live slot mutation, acknowledgement,
active-state cleanup and interrupted dispatch. Normal and ASan/UBSan each pass
one actual targeted group. Full CTest8/8 PASS68.39s,1261/1261 native groups.
Oracle9182, normal52781, sanitizer33698 and CTest70206 ended0. Initial oracle
coverage assertion used delay-slot PCs omitted by the observer; corrected to
non-delay PCs before the successful run. Logs: local/live/{oracle-day1-59-vblank,
native-day1-59-vblank,native-asan-day1-59-vblank,ctest-day1-59}.log.

This connects IRQ dispatch, not a host VBlank producer. Timer1 MMIO and the
PeVSyncClock host adapter remain missing; HostFB_VSync still uses its old shim.
Next restore those clock/device semantics and BIOS clear policy, then complete
actual-scratchpad frame history and9234C integration. Full Day1/Day2 inventory,
live/release acceptance and random-battle freeze remain unfinished. LOCAL;
latest published launcher remains DAY1-41.

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
and at DAY1-58 source0/7440C still reached an indirect boundary (resolved by
DAY1-59 above). Next restore timer1 and the VBlank producer with existing IRQ
masks/generation/BIOS policy, implement the clock adapter, and bind
public VSync. Then finish actual-scratchpad frame history and9234C integration.
Do not equate the shim's invocation count or GPU frame counter with956AC/timer1.
Full Day1/Day2, live/release acceptance and random-battle freeze remain unfinished.
LOCAL; launcher41.

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

## DAY1-43: whole transition camera setup (2026-09-08)

Previous goal turn was progress: original basis/normalizer port and validation.
Restored whole8018F05C..8018F344 (186 words), SHA256
1602c7761c55ad4056272d59a2a2ef2bdd55f0ffe793126428f7e9f178da5435.
New func_8018F05C_port.c also restores public SDK787D4..78934 matrix
concatenation (88 words), SHA256
484fff1d2984fac9730786048ad50065c006672118c2d6b4698dfb787d7b155c,
and799E4..79C70 Euler rotation (163 words), SHA256
eb29f1f088bca83efc058df957cfe2cd49bcfbd3a215d41df7986888d2b9bba3.
Full view sequence uses original template, wrapped deltas, normalized angles,
concatenation,8F344 basis, final alternate angles/matrix and GTE setup.
Scratch1F800280..2FF saved/restored even at stop boundaries. Concatenation
preserves full-word padding write, alias-sensitive store/read ordering,
signed16 translation inputs and trapping ADD before all translation stores.

pe_transition_view_oracle.py pins all original source slices, runs all real
callees without providers, and reads18 terminal GTE control/data words.
524 cases:300 whole views,124 concatenations,100 rotations. 28 original signed
ADD overflow prefixes compare RAM and explicit native stop (not terminal GTE).
Includes degenerate camera vectors, negative angles, matrix/input overlaps,
and deliberate overflow of each concatenated translation component.
First native comparison found test fixture IR0 initialized4096 vs oracle0 for
isolated SDK calls; corrected explicit GTE initialization. No product patch
was needed for that mismatch. Expanded normal targeted test PASS1 actual group.
Normal and ASan/UBSan targeted tests PASS1 actual group each. Full CTest8/8
PASS75.47s,1245 native groups; all build/test processes ended0. Logs local/live/{oracle-day1-43-view,
native-day1-43-view,ctest-day1-43,native-asan-day1-43-view}.log.
LOCAL only, launcher remains41. No live full transition acceptance yet.

Next missing transition graph: whole8F92C..8FFF4 (projection/dimensions) calls
78A94/78B38 matrix stack,78E94/78E04 setters,792D4 four projections,
791D0 four transforms and78004 four lengths. Saved original-8018F92C-region.txt
contains this routine. Then942FC,92800/93478 and outer9234C remain unported.
The unidentified user random battle freeze stays open; do not equate camera
arithmetic verification with a reproduction or a fix for it.

## DAY1-42: original camera basis and normalization (2026-09-08)

Previous goal turn was progress: published verified DAY1-41 fixes to both
launcher platforms. This turn resumes missing transition dependencies while
the user's unidentified random-battle freeze remains unresolved.
Restored whole8018F344..8018F55C (134 words), SHA256
f0c1895a53bcbf8fff5a09082fec793408ac4a1aaa4ff245ca49e7fea451b270,
and its SDK78134/78194 normalization graph. Original78134..78254 SHA256
e0e8ffd672aa993f534877c6d939d3c837fc43b0ffa8d71f6f5a53cb717ebf10.
The SDK graph uses SQR, trapping signed ADD, original inverse-root table,
GPF and arithmetic variable shifts; zero vectors are not special-cased.
Camera basis preserves the forward.z==up.z increment, two OP cross products,
three normalizations, halfword matrix writes/padding and final negative
rotated-eye translation. Source func_8018F344_port.c, public declarations
and normal runtime linkage. No partial outer frame wrapper was introduced.

pe_transition_basis_oracle.py executes untouched original code and all real
callees, then a test-only return reader captures18 GTE control/data words.
587 cases: 267 normalizer input/output aliases and edge values,320 whole
basis calls (including matrix aliases of eye/target/up and degenerate vectors).
34 cases stop at original signed ADD overflow; compare persistent RAM prefix
and native explicit stop, not terminal GTE state for these trapped cases.
Normal and ASan/UBSan targeted tests PASS1 actual group each. Full CTest8/8
PASS66.43s,1244 native groups; all build/test handles ended0. Logs oracle-day1-42-basis.log,
native-day1-42-basis.log,ctest-day1-42.log,native-asan-day1-42-basis.log.
DAY1-42 remains LOCAL, launcher41. No full transition/live route acceptance.
Next: whole8018F05C needs this now-present8F344/78134 plus missing public
SDK787D4 matrix concatenation and799E4 rotation graph. Existing794C4,
78004,79FB4,78E94/78E04 are available. Then8F92C,942FC,92800/93478 and
outer9234C remain. Do not treat isolated verified helpers as fullDay1 parity.

## Latest launcher publication: DAY1-41 (2026-09-08)

Published PE-DAY1-41-01d9ff4d63a8 for Linux and Windows, replacing DAY1-37.
This ships local cuts38–41, including the verified NTSC vertical offset/border
fix and all missing Windows controller mappings. The user random-battle freeze
is NOT reproduced or fixed by evidence; keep that investigation open.
Linux Release build and full CTest8/8 PASS56.08s (1243/1243 native groups).
Windows Release build, Wine14-button test and120-frame startup previously PASS;
Wine is not native Windows OS validation. Extracted package metadata, binaries,
disc hashes/cues and Linux packaged startup all PASS. Private and launcher
publishers exited0. Final default-channel GET and full public archive downloads
for BOTH platforms matched size, SHA256 and embedded build metadata; exit0.
Build metadata remains sourceCommit7821e7c3/sourceDirty true, no blanket commit.
Linux592482453 bytes SHA256
 a1a26a060794a6a3f82b9d547364ed7a1ab46455ff022d2df673f23200523335.
Windows592524981 bytes SHA256
 b235b708b75d59df6ebf8e0d99ffa987b5823a612233722110c290f43497ac95.
Evidence local/live/{ctest-release-day1-41,package-verification-day1-41,
publish-day1-41-private,publish-day1-41-launcher,public-verification-day1-41}.log
and local/live/publish-day1-41/launcher-verification.json. Previous launcher
channel saved there. Signed URLs stay in private files; never print them.
All publication/build/package helpers ended0. Game run42 remains separate;
it returned to opening dialogue after defeat. Full Day1 objective unfinished.
Earlier LOCAL/launcher37 statements below are historical, superseded here.

## Launcher publication PUBLISHED (user request 2026-09-07)

Compiled and published PE-DAY1-37-40d88b0bf8ad for Linux and Windows to the
launcher default channel, replacing PE-DAY1-10-b36db6858245. This is an interim
release through DAY1-37; the full Day1 accuracy objective remains unfinished.
Linux Release CTest 6/6 PASS (41.92s, 1242 native groups). Windows dockcross
Release and Wine startup 120 frames PASS (not native Windows OS proof).
Both extracted packages passed binary/disc/cue/metadata verification; the
Linux packaged wrapper passed startup. Private publication passed full remote
checksums, ranged GETs and channel readbacks. Launcher publisher exited 0.
Final public verification fetched the default channel WITHOUT a cache query,
downloaded both complete public archives and matched size, SHA256 and embedded
build metadata. Both platforms PASS, verification process exited 0.

Evidence: local/live/public-verification-day1-37.log,
local/live/publish-day1-37/launcher-verification.json,
local/live/package-verification-day1-37.log,
local/live/publish-day1-37-launcher.log and publish-day1-37-private.log.
Linux archive: 592482293 bytes, SHA256
bf351ffbced903def6089adada1ce60b5eceef31569f1ea299dd348f625791b2.
Windows archive: 592525149 bytes, SHA256
0ac6a0e6d12a1a1aeff18b7eac12a067fae933ee118feedc96abf4df7c573999.
Metadata source commit 7821e7c3, sourceDirty true. Signed private URLs remain
in private stage files; never print them. Launcher docs/channels/parasite-eve.json
was updated by its publisher. Run41 was not modified by publication; its
previously unnoticed host-quit is corrected in the DAY1-38 section below.

## CURRENT PRIORITY: random battle freeze; display offset fixed locally

User reported during DAY1-39: "random battle poped and it locked up anothert
hing there is a small black line at the bottom so i'm not sure if the image
is up a bit or not". Asked room, platform and whether launcher DAY1-37, plus
screenshot. Still no reply. Do not repeat the questions or claim the reported
freeze reproduced. No game from user's session is visible on this shared host.

DAY1-40 found and corrected a REAL display mismatch: live DISPENV source
(0,224,320,224), screen(0,8,0,224). HostFB_PresentDispEnv ignored screen.y/h,
copying224 rows to top of240 framebuffer. Local screenshot sewers42-current.png
and raw run42 frames show nonblack0..223, all16 border rows at bottom.
Original PutDispEnv75858..75880/75A14..75A60 confirms NTSC screen.y offset
from scanline16, h0 defaults240, and original start/end clipping. Host scanout
now follows those vertical ranges and blanks outside them each presentation.
No stretch/crop or hardcoded8 offset. Horizontal timing, PAL/interlace and
full GP1 implementation remain separate from this NTSC vertical correction.
Hardware background source: https://psx-spx.consoledev.net/graphicsprocessingunitgpu/
(vertical range is placement/visible line count, not scaling).

pe_display_range_oracle.py executes original instruction slices for88 cases
(y offsets, zero/default heights, negative/large clipping). Native and ASan
DAY1_display_vertical_range PASS. Full CTest8/8 PASS74.23s,1243 actual native
groups. Initial build failed on a preexisting table-name collision; renamed
new table DAY1_vertical_range_cases and rebuilt successfully. An attempted
pipe-separated test filter matched0 tests; rejected, reran single exact filter
and fullCTest. Evidence local/live/{oracle-day1-40-display,
native-day1-40-display-range,native-asan-day1-40-display,ctest-day1-40}.log.

A separate new-binary live run display40 verified the actual corrected image:
display40-final.png, framebuffer nonblack8..231, black8 rows top and bottom.
Original image was8 logical pixels too high. Display40 and its opening helper
finished exit0; game stop_reason frame-limit at1800. Do not restart it.
DAY1-40 remains LOCAL, launcher stillDAY1-37. No random-freeze fix claimed.

## DAY1-41: Windows controller completeness (2026-09-08)

Previous turn was progress: confirmed/fixed vertical scanout and startedrun42.
While investigating freeze, found Win32 vk_to_pad_bit only mapped Cross/arrows.
Added the missing9 buttons to match Linux: C Circle,V Triangle,S Square,Q/E
L1/R1,1/3 L2/R2,Tab Select,P Start. No assertion that this caused user's battle
freeze; platform/room still unknown. This is a concrete Windows parity fix.

Added Windows-only pe-win32-input-tests/test_win32_input.c, registered as
CTestwin32-input. It creates the real backend window, posts WM_KEYDOWN/UP and
SYSKEY messages, polls, checks active-low output for all14 buttons, repeats,
movement/menu combinations and focus reset. Dockcross Release game+test build
PASS. Wine test PASS exit0, and actual Windows game startup120frames PASS,
stop_reasonframe-limit/exit0. Native Windows OS validation remains unproven.
Logs local/live/build-win-day1-41.log,wine-day1-41-input.log,
wine-day1-41-startup.log,config-win-day1-41-tests.log. ctest-N confirms Win32
test registration. Linux production code unchanged by this cut; priorDAY1-40
full8/8 CTest and sanitizer evidence remains applicable. No blanket reruns.
Windows binary now includesDAY1-40 display correction too. Launcher remains37.

## LIVE RUN42 — active battle investigation (DAY1-39 binary)

GDB PID504788, exec82826 remains ACTIVE, prefixlocal/live/sewers42,
titleParasite Eve - sewers42,DISPLAY:10.0. Every10 frames RAM/RGB/JSON,VRAMoff.
Do not restart to swap binaries: this live run predatesDAY1-40/41 changes.
First helper78530 completed0 atstage_exit19850. Medicine healed19->45 at20100,
backstage20880,understage21230/21880,corridor22150. Scripted corridor rat
started around23640, defeated24080, fieldcontrol24230,g74=72,HP41/45. Captured
sewers42-rat-entry.png. No stop/boundary; this scripted encounter is NOT the
unidentified random battle from user's report, so freeze remains unresolved.

Route helper67859 ended1 at a waypoint timeout near the rehearsal door,
without a game stop. Ordinary-input recovery helper43198 unlocked door44940
and entered rehearsal45060 (HP41, PE80). Battle helper13164 then ended1:
encounter49750, battle50590, HP41->36->14; it attempted Heal1 at AT700 and
waited for a menu that requires AT9000 (299CC_ready_input gate). This was
an automation strategy error, not a demonstrated menu freeze. No helper remains.
Aya died; frame52560 modeFFFFFFFF/HP reset45 is defeat, NOT victory. By59710,
story_g74=1 and the actual screenshot sewers42-after-defeat.png shows the
opening street dialogue again. Game is running; no stop request or crash logged.
Preserved sewers42-rehearsal.log and sewers42-battle-current.png. The scripted
rat battle passed, but the user's unidentified random encounter is unresolved.
All traversal/menu actions used ordinary keys only. Full Day1 acceptance remains
incomplete. DAY1-41 publication completed (see latest publication above); do not claim
the reported random-battle freeze fixed.

## DAY1-39: whole transition exit and A9 audio fade (2026-09-07)

Previous turn was progress: whole constructor port/verification. Restored
92030..9234C in func_80192030_port.c, including every destination branch,
30-VSync fade, display cleanup, signed audio handle, late flag reads and latch
clear. SHA256940c9406cf13145b82ea57eaa96e44b1ad5707b4944c4985dd861c75359b3011.
Added real868AC command issuer and38D48 latch helper. pe_transition_exit_oracle.py
executes original control flow/helpers with explicit graphics/audio provider
contracts; 3840 whole cases plus304 stop prefixes match normal and ASan/UBSan.
Trace fingerprints include state at every provider call and injected flag
mutation during fade; covers story threshold neighbors, all10 routes plus
invalid selectors, signed handles and cleanup latch variants.

Found exit's A9 consumer was absent (callback8008B978). Added original fade
setup for active effect voices excluding group bit02000000, signed16 delta /
signed16 duration, zero full duration ->1, and original zero-divisor stop prefix.
Original49words SHA25673962a5103b98e1198bf3156c27802ec9e5826ae6e02a86694f6c3a1f53116d0.
Expanded pe_audio_commands_oracle.py to481 complete original consumer graphs
(99 A9 cases), plus2 A9 DIV prefixes. Normal+ASan ATK19_retail_audio_commands
PASS (1 actual group,1241 intentionally filtered). Audible synthesis/tick
parity is still separate unfinished work; do not equate RAM ramps with audio.

Normal full build and CTest8/8 PASS78.61s,1242 native groups. All processes
terminal0. Logs local/live/{oracle-day1-39-exit-final,native-day1-39-exit,
native-asan-day1-39-exit,oracle-day1-39-audio-final,native-day1-39-audio,
native-asan-day1-39-audio,ctest-day1-39}.log. DAY1-39 remains LOCAL; launcher37.

After addressing user freeze, next transition dependencies: 8F05C..8F344 calls
8F344 plus SDK matrix/trig;8F92C first return ends8FFF4, SDK matrix/projection;
942FC first return ends958D4 and calls958D4,3746C,375E0,38940,6DF50,6EC6C,
868AC,86C5C,F55C,95994,95BC8,95D3C. Region disassemblies saved as
local/live/original-8018F05C-region.txt,original-8018F92C-region.txt,
original-801942FC-region.txt; regions include subsequent functions, don't treat
entire region as one routine. Frame9234C and rendering92800/93478 still missing.

## DAY1-38: whole transition constructor (2026-09-07)

Previous goal turn was progress: compiled and published DAY1-37 with full
public archive verification. This turn restored the WHOLE original96498
constructor in pc_port/game/boot/func_80196498_port.c, declared and linked in
the normal runtime. Original1474 words, SHA256
c946ea3080c7e550f1ce346ee26e40cb8006fcbcd4f288d31ef9202cbb10562e.
Includes graphics/window setup, ten partially initialized records, paired
objects, thirty fixed objects, four additional pairs' members, nine variable
placement lists, camera setup, ten sampled positions and every entry-mode tail.
The generated record table preserves140 untouched bytes; source hash now pinned.
Temporary sampler outputs use saved/restored scratch1F800260..277.

pe_transition_constructor_oracle.py executes the full original with REAL
pool/packet/resource/path/camera callees. Scene/display initialization, window
setup371B0 and five GTE setters use explicit provider contracts (not real
presentation evidence). 720 cases cover all ten camera variants, four entry
modes, three resume values, dirty state, four flag patterns, window-provider
mutation and three placement profiles. Original used-list validation finds
55/121/199 objects, no duplicated slots. Persistent writes outside fingerprint
ranges are rejected. Six additional original stop prefixes cover first scene
provider, first/second window providers and zero-period camera/sampler paths.

Normal and ASan/UBSan tests PASS720 cases plus6 prefixes. Evidence:
local/live/oracle-day1-38-constructor-full.log,
oracle-day1-38-constructor-faults.log,
native-day1-38-constructor-final.log,
native-asan-day1-38-constructor-final.log.
Whole normal build PASS; CTest7/7 PASS105.91s,1242 native groups. CTest included
initial288 constructor cases; expanded720 plus6 were rebuilt/retested afterward
in both normal and sanitized binaries, with no subsequent production edits.
Initial new-target build before CMake regeneration failed with no such target;
regenerated and completed actual builds. No skipped/stale test evidence used.
DAY1-38 is local, not published; launcher remains verified DAY1-37.

NEXT: full transition frame loop9234C, still a bootstrap boundary. Exact frame
routine is9234C..92740 (253 words), SHA256
caa53e35c6e87356c8743445c2bac01cc2b77d97946ea64eb717f077bd3fc0e8.
local/live/original-9234C-frame.txt is its disassembly. The earlier
original-9234C-full.txt also includes92740 and92800 and is NOT one function.
Unported direct update/render/cleanup callees include8F05C,8F92C,92030,92800,
93478,942FC. Existing92740/94108/93AB0/91E30/91EFC/91DE8/BF8C are ported.
92800 in turn calls90E04. Port the missing real behaviors before exposing a
partial frame loop. Full Day1, visible transition and native Windows OS
validation remain incomplete.

Run41 is TERMINAL, superseding older still-live notes below. Authoritative
exec9160 returned exit0, GDB102220 is absent, sewers41-exit.txt says22:52:30,
and sewers41-gdb.log reports stop_reason=host-quit, inferior exited normally.
Stop frame151332, Aya(67,0,255),HP45/45,g74=56,tokenA80010C8. Backtrace shows
PE_PORT_STOP_HOST_QUIT, not an unresolved implementation boundary. No restart
or guest-state edits performed. Preserve captured snapshots for route context.

## DAY1-37: direct camera target setup; live girl scene (2026-09-07)

Previous turn was progress (camera path setup/start/advance). Restored
95BC8..95D3C (93words,SHA256
54414529b327f805ed554a27b70d8554e90762dacb7f348973b471cd6a1afbcd)
in func_80195994_port.c. Explicitly retain original interleaved reads and
writes for target/eye SVECTORs, 32-bit current positions, delta/accumulator
records, from/to halfwords and masked shift/count outputs. Inputs may alias
the records being written; copying inputs up front would be incorrect.
pe_camera_target_oracle.py executes original leaf,4624 history steps over
17x17 input pointer combinations, unaligned inputs, all output/source
records, eight shift pairs and repeated calls with dirty state.
Normal and ASan/UBSan target test PASS. Initial normal build terminated143
before relinking; both attempted stale-binary runs skipped all tests and
are NOT evidence. Waited for ASan completion, rebuilt normal target with-j2,
then verified the new test PASS in both binaries. Logsbuild-day1-37-retry.log,
{oracle,native,native-asan}-day1-37-target.log. Full CTest6/6 PASS89.31s (1242 native groups),ctest-day1-37.log.

Next target is the WHOLE1474-word96498 constructor: all its direct callees
are now ported (91854,90998,95F6C,GTE,6EC6C,371B0,poolAEC/B78/C1C,95994,F55C).
Raw local/live/original-96498-full.txt. 96620..96EAC builds10 records of52
bytes atEA378, constants plusA77FC bit selections. 96EAC..97118 iterates10
package positions, allocates paired objects viaC1C intoE4DB0/E4DD8, copies
position shorts to object+20/+24/+28, yaw+2E, second pitch+2C=800. Continue
full decompilation through97BA0 rather than substituting an initialization
prefix. 9234C frame loop remains separate unresolved work.

Run41 still live on DAY1-32 (GDB102220/exec9160). Two ordinary Return taps
advanced the young-girl dialogue (screensgirl-next.png: "That girl...!")
and reachedg74=56 at133500, tokenA80010C8, Aya(67,0,125),control8,HP21/45.
A240ms Up tap moved her toZ180 at138040. RGB continues showing prior
backstage backdrop (room-current.png/room-moved.png); this may be a scene
or rendering issue and is NOT visually accepted. No live abort or stop.
V opened the field menu (menu-check.png), then Return entered Use Item.
An initial empty-slot confirmation entered item-move mode and moved
Medicine from slot4 to6. Canceled/reopened Use Item with feedback helper,
used Medicine1 via popup:HP21->45; all menu windows closed148320.
Screensmedicine-use-selected.png/medicine-healed.png. No input helper active. Police supplies,
optional rooms and rehearsal victory remain unverified.

## DAY1-36: camera path setup/start/advance (2026-09-07)

Previous goal turn was progress (F55C sampler). Restored95994 (141words,
SHA256 9fa29ecbb36774667554d798102011046dae04f76180c4c30ffe3446c0bdcab7),
95D3C (68words,0b3f22c796fb92edf367a85e2d9a63a17a68aad1ec9bd0c027c5a7b770b240bc),
95E4C (72words,436f4b9066142b100822f3f363ec3a2e794db7246cc906aeb8806cf441108541)
in func_80195994_port.c. 95994 samples two IDs fromEA378+signedvariant*52,
builds eye/target deltas and from/to SVECTORs, clears step accumulators,
publishes byte-masked shift counts and MIPS-masked shift halfwords.
95E4C starts paired paths, sets remaining/index/id bytes and clears active
interpolation counters;95D3C decrements/advances only while remaining>=2,
wraps byte index and samples id+1/id again. Saved scratchpad1F800240..257
holds temporary outputs without native pointers or persistent scratch writes.
Stop epochs preserve completed first-stage updates if second sample fails.

pe_transition_camera_oracle.py pins original bodies and executes original
6EC6C/F55C/79FB4 without mocked providers. 2048 history steps: signed/high
variant IDs, integer/fractional/high-bit time, path start/advance/exhaustion,
negative signed start IDs becoming byte IDs on advance, dirty state,
shift0/1/15/16/31/32/255/10001. Six zero-period failure prefixes (first or
second sample of each function) compare persistent pre-DIV state; original
interpreter stops before zero-divisor DIV, does not emulate BREAK exception.
Native + ASan/UBSan two groups PASS, also asserting scratch restoration.
Full CTest6/6 PASS89.53s (1241 native groups); logs local/live/{oracle,native,native-asan}-day1-36-camera.log,
ctest-day1-36.log. Outer96498/9234C remain incomplete, so no visible
transition acceptance or full Day1 completion is claimed.

Next:95BC8 direct camera target setup remains unported (93words; same raw
fileoriginal-80195994.txt), requires exact read/write alias ordering.
96498 constant/object construction and frame-loop dependency closure remain
in original-96498-full.txt. New95994 is its camera-selection dependency.

Run41 still live, GDB102220/exec9160 on DAY1-32. Inner aisle helper55087
completed at111420, Aya(1782,-173,4631), auditoriumA8000248. Longer helper
37662 made further progress then completed at119640, Aya(1797,-173,5176);
this supersedes earlier speculation that every timeout was collision.
At120120 Aya(1792,-173,5176), stable auditorium. Screenshotupper-aisle.png.
Current sewers41-aisle-offset.py (exec88108, PID265867) tries(2100,5200),(2100,7400) with180s per
waypoint and10s stall timeout, ordinary X11 only. Inspect process/log before
new input. Helper completed at126880 (auditorium flags80,2390,-173,2823);
by127410 game was backstageA80004C8, Aya(-866,-328,-2542),flags200,
HP21/45,g74=48. Scene transfer followed the offset route; inspect current
scene/dialogue before more input. Screenshotsewers41-offset-result.png
shows backstage young-girl encounter: Aya says it is dangerous here and
she should go. This is a story scene, not a gameplay crash.
No helper remains active.
Optional police supplies/rooms/rehearsal victory remain open.

## DAY1-35: original transition path sampler (2026-09-07)

Previous goal turn was progress (91854/91C94). Restored8018F55C..8018F92C
(244words, SHA256 6ac9986399afc33fe8e6f847b18740745eb61dad3ce8d70ca91de52ea741830f)
in func_8018F55C_port.c. Reads relative-offset path record, uses signed
count-2 with logical time>>8 and fractional low byte, samples three
cyclic SVECTORs, computes two original79FB4 headings, resolves wrap,
interpolates heading/position and clamps/deadzones turn angle. Preserve
sticky BFCC flag when integer time exceeds period and output alias order.
Zero period preserves original pre-DIV write then explicitly stops (the
original follows zero-divisor DIV with BREAK7); no fabricated position.

pe_transition_path_oracle.py executes original sampler AND original79FB4,
no provider substitutions. 1440 cases cover count0/1/3/4/7/-3, fractional
and bit31-set times, dirty sticky flag, all four record alignments, signed
coordinate extremes and overlapping source/position/rotation outputs.
Native uses original2050-byte atan table. Also inspect original zero-divisor
prefix (interpreter stops before DIV; it does not emulate BREAK exception).
Normal and ASan/UBSan two targeted groups PASS. Full CTest6/6 PASS72.80s;
1239 native groups. Logs local/live/{oracle,native,native-asan}-day1-35-path.log,
ctest-day1-35.log. No full transition rendering acceptance claimed.

Outer96498 is1474words through97BA0. Disassembly/call graph saved in
local/live/original-96498-full.txt; no implementation yet. It calls new
91854, pool90998, gradient95F6C, GTE, then builds many original constants,
objects, package paths and camera state. Next direct missing dependency
95994..95BC8 is141words: sample two path IDs from EA378+signedvariant*52,
using package6EC6C(D0260,2); build two deltas from C810/C330 into C08C/C05C,
zero C09C/C06C, publish byte-masked shift counts and 1<<(count&31) halfwords,
and save from/to SVECTORs. Raw disassembly local/live/original-80195994.txt
also includes95BC8/95D3C/95E4C leaves. Scratch outputs must remain guest
addresses with saved/restored scratch, or refactor sampler around native
value outputs while preserving alias semantics. 9234C frame loop (253words)
also inspected but not restored; dependencies942FC,F05C,F92C,92800,93478,
92030 still need porting. Do not substitute provider tests for full behavior.

Run41 GDB102220/exec9160 remains live on DAY1-32. First return helper
68960 stopped with assertion at stage (90230). Helper3693 also stopped:
actorflags briefly0 is NOT sufficient evidence of a scene transfer.
Successful helpersewers41-stairs-cross.py (exec1723, completed) walked
(2250,1800), stopped only on map token change; auditoriumA8000248 control
at102740, Aya(2631,-173,3095). Helpersewers41-auditorium-aisle.py (exec3861) completed/stalled at105960,
Aya(2275,-173,3289), still auditorium. Target(2630,7400) hit seat geometry;
next movement must route around the seats. No input helper remains active.
Optional police supplies/rooms and rehearsal victory remain open.

## DAY1-34: transition initializer and package read control flow (2026-09-07)

Previous turn was progress. Restored original91854..91C94 (272words,
SHA256 0cf01123a041bee57cb1aa131dd7ba72d07cce3d1377d234ca5f331b9cba4da8)
in func_80191854_port.c and91C94..91DE8 (85words,
SHA256 3a5583187fcbf16da1ba8c270731bb69a8e87c07601d7633ce3fc15cc0728f2b)
in func_80191DE8_port.c. Initializer calls display setup/query/byte setter,
selects scene variant and encounter overrides while preserving untouched
fields, derives package flag fromA77FC bit2000, then reads packages.
First read table93170 ->D0260 (41sectors with original table); after
completion select table93174 (100sectors,flag0) or93172 (89,flag!=0)
->CE10. Issue/poll -1 retries current span; other nonzero poll values
keep polling. Reread table/base each issue, retain second-span selection
through retries. Explicit provider stops unwind instead of spinning.

pe_transition_packages_oracle.py:64 original cases /832 provider calls,
including dirty low-byte flags, issue/poll retry, busy negative status,
wrapping LBAs, reversed spans and provider mutations of table/base/flag.
pe_transition_init_oracle.py:2304 original cases, every listed scene and
encounter plus unknown/full-word IDs, independent flags and dirty state;
compare provider sequence/arguments and state at each call and return.
These are explicit CD/display/query/setter provider contracts, not real
I/O or live transition acceptance. Native and ASan/UBSan PASS; also18
package and4 initializer stop prefixes. Logs local/live/{oracle,native,
native-asan}-day1-34-{packages,init}.log. Full CTest6/6 PASS (77.11s;1237 native groups), logctest-day1-34.log.

Next missing outer transition routines:96498/9234C and their dependency
closure; new91854/91C94 are not yet called by a restored outer loop.
No audio/movie/full-Day1/platform completion claimed.

Run41 still live on DAY1-32, GDB102220/exec9160, unchanged guest state
except ordinary X11 controls. Earlier sewers41-backtrack.py completed at
49840 back in backstage4C8. Completed sewers41-backtrack-center.py (exec26485): returned stage2C8, moved to(0,800) at80550 then(0,1400) at81050;
last waypoint(2250,1400) stalled at(1624,1409), frame82980. Active
sewers41-stair-base.py (exec45200) tries(1600,1800),(2250,1800),(2250,1400). Check
process/log before sending more input. Stair-base approach triggered descent at85470 (Aya1604,1576); auditorium
A8000248/control reached85880 (2631,-173,3135). Final helper waypoint immediately crossed the stairs back to stage2C8
at86670, Aya(1675,-1111,1405). Helper completed; next backtrack should
stop after stage->auditorium transfer, then use auditorium coordinates.
Police supplies/optional rooms/rehearsal victory remain unverified. Current screenshots sewers41-current.png (backstage)
and sewers41-center-stage.png (front stage); logsewers41-backtrack-center.log.

## DAY1-33: transition display setup and bank reset (2026-09-07)

Previous goal turn was progress (production SDK clear + tests + run41).
Restored original9BD78 (118words, SHA256
78b8ff859d27ecee020561c6e4b0a4ef225105efc6a5c97d8e7755f2cb01a303)
and9BF50 (15words, SHA256
60445ee22d7d361772860fe5edca340fee38d56a433bd389229df620495353ff)
in func_80191DE8_port.c. Initialize both draw/disp banks, reset packet
cursors viaBF8C, publish/clear both4096-word OTs, set GTE light/fog and
projection, clear both320x240 VRAM buffers, wait, enable display and
select first bank. BF50 clears the currently selected OT then rereads
and resets its cursor. Preserve explicit stops at unresolved providers.

pe_transition_display_oracle.py executes original overlay + original
pure SDK constructors/GTE setters. 74D28/752AC/74F44/74DC0 are explicit
hardware providers; OTC links modeled, other provider calls checked.
80 history steps compare overlay environments/current pointer, all three
OTs and their tail across dirty patterns, swapped OT globals, wrapped
arena cursor, three bank identities and reinitialization. Native tests
also compare every VRAM pixel, actual clear counts/display mask/no
premature presentation and GTE controls (against prior original GTE
readback cases), plus stop-prefix behavior. Native and ASan/UBSan PASS.
Final CTest5/5 PASS (56.98s;1237 groups). Logs local/live/{oracle-day1-33-display,
native-day1-33-display,native-asan-day1-33-display,ctest-day1-33}.log.
This is setup/provider evidence, not a live transition-frame acceptance.

Next initializer dependency: original91854..91C94 (calls9BD78,5BCB0,
371A4, then91C94). Original91C94..91DE8 calls6E6A8/6E7E8 three times.
96498 calls91854,90998,95F6C, sets GTE matrices/colors/fog, computes
texture/CLUT words91650/91652 for two records, then builds package
objects from801D0260; larger remainder still needs decompilation.
No96498/91854/91C94/9234C implementation was claimed in this cut.

Run41 remains live on DAY1-32 (GDB102220/exec9160). First Eve exited18470,
control19630, stage_exit21650. Route helper36459 completed. Started only
backstage phase (exec76348, completed): hole event25800. Ordinary input
advanced "backup arrived" then chose "Forget it" instead of jumping down;
Aya regained control. Investigating optional police healing/ammo before
spending the starting Medicine, then all dressing-room items before next
rehearsal attempt. Guide is a route hint, not retail proof:
https://shrines.rpgclassics.com/psx/pe/day1.shtml .

Read-only script polygon scan located backstage->stage exit at80192D8C:
x1200..1361,z-3485..-1697. Walked there, returning to stageA80002C8 at
frame38100, Aya(1744,-1111,794). Direct walk toward positive-X stairs accidentally crossed the wing
exit first, returning to backstage4C8 at44420, Aya(937,-328,-2721).
Helper78335 completed. New active sewers41-backtrack.py (exec77138)
returns via x1300,z-2720, waits for stage control, walks to(1500,1400)
BEFORE crossing to(2250,1400). Stairs polygon801B1814 spans
x2058..2452,z1229..1597 and transfers toauditoriumA8000248.
Check active helper before sending more input. walk_to may returnFalse on a successful map
transition; inspect actual token/control instead of treating it as failure.
No new optional-item/healing or rehearsal-victory acceptance yet.

## DAY1-32: SDK ClearImage connected; run41 ACTIVE (2026-09-07)

Previous turn was progress (original clear worker, GPU status, tests).
SDK74F44 is now a real function in pe_libgpu.c, replacing the host-only
inline shim. It validates via74E28(name118BC), loads jtb[3]/jtb[2], packs
RGB bytes and returns the queue dispatcher result. Dirty dispatch is an
explicit boundary. Shared clear_worker accepts either guest RECT or
mutable transient words. PE_DispatchClearRect/76C34 preserve direct
caller width/height clamps; queued requests retain only copied guest
payload and leave the caller RECT unchanged. No invented guest scratch
address or retained native pointer. HostFB_ClearImage remains a host API;
SDK clear writes VRAM and does not itself present to the host framebuffer.

New pe_clear_sdk_oracle.py executes original74F44+74E28 at debug0,
stopping only at dispatcher provider: seven argument/return cases PASS.
Existing180 original worker packet cases still PASS. Tests cover SDK
direct/queued lifetimes, source RECT mutation before DrawSync, dirty
jtb boundary, byte truncation and actual VRAM extents. Boot/streaming
fixtures now seed the actual static libgpu data; full-RAM canaries include
the exact clear packet and dispatch writes. RGB(0,0,1) now quantizes to
VRAM black, instead of producing host RGB(0,0,1). Tests also retain the
uncleared column1023 and verify no premature presentation.
Normal and ASan/UBSan: six clear groups +five streaming groups PASS.
Final CTest5/5 PASS (55.19s;1235 groups). Logs local/live/{oracle-day1-32-sdk,
native-day1-32-clear,native-asan-day1-32-clear,native-day1-32-stream,
native-asan-day1-32-stream,ctest-day1-32}.log. General GP1 info latch is
still absent (worker info3/4/5 are explicit platform contracts); transition
9BD78/91854/91C94/96498/9234C remain to be completed.

Run40 (DAY1-30) route helper stopped at a corridor waypoint with the game
still live, Aya in control at(0,-5395). Resumed ordinary inputs with
sewers40-resume-diary.py, using corridor x0 to diary door. Rehearsal key
45610, room49430, PE-healed41->45 at49590. Rehearsal helper then lost:
HP45->34->23->12 and defeat; no recurrence of the recorded contact abort.
It stopped inputs, and game returned to the opening scene (snapshot
sewers40-after-defeat.png). This is NOT rehearsal victory acceptance.
Manual window close after defeat caused X11 BadDrawable exit1; retained
sewers40-gdb.log/exit.txt. Do not mistake that close for a gameplay crash.
No input helpers from run40 remain active. Battle strategy/resources need
improvement; do not change retail health/damage to make replay pass.

Run41 ACTIVE on DAY1-32 binary, GDB PID102220 / exec9160, title Parasite
Eve - sewers41, prefix local/live/sewers41, DISPLAY :10.0. Opening800;
ordinary Start180ms/Return180ms accepted default name at1830. Route helper
fromcourtyard tostage_exit is active (exec36459). It deliberately
stops at stage_exit; do not automatically start old healing/rehearsal
strategy. Continue optional items/equipment/resource checks before the
next rehearsal attempt. No gameplay RAM writes. Do not restart while live
or send competing input. Full Day1/transition/audio/movies/platform and
publication scope remains open.

## DAY1-31: original ClearImage worker and GPU draw-status readback (2026-09-07)

Previous goal turn was progress (DAY1-30 code/tests and fresh replay).
This cut restores original76434..76664 in pe_libgpu.c: signed in-place
RECT clamps, multiples64 quick-fill branch, precise rectangle branch,
original A3300/A3328 packet writes and linked-list submission. Registered
76434 in direct76C34 for guest RECTs and queued76EE4. Info3/4/5 are
explicit v2 GPU provider contracts via current drawing state (20/20/22
bits); no general GP1 info-read latch or complete76BE0 implementation is
claimed. The original EXE oracle stops only at76BE0/76B98 hardware
providers; no clamp or packet construction is replaced. 180 original
cases cover positions/sizes, signed/zero/unusual limits, mode/color and
state restoration. Generated header and native tests are in test_clear_image.h.

These cases exposed missing GPUSTAT readback of E1/E6. pe_gpu.c now
keeps low draw-mode bits, mask bits11/12 and texture-page bit15 coherent
with E1, E6 and textured polygon tpage writes. Added independent status
checks, actual VRAM fast/precise clears, direct and pump paths, and a
busy-DMA enqueue test that changes the source RECT before DrawSync:
only the retained8-byte queue payload is clamped/used. Targeted normal
and ASan/UBSan tests PASS (3clear groups +1status group). Final CTest5/5 PASS (58.73s;1232 groups). Logs local/live/{oracle-day1-31-clear,
native-day1-31-clear,native-asan-day1-31-clear,native-day1-31-status,
native-asan-day1-31-status,ctest-day1-31}.log.

SDK74F44 is STILL the host-only inline shim. Next connect its validator
and original jtb dispatch; do not claim production ClearImage fixed yet.
Native RECTs need mutable copied-word handling on direct issue (retail
clamps caller RECT), while queued calls leave caller RECT unchanged and
clamp retained payload. Existing76C34 Inline8 uses const payload and
currently rejects76434 direct; avoid silently losing clamp side effects.
A shared worker for guest versus mutable transient words can preserve
retail read/store order without inventing a guest scratch address.
Wrapper byte packing/name/jtb offsets are documented in DAY1-30 below.
Existing boot fixtures may need actual GPU jtb/limits once shim is removed;
HostFB_ClearImage itself remains a separate host API.

Run40 remains ACTIVE on DAY1-30 binary (GDB1844/exec69867, child1859),
so DAY1-31 worker/status changes have NOT run in the live game. Sequential
input driver exec8488 completed stage_exit22060 then launched heal-route
PID41197. Medicine21->45 at22290, backstage23110,
understage24170, corridor25710, rat defeated26390 and mirror exit27410.
Rehearsal helper will start only after
heal-route returns success. Do not start competing helpers or restart.
No new rehearsal victory, sewer/transition or full Day1 acceptance claim.

## DAY1-30: rehearsal contact crash and GPU fills; run40 ACTIVE (2026-09-07)

Run39 reached rehearsal41430, PE-healed34->45 at41560, then crashed in
1D268 at frame49207 (Aya34HP/story91). Original callback on the saved
crash RAM returns normally, with every byte of 2MiB unchanged. Retail
1D280 reads physical RAM address0 when the enemy action pointer is0;
that is valid PS1 RAM, but the host abstraction rejected it. Preserve the
read via its KSEG0 alias in actor_contact_port.c, as the effect runner
already does. Do NOT replace it with a null early return: byte1 must still
acknowledge contact. Added65 oracle cases, including low/cached aliases,
byte values0/1/2/255, whole contact pass and the crash state/tag. All279
original actor-contact cases PASS native and ASan/UBSan. Crash evidence:
`local/live/sewers39-crash-{backtrace.txt,ram.bin,state.json}`. Run39 ended;
its rehearsal helper's xdotool error followed the game abort.

Also corrected GP0(02h) quick-fill in pe_gpu.c: low-byte red/high-byte
blue, X aligned down16, width rounded up16 after10-bit masking, and
independent X/Y wrapping. It ignores clipping, offset and E6 masks and
clears destination bit15. Existing DRW1/PRS1 expectations encoded wrong
BGR/unrounded extents; corrected them. Twelve explicit hardware cases
check every VRAM pixel, including both320x240 buffers, zero sizes,
full-width rounding, both wraps and mask override. Native and sanitizer
fill tests PASS; five presentation tests PASS. Hardware authority:
https://psx-spx.consoledev.net/graphicsprocessingunitgpu/
https://psx-spx.consoledev.net/memorymap/
Full CTest5/5 PASS (57.74s,1228 native groups). Logs:
`local/live/{oracle-day1-30-contact,native-day1-30-contact,
native-asan-day1-30-contact,native-day1-30-fill,native-asan-day1-30-fill,
native-day1-30-present,ctest-day1-30}.log`.

Run40 ACTIVE on DAY1-30 binary, :10.0, GDB PID1844 / exec69867,
child1859. Prefix/title sewers40 / Parasite Eve - sewers40. Naming
accepted with ordinary Start180ms/Return180ms (menu gone1370).
ONE sequential input driver exec8488 runs replay fromcourtyard tostage_exit,
then sewers40-heal-route.py, then sewers40-rehearsal.py with check=True.
Per-step logs sewers40-{route,heal-route,rehearsal}.log; route phases in
sewers40-replay.log. Do not start competing helpers or restart on stale
snapshots. All input is ordinary keys; GDB only reads. Battle success
on the fixed binary has NOT yet been established.

Next transition prerequisite: SDK func_80074F44 remains a host-only
ClearImage shim (psx_compat.h102), so Y240 clears never reach VRAM.
No wrapper/worker changes were made in this cut; prioritized live crash.
Original74F44 validates via74E28(name118BC), dispatches jtb+0C worker
76434 through jtb+8 queue76C34 with copied8-byte RECT and packedRGB.
Worker76434..76664 clamps signed w/h to[0, signed limits95750/52 minus1].
If x and clamped w both multiples64: packetA3300 words {05FFFFFF,
E6000000,E1000000|GPUSTAT&7FF|colorbit31<<10,02000000|RGB,xy,wh}.
Otherwise packetA3300 {080A3328,E3000000,E4FFFFFF,E5000000,E6000000,
E1000000|mode,60000000|RGB,xy,wh}; packetA3328 {03FFFFFF,
E3000000|info3,E4000000|info4,E5000000|info5}. info3/4/5 via76BE0
are current area/offset hardware readbacks. Tailcalls76B98(A3300), returns0.
Must add worker76434 to direct76C34 and queued76EE4 paths, preserve
native transient RECT lifetime with Inline8, and validate against original
packets plus actual GPU/queue. Pure HostFB_ClearImage can remain its own
host test API; do not use it as retail VRAM authority. Original9BD78
transition display setup needs this before its two frame-buffer clears.
91854/9BD78/91C94/96498/9234C and full Day1/platform/publication remain open.

## DAY1-29: transition GTE setup; run39 backstage route ACTIVE (2026-09-07)

Restored original EXE transition-init GTE helpers: 78E34 light matrix,
78E64 color matrix, 78FC4 background color, 78FE4 far color, 77E64 fog
near/far setup. Preserve low-32-bit shifts/products, signed division,
short-span no-op and signed-halfword slope clamp. Far-color control
storage does not implement the remaining depth-cue rendering commands.
Invalid H/overflow division retains an explicit boundary corresponding
to original BREAK 7 / BREAK 6 before either coefficient write.

`pe_transition_gte_oracle.py --write-header`: 125 original cases (123
control-register readbacks and 2 arithmetic traps) PASS native and
ASan/UBSan. A test caller reads cfc2 after unchanged retail instructions;
no original callee is replaced. Original interpreter reports DIV-by-zero
before its following BREAK; overflow executes through the BREAK site.
Matrix terminal words/DQA are compared at their hardware halfword widths.
Full normal CTest 5/5 PASS (82.71s; 1227 groups). Logs:
`local/live/{oracle-day1-29-gte,native-day1-29-gte,
native-asan-day1-29-gte,ctest-day1-29}.log`.

Run39 remains ACTIVE on DAY1-28 binary (:10.0, GDB PID4104917 /
exec21812). First Eve started14600, scripted battle_exit16290, control17310,
stage_exit19590. Original route helper exec10316 finished exit0. Then
Medicine healed21->45 at20840; backstage21890, understage_scene22240,
understage control22860, corridor_enter23210, corridor24460. Ordinary
inputs only; screenshots/logs retained.

`local/live/sewers39-heal-route.py` is ACTIVE (PID4140392 / exec64000), log
`sewers39-heal-route.log`, continuing corridor/rat/keys to rehearsal.
Do not start competing input helpers. `sewers39-rehearsal.py` is prepared
(prefix copied from run38) but NOT launched; start only after checking
heal-route completion/"Ready in rehearsal room" and the current game state.
Do not restart the game to use DAY1-29 while run39 remains live.

Transition 91854/9BD78/91C94/96498 and rendering/input remain unfinished;
sewer/optional content, audible audio/movies, final platform validation
and publication remain open. The full Day1 goal is not complete.

## DAY1-28: transition packets; run39 ACTIVE (2026-09-07)

Restored original 80195F6C (331 words, ending at 80196498), called by
transition init 96498. Creates both banks of two fullscreen G4 gradients,
top/bottom semi-transparent fade strips and three draw-mode packets.
Native translation, not matching C. SHA256 of original routine:
`112b2d3c7bdaedac44c218a012e4b56385b77dbbe75f1e0eeccbb3ed3cbd1bf6`.

`pe_transition_packets_oracle.py --write-header`: 160 original history
steps PASS native and ASan/UBSan. Covers distinct initial byte patterns,
both banks, flag values 0/1/2/101, original 92740 OT insertion, and
reinitialization after linking. All changed non-stack RAM is covered.
Full normal CTest 5/5 PASS (52.46s; 1226 groups). Logs:
`local/live/{oracle-day1-28-packets,native-day1-28-packets,
native-asan-day1-28-packets,ctest-day1-28}.log`.

Run39 ACTIVE on :10.0, prefix `local/live/sewers39`, title
`Parasite Eve - sewers39`. GDB PID4104917 / exec21812, opening helper
exec1603 completed. Naming was completed manually with ordinary Start
then Return input (180ms), confirmed menu D154=0 at frame2880. The
previous run38 replay failed "name menu did not close"; its route helpers
therefore never had their start signal. No gameplay RAM writes.

Run39 route helper PID4109801 / exec10316 is ACTIVE: `pe_live_replay.py
--prefix local/live/sewers39 --title 'Parasite Eve - sewers39'
--from courtyard --until stage_exit`; log `sewers39-replay-route.log`.
Confirmed phases courtyard3730, lobby4100 (HP45/g74=9), auditorium5040.
Both process handles re-polled live after auditorium. Do not start
competing input helpers. Next: verify game/helper process state, continue
to stage_exit, then heal/backstage/keys/rehearsal with ordinary input.
No heal-route or rehearsal helper is running for run39 yet.

Orca skill/capabilities rechecked: desktop runtime is up; Linux provider
has no screenshot/focus support. Existing X11 input/read-only debug-frame
capture fallback was used. Game is the fresh DAY1-28 build. This does
not prove live transition rendering: 91854/96498, remaining frame-loop
render/input calls, sewer and optional content, audible audio/movies,
platform validation and final launcher remain open.

## DAY1-27: transition object-pool graph (2026-09-07)

Restored original M0000I 90998..90D3C / 91580..91854 / 9959C..995BC:
13 functions covering pool initialization, allocation/release, iteration,
depth-list attach/detach, relative metadata lookup, three constructors and
object removal. Native translation in `func_80190998_port.c`, not matching C.
Original 96498 calls 90998 and the three constructors; the whole initializer
and frame loop remain unported. Preserve the original partial initialization
(slot 0 is not reset like slots 1..199), signed halfword indices and lists.

`pe_transition_pool_oracle.py --write-header`: 2589 complete original
history steps PASS native (`DAY1_transition_pool`), comparing return values
and persistent RAM after every call. Includes all 200 slots, iteration,
head/middle/tail removal, complete permuted drain/refill, free-slot reuse,
reinitialization with three distinct byte patterns, both parent depths,
constructor stack arguments and signed/truncated resource indices. Every
changed non-stack byte is asserted to lie in the compared ranges. Focused
ASan/UBSan PASS; full normal CTest 5/5 PASS (54.85s; 1225 test groups).
Logs: `local/live/{oracle-day1-27-pool,native-day1-27-pool,
native-asan-day1-27-pool,ctest-day1-27}.log`.

No live transition/rendering claim; Run38 remains terminal. Remaining
transition work includes 91854,95F6C,96498 and the 9234C frame-loop graph
(942FC,8F05C,8F92C,92800,93478,92030). Sewer/remaining encounters, optional
content, audible SPU/score, movies, platform validation and final launcher
remain open. The goal is still all of Day1 at verified retail accuracy.

## DAY1-26: transition positional sound graph (2026-09-07)

Restored original M0000I 80191EFC (59 words, ending at 80191FE8),
called by 8019234C at 80192554. It saves matrix state, uses the
transition camera with H=0x300, projects the signed low halfwords of
three position words through 6DFA8, queues volume then pan, and restores
the camera pointers/RT/TR. Original stack locals use a saved/restored
24-byte host scratchpad window. Native translation, not matching C.
Original code SHA256:
`0b26f974cb4a5b285e42182b6798d484dcc74e99843073d22642efb010a347af`.

Also restored 868F0 / 86A28 producers and 8B698 / 8BA3C consumers
(A0/A2). Original handle masks differ (FFFF vs 3FF); preserve this,
including when volume and pan select different active voices. Group
selection uses bit overlap, otherwise exact handles. Consumers reset
the corresponding ramp count and mark updated voices.

`pe_transition_sound_oracle.py --write-header` executes 184 complete
original graphs: wrapper + both queued commands, and direct producers
with group/value/handle masking. Checks queue RAM before consumption
and voice RAM after; asserts every changed non-stack RAM byte is covered.
Native `DAY1_transition_sound` PASS, including camera/GTE/scratch restore.
`pe_audio_commands_oracle.py --write-header`: 382 complete original
command graphs PASS native (new A0/A2 active/inactive/group/handle/value
cases included). Both focused groups PASS under ASan/UBSan. Normal
CTest 5/5 PASS (62.46s); 1224 native test groups. Logs are
`local/live/{oracle-day1-26-transition-sound,oracle-day1-26-audio,
native-day1-26-transition-sound,native-day1-26-audio,
native-asan-day1-26-transition-sound,native-asan-day1-26-audio,
ctest-day1-26}.log`.

No live transition, audible playback or full frame-loop claim. Run38
remains terminal. 8019234C itself, 80196498 init and its rendering/input
callees remain unported, alongside sewer/optional route and remaining
Day1 acceptance. Latest static inspection lists frame-loop calls
801942FC,8018F05C,8018F92C,80192800,80193478,80192030 among remaining
work; restoring a sound leaf does not close those calls.

## DAY1-25: explicit music fade consumer; run38 TERMINAL (2026-09-07)

Original EA207 producer 86CA4 already queued C2, but 8CA84 had no
8008B410 consumer. The 92-word original graph (8B410..8B580) now
sets the explicit start volume, signed per-tick delta, low-halfword
countdown, selected music slot and active-voice dirty flags. Zero
duration becomes one; owner zero selects slot one; unmatched owners
leave both slots unchanged. This is a native translation, not matching C.

`pe_audio_commands_oracle.py --write-header`: 196 original command
graphs; focused native `ATK19_retail_audio_commands` PASS. Added C2
cases cover both slots, owner mismatch, empty/active voices, rising,
falling/equal/masked endpoints, zero duration, halfword truncation,
and signed 32-bit duration edges. Both focused audio and BTL94 fixture
checks PASS under ASan/UBSan. Full normal CTest 5/5 PASS (60.64s).
The oracle asserts all changed C2 RAM below the CPU stack is covered.

Full regression initially found BTL94's missing boot music arena. Its
isolated setup now supplies overlay+0x150; original 6B35C derives the
two 0x1400-byte music slots. No runtime loader bypass was added.
Evidence: `local/live/{oracle-day1-25-audio,native-day1-25-audio,
native-asan-day1-25-audio,native-day1-25-btl94,
native-asan-day1-25-btl94,ctest-day1-25}.log`.
No audible playback, sequencer or live EA207 acceptance claim.

Run38 is TERMINAL: actual exit file and backtrace record host-quit,
exit 0 at 2026-09-07 20:32:57, present 149257. Both helper logs show
route timeouts; final state story g74=1, HP45. No stage-exit or sewer
completion evidence. No game or replay process remains live. The older
"run38 is next" entry below is superseded; do not wait on run38.

Remaining scope includes the transition frame loop/init (8019234C /
80196498 / 80191EFC), sewer traversal/remaining encounters, all optional
content, audible SPU/score, movies, platform packages and final launcher.
The 80191EFC disassembly reveals additional missing audio producers
868F0 and 86A28 (and their consumers); do not silently skip these.

## DAY1-24 `80191E30`; 8CA84 0x11/0xC1; run38 (2026-09-07)

Goal remains: all of Day 1 at verified 100% retail accuracy. Still open:
sewers, remaining encounters, audible SPU/score, movies, optional
branches, the rest of `8019234C` (`80196498`, `80191EFC`), host
packages, final launcher.

Run36 died after naming on `8CA84` cmd `0x11` (`8008C3E4`). Run37
cleared naming, courtyard and lobby, then died on cmd `0xC1`
(`8008B2CC` volume ramp). Both commands and the original `8A354` id
filter are now native. 81 original audio-command graphs PASS
(`PE_TEST_FILTER=ATK19_retail_audio_commands`). Not matching C.

`func_80191E30` is a native 51-word translation. Native
`PE_TEST_FILTER=DAY1_gte_rt_leaves` PASS. `80191EFC` is still unported.

Run38 is next on `:10`, prefix `local/live/sewers38`.

## DAY1-23 `78A94`/`78B38`/`6EC6C`; 6CDA4 SPU pump; run36 (2026-09-07)

Goal remains: all of Day 1 at verified 100% retail accuracy. Still open:
sewers, remaining encounters, audible SPU/score, movies, optional
branches, the rest of `8019234C` (`80196498`, `80191E30` now has its
GTE/table helpers), host packages, final launcher.

Run35 present stall was not dest-ready. Skip-movie published
`0xA80830C8` (M0431I), dest-ready returned 1, then EA200 blocking
`6CDA4` sat in state 10: `870E0` reads `D_8009D24C` while DMA4 IRQ
never ran. Host completion is `PE_SpuDma_Service` (normally VSync);
the retail blocking poll never reaches VSync, so the IRQ is serviced
inside that loop. Diagnostic after the pump: EA200 finished, dest
`0xA8001048`, 915 presents / 20s. Not matching C.

`func_80078A94` / `func_80078B38` are original Push/PopMatrix (depth
`D_800963E8`, 20-slot array `D_800963EC`, eight cfc2/ctc2 words).
`func_8006EC6C` is `a0 + lw(a0 + ((int16)a1 << 16 >> 14))`. Three
original `6EC6C` executions PASS. Native
`PE_TEST_FILTER=DAY1_gte_rt_leaves` PASS. Python `execute()` does not
apply cfc2 to guest stack, so Push/Pop are layout-tested only.

Run36 is next on `:10`, prefix `local/live/sewers36`. Do not start
helpers until presents advance past skip-movie (frame ≫ 10).

## DAY1-22 `78E04`/`78E94`; run34 TERMINAL; run35 hung (2026-09-07)

Goal remains: all of Day 1 at verified 100% retail accuracy. Still open:
sewers, remaining encounters, audible SPU/score, movies, optional
branches, the rest of `8019234C` (`80196498`, `80191E30` still needs
`78A94`/`78B38`), host packages, final launcher.

`func_80078E04` / `func_80078E94` are the original 12-word and 8-word
ctc2 SetRotMatrix / SetTransMatrix bodies. Native loads GTE RT/TR from
the MATRIX layout. Not matching C. `80191E30` also needs 6EC6C/78A94.

Run33 TERMINAL: rehearsal Eve defeat. Run34 TERMINAL: skip-movie
returned then `host-quit` after 13 presents; naming never opened.
Run35 is TERMINAL: dest `0xA80830C8` published and overlay pointers
looked loaded, then presents stalled at frame 10 (~60% CPU). gdb was
aborted 2026-09-07 18:44 (`exit code None`). Replay never reached
naming. `:10` is free. Next live prefix is run36; identify the
post-skip-movie present stall before treating boot as healthy.

## DAY1-19 M0000I `80192740`; run33 live (2026-09-07)

Goal remains: all of Day 1 at verified 100% retail accuracy. Still open:
sewers, remaining encounters, audible SPU/score sequencing, movies,
optional branches, the rest of `8019234C`, host packages, final launcher.

`func_80192740` is a native translation, not matching C. Compared only
for `*8019C02C==0` (no `80193B5C`). `pe_m0000i_leaves_oracle.py
--write-header`: 35 original overlay cases PASS native.
`PE_TEST_FILTER=DAY1_m0000i_leaves` PASS (1222 groups).

Run32 is TERMINAL (helpers died at `rehearsal_room`; last snapshot
corridor `0xa8001148` with both keys). Run33 is ACTIVE on `:10`, prefix
`local/live/sewers33`. Diary-exit waypoints were widened. Do not start a
competing prefix.

## DAY1-18 M0000I fade `94108`/`941A4`; run32 aisle (2026-09-06)

Goal remains: all of Day 1 at verified 100% retail accuracy. Still open:
sewers, remaining encounters, audible SPU/score sequencing, movies,
optional branches, the rest of `8019234C`, host packages, final launcher.

`func_801941A4` / `func_80194108` are native translations, not matching
C. Fullscreen semi-trans G4 + DR_MODE via already-ported GPU helpers;
signed fade stepper returns the next `s16`. Called from `8019234C`.
`python3 pc_port/tools/pe_m0000i_leaves_oracle.py --write-header`: 31
original overlay-instruction cases PASS native (RAM + stepper `v0`).
`PE_TEST_FILTER=DAY1_m0000i_leaves` PASS (1222 groups).

Run32 remains ACTIVE on `:10`, prefix `local/live/sewers32`. Do not
start a competing prefix.

## DAY1-17 M0000I `80193AB0`; run32 aisle (2026-09-06)

Goal remains: all of Day 1 at verified 100% retail accuracy. Still open:
sewers, remaining encounters, audible SPU/score sequencing, movies,
optional branches, the rest of `8019234C`, host packages, final launcher.

`func_80193AB0` (43 words, overlay SHA
`766697b440a25e78…a69ca409`) is now a native translation, not matching
C. Signed `lh` at `8019C058`; if >0 decrement and add ten source
records at `801EA268` into dest records from `8019CAA8`. Called from
`8019234C` at `8019264C`.
`python3 pc_port/tools/pe_m0000i_leaves_oracle.py --write-header`: 14
original overlay-instruction cases PASS native.
`PE_TEST_FILTER=DAY1_m0000i_leaves` PASS (1222 groups). `8019234C` and
`80196498` remain unported.

Run32 is ACTIVE on `:10`, title `Parasite Eve - sewers32`, prefix
`local/live/sewers32`, DAY1-14 binary. Last seen in the aisle after
`aisle_waypoint` toward first Eve. Do not start a competing prefix.

## DAY1-16 M0000I leaves 91DE8/9BF8C; run31 host-quit (2026-09-06)

Goal remains: all of Day 1 at verified 100% retail accuracy. Still open:
sewers, remaining encounters, audible SPU/score sequencing, movies,
optional branches, the rest of `8019234C`, host packages, final launcher.

Two overlay leaves from the M0000I chunk (LBA2805,
`c51e36c27422e990…0a4edb`) now have native translations, not matching C:

- `func_80191DE8` (18 words): fade bytes `B0DCE`/`B0DCF` = 16 or 16/64,
  `B0DD0`=0, `B0DD2`=0xFFF.
- `func_8019BF8C` (14 words): store `*B0E4C` or `*B0E4C+0x15F90` at `a0`,
  add when `a0 != 8019C1F8` (`addiu` sign-extends the `-15880` immediate).

`python3 pc_port/tools/pe_m0000i_leaves_oracle.py --write-header`: 9
original overlay-instruction cases PASS native.
`PE_TEST_FILTER=DAY1_m0000i_leaves` PASS (1222 groups). `8019234C` itself
and `80196498` (1474 words) remain unported.

Run31 is TERMINAL: host-quit at present 189215, exit 0, last live pose
diary-room (757,-395) with Rehearse Key `g24=0x14c0220`. Recover4 left the
table but did not finish `rehearsal_room` before the window died. Prefix
`local/live/sewers31`. Run32 is ACTIVE on `:10`, title
`Parasite Eve - sewers32`, prefix `local/live/sewers32`. Do not start a
competing prefix. Replay uses the wider diary-exit waypoints.

## DAY1-15 EA200/201/203/204 wired; run31 recovering diary key (2026-09-06)

Goal remains: all of Day 1 at verified 100% retail accuracy. That is still
open. This cut only closes the previously unwired music start/stop EA
commands. Score sequencing, audible SPU synthesis, movies, sewers,
remaining encounters, optional branches, and `8019234C` stay unresolved.

`func_80015DAC_default_cut` now dispatches original keys 200/201/203/204
from `3420.s` (`15E30`/`15F78`/`15ED0`/`15FD4`): load/unload through the
already-translated `func_8006D2B8`, play/stop through original command
producers `86464`/`86498`/`864F8`/`86770`, channel records through
`6DB48`/`6DB9C`, and the `16758` yield (rewind `D_8009CE00` by `0x20` and
store the loader return at `task+0x10`). `86464` now returns the `8CBA8`
handle the way the original delay-slot consumer uses it. Not matching C.

`python3 pc_port/tools/pe_music_start_oracle.py --write-header`: 64
complete original cases that stay off `6CDA4` PASS against native RAM and
`v0`. `PE_TEST_FILTER=DAY1_music_start` PASS (1221 native groups). These
cases cover missing/busy/existing channels, signed IDs, EA203 volumes, and
EA204 cmd `0x90`. They do not prove audible playback or the blocking CD
load/yield path under live providers.

Run31 is still the live prefix (`Parasite Eve - sewers31` on `:10`,
DAY1-14 binary `699923ad…`). Helpers died at `diary_key`; the process
stayed up. Recover re-entered M0018I at 77170 and opened the Rehearse Key
popup at 81000 (`g24=0x14c0220`). Do not start a competing prefix.
Launcher remains PE-DAY1-9.

## DAY1-14 hit-draw callbacks restored; run30 died after heal (2026-09-06)

Codex left DAY1-13 with run30 unlaunched (sandbox X bind). This session
connected to the existing xrdp `:10` and launched run30 on the DAY1-13
binary `6ce8bc7d`. It reached aisle, first Eve, `stage_exit` 16640, and
Medicine 21→45 at 16850. `sewers30-heal-route.py` then timed out closing
the items list (`find(1,1)` still present). The process later host-quit
at present 18143 (`stop_reason=host-quit`, exit 0). No rehearsal, no
live hit-draw claim. Prefix `local/live/sewers30`.

The remaining DAY1-13 draw boundary is now translated from original
BE50C.s / B3390.s, not reconstructed: `func_800C3B04` (488-word sprite
renderer), `func_800CDD0C` (particle hit draw), `func_800CDE90` (spark
draw), plus the three COP2 wrappers those leaves call (`func_80079244`
RTPS, `func_80079E14` Y-compose, `func_80078C34` ApplyMatrix). Wired
through `PE_WeaponCallback`. Scratchpad `+0x18` retains the last scaled
color word. No matching-C claim. FLAG is stored as 0; the compared
original cases wrote 0 there.

`python3 pc_port/tools/pe_hit_draw_oracle.py --write-header`: 96 complete
original/native cases PASS in normal and ASan/UBSan builds. Coverage:
both GPU banks, brightness 0x80 and scaled, zero/signed rotation, UV/RGB
variants, CDD0C particle ages/offsets, and CDE90 sparks. Every original
non-stack write is in the compared ranges. Pinned executable
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Related DAY1 groups remain
PASS (1220 native groups).

Normal CTest 5/5 PASS 67.93s (`native-tests` 67.46s). Focused ASan
`DAY1_hit_draw` PASS. Linux runtime SHA256
`699923ad70707f51bc4ef0b9ff9273a5158f5a27237ddbc09fb0706c8de51b8f`.
Windows runtime untested this cut. Launcher remains DAY1-9.

Run31 is ACTIVE on this binary, title `Parasite Eve - sewers31`, prefix
`local/live/sewers31`, DISPLAY `:10`. Inventory close after Medicine now
retries Circle until the menu list is gone (run30 timed out there).
NEXT: monitor opening through rehearsal; capture the next original
boundary. Do not start a competing prefix while this run is live. Full
Day1 and final publication remain open.

## run29 TERMINAL: rehearsal defeat at 43480; run30 prepped (2026-09-06)

Run29's rehearsal battle was LOST: Aya HP 45→34→23→heal→45→34→23→heal→45→
34→23→19→0, hitting 0 at frame ~43480 despite movement + two PE Heal 1 casts
(23→45). Death window: HP19/AT8850/PE53 — third heal unaffordable (PE<60)
and AT unfilled, so the AT-gated heal never fired before the killing hit.
`rehearsal.py` asserted out and stopped inputs. Evidence: replay/rehearsal
logs, `sewers29-follow-live.json` (28 frames HP19→0, follow task PC
0x801D94D4 flags 48 delay 1 rate 128 target Aya 0x800bef90 identity 3 —
restored DAY1-12 follow command executing live in battle, DAY1-12 binary).
Post-defeat the game was manually driven through Game Over back to the
opening opera-exterior dialogue (defeat.png, story 1, HP45, token A8001048,
mode -1 at 44470 — inference from states/screenshot, no reset in gdb log),
then host-quit; exit code 0. No rehearsal victory, no DAY1-13 live claim.

Tree verified this turn: `cmake --build pc_port/build` fresh, binary SHA
unchanged 6ce8bc7d (DAY1-13); focused `DAY1_hit_init` and `DAY1_script_follow`
groups PASS (1219 groups, rest skipped). No code changed.

Run30 prepped on fresh prefix `local/live/sewers30`, title
`Parasite Eve - sewers30`, DAY1-13 binary: `sewers30-heal-route.py`,
`sewers30-follow-watch.py` (prefix swaps), `sewers30-rehearsal.py` with
combat fix — heal at HP<=34 (was 28), evasive-only kiting while fragile
(never open attack menu stationary waiting for AT/PE), wider waypoint box,
battle timeout 360→600s. Display :10 stale (socket present, unconnectable). Xvfb :11/:12
cannot start in this sandboxed shell: AF_UNIX bind → EPERM (verified by
raw socket probe), so no X server and no game window/inputs from here.
Full normal CTest 5/5 PASS 45.09s on this tree (native-tests 44.88s).
Run30 scripts are ready but NOT launched.
Run30 launch BLOCKED in this session (2026-09-06 ~14:05 UTC, recheck
~14:15): escalation still auto-denied (approval prompts disabled at
runtime), AF_UNIX bind still EPERM, and Xvfb with `-listen tcp` also dies
(getifaddrs EPERM, no listener survives; TCP 6012 refused). xdotool has
since appeared at ~/.local/bin/xdotool, but with no X server there is no
display for game or inputs. Nothing was launched; prefix sewers30 has no
state files yet. Recipe for an unsandboxed shell with X (all need DISPLAY set, e.g.
revived xrdp :10 or fresh `Xvfb :12 -screen 0 1280x1024x24 &`), from repo
root, xdotool installed:
`PE_LIVE_PREFIX=$PWD/local/live/sewers30 PE_LIVE_EVERY=10 PE_LIVE_VRAM=0
gdb -q -batch -x pc_port/tools/pe_live_gdb.py --args
pc_port/build/parasite-eve-port --windowed --skip-movie --disc-image
'rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin'
--scale 2 --window-title 'Parasite Eve - sewers30'`,
then `python3 pc_port/tools/pe_live_replay.py --prefix local/live/sewers30
--title 'Parasite Eve - sewers30' --until stage_exit`,
then `sewers30-heal-route.py`, `sewers30-rehearsal.py`,
`sewers30-follow-watch.py` (each with DISPLAY set, logs to matching
`local/live/sewers30-*.log`).
NEXT: launch run30 per recipe, confirm opening phases, monitor to rehearsal.

## DAY1-13 hit initialization restored; run29 active (2026-09-06)

Restored original BE180.s CD980/CDA5C/CDC24 through PE_WeaponCallback in
C2414_port: next hit target/terminal marker, copied impact coordinates,
conditional sound FIFO request/texture selection, randomized particle and
spark records. The separate CDD0C/CDE90 draw callbacks and C3B04 renderer
remain explicit boundaries. No complete hit-rendering or live hit claim.

`pe_hit_init_oracle.py --write-header`:90 full original/native cases PASS in
normal and ASan/UBSan builds. Covers nine RNG seeds, signed coordinates,
target indices/next-target flags, disabled/invalid/valid sound banks and
complete C2758 creation of all three descriptor types. The checked oracle
also asserts every original non-stack write is compared and verifies the
three created descriptors and enabled sound queue entry. Existing CEDA8 tests
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

Fresh run29 is ACTIVE under GDB, exec97609; opening helper72814 finished
stage_exit17750, healing/route helper67443 continues backstage/keys/rehearsal.
It uses the already-running DAY1-12 binary94e7b4b7, not rebuilt DAY1-13.
Prefix `local/live/sewers29`, title `Parasite Eve - sewers29`.
First Eve12690→14540, control15670 HP21; stageexit17750; Medicine21→45
at17940; backstage19380, understage20430. No gameplay RAM writes.
Orca rechecked: screenshots/focus unsupported; xdotool/read-onlyGDB fallback.
NEXT: finish fresh rehearsal battle with movement/healing, capture next gap.
Do not start competing helpers or reuse this prefix while active. Launcher
remains DAY1-9; full Day1 and final publication unfinished.

## DAY1-12 actor-follow36 restored and verified; next fresh rehearsal replay (2026-09-06)

Run28 passed the previous flash boundary, rendered the rehearsal fight, then
stopped at snapshotframe49014 (last regularframe49010) at VM36/13E84,
pc801D94D4, actor800BF210, args80120F80, HP4 alive, storyg74=5B,
tokenA80021C8. All game/input helpers TERMINAL. An attempted menu helper
started after exit was terminated while waiting to find the window; no keys
were sent. Run28 did NOT complete the battle. Evidence:
`docs/evidence/pe-day1-flash-weapon/`, `local/live/sewers28-stop-*`.
Run28 binary SHA2563bfcd7f5928bfdf18130af9ccf4094b7ae6a47f71450e5538ce1940a9699ade0
is DAY1-11, not the newly restored actor-follow command.

Native `func_80013E84_port.c` now translates all233 original words:
resolve camera/named actor, latch identity/rate, terminate removed/reused
targets, track their current position, bounded steering/fixed-point velocity,
arrival snap or rewind/yield. Wired VM36 and CMake/compat declaration.
The existing79FB4 arctangent helper now uses explicit unsigned wrapping for
negation/left shifts, preserving original32-bit behavior at signed edges.
No matching-C claim. Original3420.s plus pinned executable are authority.

`pe_script_follow_oracle.py --write-header`:536 histories /840 steps PASS
normal and ASan/UBSan, including repeated full originalVM, moving/hidden/
reused targets, target lookup, camera speed scaling and signed/overflow
cases. Native fixture includes all sine/atan tables; an initial omitted
atan table was corrected before final checks. Final logs:
`oracle-day1-12-follow.log`, `native{-asan,}-day1-12-follow-final.log`.
Original/native copied run28 handler matches every non-stack byte. FullVM
also matches, excluding only the established host VM argument vector
80120F80..80120FBF. On copies only, restore the handler nextPC or the VM's
opcodePC and already-consumed delay1. Assertions confirm the handler enters
and initializes its task flag in both executions. Evidence:
`local/live/script-follow-live-comparison.json`, input/original/native files,
`script_follow_native_live.c`, `script-follow-native-live` and original log.

Normal CTest5/5 PASS59.98s; sanitizer5/5 PASS72.19s (1218 native groups),
including final644 mixed effect-pump frames. Windows runtime crossbuild PASS;
Windows execution remains untested. Logs `ctest{-asan,}-day1-12.log` and
`build-win-day1-12-runtime.log`. Linux runtime SHA256:
94e7b4b7a452927fa53cc5d84fd525e21e731d1d82dc79e148517ef0a0b91799.
No active game/replay. No DAY1-12 live claim or publication; launcherDAY1-9.

NEXT: fresh native replay with DAY1-12. Official theater-key approach now
stays atX130 throughZ-3600 before rejoining center (run28 recovery verified).
Reuse run28 menu helpers to heal afterfirstEve and before rehearsal. Improve
battle input strategy using ordinary movement/PE/items: blind Return taps
lost41HP in ~2000frames; Aya was not defeated before the explicit stop.
Then continue rehearsal victory/M0367I/M0319I and sewer entry. Original
M0319I event6 rectangleX693..1252,Z-999..-360 supplies the sewer prompt;
see `DAY1_ROUTE_AUDIT.md` for verified script transfers, not live coverage.
FullDay1, optional branches, audio/movie restoration and final release remain.

## DAY1-11 mixed weapon/flash stack restored; live replay passed boundary (2026-09-06)

Run27's first rehearsal flash stopped because preceding pistol/hit draws
invalidated the original borrowed stack position. Proven original writers
C9ED8/DC/F8 (casing angles) and CA444/450 (muzzle scale words) now feed the
scoped69594 weapon-draw context. Empty C2414 draws and C9EA0/CDD04 preserve
known bytes. Unknown callbacks and fresh RAM still retain explicit boundaries.
Production changes: D413C_port tracker,69594_port draw wrapper,C2414_port hook,
compat declarations. No broader hit-callback implementation yet.

Expanded `pe_m0023i_pump_oracle.py`:644 frames/46 histories PASS normal and
ASan/UBSan, including signed ages, both GPU banks, draw order, empty callbacks
and pause/resume. Mixed synthetic slot code55 allows room VM updates under
retail100 weapon pause; assertions prove both flashes reached byframe2 in all
46 histories (final generator assertions). Actual run27 copied RAM:
64 complete pumps including real weapon updates/casing expiry/beams, all
defined non-stack bytes match (`m0023i-pump-run27-comparison.json`). Detailed
writer proof and limitations in `M0023I_FLASH_STACK.md`.

Normal CTest5/5 PASS67.46s; ASan/UBSan5/5 PASS87.15s (1217 native groups).
Windows runtime crossbuild PASS; Windows execution remains untested. Logs:
`local/live/{ctest,ctest-asan}-day1-11.log`,
`{native,native-asan}-day1-11-pump-final.log`, `build-win-day1-11-runtime.log`.
Python compile and touched tracked whitespace checks clean. No publication.

Final644frames PASS in both focused builds (`native{-asan,}-day1-11-644.log`).
Includes8 no-particle histories so every weapon-written byte reaches flash;
original fills00/A5 agree: casing(0,0,640), muzzle(768,0,-1234). Full suites
above used532frames; production unchanged in the final test expansion.
Added route audit8 transfers around rehearsal/M0367I/sewer entry,35total/
15chunks checked; `DAY1_ROUTE_AUDIT.md` records original story stages.

Run28 firstEve12980→14910/control16100, stageexit18040, Medicine16→45 at18220,
backstage19110, understage20180, corridor21790, rat22630/control22750 HP34,
mirror23580→23670. Original route helper39491 TERMINAL after centerline
approach stalled atX-25/Z-2695. Recovery helper30135 TERMINAL
(`sewers28-key-recovery.py/.log`): normal controls X130 throughZ-3600 then
centerline reachedkeydoor32130/trigger32470/keyroom32550. It continues
key/diary/rehearsal, PEheal, battle. Official replay waypoints now stayright
throughZ-3600 and log each waypoint (live recovery evidence). No hit-callback
implementation edits. Opening helper6640 terminal.

Run28 TERMINAL, former gdb2689858/game2689871, exec session98286;
binary SHA2563bfcd7f5928bfdf18130af9ccf4094b7ae6a47f71450e5538ce1940a9699ade0.
Prefix `local/live/sewers28`, title `Parasite Eve - sewers28`.
Opening helper6640 and original route helper39491 are terminal. Recovery
helper30135 completed both keys, PE Heal1 and rehearsal trigger, then stopped
at the new36 boundary.
Sources `sewers28-key-recovery.py`, `sewers28-rehearsal.py`.
Read `sewers28-replay.log`, `sewers28-state.json`, `sewers28-*-route.log`
and gdb log for progress. No gameplay RAM writes. All run27 helpers terminal.
Upcoming possible gap: weapon hit callbacksCD980/CDA5C/CDC24/CDD0C/CDE90 and
shared sprite rendererC3B04 are unported; originalBE180/BE50C/B3390 inspected,
no implementation edits for them. Continue route in order, capture next boundary.

## DAY1-10 Arrange Items restored and live verified; rehearsal boundary (2026-09-06)

Resumed the run26 boundary at46DFC. New native46DFC_port and5B500_port
restore the complete Arrange Items/shared-storage sorting graph: opening,
input/cancel, both submenus, label drawing, category/stat comparisons,
retail723A4/724F4 quicksort, carried/stored inventory sorting and equipped
index relocation. Main command6, menu drawing and input dispatch are wired.
No matching-C claim; authority original340EC/379E4/37B54/37CD0/40A80/486D8/
621E4 instructions. Unknown sort callbacks retain explicit stop boundaries.

`pe_inventory_sort_oracle.py --write-header`:542 complete original cases and
native comparison group PASS in normal and ASan/UBSan builds (1217 groups).
Normal CTest5/5 PASS65.17s; ASan/UBSan5/5 PASS84.44s used initial506 cases;
final542 (including corrected GPU-bank fixtures) additionally PASS in both focused runs. Logs:
`local/live/oracle-day1-10-sort-banks.log`, `native{-asan,}-day1-10-sort-banks.log`,
`ctest{-asan,}-day1-10.log`. Windows runtime crossbuild PASS in `build-win-day1-10-runtime.log`;
no Windows runtime execution yet.
Actual run26 copied RAM matches EVERY
non-stack byte (<801F0000) for six consecutive construction/draw/input/sort/
return steps; only actual host D1A0 is synchronized in the input copy.
Evidence `local/live/inventory-sort-live-comparison.json`, probe/native
source and input/output files at `local/live/inventory_sort*` and
`local/live/inventory-sort*`. This is copied-state proof, not a live claim.

Fresh visible run27 ended at an explicit boundary; all game/input helpers terminal,
normal binary SHA25607781970f1d297d9a1d8922a5d8dc4e55f6f3e66fb1432d11285059e4592e1ef.
Prefix `local/live/sewers27`, title `Parasite Eve - sewers27`. Opening helper1986
completed firstEve13350→15200/control16260 HP16, stageexit18250. Menu helper34829
completed: opened/cancelled ArrangeItems18510; cancelled/reopened/confirmed all
three sort submenus18680/19010/19380. Full inventory multisets and equipped IDs
256/257 preserved each time; item-order changes recorded. Visible screenshots
and JSON copied to `docs/evidence/pe-day1-arrange-items/`. Normal Use Item then
Medicine1 healed16→45 at20840. Route helper51635 completed backstage21670, understage22810, corridor24420,
rat battle25320, TheaterKey30810, RehearseKey37050 and rehearsalroom40880.
Normal PE Heal1 restored34→45 at46360 (PE80→20). Rehearsaltrigger47550,
battle48380 passed the previous14BA0 actor-turn boundary. Run27 stopped48900
HP34/tokenA80021C8 at M0023I Flash(mode2,state0) without known borrowed stack Z.
All game/replay/input helpers terminal. No guest writes.

Resolved in DAY1-11 above: slot0 pistol drawC9B68, slot1 hit drawCD8C8, then slot2
rehearsal drawD4704. Native6F8EC invalidates the stack for weapon draws. Full
original69594 on copied stop RAM, stack fills00/A5, proves first flash input
(-6120,-32756,640); last writersF014(XY) andC9EF8(Z). C9EF8 stores the actual
pistol casing signed-byte age times64. Evidence `m0023i-run27-stack-writers.json`,
`m0023i_run27_stack.py`, `sewers27-stop-*`. The native fix and original/native
validation are complete; fresh live run28 is in progress.

Orca status/capabilities/list-apps rechecked: game absent, screenshot unsupported;
existing xdotool/GDB fallback in use. The test fixture's bank parameter initially
was not forwarded; corrected it and regenerated all542 original cases. Final
normal+ASan focused runs PASS in `native{-asan,}-day1-10-sort-banks.log`.
The production code did not change for that fixture correction. Windows runtime
crossbuild passed in `build-win-day1-10-runtime.log`; execution untested.
No publication of DAY1-10 yet; launcher remains DAY1-9. FullDay1 unfinished.

## Latest launcher update PUBLISHED (2026-09-06)

Correction after user reported "no update for parasite eve on the launcher yet":
the initial private-channel publication did NOT update the launcher's default
channel. The launcher reads the separate public content origin's
`channels/parasite-eve.json`, which was still PE-SEW15-7fc95fa875ba.
The launcher publisher has now completed for **PE-DAY1-9-0c462bc18581**:
both platform objects copied to `banshee-realm-private`, default channel GET
returns DAY1-9, and public package HEAD sizes match. Log:
`local/live/publish-day1-9-launcher.log`. Full public downloads and exact SHA256,
size and extracted package/channel identity PASS for both platforms; evidence:
`local/live/publish-day1-9/launcher-verification.json`.
Focused launcher distribution tests23/23 PASS. Full launcher suite754PASS/5FAIL
in unrelated WoW, RuntimeTruthfulness and SettingsRefreshDuplication tests;
no launcher code changed. Logs `local/live/launcher-day1-9-{tests,full-tests}.log`.
Launcher checks this channel at
startup; restart it to discover the update. User's actual launcher UI has not
been observed. Full Day1 work remains active.

For EVERY future launcher release, after private publication run:
```
cd /home/blizz/dev/banshee-realm-client
PE_LOCAL_PACKAGE_DIR=/home/blizz/dev/parasite-eve/pc_port/build-dist/dist \
  bash scripts/publish-parasite-eve-channel.sh BUILD_ID
```
Check the default public channel without a cache-busting query and validate
both public downloads against package hashes/metadata. Private dev.json
verification alone is insufficient. Publisher documentation now says so.

Private Banshee dev channel now **PE-DAY1-9-0c462bc18581**, replacing SEW22.
Includes DAY1-1 through DAY1-9: disc/music loaders, rehearsal particle/fan/
beam/flash effects with the bound caller context, transition asset loader,
and script76 actor turning. Full Day1 implementation remains in progress.
Both Linux and Windows packages include both discs and cue files; extracted
binary/disc hashes and cues verified, Linux wrapper120-frame smokePASS.
Normal/Release/ASan-UBSan CTest each5/5 PASS (60.79s/24.57s/113.27s),1216
native groups. Windows runtime crossbuild passed; execution remains untested.
Publisher93343 completed, full remote archive hashes/ranged signed GETs,
pinned+mutable channel docs and signed channel readback allPASS. Independent
rclone channel read confirms both platforms and exact package/binary hashes.
Summary: `local/live/publish-day1-9/published-summary.json` (no signed URLs).
Signed URLs remain private in that stage directory. This private dev.json
serves explicit operator overrides; it is not the launcher default channel.

## Previous launcher update PUBLISHED (2026-09-05)

Historical private publication only; the default launcher channel was missed
and corrected by the DAY1-9 public publication above.
Private Banshee dev channel now PE-SEW22-7fc95fa875ba, replacing
PE-SEW16-25673ab3c853. Both linux-x64 and win-x64 include BOTH discs and
cue sidecars. Normal CTest2/2 PASS70.23s, Release2/2 PASS33.99s,
ASan/UBSan2/2 PASS114.66s;1210 native groups. Windows crossbuildPASS,
Windows runtime not executed. Linux extracted wrapper boot120framesPASS;
both extracted discs hashed against package metadata, cue files present.

Linux archive592459551bytes SHA256b336b27b39891f667c68e9d70627ed9f07e5b12ee1df57d204bc73789ec2ac8d;
Windows592516977bytes SHA25605cd8b7d6ea0a8a0950695efbfe12f8eb783e0f056fc71225894819b70df1000.
Publisher62019 finishedPASS: full remote SHA readbacks, signed rangeGET206,
pinned+mutable channel documents and signed channelGET. Independent rclone
channel recheck alsoPASS. Summary `local/live/publish-sew22/published-summary.json`.
Signed URLs stay private in that directory, never print/commit. Explicit
private channel overrides continue to serve latest dev.json;
release source is dirty worktree atcommit7821e7c3, identified by binary hashes.

SEW21 restoresEA205/206/207 music-volume/fade queues and signed channel
helpers6DB48/9C/E0, producers86C5C/86CA4:69 original/native casesPASS.
SEW22 restoresE2/1A390 embedded polygon wrapper, used byM0023I script801D90F4:
48 original/native casesPASS including fullVM. Both are native translations,
not matching-C claims. Original oracle scripts and native tests are tracked
worktree files. Subsequent DAY1-1 source changes below are not in this release.

LATEST RUN TERMINAL: run25 used DAY1-7 after normal and sanitizer suites passed.
gdb2251551/game2251590; binary SHA256
94b314ef4c7df72beecb62a4daef4578e2b1bc433c3609e242e22ec9856796a1.
Opening helper25719 completed normally: first Eve13720→15580, control16630
HP18. Normal inventory Medicine1 heals18→45 at19400; screenshot
`local/live/sewers25-healed.png`. Helper10857 completed: rat defeated26240,
fieldcontrol26390 HP34/g74=48; TheaterKey popup31550, confirmed31670.
Helper73785 completed: RehearseKey popup38720, confirmed40210; diaryexit41770,
rehearsal door43890, room44020. Scene helper69751 triggered Eve48290 and
the game stopped49234 HP34/g74=5B, tokenA80021C8. New boundary: script76,
func80014BA0 at801D98E0, actor800BF210, args80120F80. The previous M0023I
effect boundary was passed. Stop RAM and backtrace under `local/live/sewers25-stop-*`.
Both gdb/game exited normally; all run25 input helpers are terminal. The
last helper saw xdotool window-not-found after the game closed; this is not
a surviving input job. DAY1-9 below restores the reached turn command.
Logs: `local/live/sewers25-gdb.log`, `sewers25-replay.log` (JSON milestones),
`sewers25-replay-keys.log` (helper stdout/errors); state/capture prefix
`local/live/sewers25`. The next live replay needs a fresh prefix and the
DAY1-9 binary after verification. Orca status
and capabilities were rechecked: game absent from list-apps, screenshots
unsupported; existing normal keyboard/read-only capture fallback is in use.

Day1 transition audit expanded to27 room transfers/12 chunks plus the exact
6ECEC loader ranges and253-word8019234C frame-loop identity. The loaded
overlay isLBA2805/183sectors, SHA256c51e36c27422e990d4683d73dc9fc2633e0924721dd0c242a8efc2e8520a4edb.
Twelve original80192030 selector-prefix executions proveg74=80→M0351I
(g74=78→M0036I). M0351I's inspectedscript writes88→M0042I. This extends
the terminal-route evidence, not native/live completion. Reproduction:
`python3 pc_port/tools/pe_day1_route_audit.py > local/live/day1-route-audit.json`.

DAY1-8 VERIFIED: full214-word6ECEC transition loader translated and connected
to1220C, with nested-stop guards before the loaded overlay is called. Original
execution checks48 provider-contract cases/14064 calls; native traces and
RAM match. Covers both banks, issue failures, pending polls, timeout/retry,
ignored XA return values,265 TIM uploads, retained image base for the first
two reads and a changed/reread base for the executable read. These are explicit
CD/TIM/SDK contracts, not real loaded-overlay acceptance. Full normal CTest5/5
PASS47.83s, sanitizer5/5 PASS55.92s. The8019234C frame loop remains unported.
No DAY1-8 game executable was built; run25 retained its DAY1-7 binary.

DAY1-9 VERIFIED ROUTINE: script76/80014BA0 full128-word actor turn translated and
wired toVM. Original/native520 cases/1560 VM frames passed; live-stop copy
returns0 and matches all2097152 RAM bytes. The VM fixture excludes its native
temporary argument vector (original uses CPU stack). Source `func_80014BA0_port.c`, oracle `pe_script_turn_oracle.py`,
test `test_script_turn.h`. Preserve signed halfwords, raw heading stores,
direction tie at2048, exact4096 wrap test, task flags20 and VM retry state.

LATEST USER PUBLICATION COMPLETE (2026-09-06).
Release identity PE-DAY1-9-0c462bc18581. Linux Release SHA256
0c462bc18581bf1371c078f0aaa0b0cb8bc01ad4340e02958dacd2b3793c85ad;
Windows SHA256eae202712962f152cb914c007752cd7bcc1317354b406f58cf1e7ea8b455f947.
Normal CTest5/5 PASS60.79s; Release5/5 PASS24.57s; sanitizer5/5 PASS113.27s;
1216 native groups. Both packages finished and verified: both extracted
discs/cues/binaries match their metadata; Linux wrapper120-frame smokePASS.
Linux592477584bytes SHA2565ccc0363f79e17f3e9ab33f0923b2d938b8096556d31a5a5f6fbfb676d025101;
Windows592504338bytes SHA25638096d4e392614ce64b06cc23b668f14c65797113b5a7a405380a6f9655476f0.
Publisher93343 completed using pe_publish_runtime.py, stage
`local/live/publish-day1-9`; output `local/live/publish-day1-9-output.log`.
Published-summary and independent remote channel read both verifiedPASS.
Prepared verifier: `python3 local/live/verify_day1_9_package.py linux-x64`
and `win-x64` (run after the corresponding packaging process completes).
Keep signed URLs private. Publication authorized; do not ask again.
Windows runtime target crossbuildPASS via dockcross/windows-static-x64;
its full default build hit the existing POSIX-only native test sys/wait.h
error, so rebuilt only parasite-eve-port (log build-win-day1-9-runtime.log).
Windows runtime execution remains untested. Do not claim all-Day1 completion.

PREVIOUS RUN TERMINAL: sewers26 launched during package compression, using the
final DAY1-9 normal binary SHA256
de225166e5cf6474c7b730868162a78086e0aeb271da168d8b9238048c486344.
gdb2351372/game2351425 both exited normally. Helper48410 completed firstEve
11120→12700, control13720 HP31. Prefix `local/live/sewers26`;
milestones `sewers26-replay.log`, stdout `sewers26-replay-opening.log`.
Menu was opened (screenshot sewers26-menu.png shows Use Item), then normal
Return input reached Arrange Items (sewers26-stop.png), an unported page.
Stop14689 HP31/g74=28, fn80046DFC via43DA4(window800A22E0,event10000).
No medicine consumed; subsequent Down input failed because the game window
had closed. All input helpers terminal. Stop RAM/backtrace/state:
`local/live/sewers26-stop-*`. The Arrange Items boundary is resolved by DAY1-10 above.
Audio EA200/201/203 was inspected but not edited. Publication is complete;
fullDay1 remains active.

PREVIOUS RUN TERMINAL: run24 exited normally at an unresolved effect callback,
not defeat. No game or input helper remains active. STOP frame50516, HP34,
M0023I tokenA80021C8, g74=5B, callback8018F3C8 mode0, data80186230,
extraFD6DFB9D. Backtrace: PE_EffectCallback→D401C→D413C→6F9F0→69594.
Use `local/live/sewers24-stop-ram.bin` and `-stop-state.json`; latest RAM
can be overwritten during shutdown. All helpers80511/52129/93457/86424/
55322/62638 are terminal. gdb1973172/game1973210 exited normally.

Run24 used DAY1-1 (not DAY1-2): first Eve13450→15150, control16170 HP18;
Medicine1 normal Use Item heals18→45 at18730. Rat25210, fieldreturn25340
HP34/g74=48. TheaterKey20030740/30840; RehearseKey20137580/37680.
Diary-table exit(0,0) obstructed; recovered normally via(300,200),(300,-300),
now included in rehearsal_room replay. Diaryexit45210, finaldoor47300,
rehearsalroom47440, Eve trigger49620. Advanced past prior E2 stop into the
new effect boundary; last script cursor801D928C. Complete encounter unproven.

DAY1-3 IMPLEMENTED: M0023I callback0, its particle and descriptor command.
`m0023i_effect_port.c` translates main8018F3C8..8018F710 (210 words) and
particle8018F004..8018F3C8 (241 words), all modes and real shared callees.
Dispatcher identifies overlay code to avoid confusing M0013I's F004. Added
11-word90644 command (mode1 sets descriptor+12=1, returns80190758).
`pe_m0023i_effect_oracle.py --write-header`:167 complete original cases,
covering init, all states, random seeds, full pool, lifetimes, mixed ribbon/
sprite packets, both GPU banks, culling and complete pool update/draw calls.
No oracle callee substitutions; only existing undefined GPU packet bytes
masked. Original code hashes are pinned. Native focused group passes;
1211 native groups total. Final Normal4/4 PASS46.44s, ASan/UBSan4/4 PASS54.19s, including all167
original effect cases. Suites33870/15062 are complete; logs
`local/live/ctest-day1-3-final.log`, `ctest-asan-day1-3-final.log`.

Live COPY initialization proof: originalF3C8(mode0,data80186230) andnative
PE_EffectCallback both return252 and match EVERY RAM byte below801F0000
(original stack excluded). Ignored `m0023i_native_live.c`,
`m0023i-{original,native}-live.bin`. No gameplay RAM writes. DAY1-3 source
and debug/ASan builds are not published; no game or replay is running.

DAY1-4 IMPLEMENTED: shared D004C Gouraud triangle fan (439 original words),
with signed alternating radii, original matrix order and GPU packet behavior.
`pe_effect_fan_oracle.py --write-header`:74 original-execution cases pass;
native group DAY1_retail_effect_fan passes (1212 native groups total).
Normal CTest4/4 PASS49.76s; ASan/UBSan4/4 PASS53.98s. Evidence:
`local/live/ctest-day1-4.log`, `ctest-asan-day1-4.log`,
`oracle-day1-4-fan.log`. DAY1-4 is not published or verified in a live replay.

DAY1-5 IMPLEMENTED: M0023I beam callback8018FC14..80190644 (652 words).
All modes now run natively: joint initialization, four growth/fade states,
collision notification/latch, color curve, sprites, both fans and three mesh
layers. Original SDK785D4 matrix order and IR saturation are preserved.
The code-guarded M0023I dispatcher now routes FC14 to PE_M0023I_Beam.
`pe_m0023i_beam_oracle.py --write-header`:187 complete original cases pass,
with real collision/GTE/mesh/fan callees and no substituted formulas. Coverage
assertions establish10 consumed hit latches and all four mesh packet formats.
Cases cover all states and timer boundaries/rollover, notification disabled,
spent latch, both GPU banks, culling, signed scales and matrix saturation.
Focused native group passes;1213 native groups total. Normal CTest4/4
PASS52.13s; ASan/UBSan4/4 PASS55.98s. Logs:
`local/live/oracle-day1-5-beam.log`, `native-day1-5-focused.log`,
`ctest-day1-5.log`, `ctest-asan-day1-5.log`.

Actual stopped-room COPY proof: original FC14 modes0→1→2 and native dispatch
both return0 and match EVERY byte below801F0000 after each mode, mapping the
oracle's low0x400 bytes to PS1 scratchpad. Actual mesh8019778C is found in
the original package; drawing allocates1684 bytes. Data uses an explicit
48-byte test buffer801EF000; GTE projection controls are explicit fixture
inputs, not a live register capture. Evidence:
`local/live/m0023i-beam-live-comparison.json`,
`m0023i-beam-live-{input,original-0,original-1,original-2,native-0,native-1,native-2}.bin`,
`m0023i_beam_live_probe.py`, `m0023i_beam_native_live.c`.
No gameplay RAM writes, live replay or publication occurred. DAY1-5 debug
and sanitizer builds pass; launcher remains SEW22.

DAY1-6 CALLBACK TRANSLATED; CALLER BINDING OPEN: original321-word F710
now has PE_M0023I_Flash(mode,data,retained_position). Its six borrowed stack
bytes are explicit input/output, preserving the first ring's original
position and the later joint-position write shared with the next flash.
All modes/state transitions, two sound requests, sprites, ring and fan have
native translations. Production dispatch calls with no retained context and
stops explicitly on mode2/state0; it must not invent that position.
`pe_m0023i_flash_oracle.py --write-header`:173 complete original cases pass,
including12 paired-flash cases, timer rollover, signed scales, both GPU banks,
culling, arbitrary retained positions and found/missing sound resources.
Native comparison includes all six borrowed bytes and defined geometry.
The explicit unresolved-context boundary is tested. Normal CTest4/4
PASS52.89s; ASan/UBSan4/4 PASS58.44s;1214 native groups. Logs:
`local/live/oracle-day1-6-flash.log`, `native-day1-6-focused.log`,
`ctest-day1-6.log`, `ctest-asan-day1-6.log`. No live run or publication.

DAY1-7 IMPLEMENTED: original F710 stack writers are now connected in the
69594 effect pump. The scheduler tracks six host bytes plus a validity mask:
F3C8 update's CE934 zero store establishes Z; F004 draw's F014 saved RA
establishes X/Y; each flash writes its completed joint position for the next
one. Paused/skipped-update frames retain the previous second flash's Z.
RAM generation changes and unknown call contexts invalidate the values;
unknown state0 input still stops explicitly. No constant-position fallback.

`pe_m0023i_pump_oracle.py --write-header`:84 original/native frames across
six histories, two particle pools, paired flashes, both GPU banks and pause/
resume via flags4/100. GPU arenas/OT are fresh fixture outputs each frame;
effect state and the original CPU stack persist. The boundary on a fresh
RAM generation is tested. Normal CTest4/4 PASS52.84s; ASan/UBSan4/4
PASS58.78s;1215 native groups. Logs `local/live/oracle-day1-7-pump.log`,
`native-day1-7-focused.log`, `ctest-day1-7.log`, `ctest-asan-day1-7.log`.

Original-field trace now completes35558 with moving actor poses, including
paused frames, and confirms those last-writer instructions. Test interpreter
adds SQR, shifted OP and calculation flags for its supported GTE commands;
strict flag reads reject unimplemented results. `test_gte_oracle.py`:17 ISA
checks PASS. All173 flash and187 beam original return/hash pairs remain
unchanged; HUD oracle also passes. No guest CPU interpreter is added to the
native game. Source: PSX-SPX GTE hardware reference linked in stack notes.

Actual-room COPY proof:64 complete69594 frames using run24 data, original
script and actual meshes match native returns, state and every defined
non-stack output byte. Includes paired flashes, pause/resume and beam phase.
Only established unused GPU fields are masked; all coordinates are compared.
Projection/PRNG and fresh GPU output arenas are explicit fixture inputs;
actor pose is fixed in this pump copy. `local/live/m0023i-pump-live-comparison.json`
records64 zero-difference rows. Ignored scripts:
`m0023i_pump_live_probe.py`, `m0023i_pump_native_live.c`,
`compare_m0023i_pump_live.py`. No gameplay RAM writes. Launcher remains SEW22.

NEXT: continue run25 normally; the rehearsal effect graph is ready for live
verification. Descriptor80190720 has0=F3C8(done),1=F710(bound and tested),
2=FC14(done); sizes16/8/48.
Descriptor+30=90644(done);+34=90670 is BYTECODE, not another native callback.
Original F710..FC14 (321 words), SHA256
6bee6882d0a259e2d326e4bd70941ff28d6622266e24d4f098dcb6cf5cc228aa;
FC14..90644 (652 words), SHA256
7daa5e3bc20015de550bd7cab4a073d4e0b5ddb2f65ff52a14e9955025fb7458.
Both match independently extracted M0023I atLBA13904/172sectors. Full
per-word disassembly `local/live/m0023i-other-effects.txt`.
F710 now has native D0728/ribbons/sprites/sound and D004C available.
Its mode2/state0 early D0728 call passes stack+30 before CE8F0 fills that
vector later. Original-code probes confirm that old stack contents change
defined vertex coordinates and ordering bytes. Original D4704 does not
initialize the vector; preceding particle calls overwrite some of its bytes,
while others retain historical stack contents. Evidence:
`local/live/m0023i-stack-probe.json`, `m0023i-caller-stack-probe.json`.
The tested first triangles were degenerate; visible pixel differences are
not established. Recover faithful stack behavior; do not invent a zero or
owner position or mask defined coordinates to get a passing test.
Original-pump trace identifies the actual writers: F004's saved return
address800CE818 supplies X/Y; the prior F3C8 update's CE8F0 zero store
supplies Z in this fixture. The script creates TWO flashes: the second reads
the first flash's completed joint position. Both initial stack fills00/A5
give the same32 traced calls across fixture frames13..20. This is an isolated
69594 pump with a copied actor pose. DAY1-7 extends this with the original
35558 field loop and paused-frame evidence, as recorded above.
See `docs/ai_context/M0023I_FLASH_STACK.md` for exact stack offsets, writer
instructions, reproduction/evidence paths and integration scope.
FC14 is verified above.
After the room graph is tested, replay normally from run25, heal after first
Eve, continue both keys and rehearsal/sewers. Full Day1 remains the objective.

Run23 is historical defeat, not rat-victory evidence. Replay now checks HP/
defeat mode before accepting a cleared battle flag. Orca status/capabilities/
list-apps were rechecked: raw X11 game absent from accessibility enumeration,
screenshots unsupported; existing normal xdotool input/read-only snapshots
remain the documented fallback. No gameplay RAM writes were used.

RUN22 TERMINAL: healed17→45 viaMedicine1 normalinventory, consumed6;
diaryentry47080,key201popup50860,confirmed50920,exit52470; finaldoor54620,
rehearsalroom54730;Evescene56620. At57481 M0023I g74=5B, HP45, reached
unportedE2/8001A390 at801D90F4 and exitedunresolved-boundary. This proves
SEW20 B0 passedlive. Run22 binary wasSEW20 soE2 wasexpected; currentSEW22
buildalreadycontainsit. Helper74343 and82912 finished. Use
`sewers22-stop-ram.bin` for intact boundary evidence.

SEW22 liveCOPY proof: executeoriginal1A390 onstopRAM withargs80120F80;
result changesonly800BF314 from0 to1. Nativecopy matchesALLnon-stackRAM
below801F0000. `script_polygon_native_live.c`, `script-polygon-{original,native}-live.bin`
areignored inlocal/live. No gameplayRAMwrites. Rehearsal phases now freshly
verifiedlive, bothkeyflags/inventoryconfirmed. Run24 subsequently advanced past the prior E2 stop into the effect callback above.

DAY1-1 FULL DISC-LOADER CONTROL FLOW IMPLEMENTED, TESTED, NOT PUBLISHED:
`func_8006CDA4_port.c` now preserves the original internal loop, empty-archive
completion, read-error retry, upload/poll transitions and blocking/yield flag.
Removed the old one-state dispatcher and five obsolete cut helpers.
`pe_disc_loader_loop_oracle.py --write-header` executes the complete original
181-word routine:336 cases,1198 provider calls, with explicitly controlled
CD/SPU provider return contracts. `pe-disc-loader-tests` compares production
control flow against those original results. This proves control flow under
those contracts, not full hardware/audio provider equivalence.

The original empty-archive counterexample now matches all non-stack RAM below
801F0000. Legacy caller fixtures now represent actual pending/completed I/O;
corrected the old oracle's mislabeled zero-remaining/read-error branch.
Normal CTest3/3 PASS44.77s, ASan/UBSan3/3 PASS49.94s, including1210 native
Groups and336 loader cases. Commands: `ctest --test-dir pc_port/build
--output-on-failure` and the same for `pc_port/build-asan`.
Logs: `local/live/ctest-day1-1-final.log`, `ctest-asan-day1-1-final.log`.
Only native debug/sanitizer builds contain DAY1-1; Release, Windows and the
published launcher packages remain SEW22. Full live loader verification and
remaining audio providers are still open.

DAY1-2 MUSIC-BANK LOADER IMPLEMENTED; NOT YET WIRED TO EA START/STOP:
`func_8006D2B8_port.c` translates the complete original214-word routine,
including bank lookup, signed channel IDs, existing/busy channel return,
unloading both matching slots, bank copying and internal retry/blocking.
`pe_music_bank_oracle.py --write-header`:864 original cases,216 controlled
provider calls; `pe-music-bank-tests` compares production code's calls,
return and RAM. BIOS memcpy is an explicit contract. This does not prove
CD/SPU providers or audible playback. Normal CTest4/4 PASS52.48s; ASan/UBSan4/4 PASS58.24s. Logs `ctest-day1-2.log` and
`ctest-asan-day1-2.log`. DAY1-2 is not published or in run24.

RemainingEA200/201/203 call6D2B8(original214words, `ea-music-loader.txt`),
now translated as DAY1-2; its callees are6CDA4,86FF8,BIOSmemcpy71A34. The restored6CDA4 loop still depends on provider completion contracts;
wiring EA start/stop alone would not prove complete audio behavior. Validmusiccommands10/12/19 in8CBA8 currently
onlyvalidateheaderandreturn; originalqueuespayload andcalls8CB54 forbankmode.
Fulloriginaldisassemblies `music-command-producer.txt`, `music-command-callees.txt`,
`music-mode-full.txt`; graph8CB54→8D7C0,85A64,8CF70→85BB4,8D140,8D610,7DAE0.
SPUmode/voicehardware paths are stilldeferred perpe_stream.c. FullDay1audio
remainsunverified, explicitlytracked inDAY1_RETAIL_ACCURACY.md.

ROUTE AUDIT: `pe_day1_route_audit.py` verifies25 inspected opcode31 transfers
across11 original room chunks. See DAY1_ROUTE_AUDIT.md; alternate exits are
not yet live-verified. M0036I g74=78 branch writes80 then tokenA8000048
(M0000I sentinel). Dispatcher1220C calls6ECEC and8019234C; both are still
bootstrap stubs. Do not infer Day1 completion from reaching this transition.
Original6ECEC begins XA bank204 blockingload then overlayload.
Run24 executable SHA256 from /proc/1973210/exe (deleted after rebuild):
9f0c85f5f25ad6712d9b99a5aa592b656727aee7f01e9035ce6f2ea899a97214.

## Continuation: through the END of the sewers (2026-09-05)

SEW20 IMPLEMENTED/TESTED; see CURRENT LIVE above for run22. Run21 exited normally at
frame80464 with unresolved VM routine800198C4 at801D8F8C, Eve actor800BF210,
body800A5D5C, tokenA80021C8(M0023I), g74=5B, HP45. Use
`sewers21-stop-ram.bin` for intact actor state; shutdown overwrote latest RAM.
Run21 obtained both keys, healed, entered true rehearsal/piano roomM0319I
at73150, then triggered Eve dialogue76450 and battle-room transition.

SEW20 ports original198C4 and2FAD8 into func_8002FAA4_port.c and wires VM
opcodeB0. It writes two full32-bit timing values into the indexed actor attack
record.45 original-execution cases/native PASS, including complete17018 VM
calls and byte-index truncation. On a COPY of run21 stop RAM, original and
native change only800A5D7C and800A5D80 from10000 to20000; all RAM below
801F0000 matches. No gameplay RAM writes. Normal CTest2/2 PASS47.64s;
ASan/UBSan2/2 PASS50.87s,1208 native groups. Logs `ctest-sew20.log` and
`ctest-asan-sew20.log`. Oracle `pe_attack_timing_oracle.py`.

Run22 launched gdb1824084 with this tested binary. Initial replay64085 later
stopped on a navigation assertion; recovery/current helper recorded above.
Logs `sewers22-replay.log`, `sewers22-driver.log`. Next: heal via
visible inventory if needed, then replay fromdiary_room throughrehearsal_room,
trigger Eve dialogue, verify B0 passes live and continue through sewers.
New replay diary_room/diary_key/rehearsal_room phases follow proven run21
inputs but need a fresh replay verification. Do not restart for a timeout.

SEW19 source: EA sound keys300,350,351,352,353 execute original room sound
lookup, attenuation, actor/explicit position, projection/FIFO and output handle.
64 original cases/native PASS; normal2/2 PASS66.35s, ASan2/2 PASS80.17s.
Other EA music cases remain partial, including200/201/206. Disassembly
`ea-full.txt`, `ea-music-loader.txt`. ED2100 is an ORIGINAL no-op.
Run21 used SEW18 binary; run22 is the first live replay with SEW19/20.

Run21 RehearseKey201: door7/M0018I diary, entry48810; approach(300,-300),
(0,0),(-181,223),Up120ms thenCross. Popup57870,capture61630,g24=010C0220,
inventory201+200,HP45; `sewers21-rehearse-key-proof-*`. Diary exit via(0,0),
(750,-400),(1000,-400), corridor66900. Final door unlock thenM0319I73150.
All run21 helpers finished; run21 game exited at explicit boundary above.


SEW18 + SEW17 LIVE PASS: run21 acquired Theater Key200 through normal
body approach / Cross input. Body examination28390,
popup30270 (captured30870), g24=01000220, inventory800C0E48 contains200.
`sewers21-key-popup.png`, `sewers21-key-proof-*` capture the popup and RAM.
Confirmed popup, then used Medicine1 via inventory:HP10→45, item6 consumed,
key200 retained. Closed menu, control35580, field4000000A.
`sewers21-key-healed-proof-*`, `sewers21-key-healed.png` preserve healed state.
At this milestone run21 was idle in key room, Aya425.679/179.748, HP45/45.
The current run state is recorded above.
No replay/input process active (53605,66431,36568 all finished). Run20 closed
normally with Escape; the key-up reported BadWindow after window destruction.
Do not restart run21 just for observation timeout. Next: exit key room,
enter door7/dressing room, recover RehearseKey201 and continue final door/sewers.
EA command omissions below still need recovery for faithful scene behavior.

New `actor_contact_port.c` translates full36448,35F54,1D268 and12774 and
wires contact/task retirement after1A4AC with original second D1A0&4 check.
Corrected29810 callback8002D268→8001D268 (signed low-half instruction).
`pe_actor_contact_oracle.py` and native test:214 synthetic full-original-MIPS
cases PASS. Full live-position copy comparison PASS: zero non-stack word
differences below801F0000 (`local/live/contact_native_live.c`). No RAM writes
to the game. Full normal CTest2/2 PASS47.96s and ASan/UBSan2/2 PASS63.38s,
1206 groups (`ctest-sew18.log`, `ctest-asan-sew18.log`). Only later C changes
are comments and SEW17's test label corrected toSEW17_field_pickup; no runtime
change since tests/live run. Replay now has `theater_key` phase from the proven
waypoints/input sequence; Python compilation passed.

Run21 earlier milestones: Eve12210→exit14090→control15210, HP21;
understage control19240; corridor rat won21800,control21950,HP10;
mirror entry22740,exit22830; theater-key room26100. No unresolved boundary.

SEW18 original live evidence: mandatory body interaction was blocked by
omitted actor-contact pass func80036448, proven on the live-position
snapshot. Run20 was captured in key room, Aya X395.5875,Z189.3808,HP28/45,
gdb1392495 (now exited). This was oldSEW16 runtime,
although executable on disk includes testedSEW17. Key room entry28020, token
A8002048, Aya801A08A0. Mirror exit proof above is complete.

`local/live/sewers20-contact-proof-*` and PNG preserve current body approach.
`python3 local/live/contact_live_oracle.py` executes ORIGINAL36448 on a COPY:
16 word changes in `contact-live-diff.txt`; allocates tasks8009D4C8(body script
801A1620) and8009D4F4(Aya script801A09F0), links actor+A4. Native keeps both
A4=0 because35558 skips36448. No gameplay RAM edits. Earlier farther positions
produced no original contacts either; positioning was necessary, now resolved.
Read `local/live/actor-contact-full.txt` (610words incl2 of nextfunc) and
`contact-task-tail.txt`. Full36448(608words),12774(55words),35F54(50words),1D268(54words) are
now translated and wired at original35558 sites35C1C/35C24. Corrected
29810 callback and BTL3 assertion: original lui8002/addiuD268 sign-extends
to8001D268, previously erroneously8002D268. Original
1D268 is at1D268..1D340, not2D268 (which is unrelated instruction middle).
Actor+194 nativecallbacks otherthanthis must be explicit boundaries if unknown.
Full contactwalk calls12700,35F54, and actor+194. Initial table915E0 entries0.

Next route proven from original disc table/scripts: theater key200 opens
corridor door7 at Z~-3500, tokenA8001448=M0018I, chunk2 LBA13089/48 sectors.
Its script8019CF24 gives Rehearse Key201 at8019D8B0, opens E8 at8019D8E8,
sets g24bit80000 at8019D8F4. This unlocks final corridor door10 tokenA80614C8
(M0319I chunk2 LBA78563/61). Door8 M0022I contains no key award.
Offline evidence ignored `key_route_scan.py`, `door7-full-script.txt`.
Diary script uses EA keys300(sound),206(fade),201(stop); existing15DAC
silently skips most non-default cases. These need original behavior recovered
before claiming that scene complete; source disassembly `ea-full.txt`.

Original SHA256 windows:
36448..36DC8 10e6b058a1eb554abf36c1def4c7b7f23aab0c4c6738fefbb65980e3043a19c2
35F54..3601C a399991f1b401026d752c1f1485df6b677898488df8fac49b6b59a004819ac2e
1D268..1D340 1d6b5fb202b307fff1746b739f662efe03610dbd170fe949ed412f8b6bf9161c
12774..12850 29fe15125ee3dc5a9d7c9578e7fd0da61e0bab15b24127e4838b620c0ac379c9

Room body: actor800BEF90,world179,241,contactcenter237,260; chair blocks direct
approach. Floor vertices verified; approach from RIGHT: X~585,Z~180, then
Left towardX395,Z189 (narrow gap between chair and desk). Script-trigger radius
~193; original contact task now generated atthisposition. First contact triggers
corpse examination, then Cross/Return (processedbutton100, NOT Square/S) gives
key viaA7/E8 afterg24bit200 examination. `theater_key_room` replay phase reaches
entry frommirror exit with no more battles this run. `pe_live_drive.walk_to`
now updates each chosen key's measured gain and checks stop_when while probing;
first-use gain cache previously stuck against furniture. PythoncompilePASS.


SEW17 IMPLEMENTED/TESTED: mandatory theater key in door9, tokenA8002048=M0020I,
chunk2 LBA13260, Aya script801A08A0. Its item200 award at801A0BA4 (A7),
dialogE8 at801A0BDC, then g24bit20. Original offline scripts in
`local/live/door9-script.txt`, `door9-offline-ram.bin`. Door5 is optional
g24bit4 event room, not needed for key route. New field_item_pickup_port.c
translates194B0/15BAC,532B4,4F490 constructors/draw/input,50204/5022C/51060.
Wired VM/input/draw. New pe_field_pickup_oracle.py:74 full original cases,
including inventory full, both dialog types, weapon/armor bonus slots, both
render banks, scripted yield/retry, confirm/cancel and event queue dispatch.
Normal CTest2/2 PASS1205 groups44.88s; ASan/UBSan2/2 PASS81.59s; final logs
`ctest-sew17-final.log`, `ctest-asan-sew17-final.log`. Tools30024/57934 done.
No checks pending unless new changes. Live key pickup now proven in run21 above.

Run19 EXITED23890 because blind healing Return selected retained cursor
"Arrange Items" unported46DFC. It is optional, not needed for key route.
Next healing MUST openV and inspect/select UseItem before Return; do not assume
cursor0. Run18 died from exec process cleanup; launch now nohup setsid with
stdin/dev/null and in a separate finished exec, so run20 survives replay end.
No gameplay RAM writes. Read-only inspector D280 now host scalar (old guest
copy was always0); run20 includes fix. Replay checks script change too.

Shared mirror code EFF4..FB72 is byte-identical in source M0017I chunk2LBA12868
and live M0021I chunk2LBA13469, SHA256
5cd8e8eb4ad71e0fd11c30c015f40c2ffdd98230d0fbc1b674a15e2324993fd1.
Earlier mirror oracle12868 is valid shared code; whole chunk headers differ.
The end-of-sewers objective remains active.


SEW15 LIVE PASS: run17 mirror constructor and four commands at29350 use
actual values, source type/id0/0, endpoints838/323 and838/-690, visibility1.
Control29400. Aya X462/Z-242 reflects atX1214, source800BED10,
dest800BF144; `sewers17-mirror-proof-*`, `-mirror-close.png` preserve proof.
Medicine1 via ordinary inventory healed7→45/45 and consumed one item.
No optional drawers opened. End-of-sewers objective remains active.

Opcode6B wrapper func_800187C0 now double-dereferences args2/3/4 as original:
previous pointer values caused run16 null mirror actor and wrong rat parameters.
Mirror oracle92 cases; M0013I120. Final SEW15 normal/ASan suites both PASS2/2,
1203/1203 groups, logs `ctest-sew15-final.log`, `ctest-asan-sew15-final.log`.
Live GDB tool captures signal backtraces/RAM as `<prefix>-crash-*`.
Replay aisle encounter uses g74>=0x28/battleflag (field0x2000 also mapchanges),
and stage/backstage waypoints stop on transitions; aisle stall timeout10s.
No gameplay RAM writes. Door5 ungated tokenA80013C8, X-700..-571,
Z-2027..-1874; most other doors need g24bit0x20, finaldoor needs0x80000.
Window resize/max fix live-verified on Linux, including actual maximized game;
Win32 implementation compiled only when a Windows toolchain is available.


Latest continuation: run 15 entered door action 6 at X~535/Z-1944 through
ordinary Left input. The suspected floor collision issue was a positioning
false alarm: original 1AE40 and native both allow X535 there; no floor edit.
The door loads token A80020C8, then stops at frame 74034 inside opcode 6A
(code 0x45), constructor callback 8018EFFC. Run 15 EXITED; final RAM and
state are `local/live/sewers15-stop-*`. CE00=801A2E24 (after opcode at
801A2E14), current actor 800BEF90. `mirror_effect_port.c` now translates
the dressing-room mirror overlay (Disc 1 sectors 12868–12869 SHA256
bff195a272f6b7fc19a4eba453c8ad6b138537b7dfa8c64a0da44c534777a77f).
87 original instruction oracle/native comparisons PASS; normal CTest 2/2
PASS (45.99 s), 1203 groups. ASan/UBSan CTest 2/2 PASS (51.46 s).
Live run 16 is in progress (`sewers16-*`, gdb PID 1183623).
Do not claim a live room pass yet. Run 16 ordinary-input stage-exit walk
continued into the hole scene before its waypoint timeout; terminated only
replay PID 1183624, released keys, resumed at `--from understage` (tool
session 49302). Replay stage-exit/backstage walks now stop when the room
transition takes over; the hole check uses field bit 0x2000, not the
actor idle bit which also clears during walking. Maximized game pixel
capture: `local/live/sewers16-maximized.png` (1280x909 client). Inspector now
also records a native backtrace on stop. Next: finish comparisons, rebuild,
replay through the rat and enter door 6, then explore for the keys.

User steering: expanding the window left the image at its initial size.
Fixed X11 ConfigureNotify handling with resized XImage ownership, shared
aspect-preserving arbitrary-size nearest-neighbor scaling, and Win32
resizable/maximizable style plus client-size buffer updates. Native build
PASS; actual X11 four-quadrant pixel checks PASS at 960x720, 1001x713,
713x1001, 240x180 and restored 640x480 (all image corners visible; bars black). Maximize to 1280x909 and restore
to 640x480 also passed actual pixel checks (`check_maximize.py`); Escape
closed the test window before xdotool sent its key-up (benign BadWindow).
Ignored reproducible fixture/check: `local/live/resize_smoke.c`,
`local/live/check_resize.py`; screenshot `local/live/resize-large.png`.
Win32 runtime was not tested on this Linux host. New binary includes fix;
existing running copies need a normal restart. Sewer objective remains active.


Current user objective extends the earlier "into the sewers" scope through
the end of the sewers, in story order, decompiling and porting boundaries.
SEW14: resumed the actual live run 14 from the post-Eve stage;
ordinary-input stage exit / hole / under-stage / corridor reached the same
M0013I main callback boundary 8018F20C mode 0 (frame 113537, g74=0x40).
Recovered its code directly from Disc 1 user sectors 12007–12008, SHA256
5be7c97af6b8fe3f51dc34350c6e16c2bc0a659e187ed7fd7aa5d6506ed063a1.
`m0013i_effect_port.c` translates main 8018F20C, particle 8018F004, D3F64
positional sound wrapper, C6B90 radius collision. 6DCE4 now forwards its
retail return value; CE9D4 has a host-vector output helper for stack locals.
Callback dispatch identifies M0013I's prologue before using its addresses.
`pe_m0013i_effect_oracle.py` executes the full original graph (117 cases,
no substituted callees), including successful sound handle allocation and
collision cleanup. All native comparisons PASS. Normal + ASan/UBSan CTest
2/2 each (45.16 / 58.44 s; `local/live/ctest-{,asan-}sew14.log`), 1202 native
groups. Live run 15 PASSED the old boundary: projectile screenshot/RAM at
frame 33190 (`local/live/sewers15-projectile*`), rat HP reached 0, battle flag
cleared at 33520, player control returned at 33670 (mode 9; Aya HP 21/45).
No unresolved boundary. `sewers15-after-rat.png` captures the result.
Run 15 uses `local/live/sewers15-*`; replay tool session launched from boot.
Existing Eve fragment investigation and all later sewer work remain open.
Replay now stops an aisle walk when the scripted encounter takes control
(formerly it waited 90 seconds for an unreachable waypoint during dialogue).
Inspector now reads HP/max HP/AT as the actual B8A2C/B8A3C/B8A30 halfwords,
and records actor type/id/script. These tool edits apply on the next launch;
run 15 still uses the earlier inspector. Python syntax checks PASS.
Route research from the retained field scripts: Aya base 801A0BC0, controller
801A63C0. Far door action 10 (at X-4/Z-6518) requires g24 bit 0x80000 and
loads token A80614C8; script 801A2918 tests the flag, otherwise message 0x75.
The key must be obtained through normal room exploration. Next: continue
Up from the post-rat corridor and inspect doors/rooms. No gameplay RAM edits.

## Active goal: 100% retail accuracy from boot into the sewers (2026-09-05)

User steering 2026-09-05: "make it a goal to ensure everything is 100%
accurate into the sewers". The goal now extends past the first Eve battle:
every field, script, menu, battle and transition from New Game through the
Carnegie Hall aftermath and onward until the sewers must match retail. Work
it by driving the visible app forward with ordinary input, stopping at each
native boundary, restoring the original code with full original-instruction
oracles, and recording proof. The prior Eve-battle/naming goal below is a
sub-goal of this one and stays open where noted.

SEW1 (2026-09-05, after a host reboot at 13:51 wiped /tmp and every live
process/helper): durable live tooling now lives in the repo, so nothing
depends on /tmp any more:
- `pc_port/tools/pe_live_gdb.py` — gdb Python breakpoint on
  PE_Port_InvokePresentHook; every N frames writes `<prefix>-state.json`
  (frame, D1A0/D28C/D1F0/D244, actor list with +0x190 identity and 16.16
  positions, message records) plus `<prefix>-ram.bin` and `<prefix>-rgb.bin`.
  Read-only; the handler returns False so the game never pauses.
- `pc_port/tools/pe_live_drive.py` — xdotool keys to the game window, state
  readback, PNG from the RGB dump, position-feedback `walk_to`.
- `pc_port/tools/pe_live_replay.py` — ordinary-input replay: opening →
  naming → courtyard → lobby → auditorium → Eve battle → stage exit →
  backstage hole → under-stage scene. Dumps go under git-ignored `local/live/`.
- `local/pe_disc1.path` now points at the Disc 1 image so the disc-gated
  B54KY test runs (it was missing in this tree, not removed by the reboot).
Run 1 (old binary) proved with ordinary keys only: naming, courtyard walk,
lobby, chandelier/performance, retail's own scripted aisle run to the stage
(no seat-route helper needed), the complete scripted Eve encounter (mode 12
exit, messages 50–60, control back at X345/Z633), stage-right exit to the
backstage scaffold map with the burned hole, the hole trigger and the
under-stage Eve+child scene. That scene FROZE: Eve's script task at
801A074C executed opcode 0x5D (0x800182E0, unported) — the VM fallthrough
returned 0 without rescheduling, leaving the task with delay 0 (suspended),
while Aya's script polls m19 (0x800B6ACC) that Eve's script sets at 801A07AC.
Fix in func_80017018_port.c: opcodes 0x5C (182C0: BCF88 |= 0xC0), 0x5D
(182E0: actor+0x20 = *arg, both from matching src/ C) and 0x93 (190BC:
3C5D8(actor+0x1B4,(s16)arg); actor+0x250 |= 2; if camera target also
3C5D8(B0CEC,arg) and B0D88 |= 2; decoded from the retail EXE words) are
wired, and ANY remaining unported table slot is now an explicit
PE_PORT_STOP_UNRESOLVED_BOUNDARY that retains the opcode PC and prints
`[VM] unported script opcode fn=… pc=… actor=…`. Native tests
SEW1_opcodes_5c_5d_93 and SEW1_unported_opcode_is_explicit_boundary added.
Normal build: `Results: 1189 run, 1168 passed, 1 failed, 20 skipped` before
the disc path file existed (the one failure was B54KY missing
local/pe_disc1.path). Sanitizer suite and run 2 (new binary) in progress.
Script decoder note: each script instruction is TWO header words (opcode
word, second kinds word) then argc×4 argument words; `local/live/sdis.py`
and `tasks.py` are throwaway decoders using that layout.
SEW2: 116 of the 241 script table entries were unported. Every leaf with
self-contained matching src/ C (or an existing native callee) is now wired
in the dispatch: F0 16FE0 (`*a0 = !(D2E8&1)`, decoded from EXE words), 19,
25, 26, 27, 29, 2B, 2C, 3E, 46, 47, 49, 4D, 69, 8A, 4C/8D/8E/90 return-1
twins, 99, CA, and the actor+0x98 / +0x250 flag leaves 37/39/42/50/78/B9/BA/
C2/C8/C9/BE/C0. Leaves whose callee is still missing natively (3746C, 665A0,
6F820, 67678, 659F8, 65A60, 65A9C, 1ACE0, 375B4, 375C4, 676CC, 67730, 37454)
stay explicit boundaries: 23, 48, 6D, 6E, 72, 7C, 7D, 7E, 7F, AF, B4, B5,
BB, BC, D0. Test SEW2_leaf_opcodes. Normal + ASan/UBSan CTest 2/2 each at
1,190 native groups (37.65 s / 47.98 s; local/live/ctest-*.log).
Run 3 finding: with F0 correct, retail gives the player CONTROL at the top
of the auditorium aisle (X-2049 Z7468) after the performance; the automatic
aisle run seen in runs 1–2 was an artifact of the old silent-yield path.
pe_live_replay.py gained an `aisle` phase using the INV22 seat route
(-1790,3420)→(-1790,3100)→(-2490,3100)→(-2490,2500)→stairs→stage and a
`--from PHASE` switch; the full ordinary-input replay then completes the
Eve fight, stage exit, backstage hole and under-stage scene with control
returned (frame 81220) and NO boundary. Walking Up from there enters the
next map (Aya X293 Z3948, story var g74=0x800A7918 is 0x39) whose
room-controller actor (first in the D20C list, script base 801A63C0) polls
`F0 a7 = !(D2E8&1); 09 s0 = (a7==0); 05 jump if !s0` at 801A6460 — it
proceeds only once D2E8 bit 0 is CLEAR — yet its a7 stays 0 and the
presented frame never changes (screen frozen on the under-stage Eve view).
Run 4 is armed with the inspector's new hardware watchpoint on D2E8
(PE_LIVE_WATCH) and per-visit task tracer (PE_LIVE_TRACE_TASK) to find who
sets bit 0 / whether the controller task is visited at all.
CORRECTION (run 5): the "frozen" corridor map was a misdiagnosis. The
inspector now also dumps VRAM (`<prefix>-vram.bin`, `pe_live_drive.py
vramshot`), and it shows the backstage corridor fully drawn in both frame
buffers; the first screenshot was taken during the fade-in and the identical
frames afterwards were Aya standing still. The controller loop at 801A6460
is retail's per-frame camera-zone poll (op 5E reads Aya's position into
a5/a6/a7; a7 = her Y = 0 on that floor), not a stall. BCF88 bit 0x40 is
cleared by op 5B in that map's own script (801A2E70 …) and restored by 5C
on the other camera branches; the map draws regardless.
SEW3: `pc_port/game/boot/func_800659F8_port.c` adds native 659F8/65A60/
65A9C (container+0x10 16-byte records), 67678/676CC/67730 (container+0x14
56-byte records), 3746C (close message record by id), and the matching-C
37454/375B4/375C4; opcodes 23/72/7C/7D/7E/7F/B4/B5/BB/BC/D0 are wired
(test SEW3_container_leaves). SEW4: opcode CB (19C4C, heading to a point
via 79FB4, normalized against actor+0x3A) wired (test SEW4_heading_opcode_cb)
after the first backstage-corridor rat encounter stopped on it at
801981E8 (Aya's battle script). Battle rendering in the corridor is correct
(local/live/sewers5-stop.png: Aya vs mutated rat). Still explicit
boundaries: 48 (665A0 camera, contains GTE ops capstone won't decode),
6D/6E (6F820 progress-flag table), AF (1ACE0), and ~80 other table slots
with no matching C; each is ported when the route reaches it.
Replay gained a `corridor` phase (Up through the exit door, then Up along
the corridor fighting random encounters with a generic Cross-tap `fight`).
SEW5: 6F820 (progress-flag records: D_800942E4 + 2572*i for i<0xB, else
D_800942E8 + 268*(i-0xB); kind clamp 0x55 via D_800942E0; -13/-15 errors)
ported in func_800659F8_port.c; opcodes 6D (write) / 6E (read) wired.
SEW6: 1ACE0 (walkmesh placement: 22-byte flat / 28-byte sloped records at
mesh+0x1C, height from D_8009CE08 table or the D1D8 plane via 3708C, then
1C614; +0x40..48 mirror) ported; opcode AF wired. SEW7: opcode 45 (heading
toward an actor found by type/id or D254; -1 when absent) shares the CB
math. SEW8: new `pc_port/game/boot/func_80013988_port.c` — opcodes 18
(terminate task by id), 92 (dest scale-fade wait through 3C5D8, busy bit 4
of actor+0x250 / B0D88), A5 (wait for camera phase 0/4), C3 (wait while
actor+0x98 bit 3), DB (walk to point with turn rate: latches target in
task+0x14/18/1C, velocity = -speed*(sin,cos), short-arc turn, arrival by
|v|^2 >= |d|^2 then snap), and 48 = 665A0 camera pan (RTPS through the
port GTE, view-record clamp, BCF9C/9E targets, phase 3). Tests
SEW5_progress_flag_opcodes, SEW6_walkmesh_place_opcode_af,
SEW7_heading_to_actor_opcode_45, SEW8_wait_opcodes, SEW9_walk_opcode_db,
SEW10_camera_pan_opcode_48. Unit RAM has no sine/atan tables, so SEW9
checks self-consistency against the leaves. Normal suite: `Results: 1198
run, 1177 passed, 1 failed (B54KY cwd), 20 skipped`. These six ops were
found by scanning the corridor battle map's scripts (sewers6-stop dump)
for unported table entries — every op those scripts use is now wired.
Runs 6 and 7 each stopped on the next unported op in Aya's corridor
battle script (45 at 801982E8, then DB at 80198508); run 8 carried all of
them and fought (HUD 18/45 visible, local/live/sewers8-stop.png) until the
room overlay's descriptor command callback fired: `[EFFECT] Unported
callback 8018FC54 mode 1`. SEW11 wired the corridor field map's last two
unported ops BF (actor+0x250 |= 0x10; +0x24E = arg) and D2 (*a1 =
D_800A76A4[12*i]). SEW12: the corridor room is overlay **M0013I** (name
found in RAM; code at 8018EFE8, descriptor at 8018FCB4 = {8018F20C main,
+0x30 8018FC54 command, +0x34 8018FC78 table}). 8018FC54 is a 9-word leaf
(sw a1/a2/a3 → 8018FCEC/F0/F4; return 8018FCEC); PE_EffectCallback now
handles it keyed on its code words (so another overlay at that address
stays a boundary), and D4698 publishes retail's fourth argument through
PE_EffectCallback_SetExtra1 (it was dropped before). Test
SEW12_m0013i_command_callback. Overlay code is NOT in the EXE: disassemble
it from a RAM dump with `local/live/rdis.py <ram.bin> <addr:words>`.
Next likely boundary there: 8018F20C (M0013I main effect, >670 words,
uses GTE; callees 866A4/1CAB0/CE9D4/CE610/CE560/CEE20/CE8F0/D3FD8/77AA4
exist natively, D3F64 and C6B90 do not). Normal suite `Results: 1200 run,
1179 passed, 1 failed (B54KY cwd), 20 skipped`; sanitizer CTest 2/2 at the
SEW8 state (local/live/ctest-asan4.log).
EVE-DRESS investigation (user report 2026-09-05: after Eve runs away a red
piece of her dress stays visible at the left screen edge). Facts so far:
- Post-battle dump (run 10, local/live/sewers10-*): Eve = field actor
  0x800BF710 type 2 at (547,-1111,-959), flags 0x2B0, dest+0x9C 0, +0x9E 1
  (visible), animation clip 0x17 stopped at its last frame 50; the fight is
  in-map (no separate combat map; the op-31 map jumps are only the door
  exits). The fragment is at screen x≈50 y≈155 (local/live/sewers10-fragment.png).
- Retail hide chain (from scripts + retail asm): Eve's retreat handler
  (msg 0x1A) runs clip 0x17 to frame 0x33, sets m18=1, sleeps (op 01).
  The type-6 room controller (script 801B2404) waits m0&4 (set by Aya's
  0x65 handler), spawns subtask base+0x630, polls Eve attr 0x2C (op 8B),
  waits mode!=0 / mode 7 loops, then `40; AA; 1C 2 0 0x7D` (send 0x7D to
  Eve) and ends. Eve's 0x7D handler: `98 2 0; a7=-1; 2E 2; 04; 4B 0 0 0x80;
  02 1; 1C 6 0 0x84; 20`; the controller's 0x84 handler continues with
  `1C 2 0 0x79` then `0x7E` (Eve: `02 1E; 4B 0 0 0x20; 98; 0D 0x32 …`).
- Run 11 (task tracer on 0x8009D33C, local/live/sewers11-task.log): the
  controller DID run that chain in the port — left its poll at frame
  14951, waited through the fight, reached 801B2818 (after the 0x7D send)
  at frame 15289. But Eve's state afterwards shows a7=0 and clip 0x17 (her
  0x7D handler never ran) and her task list holds no 0x7D handler task.
  65400 delivery (type/id/+0x19C match, no 0x10 filter) and 653B8 record
  layout match retail; 6536C (boot only) is the only other queue reset.
  NEXT: trace the 0x7D message from 653B8 (queue count D_8009CDB4, record
  at A3180) through 65400 the following frame — watch 0x8009CDB4 and
  Eve's +0xA8 task-slot head; suspect the queue is drained/cleared before
  delivery or Eve's +0x19C is 0 at that frame. Also the dest tick 3AF14 is
  a cut: retail branches 0x800 (3CEF8/3CCB0), 0x10 (3C2E0 floor clip, GTE),
  0x20/0x40 (3B708), 4 (3C638 colour ramp), 8 (3B97C+3C0B4), 1 (3B97C+3BCE0)
  are all deferred (retail bodies disassembled into local/live/3c2e0.s,
  3c638.s; sizes 190/120/86/146/217/157/139 words). Hardware watchpoints
  silently did not arm in run 11 (no -watch.log); the tracer did.
- RESUMED (post-release): the inspector gained PE_LIVE_CALLS (log native
  calls with argument values) and the state now carries g74/m18/D280.
  Runs 12–13 show the whole retail chain DOES run natively: controller
  sends 0x7D (Eve's handler runs: a7=-1, 04, 4B, then 0x84 back), 0x7E
  (Eve plays clip 0x10 at +30 frames, sends 0x7C), controller 0x7C sends
  0x82 to the type-1 spawner, spawner sends 0xE to Aya (dialogue 50–60).
  Then TWO dest changes follow: 6C4C4(1) at ~18712 spawning a foreign
  descriptor 801AB6BD (+ child 801AB218), and 6C4C4(1) at ~18999 re-running
  the theater spawner script 801B0660, which spawns type 3, 0, 5, then
  `09 s0=(g74<0x28); 05 skip; 08 type 2` — i.e. **Eve is re-spawned only
  while g74 < 0x28**, and the port spawned her at 19000 (command 2 on
  0x800BF710), so g74 was still below 0x28 at that reload. Aya's script
  stores g74=0x28 at 801AF88C after its m18 wait. Retail must reach that
  store before the reload (or not reload then): the next run (14) logs
  g74/m18/D280 per snapshot plus op 31 (17BB4) calls to see which script
  issues the two map jumps and in what order relative to g74=0x28.
  SEW13: retail 3C2E0 floor clip is now native in func_8003C5D8_port.c
  (called from 3AF14 when dest+0x9C bit 4 is set; test
  SEW13_floor_clip_3c2e0); the other 3AF14 branches remain cuts.
- Run 14 (calls log, story fields): opcode 31 is `func_80017BB4_btl1_cut`
  (log that symbol, not 17BB4). After the fight two dest changes run
  (6C4C4(1) at ~19652 spawning foreign desc 801AB6BD, then at ~19938 the
  theater spawner again). g74 is still 0x27 at that reload, so the
  spawner legitimately re-creates Eve (type 2) — the SAME actor record
  0x800BF710 is reused and her OLD sleeping task/finished handlers stay in
  her slots. Aya's restarted script then replays the confrontation in the
  field: 0xB, 0x17 (walk clip 0x16), 0x1A (retreat clip 0x17) to Eve, waits
  m18, then `1C 2 0 0xFB` (Eve: `04; 01` sleep), `86 0x3C; 9C` (fade),
  g0 &= ~2, **g74 = 0x28**, `2E 0x15; 3F; AB; 20`. So the visible Eve is
  the retail field-side retreat; her end position (547,-1111,-959) comes
  from clip 0x17's root motion (package 801AE390: 51 frames; a channel
  falls 0 → -1524 over ~42 frames, another 0 → -465…). NEXT: verify the
  port's clip root-motion integration against those channel sums (a
  native harness test playing clip 0x17 from (97,286) and comparing the
  final pose), and check whether retail's mode-12 teardown (BTL83 "field
  return / teardown: not this cut") hides or removes field enemies; the
  reused-actor-with-stale-tasks behaviour on reload is also suspect
  (retail 35038 on an existing type/id may reset the task slots).
- PCSX-Redux retail replay: tools/pe_retail_replay.lua (LuaJIT; pad
  override via PCSX.SIO0.slots[1].pads[1].setOverride, CROSS=14 START=3
  UP=4 RIGHT=5 DOWN=6 LEFT=7) skips the STR (Start 30 frames after
  D_800B0DBA rises) but build293 then SIGSEGVs ~2 s later at the title
  (exit 139) with -softgpu -interpreter; no real BIOS or other build on
  disk; no memcard save survives (the earlier DAY1 Theater card is gone).
  Helpers: local/live/{fg_retail,launch_retail,kill_emu,status_emu}.sh.
Process-control helpers: `local/live/{launch,kill_game,kill_replay,status}.sh`
(keep process patterns out of interactive command lines: pkill/pgrep -f
matched and killed the calling shell twice).

RELEASE PE-SEW12-19523e8c549b (published 2026-09-05, user request: "compile
a new version and push it as an update to the banshee launcher", "add a
windows compile too… separate it between linux and windows install",
"make sure to add the discs too"). R2 head `channels/dev.json` now names
PE-SEW12-19523e8c549b (previous PE-PACE1-2c968d754c54 retained). Two
platform entries: linux-x64 (592421511 B, sha256 f4690c5b…9ce176, binary
19523e8c…, executablePath parasite-eve-port) and win-x64 (592501076 B,
sha256 3f2561b5…da9cda, binary ad7a380d…, executablePath
parasite-eve-port.cmd). Both packages carry data/disc1.bin + disc2.bin
with cue sidecars (build-info `discs`). Release build from the current
dirty working tree (commit 7821e7c3); normal + ASan/UBSan CTest 2/2 each
(1200 native groups; local/live/ctest-asan5.log). Linux package smoke:
extracted wrapper ran headless 120 frames to frame-limit
(local/live/pkg-smoke/run.log). Windows: FIRST cross-compile ever, via
`docker run dockcross/windows-static-x64` into `pc_port/build-win/`
(static PE32+ console exe; the test suite does not build there because
tests use fork/waitpid — pre-existing); NOT executed anywhere (no wine).
Packaging: `pc_port/package_runtime.sh` gained PE_PLATFORM (linux-x64|
win-x64), disc 2 and cue sidecars, and a .cmd wrapper. Publishing:
`pc_port/tools/pe_publish_runtime.py` (multi-platform, immutable archive,
full remote sha readback, ranged signed GET, pinned + mutable channel
docs, readback, signed channel URL in local/live/publish/channel-url.txt —
NEVER print/commit; summary local/live/publish/published-summary.json).
The launcher's existing signed dev.json URL stays valid (same object key).
Evidence: docs/evidence/pe-sew-day1-corridor/REPORT.md.

## Prior goal: complete retail Eve battle and naming (2026-09-04)

The active, unbounded goal is: implement the battle HUD and attacks, make the
first Eve battle retail accurate and completable, and implement name selection
and missing/inaccurate behavior. Do not narrow or mark this goal complete at a
HUD milestone. Keep the native app visible; no headless gameplay. No subagents
are authorized. All prior working-tree changes belong to the cumulative task.

PACE1 user steering: speed was confirmed too fast; user says drift is fine.
User explicitly authorized publishing the verified speed update to the existing
private R2 Parasite Eve channel for Banshee Realm. No git push/commit requested.
Implemented host present pacing60000/1001Hz, F6 fast-forward toggle with title
indicator and fresh clock on mode changes; input still polled while waiting.
Digital host source now publishesBE9A0=4100; focus loss clears held keys.
Normal+ASan/UBSan CTestPASS2/2 each,1187groups,59.87/71.97s:
/tmp/pe-pace1-{tests,san-tests}.log. Release build-dist also built successfully.
Original Scan/notice research deferred; temporary375E0 refactor fully reverted.
Older completed games GDB53564/11494 are paused to avoid timing-test CPU load.
GDB82937 speed-key test exitedHOST_QUIT (user inputZ thenwindowclosed); no crash.
F6 physicalX11 keycode72 successfully switched toFast-forward. Localxdotool
incorrectly sendsAlt forF6; use/tmp/pe-x11-speed-key.py forordinaryXTestF6.
Final normal300/600andfast600 Release benchmark usesdefaultscale2, currently
running /tmp/pe-pace1-release-benchmark-final.log. Earlier scale3 was CPUlimited.
PACE1 publish DONE 2026-09-05 13:39 local (18:39Z). F6 Normal→Fast→Normal
verified, hold produced one toggle (toggle.json). Package
`pc_port/build-dist/dist/parasite-eve-PE-PACE1-2c968d754c54-linux-x64.tar.zst`
290631257 B, sha256 c3ec4bb1…86bfe, binary sha256 2c968d75…d346f (matches
the tested `pc_port/build-dist/parasite-eve-port`). Uploaded to
`banshee-preservation-private/games/parasite-eve/builds/PE-PACE1-2c968d754c54/`;
published-summary.json in /tmp/pe-pace1-publish records
archiveReadbackSha256Verified, signedPackageRangeVerified,
pinnedAndMutableChannelVerified, signedChannelVerified all true.
Independent read-only rclone re-check (next session, 2026-09-05): R2 head
`channels/dev.json` = PE-PACE1-2c968d754c54 (sourceCommit 7821e7c3,
sourceDirty true), pinned `dev-PE-PACE1-2c968d754c54.json` present, package
listed at the recorded size. Previous head PE-INV17-7821e7c3 retained for
rollback. Signed URLs live only in /tmp/pe-pace1-publish/{channel-url,
previous-channel.json,dev.json}; NEVER print or commit them. Package metadata
records sourceDirty=true for the uncommitted working-tree build.
Next: broader retail fidelity (Eve battle PE effects, Liberation presentation,
Scan/notice, audible SPU) — goal remains open.

INV22 continuation: ordinary weapon/armor Equipment selection is implemented
and wired from main43DA4, list draw/input dispatch and44E98 confirmation.
New46574_port contains equipped/replacement labels, comparison properties,
54520 availability,59534/5968C commit and45D0C/4620C/466C0/46574 input.
Tool/upgrade paths remain explicit boundaries; do not claim those complete.
215 original-executable cases PASS natively /tmp/pe-inv22-filtered.log.
Full normal and ASan/UBSan CTest PASS2/2 each,1186 groups:
/tmp/pe-inv22-tests.log42.81s and/tmp/pe-inv22-san-tests.log52.84s.
INV21 foundation232 comparisons remain included. No builds/tests running.
Fresh visible run GDB53564, DISPLAY:10.0/window48234497, title
Parasite Eve - Equipment verification. Prefix /tmp/pe-inv22-latest;
/tmp/pe-inv22-live.log records read-only frame/shot/effect observations.
INV22 second visible attempt completed: full stage/name/stairs; Medicine1
healed42→45 at29844, effect29864; seven shots30149,30988,31004,31353,
31369,31718,31797; Eve scriptedexit31945; message60,HP10 live/saved.
Evidence complete-encounter-inv22.json,performance-inv22.png,
battle-medicine-effect-inv22.png,battle-complete-inv22.png plus RAM/RGB.
Field Equipment visibly verified: weapon0(M84F)→2(Club1)→0, real comparison
and equipped labels; Down enters weapon property list6 and Up returns to5;
N Vest armor display/select-already-equipped/cancel;
main menu closes and ordinaryUp movesZ633→598. Equipment evidence
in equipment-*-inv22 PNG/RAM/RGB and equipment-use-inv22.json.
No native boundaries. No input helpers remain after final verification;
GDB53564/window48234497 stays open in free field control.
First INV22 attempt lostfight after a singleV press was not accepted; its
waiter stalled and Aya died. Secondattempt usesVretry and correctedseatroute:
(-1790,3420),(-1790,3100),(-2490,3100),(-2490,2500), tolerance50.
ATmaximumis9000(original299CC). No collision/source changes for replay route.
Previous completedINV20 process/window46137345 remains open unchanged.
No guest state writes, commits, pushes or subagents. Goal NOT complete.

Current retry (2026-09-05): INV18/19/20 source is implemented and normal
and ASan/UBSan CTest pass 2/2 each (1184 native groups), logs
/tmp/pe-retry-tests3.log (53.03s), /tmp/pe-retry-san-tests3.log (62.55s).
INV18 has 200 original PE application cases; INV19 has 161 PE menu cases;
INV20 has 142 battle action/recovery cases. Corrected fixture seeds include
original effect/sound tables. INV20 Liberation phase fixture now uses D25C;
phase0 restores saved pose, input bit, background fade flag and effect cleanup;
phase1 calls the original streaming gate. Four older fixtures now seed an
empty effect pool. Liberation's remaining presentation states and several PE
effect callbacks remain incomplete; these comparisons do not prove all PE
abilities can be played end to end. An additional reference hash range for
saved position9E054 passes focused checks in both builds; no builds are running.

Visible first retry: name accepted, full stage backdrop and three performers
captured at /tmp/pe-retry-latest-message-20.png; normal aisle/stair route reached
Eve and battle. Helper tried PE without checking menu mask7D (PE excluded),
selected Equipment and hit its explicit45EE4 boundary atframe9301. Stopped
helpers and exited GDB29085. This was not a completed fight or PE cast.

Fresh INV20 visible retry completed without native stops: name/stage/stairs,
Medicine1 frame11048 HP40→45, particles/rings11068, seven shots
11356,12129,12145,12424,12440,12738,12797; Eve exit12945, message60,
HP17 live/saved, ordinary movement Z633→593. Evidence:
`docs/evidence/pe-aya-visible-dialogue/complete-encounter-inv20.json`,
`performance-inv20.png`, `battle-medicine-effect-inv20.png`,
`battle-complete-inv20.png` plus paired RAM/RGB snapshots.
GDB11494 remains visible DISPLAY:10.0/window46137345, logs and snapshots
/tmp/pe-retry2-live.log and /tmp/pe-retry2-latest*. Main replay helpers ended.
Post-battle mask7F enables PE. Visible Heal1 menu, HP preview and confirmation
were exercised with ordinary keys. Field51770 ability0 atframe18514 healed
live/savedHP17→45 and PEfixed5242880→1310720 (80→20; cost60).
Evidence field-pe-use-inv20.json, field-pe-menu-inv20 RAM/RGB and
field-pe-before/after-inv20 RAM/RGB/afterPNG in the evidence directory.
Both menus closed normally with C; field input unlocked, no native stops.
All input helpers finished, no held keys. The game window remains open.
Battle PE casts remain oracle-tested only; Liberation presentation and other
ability effects remain incomplete. Do not conflate field Heal1 with battle PE.
No guest state writes. No commit/push; no subagents. Goal not complete.
Defeat fade still needs a fresh visible replay. Equipment/status/config/upgrade,
remaining PE effects, and full audible SPU synthesis remain unfinished.

Latest implementation:

- HUD1/2 + EQP1: original HP/AT/PE packets, raw texture sprites, equipment
  binding and AT speed work in the visible battle. User confirmed health
  and AT bars on 2026-09-04. Evidence: `battle-hud.png` in the dialogue folder.
- ATK1..4: original seeding, first strike, target collection/sort/distance,
  command reset/effect filtering, ready input/pulse and sound lookup/queue.
- ATK5..9: full target highlighting, all nine command panels, ammo/shot/action
  counts, target marker, GPU flat/shaded lines, 347B4/7041C range wireframe,
  and attack-selection/queue/cancel paths of 25EE8. Mode 1 is now wired.
  Original instruction oracles cover full drawing call graphs and nine
  selection paths. Normal and ASan/UBSan CTest **2/2 each, 1,124 cases**:
  `/tmp/pe-atk9-{tests,san-tests}.log`. All nine new oracles reran successfully.
- ATK10/11: full positional sound 6DFA8/6DED4 plus physical/cached PS1
  scratchpad support. All range oracles also pass with the real scratchpad
  addresses 1F800040/1F8002C0. Visible targeting no longer crashes.
- ATK12: C22F8/C6CE0 effect storage, C9A70/CD728 pistol constructors,
  21128 weapon-effect selection, full 518A8/574A8 reload and final command
  commit are native. Seventeen full original-instruction fixtures match.
  Other weapon constructors remain explicit boundaries in 6F39C.
- ATK13: targeting generated packets but did not show them. Removed the
  stale native D_8009CDDC copy: every consumer/reset now uses guest 9CDDC,
  the index flipped by 70E54. This includes target highlight, range mesh,
  command panels, ready pulse and actor packet-bank caching. Cancel with
  normal Circle input returned the ATK11 run from mode 1 to mode 0.
  Normal and ASan/UBSan CTest **2/2 each, 1,127 native cases**:
  `/tmp/pe-atk13-{tests,san-tests}.log`. Filtered ATK: 14 groups pass.

ATK13 visible confirmation: the user saw target selection and command commit;
AT resets and refills, but no firing or Eve action was visible.

- ATK14: full 21F38/2312C/23008 shot animation, elevation, delay, ammo and
  reload paths; 22 original-instruction fixtures. 22D7C status effect
  selection and positional weapon sound 6DD38 also native. 6F6D4 routes
  C9B3C/CD89C setters. 1A680 propagates animation to attached children.
  Normal/sanitizer CTest 2/2 each at 1,128 cases. Visible replay uncovered
  a camera pan that stayed pending, so firing still lacked visible proof.
- ATK15: full 21278 hit judgement + 236E8 shot-frame dispatch, including
  baton / secondary / cone targets, critical/range modifiers, miss result
  and frame reset. Item-result strings 5DC9C/54A88 added for baton steal.
  53 original-instruction fixtures cover hit/shot paths.
- STG8: user reported the performance black and actors cut off again.
  Read-only message-20/26 captures confirm camera Y=400 and geometry
  offset=-32 with a 320x224 view and pending pan to Y=112 (flags79).
  Implemented full 66268 and restored its 35558 call; it advances the
  scripted pan before 65E48 follow. Ten original-instruction cases match.
  This also exposed inverted third/fourth quadrant branches in both
  77DC4 and 77D30; corrected against the original assembly.
  Normal and ASan/UBSan CTest **2/2 each, 1,130 native cases**:
  `/tmp/pe-atk15-{tests,san-tests}.log`.

ATK16 restores 1930C/VM98 and the full 6FE14 owner cleanup across both
11-slot effect pools. Eighteen original-instruction fixtures match. The
visible first-hit script continues into messages 46–48 instead of stalling.
ATK17 restores full 28574 damage and 28C48 hit reactions, with 82 complete
original instruction fixtures, plus VM4F/50 animation pause/resume. Normal
and ASan/UBSan CTest pass 2/2 each, 1,132 cases. ATK17 visibly fires, applies
six damage, finishes first-hit dialogue through message 49 and returns to
mode 0/input 4. `first-hit-dialogue.png` records the visible dialogue.
ATK18 completes 28E94 death/fade/rewards/drops, 2F300 victory initialization,
293F4 status reset and 21D4C action refunds. Twenty-four full original
fixtures pass. Old integration fixtures now supply the real command-20
victory clip. Normal full CTest passes (`/tmp/pe-atk19-pre-tests.log`);
normal and ASan/UBSan CTest now pass 2/2 each, 1,135 cases at ATK19.
Logs: `/tmp/pe-atk19-{tests,san-tests}.log`.

Previous visible app: ATK17 GDB tty 85517 was killed cleanly, window 46137345,
DISPLAY :10.0. `/tmp/pe-atk17-live.gdb` / `.log`, dumps
`/tmp/pe-atk17-latest-{ram,rgb,vram}.bin`. The automatic input helper expired.
Do not resume this corrupted run as validation. The AT bar stalled after
message 49 because 31 queued sounds were never consumed. Entry 28 at
800B8A18 overwrites Aya's battle record at 800B8A20 (HP/AT speed included).
The displayed 36 HP is corruption, NOT enemy damage. Read-only evidence:
`audio-overflow-before.json`. No guest stats or progression were repaired.
Previous visible app: **ATK19 GDB tty 44144, killed cleanly**, window 46137345, DISPLAY :10.0.
`/tmp/pe-atk19-live.gdb` / `.log`, dumps `/tmp/pe-atk19-latest-{ram,rgb,vram}.bin`.
Replay tty **2497**, `/tmp/pe-visible-atk19-replay.py`, 480-second deadline.
Ordinary arrows/dialogue confirmation plus up to three full-AT attacks; no
RAM writes. Early opening samples now consistently show queue count zero.

ATK19 adds 8CA84 command consumption and original voice allocation/control
in `pe_stream_commands.c`; timer registration/enable and producer/critical
gates schedule that command portion at host VSync. The full 8DB7C music
sequencer and audible SPU synthesis remain unported; this host scheduling
adaptation does not claim them. 67 complete original command call graphs match native RAM, including
stereo allocation, full voice banks, group/handle/oldest stops, pause and
volume routing. A frame-service regression processes 240 commands while
preserving Aya's entire 112-byte battle record; registration/enable, producer
lock, critical sections, query and reset gates are verified. Visible ATK19 replay proves seven actual shot/damage events across four
normal command selections, including reload. Aya HP remains45/45, AT
refills to9000, rate50, queue0. EveHP naturally reaches999995 and her
retreat/message50 runs, then controller hits unported VM96/192C8
(sets mode8). This requires the currently absent 2DC58 scripted battle
exit path; no victory/completion claim. Proof: repeated-shots-proof.json.
ATK20 restores VM96/192C8, mode8 dispatch, full 2DC58 scripted exit,
27A08 hit recording/flash, 295E4 cleanup, 2F9CC pool release, 51510 HP/ammo
saving, and 703F4/702DC/701B4 complete effect-pool cleanup. State64 of
6D60C now decrements its timer and issues music stop on expiry. Ambient
reload 3E/33 remains an explicit stop; other earlier 6D60C cuts remain.
69 complete original call-graph cases pass, with shared sparse input
fixtures in pe_scripted_exit_oracle.py. Normal and ASan/UBSan CTest
**2/2 each, 1,136 native cases**; /tmp/pe-atk20-{tests,san-tests}.log.
Previous visible app: **ATK20 GDB tty 17841 killed cleanly**, window46137345,
DISPLAY :10.0, /tmp/pe-atk20-live.gdb / .log and latest-{ram,rgb,vram}.bin.
Ordinary-input replay tty45269 /tmp/pe-visible-atk20-replay.py, 600sec.
ATK20 visibly fired seven shots/reloaded, requested VM96 at frame12248,
completed mode12 cleanup, continued messages50–60, and restored input0.
A normal Right key moved Aya x345→125. HP45/45 and ammo5 match saved
inventory; all seven body slots are released and sound queue remains0.
Proof: scripted-exit-proof.json and battle-return-to-stage.png. No guest
HP, position or progression writes. This verifies scripted retreat;
Eve's own attacks and naming are still incomplete.

ATK21: 2F7D8 and full 2FAF8 now call native1A680 and the
6DCE4 positional sound wrapper. 82 original call graphs pass in filtered
native tests. Full normal and ASan/UBSan CTest pass2/2 each,1,137 cases.
Logs /tmp/pe-atk21-{tests,san-tests}.log. Test fixtures for
CH1 allocation, BTL51 transition and BTL122 child allocation now supply
the clips that the formerly stubbed calls never read.
ATK21 was killed cleanly. Its read-only enemy-animation-progress.json
records Eve's commands 2→6→7, then action3 stalled at state1 with every
effect tick counter zero.

ATK22 completes 27D14, damage/MISS sprites 32B0C and script restoration
36254. Slow AT uses the original 2/5 modifier; movement only stops under
hit/status/death guards. All 115 original call graphs match. Normal and
ASan/UBSan CTest pass 2/2 each, 1,138 cases (/tmp/pe-atk22-tests3.log and
/tmp/pe-atk22-san-tests.log). Older fixtures gained real drawing resources,
maximum HP and battle state. The visible ATK22 replay reached dialogue49,
Aya45/45 and active AT; Eve's effects still remained unticked.

ATK23 restores full 6F9F0, 69594 draw/update ordering and pause filters,
D413C room-effect VM, D401C child allocation, D4704 draw dispatch,
CE560/5AC/610/688/78C particle pools, CE870/8F0 joint positions, and
6DC18/D3FD8 sound owner lookups. D4698 now runs the real M0005 command
callback80190A6C. Other callbacks stop explicitly at PE_EffectCallback
instead of silently returning zero. 157 complete original scheduling,
VM/handshake, allocation and position cases match native; they do not claim
coverage of the still-unported child callbacks. Filtered test passes and
normal and ASan/UBSan CTest pass 2/2 at 1,139 cases
(/tmp/pe-atk23-{tests,san-tests}.log).

ATK24 restores C2414/C251C/C2758 shared weapon rendering, ticking and
bytecode execution; C2B90/C2D0C/C2DA0/C2E08 allocation/release; the
C9B68/C9B90/CD8C8/CD8F0 wrappers and six particle/timer tick leaves.
101 complete original call graphs match. Normal and ASan/UBSan CTest
pass 2/2 each, 1,140 cases (/tmp/pe-atk24-{tests,san-tests}.log).
Unported constructors and draw callbacks still stop explicitly.

ATK25 restores full M0005 8018F330 charging and 8018F018 particles,
CEE20 sprites, CF3AC color curves and 783E4/78554/78CC4/786E4 math.
GTE now supplies GPF/GPL and projection MAC0/IR0. 125 complete original
call graphs match, excluding only unused packet stack padding. Full normal
and ASan/UBSan CTest pass 2/2 each, 1,141 cases; old HUD, range and spatial
sound oracles also pass (/tmp/pe-atk25-*). The visible replay shows Eve's
green charging particles, HP45/45 and queue0. The stage again shows its
full backdrop and performers. Evidence: eve-charge-progress.json,
eve-charge.png, performance-atk25-20.png.

Previous visible app: **ATK26 GDB tty48897 killed cleanly**, DISPLAY:10.0,
window46137345; /tmp/pe-atk26-live.gdb/log/latest-{ram,rgb,vram}.bin.
ATK25 was killed cleanly after capturing charge tick162/frame10086.
ATK26 restores F614 eye/hand flare and D0728 shaded rings; all73 original
call graphs match. Normal and ASan/UBSan CTest pass2/2 each,1,142 cases
(/tmp/pe-atk26-{tests,san-tests}.log). Ordinary-input helper PID2069591
was SIGINT'd cleanly. F614 draw tick24 is visibly captured at Present
frame10029: eve-flare.png and eve-flare-progress.json. HP45/45, queue0.
At frame10039 the next callback8018FDC4(mode0,data8018642C) is paused
before execution. ATK27 completes CE9D4/CFAA8/CFB7C direction math,
78004 lookup square root, and CEB8C/C6B20/C62DC beam collision, including
physical scratchpad writes and GTE OP. All91 original call graphs match;
normal and ASan/UBSan CTest pass2/2 each,1,143 cases
(/tmp/pe-atk27-{tests,san-tests}.log). ATK28 C71E4 mesh rendering and its
color/UV helpers match94 complete original call graphs across all four
facet formats, culling, depth, matrix concatenation, colors and UV edits.
Full normal and ASan/UBSan CTest pass2/2 each,1,144 cases
(/tmp/pe-atk28-{tests,san-tests}.log). ATK29 FDC4 beam, FB84 streaks and
D2370 textured ribbon rendering match121 complete original call graphs.
Full normal and ASan/UBSan CTest pass2/2 each,1,145 cases. The visible
ATK29 run renders the beam at frame10242: eve-beam.png. HUD42/45,
post-update RAM39/45, audio queue0. performance-atk29-20.png again shows
all performers and backdrop. The old partial 1D340 player tick applies
the same hit on later frames; full hit acknowledgement/recovery is next.

Previous visible app: **ATK30 GDB tty30365 killed cleanly**, DISPLAY:10.0,
window46137345. Ordinary-input helper has exited. /tmp/pe-atk30-live.log
shows the second charge starting at frame10000 with current-action0,
which original D4488/D44B4 read as physical RAM0. Native checked RAM
rejected that legal PSX access. ATK30 normalizes the action address to
its cached RAM alias; all170 complete original VM cases match, including
13 new physical0 read/write cases. NAM1 menu node allocation/cursor
restoration matches84 original call graphs. Normal CTest passes2/2,
1,146 cases; ASan/UBSan also passes2/2 (20.91s). ATK31 source now
restores full1D340/1F4D4 player hit acknowledgement, PE timing, HUD colors,
1F9C4/201DC recovery,20288 attributes,23E14 items and inventory helpers.
280 original player call graphs match native. Full normal and ASan/UBSan
CTest pass2/2 each,1,147 cases (/tmp/pe-atk31-{tests2,san-tests2}.log).
Legacy damage fixtures now supply valid PE/armor and pending hit state;
BTL99 checks that a single acknowledged hit is not repeated.
Inventory armor removal still calls the older partial512AC command3;
this rare path and automatic item effect rendering need further validation. NAM1 is infrastructure only,
not a working naming screen. Correction from the later 4E074/4E2E4 audit:
EF(0)/4DCA4 **is the naming screen**. 4DD64 opens its character grids;
4E074 selects characters through5BD10 and accepts a nonblank name through
5BEE8/52594. Earlier inventory/profile classification was wrong. Assembly
is saved in /tmp/pe-nam2-{menu-entry,menu-controller,name-candidates}.s.

ATK31 visible replay completed seven beam attacks without a RAM0 crash,
one hit per beam (45→42→35→28→21→14→7→0). No shots were selected:
Aya's hit animation never returned to idle because 299CC omitted its
2A5BC continuation. Run killed cleanly, GDB98919/helper61184 stopped.
ATK32 restores that continuation, interrupted command/frame restoration,
equip animation completion, frame phase counter and 306E0/25BD8 automatic
counterattacks. 127 original cases match native, including queue behavior
with absent effect resources; full normal and ASan/UBSan CTest pass2/2
each,1,148 cases (/tmp/pe-atk32-{tests2,san-tests2}.log).
ATK33 independently validates251 recovery/inventory/status-item call graphs;
native filtered, full normal and ASan/UBSan CTest pass,1,149 cases.
Equipped armor removal and recovery effect rendering remain outside these
fixtures; the inventory/profile menu is still incomplete.
Previous visible app: **ATK32 GDB tty50533 killed cleanly**, DISPLAY:10.0,
window46137345, /tmp/pe-atk32-live.log and latest-{ram,rgb,vram}.bin.
/tmp/pe-visible-atk32-replay.py uses ordinary keyboard input and up to12
full-AT attack commands with a4-second cooldown. Helper65788 stopped with
SIGINT. No guest state writes. Eve's beam hits Aya45→42, ordinary command
selection succeeds, Aya enters shoot command8 and23008 fires at frame11200.
New boundary: PE_WeaponCallback800C9C20 (slot80186BAC,record80186C2C),
from C9B90/C2758 code0. ATK34 restores C9C20/C9C8C/C9D9C constructors,
C9EA8/C9FD8 draws, CEDA8 texture upload and shared C42A4 quad rendering,
including its page, palette, dimension, blend and brightness helpers.
158 complete original call graphs match native memory/packets; the separate
texture test verifies both 64x256 banks after explicit GPU DMA completion
and confirms that cache hits do not re-upload. Normal and ASan/UBSan
CTest pass2/2 each,1,151 cases (/tmp/pe-atk34-{tests,san-tests}.log).
Assembly research saved in /tmp/pe-atk34-{muzzle,effects,render-deps,quad-deps}.s.
Previous visible app: **ATK34 GDB tty8803 killed cleanly**, DISPLAY:10.0,
window46137345, /tmp/pe-atk34-live.gdb/log/latest-{ram,rgb,vram}.bin.
Ordinary keyboard helpers tty97733 and88481 stopped withSIGINT.
/tmp/pe-visible-atk34-replay.py uses up to12 full-AT selections with4-second
cooldown. Eve beam hits once at10733 (45→42), Aya shoots10762, damage10763,
and muzzle/casing draw presents10764..10769. Viewed shot.png and beam.png
under /tmp/pe-atk34. Dialogue46/47 advances, then unknown D751C mode0
at frame11022 pauses the scene. No guest state writes; no completed fight.
ATK35 source now restores D751C, D71B8, D70C0 Aya reaction particles and
D1DEC point glow.114 original instruction cases generated successfully;
all114 native comparisons pass. Normal and ASan/UBSan CTest pass2/2 each,
1,152 cases (/tmp/pe-atk35-{tests,san-tests}.log). Research /tmp/pe-atk35-*.s.
Previous visible app: **ATK35 GDB tty56055 killed cleanly after completed fight**, DISPLAY:10.0,
window46137345. Ordinary helper tty34844 stopped withSIGINT after dialogue60.
/tmp/pe-atk35-live.gdb/log, latest-{ram,rgb,vram}.bin and shot/beam/reaction
Present captures. Seven shots/four selections, four beams; HP45→42→35→25→18.
Reaction draw captured at11164; scripted retreat VM96 at12474, then
messages50–60, input0, all battle pool slots0. Ordinary Right800ms moves
AyaX345→120. SavedHP18 and ammo5 verified; maxHP is record+28 (45), not
record+14 (previousHP). Inventory lookup uses host D048 (guest D048 is0).
Evidence complete-encounter-atk35.json, aya-reaction-atk35.png,
battle-return-atk35.png. No guest state writes. First encounter is completable
with both sides attacking. Naming/menus and remaining retail paths still need work.
NAM2 code in func_8004DD64_port.c now supplies original name buffer editing,
default name loading, remaining-count/selection helpers and grid constructor.
It is not wired to the VM yet: original rendering/input callbacks still need
restoration. All88 original comparisons pass; the fixture explicitly seeds
host D048/D050, matching the existing inventory test convention. Full normal
and ASan/UBSan CTest pass2/2 each,1,153 cases (/tmp/pe-nam2-{tests,san-tests}.log).
NAM3 implements the raw menu buttons, press/release/repeat FIFO, four menu
sound wrappers, and original 4E074/4E2E4 name controls. All174 complete
original comparisons pass. Full normal and ASan/UBSan CTest pass2/2 each,
1,154 native cases (/tmp/pe-nam3-{tests,san-tests}.log). NAM2's compared
inventory range now includes the full armor name buffer too.
NAM4 restores generic 63E0C list controls, 650E0 scrolling, and full5E30C
event dequeue/owner dispatch. All249 original comparisons pass (filtered
NAM:4 groups,1,155 native cases). Old score fixtures now supply released
Cross events instead of relying on fabricated auto-confirmation. Full normal
and ASan/UBSan CTest pass2/2 each (/tmp/pe-nam4-{tests2,san-tests2}.log).
Other inventory transfer callbacks stop explicitly.
NAM5 source restores menu font/icon packets, strings/width, drawing stack,
control glyph callbacks and field-bank setup5E6F0 in func_8005EED4_port.c.
All137 original complete call graphs pass. Full normal and ASan/UBSan CTest
pass2/2 each,1,156 native cases (/tmp/pe-nam5-{tests,san-tests}.log).
NAM6 restores6153C/61878/61C34/62090/622BC beveled windows, texture and
draw-area SDK packets, and cursor pulsing. Host waiting VSync modes now
advance the shared PE_GPU VBlank clock; query modes do not. All137 original
window comparisons and a host-clock regression pass. Full normal and
ASan/UBSan CTest pass2/2 each,1,158 cases (/tmp/pe-nam6-{tests,san-tests}.log).
NAM7 source restores634D4/6374C/65260/638D8 list draw/clipping/scrollbar,
four naming-list callbacks,62830 recursion and62FEC window ordering.
107 original comparison cases pass. Full normal and ASan/UBSan CTest pass
2/2 each,1,159 cases (/tmp/pe-nam7-{tests,san-tests}.log).
The tree fixtures deliberately omit the name-bar body4DF74;
its window and all other naming-list drawing stay in those original graphs.
Other menu drawing callbacks/cell predicates stop explicitly.
NAM8 restores4DF74 name display,4C608 naming help,61A3C slot borders,
item labels and timer digits. All188 original comparisons pass, including
complete62FEC naming screens. Full normal and ASan/UBSan CTest pass2/2 each,
1,160 cases (/tmp/pe-nam8-{tests,san-tests}.log).
NAM9 wires16F10/4DCA4 mode0 and the field5C498 frame, including help,
input,confirm/resume,palette42D40/42F44 and field return handling.139 original
comparison cases pass. Full tests exposed a stale skip-menu boundary assertion
(updated to the original first-frame handshake) and missing StoreImage at boot.
NAM10 adds original750CC/768A0 StoreImage, GPU C0 readback and deferred
GPU-to-RAM DMA, plus direct/queued worker dispatch.47 original worker cases
and the queued upload/readback regression pass. The first full run exposed
an older --skip-movie shortcut returning before title teardown. The shortcut
now executes the original801916DC..801918C8 nonnegative-selector exit:
display mask/background reset, clear/sync, saved environment restoration with
the current screen Y retained,5C1EC(0) and5E57C(0). The authenticated overlay
is /tmp/pe-nam10-title-overlay.bin; disassembly is the adjacent .s file.
The existing SKIP2 opening integration regression passes again, with an added
assertion that the title renderer/card timer are disabled in the field.
Full normal and ASan/UBSan CTest pass2/2 each,1,163 native cases:
/tmp/pe-nam10-cleanup-{tests,san-tests}.log (30.04/31.69 seconds).
The readback fixture includes canonical jtb[7]=768A0; the old LoadImage-only
B52 fixture did not seed it. CPU/DMA transfer timing is preserved.
Inventory/card callbacks remain explicit boundaries. Naming now has visible
proof in name-entry-proof-nam12.json and name-{added,default}-nam12.png.
NAM10 visible replay reached4DCA4 mode0 at frame704, then stopped at705:
the name-bar SPRT64 uses8bpp page87/CLUT38DC/V152, while GPU admission
allowed8bpp only for fixed SPRT16. The sampling code already handles8bpp.
NAM11 unifies indexed-depth admission for both sprite sizes and adds a
real draw-chain pixel regression with palette indices129/254, both byte
positions, transparent zero and raw/neutral modulation. Normal/ASan/UBSan
CTest pass2/2 each,1,164 native cases (/tmp/pe-nam11-{tests,san-tests}.log).
NAM11 visible replay reached name entry694 and aborted695 on bank0:
5E588 had mistranslated original5E664's SetDefDispEnv as SetDefDrawEnv,
overwriting A21F0/A21F4 with0101000A/0. Its two valid descriptors now
survive setup; the second display call and original color00808080 are also
corrected. The old direct test had incorrectly asserted that corruption
as retail behavior. Eight complete original setup/bank-selection cases
pass in pe_name_frame_oracle.py (entry12;147 total comparisons). Full suites
initially found an older canary-footprint expected table retaining the wrong
color/pointer overwrite; that expectation now follows the authenticated
initializer. Full normal and ASan/UBSan CTest now pass2/2 each,1,164 native
cases (/tmp/pe-nam12-{tests2,san-tests2}.log;31.56/34.90 seconds).
Previous visible app: **NAM12 GDB tty75475 killed cleanly after full fight**, DISPLAY:10.0, window46137345.
/tmp/pe-nam12-live.gdb and .log; latest-{ram,rgb,vram}.bin captures.
Started --windowed --skip-movie, with opening-menu skipping disabled.
NAM12 opening helper tty28355 exited at the name menu. Ordinary inputs
proved Aya→AyaA→Aya (Circle), AyaF→Aya (Default), Start focus→End
confirmation, then message12 with menu removed and pause flag cleared.
Replay /tmp/pe-visible-nam12-replay.py tty29296 completed and exited after
message60. Seven shots across four selections; scripted exit19719;
18/45 HP, savedHP18/ammo5, empty sound queue and all seven body slots free.
Normal Right800ms moved X345→140, key released. Proof:
complete-encounter-nam12.json and battle-return-nam12.png. The visible
NAM12 app remains running after the fight. No guest state writes.
NAM11 was killed cleanly.

INV1 restores647D0 list resizing,6269C recursive release,64A54 saved cursor,
64E90 scrollbar release and62F3C window-by-ID removal in62D2C_port.c.
All79 original complete call graphs match in pe_menu_lifecycle_oracle.py,
retail_menu_lifecycle_cases.h/test_menu_lifecycle.h. Filtered native test
passes1/1 at1,165 total. Full normal and ASan/UBSan CTest pass2/2 each:
/tmp/pe-inv1-{tests,san-tests}.log (29.71/34.04 seconds).
Inventory entry/drawing is not wired yet.
Next research: /tmp/pe-inv1-menu-{entry,draw}.s, /tmp/pe-inv1-node-delete.s,
/tmp/pe-inv1-list-delete.s, /tmp/pe-inv1-save-cursor.s. Entry5C174 calls
33A20,339A0,51510,52F70,438EC,525EC. 438EC builds the main command list
and calls439D8 stat/equipment panels,4C594 help. Drawing callbacks include
43B0C,43C64,4905C,4F838/50748 plus4C608 inventory help. These remain next.
INV2 source func_800438EC_port.c now implements33A20/5B89C/5257C getters,
4C594 help construction,439D8 stat/equipment panels,438EC main menu and
5C174 entry. Guest mirrors accompany the existing native inventory globals.
The real entry remains unwired until its input/drawing callbacks are ready.
pe_inventory_menu_oracle.py produces75 complete original comparison cases;
retail_inventory_menu_cases.h/test_inventory_menu.h added. Normal/sanitizer
builds and full CTest pass2/2 each,1,166 native cases
(/tmp/pe-inv2-{tests,san-tests}.log). Case68 required seeding the existing
host D_8009D018 from its guest fixture, just as D048/D050 already were;
production52F70 was correct. The visible game remains the completed NAM12 replay.
INV3 adds original numeric fields, inventory labels, stat/equipment/BP panels
and main command-list drawing in func_80043B0C_port.c. All258 full original
call graphs match (pe_inventory_draw_oracle.py). Full normal and ASan/UBSan
CTest pass2/2 each,1,167 native cases (/tmp/pe-inv3-{tests,san-tests}.log).
The fixture now seeds956AC and9CDDC explicitly: its inherited constructor
fixture intentionally does not forward drawing frame/bank options.
Input/help callbacks still need implementation before entry is wired.
INV4 restores full512AC command dispatch and218D8/21AF8 equipment bitfields,
254BC field/battle application,51244/5D970 leaves.209F0 now has a full entry
with its immutable original executable tables; the old cut name forwards,
and invalid categories no longer silently become category0. All166 original
call graphs match; full normal and ASan/UBSan CTest pass2/2 each,1,168 cases
(/tmp/pe-inv4-{tests,san-tests}.log). Inventory ammunition-name baseD03C is
also an existing HOST scalar:536B8 now reads it and INV3 seeds it correctly.
INV5 restores5DCEC/5415C/54288/556E8/58E08/57ED8/58BBC/59F08/55610
and every non-card4C608 help selection.251 complete original call graphs
match, including62FEC main inventory drawing. Full normal passed at1,169;
concurrent sanitizer exposed fixed /tmp PPM filenames shared by both suites.
The PPM test now uses unique mkstemp files and cleans them on every path.
Both full suites pass again at INV6 (below), with no sanitizer errors.
INV6 restores55760 usability across both inventory banks,57C54 rearrangement,
44174 item-list construction,447F0 capacity footer,4F8D0/50804 item drawing,
64C80 equipped borders and64EB4 scrollbars.263 complete original call graphs
match, including real63E0C transfer/cancel callbacks and whole62FEC item
screens. Normal and ASan/UBSan CTest pass2/2 each,1,170 native groups:
/tmp/pe-inv6-{tests,san-tests}.log (29.81/33.82 seconds).
New source func_80055760_port.c; pe_inventory_items_oracle.py and
retail_inventory_items_cases.h/test_inventory_items.h. Entry remains unwired
until item/main input and other menu pages are ready. NAM12 remains visible.
INV7 restores44444 item-window input,44924 action-menu construction,
451D0/56C40 pair confirmation setup,562A4 ammunition compatibility across
both inventory banks,5600C next compatible item,55724/55FB4 usability bits,
57654 discard eligibility,5833C ammunition pickup and210D4/52558 action gate.
331 complete original call graphs match. Native command values use
PE_MenuCommitResult rather than a fabricated guest stack address;512AC0/1
share that same publication/cleanup. Normal and ASan/UBSan CTest pass2/2 each,
1,171 native groups (/tmp/pe-inv7-{tests,san-tests}.log,30.80/33.58 seconds).
New func_80044924_port.c; pe_inventory_actions_oracle.py and matching test.
A shared53D2C declaration was missing during prebuild; both final builds clean.
Main43DA4 and44B0C action execution remain next; live entry is still unwired.
INV8 source func_8004F910_port.c restores centered strings5F354/5F594,
64C30/64C54/62A7C,53068 item names,52BCC/52C08 composition,4F910/50878
item-action drawing,Yes/No and notice drawing,4CC50/4D030 notices,
44F8C/44E98/45110 discard confirmation and58C4C filtered inventory refresh.
277 complete original fixtures match native in pe_inventory_dialogs_oracle.py.
Full normal and ASan/UBSan CTest pass2/2 each,1,172 native groups:
/tmp/pe-inv8-{tests,san-tests}.log (32.00/35.71 seconds). The invalid-cursor
confirmation case caught a delay-slot return value:44E98 always returns1.
That is corrected; the complete filtered group passes.
Unknown notice/confirmation callbacks remain explicit boundaries; generated
call graphs cover zero and named native callbacks only. Live still NAM12.
INV8 research: /tmp/pe-inv8-{actions-dependencies,confirmations,drawing}.s;
/tmp/pe-inv7-{item-actions,use-menu,use-leaves,availability-leaves}.s.
INV9 completes ammunition selection44274, quantity transfer56B24,
commit/history57094/51098, repeatstep5E120, pair draw56FB8/453E8 and
confirmation input452C0. PE_CopyItemRecord preserves the original grouped
copy ordering; PE_MenuApplyAmmo handles original stack-local commits.
512AC case5 retains its guest pointer reloads, including output aliasing.
246 original full call graphs match: pe_inventory_ammo_oracle.py and its
native test. Full normal and ASan/UBSan CTest pass2/2 each,1,173 groups:
/tmp/pe-inv9-{tests,san-tests}.log (32.29/36.30 seconds).
INV10 source func_80044B0C_port.c implements Use/Move/Discard/Reload,
516B4 saved HP/ammo updates and57834 ordinary item/battle command use.
4F23C rename and48918 upgrade subtypes remain explicit native boundaries.
INV10 has206 full original call graphs matching native, including actual
medicine IDs6..17, battle history/consumption, queued action input and whole
post-action draw trees. Full normal and ASan/UBSan CTest pass2/2 each,
1,174 groups: /tmp/pe-inv10-{tests,san-tests}.log (32.71/37.93 seconds).
The first oracle reached post-action drawing then found a fixture PE maximum
of zero; corrected to the actor's fixed-point PE. No production failure.
INV11 adds43DA4 Items/Escape/cancel routing and restores live field Triangle
entry via5C174(0),67CBC and original pause flags. Other main pages retain
explicit boundaries; battle Triangle remains unwired because25EE8's menu
command branch is still missing. Do not expose it before that branch works.
84 original whole main/input/draw/field-block cases match native; original
355F8..356D0 uses initial s0=B0CD8 and actual original nested calls.
Full normal and ASan/UBSan CTest pass2/2 each,1,175 groups:
/tmp/pe-inv11-{tests,san-tests}.log (34.76/39.72 seconds).
Current visible app: **INV11 GDB tty80939 running**, DISPLAY:10.0,
verified window46137345, /tmp/pe-inv11-live.gdb and .log. Read-only dumps
every30presents at /tmp/pe-inv11-latest-{ram,rgb,vram}.bin. No guest writes.
INV11 visible proof COMPLETE: normal P/Return accepts Aya; field V opens
main/Items with name/portrait/stats. Reload Up changes pistol/crate6/6→5/7;
Confirm saves both persistent and live ammo, then Down restores6/6. Cancel
closes Items/main and releases field pause. Ordinary replay tty55767 is done:
seven shots26073,26985,27001,27374,27390,27767,27826; scriptedexit27974;
messages50..60 finish; live/savedHP14, ammo5, queue0. No beam count logged.
Fresh performance-inv11.png confirms backdrop, all3performers and stairs.
Medicine1 moved slot4→6 via ordinary list input (long350ms keys overshot and
entered Move on an empty row; corrected using80ms presses and750ms release
between snapshots). Then Use atframe45926 heals live/savedHP14→45 and
consumes ID6. Two Circle presses close menu; short Right movesX345→330 with
Y−1111/Z633 unchanged. inventory-proof-inv11.json contains all values.
Evidence PNGs and RAM/RGB captures in dialogue evidence folder. No held keys,
all keyboard helpers ended. INV11 remains visibly running after healing.
INV12 source func_8005112C_port.c adds533D4 record lookup and5112C full undo,
and expands26CF0_attack_cut to menu-command undo.162 original cases match,
including committed ammo/item rollback and all history kinds. Full normal
and ASan/UBSan CTest pass2/2 each,1,176 native groups:
/tmp/pe-inv12-{tests,san-tests}.log (34.49/38.47 seconds).
Next INV13:26824 item/PE/equipment command handling, full25EE8 menu branch,
and then299CC ready Triangle wiring. Battle menu remains an explicit boundary
until this is done. Research /tmp/pe-inv12-{battle-command,battle-item,undo}.s;
ready path /tmp/pe-inv11-battle-menu.s2A3E0..2A408 setsD1F0=1,5C174(1),67CBC
then shares attack-mode1 setup.26824 PE jump table108AC:393→panel4,394→5,
395→6,397→7,406→seven random targets; other387..406 self-target immediately.
Remaining main PE/equipment/status/config/upgrade pages and rename/tool item
subtypes stay explicit native boundaries. INV13 source is now written but NOT YET VERIFIED: new26824_port.c replaces
older partial26824 from27D14_port.c; full25EE8 item/menu/R1 branches and
ready Triangle entry are wired in source. Live app is still INV11. Existing
26824 ABI stays int(int), truncating mode to signed8bits internally.
Old26824 used the wrong target tableAE000 instead of9E000; removed that
partial implementation and fixed three old fixtures to use the original
9E000 table and a valid nonzero count. No production zero-divide fallback.
INV13 prebuild2 passes. pe_battle_items_oracle.py generated183 original
full cases; rerunning /tmp/pe-inv13-oracle2.log after correcting the test
case default mode for attack-side input combinations. Native test and header
are added; filtered/full normal/sanitizer verification still pending.
INV13 now passes183 full original cases and full normal/ASan+UBSan CTest,
2/2 each,1,177 native groups (38.30/43.56s):
/tmp/pe-inv13-{tests,san-tests}.log. Test RNG is reset to BIOS seed1 per
fixture; command-entry fixtures now allocate dormant effect pools instead
of incorrectly passing a null pool into70064. Production guards unchanged.
INV11 visible app reached a HOST_QUIT stop atframe102243 after the completed
fight/medicine proof. GDB tty80939 is paused at that stop. No held keys.
INV14 completes22210 medicine animation/heal/effect start,255E4 escape
judgement and21DE0 medicine/equipment/escape dispatch.182 full original cases
match, including full effect construction, exhausted pools, animation phases,
status cures, signed escape probability, blocked enemies and failure messages.
Full normal and ASan/UBSan CTest pass2/2 each,1,178 native groups
(34.27/40.20s): /tmp/pe-inv14-{tests,san-tests}.log. The test-only BIOS oracle
now models printf diagnostics with signed integer arguments; no runtime VM.
Full22394 PE execution remains partial and is not covered by queue tests.
Research /tmp/pe-inv14-{medicine,item-action}.s; pe_item_execution_oracle.py.
User said retry; fresh visible INV14 launched in GDB tty97215, DISPLAY:10.0,
window46137345. /tmp/pe-inv14-live.gdb/log/latest-{ram,rgb,vram}.bin.
Prior INV11 GDB80939 exited cleanly after its HOST_QUIT stop. Opening helper
/tmp/pe-inv14-opening.py is running until the name menu; no guest writes.
Next replay /tmp/pe-visible-inv14-to-medicine.py stops after opening Triangle
at full AT following a natural hit. Then select Items/Medicine1/Use visibly.
INV14 fresh replay proves stage/name/stairs again; performance-inv14.png
shows the full scene. Opening helper50794 ended; name accepted with P/Return.
Replay95364 ended after a natural hit45→42 and Triangle opening the battle
inventory. Ordinary Return/Down/Down/Return/Return selects Medicine1/Use.
Aya's command14 animation runs, ID6 is consumed, and23E14 heals42→45 at
frame15696. Then the healing effect hits unported callback800D4C24(mode0,
data80186230,extra80190B80). Current GDB97215 is paused at that stop, frame1
PE_EffectCallback; do not resume or claim battle continuation. No held keys.
Read-only proof battle-medicine-progress-inv14.json plus before/after RAM/RGB.
INV15 now addresses that live healing effect: D4C24 emitter and D4928 particle
are implemented in newfunc_800D4C24_port.c; dispatch/header/CMake wired.
pe_medicine_effect_oracle.py andtest_medicine_effect.h added, generation in
progress (/tmp/pe-inv15-oracle.log). First build and115-case filtered test PASS.
Read-only live descriptor E14BC revealed a second callback800DF87C, scheduled
twice by E13F0 (after8and11frames). DF87C ring is now included in the same
file and dispatch; generation expanded to152cases in/tmp/pe-inv15-oracle2.log.
Latest normal/sanitizer builds pass,152-case filtered test passes. Full normal
and ASan/UBSan CTest pass2/2 each,1,179 native groups (40.49/48.13s):
/tmp/pe-inv15-{tests,san-tests}.log.
INV14 GDB97215 killed cleanly after recording the heal/effect boundary.
Fresh visible INV15 GDB60961 is running, DISPLAY:10.0,window46137345.
/tmp/pe-inv15-live.gdb/log/latest-{ram,rgb,vram}.bin. Ordinary-input master
/tmp/pe-inv15-visible-replay.py is running: opening/name→stage/stairs→natural
hit→Triangle/Medicine1/Use→wait for effect/AT recovery→normal firing replay.
It stops on any native boundary before further input. Log/tmp/pe-inv15-replay.log.
GDB captures the presented healing effect20frames after heal; no guest writes.
D4928 uses existing CF3AC/CEE20 andDF87C usesD0728. Research /tmp/pe-inv15-medicine-
{effect,particle}.s. PE menu work is deferred; research retained in
/tmp/pe-inv15-pe-{menu,menu-leaves,use,support}.s (no PE page code changed).
INV16 source now adds PE reservation515F8 (native-local helperPE_MenuPEValues),
table5DC10,shield524D0,cost579D4 andavailability4324C in515F8_port.c.
Header/CMake/tests wired; pe_pe_cost_oracle.py generation running in
/tmp/pe-inv16-oracle.log (session78351). No INV16 build/tests yet. First-party
instruction authority is the same original executable. Live INV15 replay61732
continues independently; no new PE menu entry is wired yet.
Additional research: /tmp/pe-inv4-{menu-input,item-leaves,equipment-apply}.s;
/tmp/pe-inv5-item-menu.s; /tmp/pe-inv6-{item-draw,marker,scrollbar}.s.
Original4C608/jump table is in /tmp/pe-nam2-menu-controller.s and exe1104C.
Frame/input research is saved in /tmp/pe-nam3-menu-frame.s and
/tmp/pe-nam3-field-frame.s. NAM9 restores field355F8..356CC, including
5C498(A76D8) and release of menu pause after a nonzero return.

STG8 remains visually verified: message 20/26 show the full backdrop and
actors. Before/after PNGs and performance-camera-proof.json are saved in
the dialogue evidence folder. Fresh NAM12 message20 also confirms all
three actors, backdrop and staircase: performance-nam12.png. Naming has
now been verified visibly through End and resumed opening dialogue.

Remaining substantial work: battle inventory/PE/shoulder input remains an
explicit boundary. Complete those paths and remaining status/equipment/audio
behavior. Naming and field inventory already have visible proof. The first Eve
encounter is visibly completable with both sides attacking at ATK35. Full host
button mapping and battle pause ordering are implemented. **6FE14 is effect
cleanup by userdata, not the per-frame effect tick** (correcting old notes).
Weapon effect constructors needed: code 0 C7BA0, 1 C8D34, 2 C9A70, 4 CD728;
all call C22F8/C6CE0. Many small effect callbacks have matching `src/` C,
so /tmp/pe-read-asm.py may report NOT FOUND for those. Read matching source.
Final proof must remove `--skip-opening-menu`, visibly select a name, and
win through real input; no artificial HP, position or progression writes.


## Earlier milestone: auditorium, stairs and battle transition (2026-09-04)

User reports: most stage scenery missing, stairs to Eve inaccessible, Aya
walking through auditorium seating, then a black screen entering battle.
Continue to the first battle in the visible window; do not run headless.

Current native changes and observations:

- STG1/2 restore auditorium Aya activation, script walkmesh flags and the
  signed `BD020/22` movement heading addresses. ENT2 restores scripted
  walking and field idle on released arrows.
- STG3 ports camera follow `65E48`, called from `35558`. The old camera pan
  remained at Y=256 when the view changed from 512 to 224 pixels high.
  The stage, stairs and full Eve conversation background now render.
- COL1 adds `1AE40` floor collision, connected-triangle traversal, radius
  checks, wall sliding, sloped height and landing. Both triangle layouts
  are covered. Actor-to-actor collision and the cached-wall optimization
  remain unported. The visible replay reaches the stage through the stairs.
- STG3 restores `6C5BC` texture/weapon package states 1–7 and frame polling;
  host VSync now services actual asynchronous SPU DMA completion.
- STG4 implements music/effect bank uploads `871AC`, `87428`, `875FC` and
  the `6CDA4` dispatch. Normal and sanitizer suites passed at this point.
- STG5 corrects `6D078`'s packed sound count from shift 16 to retail shift
  22. M0005 has two entries, previously interpreted as 130; the loader
  reached unrelated data and retried invalid audio indefinitely. Also
  corrects `6914C` texture count/offset to the same 10/22-bit format.
  The visible replay now finishes `55/144FC` and shows Aya and Eve in
  the battle view (`battle-entry.png`), with all loader state bytes zero.
- STG6 restores AC/196E8 and E3/1A3FC script-list registration. The
  STG5 battle controller had stopped at AC (next PC 801B34F4, delay 0).
  Also restores `2BC90`'s ready tail (volume command, UI colors, input
  unlock), and battle idle `716A4` with `1A784` clip/attachment handling.
  Normal and ASan/UBSan CTest passed 2/2 each, 1,108 native cases, no skips.
  The visible script continued to active mode 0 with input mask 4, then
  crashed on Eve's first effect (6B/187C0 -> D4698).
- STG7 restores the room effect exports at `6B968..6BA08`. M0005 exports
  effect 0x75 to E1044[0x20] = 80190B44. Without that registration,
  CE49C failed and slot+8C retained old texture bytes (688A7433), later
  dereferenced by D4698. Normal and ASan/UBSan CTest pass 2/2 each, with
  1,109 native cases and no skips. The visible STG7 replay completes the
  transition and stays in active battle mode for 4,470 observed frames.
  All four sampled loader bytes are zero; the effect slot now contains
  the actual 80190B44 descriptor. Arrow input moves Aya, and release
  returns her to command 4 with zero velocity. See `battle-control.json`
  and `battle-control.png` for the measurements and final visible view.

**Remaining combat work:** no battle HUD or attack menu appears. D4698
still skips the loaded effect callback 80190A6C; full attacks/effects are
not implemented or verified. Mode 7's 2D1F0 HUD and much of 2BC90 remain
partial. Battle entry and directional movement are verified, not full combat.

The STG7 window is left running under GDB session 99902, with dump/log
breakpoints disabled and the input helper stopped. `/tmp/pe-stg7-live.gdb`
and `/tmp/pe-stg7-live.log` retain the replay setup/history. Final read-only
dumps are `/tmp/pe-stg7-battle-final-{ram,rgb}.bin`. Use normal window input;
do not patch guest progress flags or position. The reliable auditorium
stair approach is around X=-1970,Z=3110, then X=-2490,Z=3110, then up to
Z=2500; Z=3290 is too close to the aisle corner for a direct lateral step.
Evidence PNGs in
`docs/evidence/pe-aya-visible-dialogue/`: `stage-backdrop.png`,
`auditorium-aisle.png`, `eve-conversation.png`, `battle-entry.png`,
`battle-control.png`.

Older milestone/test counts below are historical.

## Visible progress preference (2026-09-04)

User requested: "don't do it headless i'd like to see the progress".
Run the game with `--windowed --skip-movie --scale 3` so development progress
is visible on their desktop. Leave the live preview open between changes;
avoid frame-limited runs that immediately close the window when demonstrating.
Current preview title: `Parasite Eve - Live Native Port`, on display `:10.0`.
X11 verified the 960x720 window is viewable and active. Orca computer-use
currently cannot enumerate this raw X11 app or capture screenshots.
Current progress and limitations are described immediately below; the older
milestone sections are historical.

## Visible Aya and dialogue (2026-09-04, active)

**Latest user direction:** "the name pop up doesnt come up ... characters
aren't on the front. just continue to the battle". Continue toward the first
battle in the visible window; no need to wait on the earlier optional menu
preference question. The opening menu shortcut keeps current defaults.

Aya is visible and her opening animation advances. User confirmed seeing
her. Restored 6BECC texture states 1–3 (the old forced state 4 bypassed Aya's
texture/CLUT upload), nonempty 3A6A8 world/screen anchors, and VM CD/19CEC
position synchronization. Field entry now rebinds 371B0/CE90 to the current
language stream. 37870 draws ordinary retail font SPRTs, preserves page
anchors, treats F7 as newline, and advances F8 correctly. Fixed 371B0's
800AECxx typo to the retail 8009ECxx addresses. Window pad input owns
dialogue confirmation; Enter/Space/Z/X confirm, arrows are the D-pad.

Normal + ASan/UBSan CTest passed 2/2 each at 1,090 native cases. After adding
the optional menu shortcut, normal CTest passes at 1,091; final sanitizer
validation and playable-entry verification are still in progress. Evidence:
`docs/evidence/pe-aya-visible-dialogue/REPORT.md` and PNGs alongside it.

Next dependency found in the visible run: after profile message 11, Aya's
task 8009D33C reaches EF(0) at 801B1BD0, function 80016F10 → 8004DCA4 (menu).
It used to silently leave task delay zero at PC 801B1BDC. It now preserves
the instruction and reports an unresolved boundary. Separate opt-in
`--skip-opening-menu` retains defaults only for Aya's EF(0) in M0010; it is
HOST_ADAPTED and does not activate automatically with `--skip-movie`.
Current visible run uses both options. Original movie skipping preference
is preserved.

ENT1 translates actor setup opcode 1B/18F74 (26 words, 9774.s). This fixed
the absent entrance characters: Aya and three NPCs now appear in M0001I,
conversation 13 finishes, and D2E8 becomes 2 (movement unlocked). Actual X11
Up input takes Aya through the doorway into token A80001C8. Evidence:
`docs/evidence/pe-aya-visible-dialogue/theater-entrance.png`.

ENT2 adds full scripted move opcode 44/136C0 (3420.s), which stopped an
interior NPC at 801A95BC, and the field idle callback 71034 (617AC.s), which
was leaving stale velocity after key release. Normal CTest passes 2/2 at
1,094 native cases (script target latch/arrival and actual idle integration
regressions). Rebuilt sanitizer validation and visible interior progression
are in progress. First battle has not been reached.

GDB was launched as the parent of the visible app because ptrace attach is
unavailable. `/tmp/pe-ent2-live.log` has message/field/input state every
120 frames and logs previously unwired VM functions. Latest captures:
`/tmp/pe-ent2-latest-{ram,rgb}.bin`. UI input fallback: Orca computer-use
does not enumerate this raw X11 window; `xdotool key --window <verified-id>
--delay 160 Return` successfully advances dialogue. Avoid very short key
pulses, which can be processed entirely between field ticks.

Known separate issue, not fixed: 6BE4C computes its CE3 texture-family
comparison using CE3 instead of (CE3-10) after extracting the sign. Retail
6BE94..6BEA0 in 5B1E4.s proves the subtraction; BTL127 tests currently mirror
the old bug. Audit before relying on reload-family parity.

## PE-POLY1 — visible character models; script position dependency (2026-09-04)

Two models now render beside the limousine and change pose/location between
120/600 frames. **Aya is not controllable; auditorium entrance not reached.**
Inspected images: `docs/evidence/pe-poly1-visible-models/opening-120.png` and
`opening-600.png` in the same directory. Camera 65B70 init is restored at
68B94; GPU handles polygon packets with a native affine rasterizer; existing
3B97C lighting is wired; 35558 publishes actor world rotation/position/scale
before the hierarchy walk. See report in that directory for authority/limits.

Normal + ASan/UBSan CTest 2/2 each, 1,088 native cases, no diagnostics.
POLY1 tests cover shared edges, paletted sampling and masking; opening test
requires camera initialization and actual polygon pixels. Raster rounding
has not been independently compared against PS1 images; don't call it exact.
CLI now reports polygons/polygon_pixels. All prior uncommitted work retained.

**Next dependency from actual 600-frame RAM:** Aya 800BED10, input D2E8=3,
task 8009D33C delay=0, PC=801B1A80. Preceding CD at 801B1A78 maps to unwired
80019CEC (14w in A44C.s): copy actor+21C/+21E/+220 into pose+28/+2C/+30,
return 1. The inputs are generated by 3A6A8's nonempty model path (currently
no-op). Finish that path, then wire CD and trace next opening instructions.
Do not wire CD against uninitialized source coordinates. The temporary RAM
probe is removed; local evidence `/tmp/pe-poly-600-ram.bin` and
`/tmp/pe-poly-probe-600.log` are outside the repo. No actual input response,
collision, doorway or auditorium claim yet.

## PE-DRAW1 — polygon submission; zero camera identified (2026-09-04)

Goal incomplete, Aya still invisible/not playable. Added 3B144 (3B144..3B708
in 2A19C.s): GT4/GT3/G4/G3 bank selection, signed NCLIP area, depth averaging,
OT insertion and XY packet writes. Backfaces clear low tag bits; far depth
preserves the prior tag. 3AF14's +9E==1 branch invokes it when packet storage
has been published. Other 3AF14 flag branches remain partial.

`DRAW1_polygon_submission` checks all four formats, both banks, same-depth
OT chaining, packet length/OT high-byte preservation, XY stride, backfaces
and far rejection. Normal and ASan/UBSan CTest pass 2/2 each; 1,087 native
cases. Logs `/tmp/pe-draw-tests.log`, `/tmp/pe-draw-san-tests.log`.

**New runtime evidence / next action:** a temporary probe sampled draw calls
through 120 frames (`/tmp/pe-draw-probe.log`): B89F8 camera rotation words
remain zero, projected XY repeats 007000A0, and depth is zero. Probe removed.
68B94's native tile cut omits `jal 65B70(B89F8,B8A18)` at retail 68BAC.
That initializer publishes BCFA4/BCFA8. Without it, 677FC's host guard skips
66800's camera matrix copy. Restore the complete 65B70 initializer at field
entry, then handle real polygon GP0 commands/rasterization. This is the next
concrete dependency, before treating missing pixels as just a draw issue.
No camera matrix was fabricated to conceal the missing initialization.

## PE-PROJ1 — mesh vertex projection (2026-09-04)

Goal incomplete: Aya is still not visible or playable. Added 3AC90 mesh
projection (3AC90..3AF14 in 2A19C.s): view x joint matrix composition,
record-indexed vertices, padded triples, screen XY output at B1644 and SZ
at A636C. 35558 now runs 6698C, clip decode, mode-0 hierarchy, existing
3A6A8 cut, and projection for resource-bearing actors before its deferred
3AF14 draw. Fixtures with unpublished joint storage retain their prior cut.

`PE_GTE_RTPT_coordinates` implements sf=1/lm=0 coordinate outputs with
integer UNR reciprocal, 44-bit MAC wrapping and coordinate saturation.
This is a coordinate-output view, not complete RTPT register emulation:
FLAG, IR0/MAC0 and persistent coordinate FIFOs are not modeled. 3AC90
consumes only returned XY/SZ. Hardware authority: psx-spx GTE coordinate
commands and division specification. Tests cover composition, indexed writes,
padded triples, inactive bones, near-plane/zero-depth saturation and screen
limits. Normal and ASan/UBSan CTest pass 2/2 each (1,086 native cases),
no sanitizer diagnostics. `docs/evidence/pe-proj1-mesh/REPORT.md` records validation.

Next: wire existing lighting cut where appropriate; finish 3A6A8's nonempty
path (currently no-op), 3AF14 → 3B144 draw submission and GPU polygon
rasterization. Then verify animation, pad motion and actual auditorium entry.
120-frame CLI still reaches frame-limit (`/tmp/pe-proj-120.log`); no visible
model claim. New regression initially exposed old isolated actor fixtures
without allocated joint storage; the actor continuation now requires that
storage to be published, consistent with constructor prerequisites.

## PE-POSE1 — branched joint hierarchy (2026-09-04)

Goal still incomplete: no visible/controllable Aya. 3A088's mode-0 walk now
handles -1/-2 hierarchy markers by saving/restoring GTE rotation and
translation, without consuming output matrix rows. Non-root bones use
record+8's signed Z offset (previously zero). 3D834 now invokes this walk
instead of its empty cut, so clip binding constructs world joint matrices.
Authority: 3A3B4..3A680 in `asm/disc1/2A19C.s`.

`POSE1_branch_stack_and_bone_offsets` covers nested branches, a rotated
parent, nonzero root/child offsets, sibling restoration, and output canary.
Normal CTest 2/2 (1,085 native cases); ASan/UBSan 2/2, same count, no
diagnostics. Logs `/tmp/pe-pose-tests.log`, `/tmp/pe-pose-san-tests.log`.
The final 31-slot scratchpad bound was additionally rebuilt/tested normally;
the sanitizer suite includes that bound. Unsupported stack underflow/overflow
records named boundaries. Modes 1/3/4 remain outside the mode-0 cut.

Correction to earlier next-step notes: 3B97C is **lighting**, not projection.
Its existing `lighting_cut` is still not called by 3D834/15240 (empty cut
remains). Vertex projection is 3AC90 (161 words), then 3AF14 → 3B144
submission. Both remain omitted from the actor walk. These are next, along
with animation advancement and input/motion verification at the entrance.

## PE-PKT1 — model polygon packets and texture relocation (2026-09-04)

Progress from INIT1; **Aya remains invisible and not playable**. Resource
construction now emits both packet banks for GT4/GT3/G4/G3, copies UV/CLUT/
tpage payloads, and applies retail 3D94C relocation. Actor type zero uses
353B4's texture parameters; other actor types use the field's packed table
at 35400..354B0. The returned packet cursor feeds matrix storage directly,
replacing the previous obj[0]*8 reserved-size guess. Pose and draw stay partial.

Independent MIPS execution oracle `pc_port/tools/pe_pkt1_model_oracle.py`
runs 3D078..3D5D8 and 3D94C..3DBE4 from the authenticated retail EXE.
`PKT1_retail_packet_bytes` compares whole 2-KiB packet-buffer FNV hashes:
all four kinds, both banks, emission disabled, four signed/half-page
relocation cases, and already-relocated sentinel. Normal and ASan/UBSan
CTest pass 2/2 each, 1,084 native cases, no sanitizer diagnostics. 120-frame real-disc CLI reaches frame-limit with the
background (screenshot `/tmp/pe-pkt-120.ppm`). Sanitizer result and details:
`docs/evidence/pe-pkt1-model-packets/REPORT.md`.

Next: full joint hierarchy/pose projection (3A088 negative parent markers,
3B97C) then 3AF14 → 3B144 draw submission. 3B144 uses projected XY at
B1644 and depth at A636C, performs NCLIP and inserts packets into B0E38's
active OT. Do not wire draw against unprojected vertices or claim completion
from construction alone. No changes to src/, YAML, or the retail executable.

## PE-INIT1 — visible M0010I background and actor resources (2026-09-04)

Goal remains active: **Aya is not yet visible or playable**. The retail
opening script now clears the fade and shows the limousine street outside
the theater (`/tmp/pe-init.png`, 120 frames). A 600-frame CLI run reaches
frame-limit without crashing. This is M0010I, not proof of auditorium entry.

35038 now allocates model storage, selects the retail animation command,
publishes the existing 3D050 instance state, initializes rotation/fade, and
binds the clip. Polygon packet/UV construction and full pose propagation
remain partial. Added VM dispatch 5B/2D/9D/75/74/7B; 9D uses the now-valid
model header. Added 39B74 clip decoders (byte and halfword tracks, metadata,
translation and joint overrides) plus alternate Euler rotation 79754.
Override indices outside this call's decoded rows retain a named boundary;
persistent scratchpad contents are not modeled.

Three real-disc boot fixtures now adopt retail startup globals after loading
the EXE, as the CLI does. Their previous arena address differed by eight
bytes and corrupted GPU packet lists with the newly active constructor.
New tests cover seven independently executed retail rotation vectors, both
clip encodings and a joint override; opening integration checks model storage,
clip binding, script pose and nonblack framebuffer output. Final normal and
ASan/UBSan CTest: 2/2 each, 1,083/1,083 native cases, no sanitizer diagnostics.

Next: finish 3D050 polygon constructors/3D94C, pose propagation (3A088,
3B97C), and actor draw 3AF14 → 3B144 (currently deferred). Trace script
progress and pad-controlled motion after rendering; don't infer playability
from nonblack pixels or passing tests. Report:
`docs/evidence/pe-init1-actor-resources/REPORT.md`.

## PE-SKIP2 — opening script reaches M0010I and spawns Aya (2026-09-04)

Continued progress; goal remains active, **not playable**. Added VM dispatch
for opening-field movie opcode 35 under --skip-movie (explicit HOST_ADAPTED
log), retail media waits 8C/8F, and return-one opcode 91. Default opcode 35
now stops honestly with its PC retained. M0431I's own script selects M0010I
(A8001048); its script creates Aya (type 0 / update 80035C84). No spawn or
story-state injection. Three SKIP2 tests exercise skip/default, wait retries,
and 120-frame real-disc transition/spawn.

Fixed 6B4F8 synchronous CD staging lifetime: consume previous-chunk texture
entries and drain their GPU uploads before the next read overwrites shared
memory. Before the fix the natural M0010I transition aborted at 81ABBFB0.
Report: `docs/evidence/pe-skip2-field-movie/REPORT.md`.

Next: Aya task stops after unwired 5B (800182A0, matched camera-flag clear).
Follow-on 2D/9D/75/74/7B require verification; 9D depends on constructor
collision state currently omitted by 35038. Presentation remains black
(`/tmp/pe-entrance-media.ppm` at 120 frames). Prior fade is subtractive white;
no player movement or visible model claim. Temporary probe code was removed.
Local RAM probe `/tmp/pe-entrance-ram.bin` is ignored/untracked outside repo.
Validation: normal and ASan/UBSan CTest 2/2, 1,081/1,081 native cases,
zero sanitizer diagnostics. Logs: `/tmp/pe-skip2-full.log`, `/tmp/pe-skip2-san.log`.

## Entrance continuation — clipping and reset fixes (2026-09-04)

Worktree contains earlier uncommitted BG1 GPU/tile-publisher work. Continued
from that state; do not discard it. Runtime `--skip-movie` loads M0431I,
but Aya is **not playable**. Current captured display remains black.

Fixed `func_80067294` using `asm/disc1/55C00.s`: X/Y/depth clip
comparisons mask to 16 bits; packet cursors advance on rejected tiles;
wrapped-layer X/Y adjustment follows the retail signed-halfword branches.
Previously the real entrance's container Y offset -8 rejected every tile.
Now the disc-backed 12-frame test observes 1,800 textured rectangles over
six field ticks, versus zero before. Added a two-bank clipped-first-tile
regression and strengthened BG1 integration to require GPU rectangle work.
The older tile-pointer-only assertion was insufficient.

Full-suite verification exposed a host destination cache surviving RAM
reset. Added `PE_RamGeneration()` and invalidated the field destination cache
when RAM is initialized/reset, so same-token New Game reloads its data.
Normal and ASan/UBSan CTest both pass 2/2 (1,078 native cases plus
external library link check). Local logs: `/tmp/pe-with-disc-tests.log` and
`/tmp/pe-asan-tests.log`; no sanitizer diagnostics in LastTest.log.
The BG1 byte-anchor/hash oracle passes; it does not execute the tile builder.

Next: trace display/fade/draw state (120-frame capture
`/tmp/pe-entrance-fixed.ppm` is black despite submitted tiles), then field
script/actor initialization. Probe at 12 frames found `D_8009D254 == 0`
(no Aya actor), overlay `0x4000A002`, camera 0, and valid tile lists.
Do not claim playability from rectangle counts or suite status.
Disc available at `rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin`;
`local/pe_disc1.path` is absent. Use an absolute `PE_DISC1_BIN` for CTest.

## PE-SKIP1 — skip-movie New Game → M0431I (2026-09-03)

`--skip-movie` now skips the 480-frame logo, `FMV001`, and the
untranslated title tail: `func_801909B4` returns 1, `func_8006E9A0(1)`
publishes `0xA80830C8`, and the first field tick's dest-change loads
M0431I (chunk2 head `0x8A1C` proven by `SKIP1_1220C_publishes_new_game`).
The 6E9A0 fade poll honors the host stop and, under skip-movie only,
forces `CFEE=1` after the first tick. Windowed runs write
`HostWindow_PadRaw` into `D_800BE9A2` before `3EB04`. Default boot
(no flag) is unchanged (frontier still `func_8010C89C`). Suite
1073/1073 with disc; 1054/1/18 gateless (1 = B54KY env). Evidence:
`docs/evidence/pe-skip1-new-game/REPORT.md`. Next: field spawn / walk
presentation in M0431I. FMV/title deferred until Aya is playable.

## PE-MV1d — func_8010C89C VLC decoder transcribed; frontier held (2026-09-03)

`func_8010C89C` (overlay `0x8010C89C..0x8010CBF8`, ~225w, resumable
VLC-style block decoder) is now transcribed in
`pc_port/game/boot/func_8010C89C_port.c`, proven by
`pc_port/tools/pe_mv1d_c89c_oracle.py` (module carve SHA + call-site
anchors + an independent decoder model over 6 vectors incl.
resume-equivalence) and 6 `MV1D_` tests. It is deliberately NOT wired
live into `func_801924F8`'s `got_frame` tail: the decoder's output is
bounded only by the VLC stream's own pad/terminator codes, and the
streaming pump does not yet deliver a fully MDEC-ready STR frame at
`s1` (Stage-1b), so decoding the current partial frame overruns the
2 MiB guest RAM (`PE_StoreU16 @ 0x80200000`). The production path keeps
its honest boundary stop at the decoder entry; strict real-disc
frontier is unchanged: `func_8010C89C` from `func_801924F8`. The prior
session's `/tmp/c89c_*` debug dump ("REVERT BEFORE COMMIT") is removed.
Suite: 1070/1070 with disc; 1052/1/17 gateless (the 1 = pre-existing
B54KY env case); ASan/UBSan CTest 2/2 with disc; oracle green. Leaves
560; no src/YAML changes. Evidence: `docs/evidence/pe-mv1d-c89c/REPORT.md`.
Next: Stage 1b STR/MDEC frame delivery so the decoder can go live and
the frontier moves into the `func_80192CE8` media loop after `0x80192E08`.

This is rung 0 of the boot->FMV->title->New Game->M0431I playable
ladder (plan `boot-to-theater_playable_ladder`); the remaining stages
(MDEC video pipeline, title/menu overlay translation, pad substrate,
field entry, field tick + collision, GTE/GPU presentation) are each
multi-rung retail-accurate efforts tracked in that plan.

## MACHINE / WORKTREE TOPOLOGY (read before running or committing anything)

One GitHub remote, several checkouts, one build environment. Every
failure of 2026-08-2x week (false-claim commit, wrong-machine `mv`,
a Codex session that couldn't find its branch) was an agent not knowing
one of these lines:

- **Remote (single source of truth):**
  `github.com/Blizz127/Parasite-Eve-Decompilation.git`. All lanes are
  branches of this one repo: `main` (published), `grind/continuous-decomp`
  (grind lane: PE-BTL leaf work + `pc_port/` native), `leaves/*` (desktop
  leaf lane), `sync/laptop-*` / `wip/laptop-*` (laptop machine). **Push
  after every commit** — local-only progress on any machine is unbacked.
- **`~/dev/parasite-eve`** (desktop): primary decomp worktree. This is
  the ONLY checkout where leaf work is built and committed.
- **`~/dev/pe-continuous-decomp`** (desktop): separate clone of the same
  remote, checked out on `grind/continuous-decomp`. Read-only source for
  ported leaves and evidence. Do not build or commit there unless the
  task explicitly says so. After `grind/continuous-decomp` is updated
  from elsewhere it needs a `git pull` before use.
- **Build environment:** the host has NO mipsel toolchain and no
  distrobox. Builds run in docker image `pe-mipsel-img:latest`, built
  from `dev/mipsel/Dockerfile` (tracked in-repo):
  `docker run --rm -v "$PWD:/workspace" -w /workspace --user
  "$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh`.
  If `python3: command not found` appears, the image is stale — rebuild
  it from the Dockerfile. Splat runs on the host (`.venv/bin/splat`).
- **Git-ignored build inputs that exist only locally** (never in the
  repo, must be provisioned per machine): `rom/image/`,
  `build/extracted/` (retail EXE), `tools/era/` (fetched by
  `scripts/setup_era.sh` around the tracked maspsx patch), `.venv/`.
- **Agent prompts:** hand work to agents using
  `docs/ai_context/PROMPT_TEMPLATE.md`. Named branches in the prompt,
  worktree path included, always.

## Native field-runtime library boundary (2026-08-31)

The reusable CMake target `pe_field_runtime` now produces
`pc_port/build/libpe_field_runtime.a` from the 201 current translated/runtime
translation units. `parasite-eve-port`, the 994-case native suite, and a
standalone external-consumer smoke test all link the archive; CLI-only
`port_main.c` and `host_window.c` remain outside it. Normal and fresh
ASan/UBSan CTest runs pass both consumers, and real-disc strict execution
now stops at `func_80081314_func_8007F0C8_cut`, after the authenticated
libpress, record-pool, stream-control, CD idle-wait, blocking CdlSetloc, and
CdlReadS callback-registration sequence.

This is a verified product/build boundary, not a semantic-completeness claim:
there is still no complete Day 1 field runtime and scheduler provenance is
still `NEEDS_ARTIFACT`. Evidence:
`docs/evidence/pe-field-runtime-library/REPORT.md`; consumer notes:
`pc_port/docs/field_runtime_library.md`.

## PE-OTC1 — ClearOTagR translated + DrawOTag decision (2026-09-02)

`func_800752AC` (43w) + its `jtb[11]` worker `func_80076354` (56w) are now
translated in `pc_port/game/boot/func_800752AC_port.c`. Static data proves
the worker programs DMA6/OTC (`D_80095864/68/6C/70` =
`1F8010E0/E4/E8/F0`); native performs the hardware terminator fill
synchronously (`OT[i] = OT[i-1] & 0xFFFFFF`, tail `*ot = 0x0009580C` /
`D_8009580C = 0x004957F8`) and completes during the first wait poll.
DPCR goes through the shared PE_GPU authority. The boot adapter's NULL-OT
is a logged no-stop skip until adapter removal. 5 focused tests; full
normal suite 1015 run / 998 passed / 1 pre-existing environmental failure
(B54KY missing disc path, identical on base) / 16 skipped; ASan/UBSan
(leak check off, sandbox ptrace) zero diagnostics. Evidence:
`docs/evidence/pe-otc1-clearotagr/REPORT.md`; oracle:
`pc_port/tools/pe_otc1_clearotagr_oracle.py`. The four 6E9A0-calling tests
now seed `jtb[11]` for the production arm. Landed incidentally: an
unguarded `jtb` deref crashed the suite (buffered output hid the site;
`stdbuf -o0` used after); all guest-pointer derefs are now range-checked.

`pc_port/docs/drawotag_decision.md` records the DrawOTag rule before any
DrawOTag work: guest builds the OT in guest RAM, host walks it read-only
at the existing `jtb[2]` dispatch; adapting DrawOTag would make field work
unbounded and break read-only presentation. It also flags the known
level-`>= 2` early-return defect in the partial `func_800754E4` port
(retail prints then continues) for the DrawOTag rung.

Byte-accuracy pass (same day): every retail word of both bodies re-audited
against the EXE bytes — one real fix (77404-prefix stores `polls+1`
BEFORE the limit test, as retail orders it), all 15 load-bearing
immediates now machine-checked in the oracle, tail constants re-derived
bit-by-bit (`0x004957F8` / `0x0009580C`). Explicitly NOT a matching leaf:
no `src/` code added or claimed; the era/docker byte-exact gate is unrunnable
in this session (no mipsel toolchain, `cc1` seccomp-killed, docker denied).
Incidental: `asm/disc1/66B54.s` hex comment at `0x80076390` misprints the
`sll` word (`80101000` vs EXE `00101080`) — mnemonic assembles correctly,
comments are not authority.

## PE-EV1 — func_80042798 translated (2026-09-02)

44-word event-record cleanup walk now translated in
`pc_port/game/boot/func_80042798_port.c`: two records at `D_800A0ED4+1` /
`+0x419` (stride `0x418`), tags 8/10 fire the `0xB0`-vector trampoline
`func_80072774` (narrowed callee boundary) then stamp word `-1` / tag 12.
Fits the translated `func_80042538` lifecycle (memsets the block, writes
the two `-1` sentinels this walk consumes). The 5C1EC zero path calls the
real walk; its test migrated to `_translated_walk`. 3 focused tests; full
suite 1018 run / 1001 passed / same single pre-existing environmental
failure / 16 skipped. Evidence: `docs/evidence/pe-ev1-42798-cleanup/REPORT.md`;
oracle: `pc_port/tools/pe_ev1_42798_oracle.py`.

## PE-FD1 — func_8006E9A0 adapter removed (2026-09-02)

First adapter removed rather than catalogued. The HOST_ADAPTED
single-pass stand-in is now the retail-faithful translation of the
141-word matched leaf `src/func_8006E9A0.c` in
`pc_port/bootstrap/func_8006E9A0_port.c`: display init, republished
19-store arena, `5E588` + `66B60(2)`, the real
`do/ClearOTagR(lookup[CDDC])/68E24/70E54/while(CFEE&3 != 1)` loop, and
the arg-1 `0xA80830C8` / arg-3 dispatch. Termination proven: 66B60 arms
2/arg/0 unconditionally, 68E24 counts down to CFEE=1 (2 iterations in
fixtures). `70E54` stays a stub boundary inside the loop — the honestly
exposed next New-Game rung; strict production unaffected (frontier stops
inside 801909B4 first). `BTL41_6E9A0` migrated to the retail contract;
new `FD1_` test proves the arena-OT fill. Full suite 1019 run / 1002
passed / same single pre-existing environmental failure / 16 skipped.
Evidence: `docs/evidence/pe-fd1-6e9a0-loop/REPORT.md`.

## PE-TOK1 — token 0xA80830C8 is M0431I (2026-09-02)

The New-Game token unpacks through the retail `6E2D0`/`6E454` pair to
name `M0431I`, index 431, package slot 430 (`rel 0x182CF`,
`packed 0x01200911`: 17+9+18 PE.IMG sectors into overlay
`+0x194`/`+0x168`/`+0x18C` via the translated `dest_load_cut`). Arg-3
token `0xA80651C8` is `M0353I` (slot 352). Decode reuses the
BTL150-proven 5-bit scheme, validated against both proven tokens;
route is the 1220C else arm (`0xA80830C8 < 0xA9400048`,
`!= 0xA8000048`) into the field tick's dest-change latch. Two focused
tests run the retail unpacker itself. Full suite 1021 run / 1004 passed
/ same single pre-existing environmental failure / 16 skipped.
Evidence: `docs/evidence/pe-tok1-m0431i/REPORT.md`; oracle:
`pc_port/tools/pe_tok1_m0431i_oracle.py`. Incidental: the unpack
charset is 32 entries (`...r-w` + `y` at 31; no j/q/x/z) — the oracle
pins all 32.

## PE-FLD1 — M0431I field entry proven (2026-09-02)

`dest_load_cut(0xA80830C8)` loads the New-Game map through translated
machinery with zero new port code: 17+9+18 sectors from `B0DD8+0x182CF`
into the overlay dests, heads `0x8064`/`0x4050`/`0x8A1C`/`0x89B4`
(mirror-BTL63 disc-gated test, skips without the image). Expected heads
derived from the user-supplied Disc 1 image at 2352+24 layout; the
method was validated by reproducing all three `m0367i` heads first.
With `PE_DISC1_BIN` set (env only, no repo/`local/` changes) the FULL
suite is green: 1022/1022/0/0. Standard gateless-disc gate unchanged:
1022 run / 1004 passed / 1 pre-existing environmental failure / 17
skipped. Evidence: `docs/evidence/pe-fld1-m0431i-entry/REPORT.md`.

## PE-DRW1 — DrawOTag read-only chain walk + FILL (2026-09-02)

Per the decision note: `753B4` translated (28w, worker passes through
to `76C34`); `76B98` generalized from single terminal packet to the
multi-node DMA walk (head validates pre-GP1, later nodes progressive,
size-0/zilch links and out-of-RAM tails end silently, unknown shapes
stay named cuts, no cycle guard — retail hangs identically);
`754E4`'s deferred dispatch completed + its inverted level gate fixed
(7506C pattern); GP0(02h) FILL implemented (every `isbg` DR_ENV ends in
one — no field frame draws without it) with `fill_*` telemetry. 6
focused tests; BTL88 now runs the production arm (draw executes, FILL
renders). Incidents fixed along the way: `jtb[6]` seeded at the wrong
slot in my own fixture, unguarded `jtb` derefs in `754E4`/`75424`, and
a zero-tag spin on uninitialized links. Full suite 1028 run / 1010
passed / same single pre-existing environmental failure / 17 skipped;
ASan/UBSan zero diagnostics. The vis1 `float_in_path` defect list
targets the `native/` tree, which is not in this repo — the in-repo
answer is this rung (fixed-point-hostile doubles never entered the
walk; all coordinates stay integer words into the parser).
Evidence: `docs/evidence/pe-drw1-drawotag-walk/REPORT.md`; oracle:
`pc_port/tools/pe_drw1_drawotag_oracle.py`.

Next: VRAM→host presentation at PutDispEnv (PRS1), then the CD
completion-selector tail (CDS1) so the strict run passes the first
CdlReadS, then field frames, then the theatre.

## PE-FTE1 — func_80070E54 frame tail matched + translated; field tick routed (2026-09-03)

Two new byte-exact leaves on era `-O2 -G8`: `func_80070E54` (86w, carve
`0x61654`, resume `0x617AC`) and `func_80042FE8` (20w, carve `0x337E8`).
Docker gate EXACT SHA-1 `452fb033…37b`, `verify_us.sh` PASS, **560
leaves** (was 558). Levers: unknown-size-array externs keep `D_800B0CD8`
/ `D_800B0E54` out of sdata under `-G8`; the OT pointer is
`D_800B0CD8[0x58 + CDDC]` (retail reuses the `$a0` base for
`lw 0x160($v0)`); the 6EC08 test is a `(signed char)` cast (`sll 24`).
Native: `pc_port/game/boot/func_80070E54_port.c` translates 70E54 /
42FE8 / 6EBE4; the `psx_compat.h` 70E54 stub is gone, the 6E9A0 fade
loop runs the real tail (guest-RAM `D_8009CDDC`, same word the 3F3C4
port flips), and `func_8003F3C4_port.c` calls the real 70E54 at the
retail `0x8003F590` site instead of an inline prefix — the field tick's
draw now goes DrawOTagEnv → 754E4 → 76C34(76B98) walk. 7 `FTE1_` tests;
stub guard pins the three names; 6E9A0 fixtures seed `jtb[2]/[6]`.
Incidental (ASan): `func_8007F0C8` (CDQ1) walked its packet frame from
`+0x30` and read past it; retail walks `sp+0x10` upward for 4 packets
(9, 0x0E, 2, cmd), the loc bytes land at `fr[0x21..0x24]`, and packet
1's gate word is the nonzero `sp+0x21` address — all three fixed, the
CDQ1 queue test now asserts the retail descriptor layout. Suite: 1043
run / 1025 passed / 1 pre-existing env failure / 17 skipped; with
`PE_DISC1_BIN` 1043/1043; ASan/UBSan zero diagnostics. Strict real-disc
run still stops at `func_8007F0C8_completion_selector`. Evidence:
`docs/evidence/pe-fte1-70e54-frame-tail/REPORT.md`; oracle:
`pc_port/tools/pe_fte1_70e54_oracle.py`.

## PE-PRS1 — VRAM → host framebuffer display presentation (2026-09-03)

`func_800755F0` (PutDispEnv) is now the host display authority:
`HostFB_PresentDispEnv(guest env)` copies the DISPENV disp-RECT VRAM
window through `PE_GPU_ReadVRAM` + `HostVRAM_DecodePixel` (mask off →
black; budget kept; zero guest writes; no retail words translated, so
no oracle — retail's 318-word GP1 body stays out of scope). All six
call sites pass guest addresses; `game_port` gained a present hook
(`PE_Port_SetPresentHook`, cleared on reset) that windowed `port_main`
uses for live blit+poll. 5 `PRS1_` tests; suite 1048 run / 1030 passed
/ 1 pre-existing env failure / 17 skipped, 1048/1048 with
`PE_DISC1_BIN`; ASan/UBSan zero diagnostics; `--bootstrap-disc
--max-frames 1 --screenshot` is byte-deterministic across runs.
Evidence: `docs/evidence/pe-prs1-dispenv-present/REPORT.md`.

## PE-CDS1 — CD completion-selector tail translated; stop at func_8007FCFC (2026-09-03)

Both 7F0C8 non-passing tail arms return the queue sequence (retail
delay-slot `addu $v0,$s5,$zero`), verified word-for-word; the passing
arm runs the real `func_8007E8F4` → `func_8007FB44` (28/31 words,
`pe_libcd.c`), which latches 0x1F/lane-2/0xB and stops at the
control-heavy `func_8007FCFC` (74 words, 7B9EC/7B558 dispatch — new
named stop, 7B558/7C564 out of scope). Old
`7F0C8_completion_selector` boundary deleted. 6 `CDS1_` tests;
migrated CDQ1×2, B54KAD/AE, B54KY to the new stop (+ live latches).
Incidental: 6AD40 indexed the DISPENV pair by wild `[0x800ACDDC]`
(strict-run FATAL); retail uses CDDC — fixed. src/ attempts for
7E8F4/7FB44 parked (shared-lui coloring; count stays 560). Suite
1054 run / 1036 passed / 1 env failure / 17 skipped, 1054/1054 with
disc; ASan 100% with disc env, zero diagnostics; strict stops at
`func_8007FCFC` from `func_8007FB44`, zero HOST_ADAPTED. Evidence:
`docs/evidence/pe-cds1-selector-tail/REPORT.md`; oracle:
`pc_port/tools/pe_cds1_selector_tail_oracle.py`.

## PE-WIRE — 7FB44 wired live; new frontier at func_8007B9EC (2026-09-03)

One boundary removed: 7FB44 calls the real 7FCFC (retail has no
`sltu` there — it lives in 7E8F4). First live run died on
`PE_StoreU8 @ 0x1F801800`: the EXE pre-initializes the pointer
tables to CD registers (B27C/B280/B284/B288 = 0x1F801800-03,
B28C = 0x1F801020, read back from the SHA-1 EXE), so 7B9EC's
first effect pokes hardware and only its RAM tail runs last —
reverted to an entry stop (no caller consumes the 0x1325).
Strict exit 1 at `func_8007B9EC` from
`func_8007FCFC/8007B010/8007B558` (NOT 7C564 — next rung is the
CD-register handshake). Non-strict aborts honestly at the first
hardware touch; PPM unchanged 3912/76800. 7 sites migrated
(B558_PlantChain; B54KY keeps real state, 7B9EC == 2 via the
B010-timeout trail, 801918F8 arg4 assert caller-scoped). Suite
1064/1064 disc twice; CTest + ASan 2/2. Evidence:
`docs/evidence/pe-wire-7fb44-live/REPORT.md`.

## PE-B558 — CD command-issue controller translated; stop stays at func_8007FCFC (2026-09-03)

Second attempt (G6): `func_8007FCFC` (74w) + `func_8007B558` (259w)
+ `func_8007B010` (160w) + `func_8007B9EC`/`73DE8` transcribed in
`pe_libcd.c`; attempt 1 misread four retail details, corrected
word-by-word (7FCFC always-latch via FD28 delay slot, conditional
8-arm via FDC0 beq, B010 entry-hoist with B258→B090 restart, B558
B74C explicit branch + timeout-print a2 = AFDC[[AFD5]]). 10
`B558_` native tests (latches, arms, both print arg sets, wrapper
end-to-end returning 1 with `[B580] = [B558]`, no stop on any
exercised path); oracle extended with the B558 prologue/gate
spot-words (fail-once proven, exit 1 perturbed / 0 restored). The
CDS1 stop is NOT flipped: 7FB44 still stops, so no existing test
moves. src/YAML untouched; count stays 560. Evidence:
`docs/evidence/pe-b558-cd-controller/REPORT.md`; oracle:
`pc_port/tools/pe_b558_cd_controller_oracle.py`.

## PE-MV1a — 870F0 translated + 801924F8 tail to the 7A88C frontier (2026-09-03)

`func_800870F0` (42 words) transcribed (`870F0_port.c`): `[D2C0] & 2`
predicate, `((2903 * a0) >> 13) & 0xFF` scale chain (proven x256 in
the oracle), four `sb` per arm, tail call to the `func_8007A88C`
named stop. `func_80191B64` transcribed (`91B64_port.c`, scratch
`0x801FFF20+`); `func_8007C484` transcribed into `7A214_port.c`;
74F44 needed nothing (pre-existing ClearImage wrapper — duplicate
deleted). `func_801924F8` tail transcribed in place (`cdready_wait`
re-poll, 870F0 issue, BD4C stop for MV1b, E0 poll + give-up
re-issue, got_frame entry-stop at C89C with `s3 = 0` sole-caller
evidence); old `801927B0_cut` deleted. Strict disc run now stops
at `func_8007A88C` from `func_800870F0` (whole translated boot
travelled). B54KAD/AE migrated to the new boundary, intent
intact. Suite 1064/1064 disc twice, 1046/1 env/17 gateless;
CTest 2/2; fresh ASan/UBSan CTest 2/2 zero diagnostics. Oracle:
`pc_port/tools/pe_mv1a_870f0_oracle.py` (green). Evidence:
`docs/evidence/pe-mv1a-870f0-tail/REPORT.md`. Leaves 560; no
src/YAML changes. Next: MV1b (BD4C/C89C decoders, 7A88C callee).

## PE-MV1b — 7A88C/7B964 live; BB0 fix; BD4C parked on delivery (2026-09-03)

7A88C (8w, splitter gap) + 7B964 (34w, CD command-poke through
the shadow) live in `870F0_port.c`; B54KAD/AE pin the shadow
bytes (3,0,0,0x20 + mailbox 0x1325). 91B64 BB0 inversion fixed
against the overlay bytes (zero proceeds, nonzero retries to
the DB0 return-0). BD4C transcribed + verified on the real
B54KY stream (2239 ops, full 69632-byte frame) but REVERTED to
its stop: E0 give-up needs the 7C564 delivery pump (no state-2
slots, lane never -1 — no producer in the translated tree), so
live BD4C hung B54KY/strict in give-up. Reland transcription +
plant design preserved in
`docs/evidence/pe-mv1b-7b964-bd4c/REPORT.md`. Strict frontier
stays `func_8010BD4C`. Suite 1064/1064 disc twice, 1046/1
env/17 gateless; CTest 2/2; ASan 2/2 clean; MV1a oracle green.
Leaves 560; no src/YAML changes. Next: 7C564 delivery pump
(per-sector 7C214 + lane -1), then BD4C reland → C89C → movie.

CDQ2 analysis (2026-09-03, no source changes): 7C564 fully
mapped (583w, all exits → `[B374]` status, callees
7A488/7CE80(pure)/7CEAC(HW spins)/7C444/7C214-conditional) but
contains NO lane write and NO -1 — transcribing it alone does
not unblock E0. Handlers 7F7E8/7F88C also lane-clean. Only ONE
absolute-addressed lane-area store in the EXE (B570); all lane
writes are `$gp`-relative (BIOS `$gp`, no `lui $gp` found).
Next rung must find the -1 writer first (gp-relative analysis),
then design the E0 pump, then reland. Evidence:
`docs/evidence/pe-cdq2-delivery-firewall/NOTE.md`.

CDQ2b (2026-09-03, no source changes): retail 7FB44 bytes prove
lane access is pointer-base + small offset
(`a2=&B598`: `[B554]=[a2-0x44]`, `[B574]=[a2-0x24]`,
`[B570]=[a3+0x1C]`, `[B578]=[a3+0x24]` in the jal delay slot)
— prior scans were structurally blind; `$gp` is dynamic
(`addiu $gp,$a2,imm`), killing static gp analysis. REFRAME:
pump first (-1 may be error recovery never taken on the happy
path); E0 success needs 7C214 invocation cadence with record
setup, B0's 81314 arm already installs it. Evidence:
`docs/evidence/pe-cdq2-delivery-firewall/CDQ2b-pump-first.md`.

CDQ2c (2026-09-03, probe reverted, tree clean): B54KY state at
the BD4C boundary is `C0DC8=80142100, indices 0, B0CC8=0,
B574=2` — a single 7C214 pump publishes state 2 with no stop,
and E0's first poll then takes got_frame (give-up/-1 never
reached). AD/AE need the MV1B plant for the pump. Next rung:
prototype the pump → BD4C reland → C89C.

CDQ2d (2026-09-03): pump live (7C214 once per E0 poll,
interrupt-surrogate, 3rd bounded adaptation in
RETAIL_ACCURACY.md); BD4C relanded verbatim; E0 takes
got_frame poll 1 on synthetic AND real paths (B54KY pins
C89C == 1, record promoted to 4, `[DBD]`/`[D111B0]` latches).
Strict frontier `func_8010C89C` from `func_801924F8`. Suite
1064/1064 disc twice, 1046/1 env/17 gateless; CTest 2/2;
ASan 2/2 clean; MV1a oracle green. Evidence:
`docs/evidence/pe-cdq2d-e0-pump/REPORT.md` (+
`pe-mv1c-c89c-map/NOTE.md`: C89C fully mapped — resumable VLC,
11 static state words, COP0 IEc touch, exits 0/1 — parked on
inputs: `a1 = lw[4]` BIOS word proven byte-exact, `a2` caller
setup undumped). Leaves 560; no src/YAML changes. Next: s1
`[4]`-blocker (BIOS/low-RAM model or measured value), then
C89C transcription → EC → movie.

MV1c-forensics (2026-09-03): s1-path byte-verified from the
tail dump (`lbu→xori→sb→sll→addu s3→lw a1`); `s3 = 0`
triple-proven (zero + full-function one-spill scan +
post-jal `move v0,s3`); `jal 801924F8` at `80192E00` leaves
`a1`/`a2`/`a3` as 80191FB8's exit leftovers — C89C has TWO
unknowable inputs, transcription parked per the BB0 lesson
(structural-only verification refused). Bit-bucket hypothesis
recorded (KUSEG low RAM never read post-boot; real handoff
via 7C394/EB90/COP0) with its proof obligations. TEMP dump
probe reverted; suite 1064/1064 after revert. Evidence:
`pe-mv1c-c89c-map/NOTE.md`. Next: `[4]`-blocker decision
(low-RAM model vs measured BIOS value) or EC-side progress
(7C394-after-C89C needs C89C first — same blocker). Follow-up: a2 chased
two levels (91FB8 epilogue returns the 2nd MoveImage result; MoveImage
path A on retail args returns its [[D_80095744]+8]-callee exit-a2, path B
inherits entry-a2 with ret -1); level 3 needs runtime-pointer resolution
— parked under new blocker `mv1c-c89c-unknowable-inputs`. TEMP probe
residue found un-reverted and reverted; tree clean. CORRECTION
(2026-09-03, same session): both C89C "unknowables" were forensic
errors. The verified tail (correct disc carve: PE.IMG extent 1013 +
`0x3D2`, oracle anchors match) sets `s3 = 0x801D1464` @`801927C0`
(the "s3 = 0 triple-proof" covered 80192CE8's lifetime, not the
tail's), loads `a2 = [0x801D0DF8]` @`80192814`, `a1` @`8019284C`,
`a0 = s1` in the `jal C89C` delay slot; C89C kills entry-`a3`
(`a3 = a2+0x10000` @`8010C8A8`). First pass:
`a1 = [0x801D1468]` = movie-buf pointer — no BIOS word, no
low-RAM model needed. State area is `0x8011EB8C..` (not `0x8012`).
Blocker marked RESOLVED; full Correction record in
`pe-mv1c-c89c-map/NOTE.md`. Next: transcribe C89C + got_frame/EC.

## Run summary 2026-09-03 — field-frame run (this session)

- Landed: Rung A (FTE1 frame tail, matched 70E54/42FE8, 560 leaves)
  + Rung D (3F3C4 routed), Rung B (PRS1 VRAM→fb present + hook),
  Rung C (CDS1 selector tail, stop at 7FCFC), RUNGE analysis,
  B558 controller chain, WIRE (7FB44 live, frontier 7B9EC).
  Repaired:
  stale `DISC1_MATCHING_STATUS.md` (verify_us.sh PASS again).
- Parked with reason: src/ 7E8F4/7FB44 (shared-lui coloring, G6;
  `parked_blockers.json`). Next rung: wire 7FB44 → real 7FCFC,
  then the 583-word 7C564 delivery machine behind the completion
  callback (RUNGE unblock list).
- Final stops: strict exit 1
  `FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider:
  func_8007B9EC / called from: func_8007FCFC/8007B010/8007B558`;
  non-strict aborts honestly at the hardware wall
  (`FATAL: PE_StoreU8: invalid guest address 0x1F801800`, B558 B6A0
  through the real table — no guards invented). Screenshot
  `/tmp/pe-wire-final.ppm` 320x240 non-black 3912 of 76800
  (identical to baseline; CD commands don't draw). Zero
  `HOST_ADAPTED` lines. Field frames NOT reached: the frontier is
  the CD-register handshake now, before any New-Game menu.
- Suites: `Results: 1064 run, 1046 passed, 1 failed, 17 skipped`
  (gateless; 1 = pre-existing B54KY env case),
  `Results: 1064 run, 1064 passed, 0 failed, 0 skipped` (disc,
  twice). Normal CTest with disc:
  `100% tests passed, 0 tests failed out of 2`. Fresh ASan/UBSan
  CTest with disc: `100% tests passed, 0 tests failed out of 2`,
  zero diagnostics. Oracles green: otc1, ev1, tok1, drw1, fte1,
  cds1, b558 (fail-once proven). Leaf count 560.
- Windowed smoke (`DISPLAY=:10.0`): window opened 640x480, boot ran
  to the 7FCFC frontier, present hook executed without crash; no
  still frame held (strict aborts at the boundary).
- Commits (all pushed): `93479c0` FTE1, `c8af84e` status-doc,
  `0cd55ad` PRS1, `0958845` CDS1, Rung-E docs, B558, WIRE (this).

## Main-lane YAML build authority (merged 2026-09-01)

`configs/USA/disc1.yaml` owns Disc-1 span edges, source/object mapping, trim
size, link order, verifier entry, and the published exact count; compiler-only
exceptions live in `configs/USA/disc1_build_profiles.json`. The build and
verifier scripts are generic drivers with no leaf lists, and tracked extra
function C files require explicit nonmatching dispositions. Generated status
documents provide the current matching and native metrics. The preceding
main-lane B54K-B1..B6 narrative remains historical evidence; later B54K and
field-runtime work in this handoff supersedes its old strict frontier.

## Matching leaf — func_8004CDAC (2026-09-01)

`func_8004CDAC` is now a registered 10-word exact C leaf: it forwards the
constants `0x28` and `0x3D` to `func_80062F3C` in order. The YAML split now
ends the preceding assembly span at `0x3D5AC` and resumes it at `0x3D5D4`.
The full rebuild and packed-span verifier pass at retail SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; matching count is 412.

## Matching leaf — func_8004CDD4 (2026-09-01)

`func_8004CDD4` is now a registered 21-word exact C leaf. It calls the
two-component accumulator with `(0, 10)`, selects `D_800A1A20` or its
`+0x40` record by `state+0x24 == 0x3D`, then calls `func_8005F594`. The
record pointer is deliberately constrained to `$a0` only after the first
call; that matches retail's address materialization and avoids extending the
frame. The full build and all packed-span checks are exact at 413 leaves.

## Matching leaf — func_800703F4 (2026-09-01)

`func_800703F4` is now a registered 10-word exact C leaf. It invokes the
retail teardown phases `func_800702DC` then `func_800701B4`; the YAML spans
the wrapper at `[0x60BF4, 0x60C1C)` and resumes generated assembly afterward.
The full rebuild, public verifier, and all packed-span checks are exact at
414 leaves.

## Matching leaf — func_800504F4 (2026-09-01)

`func_800504F4` is now a registered 10-word exact C leaf. It forwards the
two GP-relative words at `D_8009CF44` and `D_8009CF48` to `func_80042020`.
It uses the existing era `-O2 -G8` profile, which reproduces retail's two
loads before the call frame. The full rebuild, public verifier, and all
packed-span checks are exact at 415 leaves.

## Matching leaf — func_8005051C (2026-09-01)

`func_8005051C` is now a registered 10-word exact C leaf. It is the adjacent
GP-relative twin of `func_800504F4`, forwarding `D_8009CF44` and
`D_8009CF48` to `func_80042170` under the same era `-O2 -G8` profile. The
full rebuild, public verifier, and all packed-span checks are exact at 416
leaves.

## Matching leaf — func_80050544 (2026-09-01)

`func_80050544` is now a registered 15-word exact C leaf. When its second
argument is nonzero, it performs the two retail constant calls (`0x1F`, then
`0x45`) and publishes `func_800504F4` through `func_80042B50`; otherwise it
returns after the shared epilogue. The full rebuild, public verifier, and all
packed-span checks are exact at 417 leaves.

## Matching leaf — func_8005DA8C (2026-09-01)

`func_8005DA8C` is now a registered 10-word exact C leaf. It returns null for
an unsigned index at least `0x41`, otherwise it returns the corresponding
16-byte record address in `D_80092478`. The source uses the retail inverse
predicate (`>= 0x41`) so era emits the required `beqz` branch polarity. The
full rebuild, public verifier, and all packed-span checks are exact at 418
leaves.

## Matching leaf — func_8005DAB4 (2026-09-01)

`func_8005DAB4` is now a registered 10-word exact C leaf, the 32-byte-stride
twin of `func_8005DA8C`. It returns null for an unsigned index at least
`0x41`, otherwise the matching record address in `D_80092888`. The full
rebuild, public verifier, and all packed-span checks are exact at 419 leaves.

## Matching leaf — func_8005F844 (2026-09-01)

`func_8005F844` is now a registered 12-word exact C leaf. It writes two
GP-relative state words from independent boolean-selected literal expressions
(`0x3A1C`/`0x395D` and `0xCC`/`0x84`), then always writes `0xA4`. The
independent assignment form is required to reproduce the two retail `bnez`
delay slots. The full rebuild, public verifier, and all packed-span checks are
exact at 420 leaves.

## Matching leaf — func_8005E54C (2026-09-01)

`func_8005E54C` is now a registered 12-word exact C leaf. It reads the
GP-relative activation word `D_8009D0E8`, returns zero when inactive, and
otherwise returns `func_8005E038()`. The full rebuild, public verifier, and
all packed-span checks are exact at 421 leaves.

## Matching leaf — func_8005DC28 (2026-09-01)

`func_8005DC28` is now a registered 9-word exact C leaf. It adds its input to
the global offset `D_800A8050` and returns the resulting byte from
`D_800A8028`. It uses the existing era `-O2 -G0` three-word-symbol profile so
maspsx retains retail's indexed symbolic byte-load macro shape. The full
rebuild, public verifier, and all packed-span checks are exact at 422 leaves.

## Matching leaves — func_8005DBF8 / func_8005DC10 (2026-09-01)

`func_8005DBF8` and `func_8005DC10` are now registered six-word exact C
leaves. Each loads a global word then adds its record-base address minus its
field offset (`0x18` and `0x20`, respectively). Explicit `$v0` address and
`$v1` loaded-value lifetimes reproduce retail's materialization order. The
full rebuild, public verifier, and all packed-span checks are exact at 424
leaves.

## Matching leaf — func_8005DE70 (2026-09-01)

`func_8005DE70` is now a registered six-word exact C leaf. It loads
`D_800A8044` and adds that record-base address minus `0x1C`; the explicit
`$v0` address and `$v1` value lifetimes preserve retail materialization. The
full rebuild, public verifier, and all packed-span checks are exact at 425
leaves.

## Matching leaf — func_8005421C (2026-09-01)

`func_8005421C` is now a registered nine-word exact C leaf. It passes its
input minus one to `func_8005DB44` and returns byte 6 of the resulting record.
The full rebuild, public verifier, and all packed-span checks are exact at 426
leaves.

## Matching leaves — func_80057D18 / func_8005C144 (2026-09-01)

`func_80057D18` reads and clears a signed halfword in the GP-relative
`D_8009D048` table, returning the original `int` value; the return width keeps
retail's `lh` plus return-delay `sh` form. `func_8005C144` stores the low byte
of `func_80033A20()` to `D_8009D02C` and then calls `func_800339A0(0)`. Both
use era `-O2 -G8`. The full rebuild, public verifier, and all packed-span
checks are exact at 429 leaves.

## Matching leaf — func_8006EC6C (2026-09-01)

`func_8006EC6C` is now a registered six-word exact C leaf. It sign-extends a
halfword index, scales it by four, reads an offset at that base location, and
returns base plus the offset. The full rebuild, public verifier, and all
packed-span checks are exact at 430 leaves.

## Matching leaf — func_80073E10 (2026-09-01)

`func_80073E10` is now a registered six-word exact C leaf. It returns the old
unsigned halfword from `*D_80095674` and stores its input to that same address
in the return delay slot. The full rebuild, public verifier, and all
packed-span checks are exact at 431 leaves.

## Matching leaf — func_8007A3CC (2026-09-01)

`func_8007A3CC` is now a registered eight-word exact C leaf. It is a direct
frame-and-return wrapper around `func_8007B9EC`. The full rebuild, public
verifier, and all packed-span checks are exact at 432 leaves.

## Matching leaf — func_8007A468 (2026-09-01)

`func_8007A468` is now a registered eight-word exact C leaf. It is a direct
frame-and-return wrapper around `func_8007B010`. The full rebuild, public
verifier, and all packed-span checks are exact at 433 leaves.

## Matching leaf — func_8007D054 (2026-09-01)

`func_8007D054` is now a registered eight-word exact C leaf. It forwards zero
to `func_8007D074`, with the zero materialization in the call delay slot. The
full rebuild, public verifier, and all packed-span checks are exact at 434
leaves.

## Matching leaf — func_8007EE64 (2026-09-01)

`func_8007EE64` is now a registered eight-word exact C leaf. It is a direct
frame-and-return wrapper around `func_8007FB04`. The full rebuild, public
verifier, and all packed-span checks are exact at 435 leaves.

## Matching leaf — func_800C6EE8 (2026-09-01)

`func_800C6EE8` is now a registered four-word exact C leaf. It truncates its
input and stores the resulting unsigned halfword to `D_800F3420`. The full
rebuild, public verifier, and all packed-span checks are exact at 436 leaves.

## Matching leaves — integer exchanges (2026-09-01)

`func_80081E5C`, `func_800824B4`, `func_800824C8`, and `func_800824DC` are
now registered five-word exact C leaves. Each loads an integer global, stores
its input through a pointer pinned to `$v1` in the return delay slot, then
returns the old value. The full rebuild, public verifier, and all packed-span
checks are exact at 440 leaves.

## Matching leaves — forwarding wrappers (2026-09-01)

`func_80080B24`, `func_80082514`, and `func_80082554` are now registered
eight-word exact C leaves. They are direct frame-and-return wrappers around
`func_8007BDDC`, `func_80082CDC`, and `func_80082DBC`, respectively. The full
rebuild, public verifier, and all packed-span checks are exact at 443 leaves.

## Matching leaves — boolean and forwarding wrappers (2026-09-01)

`func_8007A8AC` and `func_8007A8CC` return the boolean negation of their
callees; `func_8007A910` directly forwards to `func_8007BDDC`. All three
eight-word leaves are byte-exact C matches. The full rebuild, public verifier,
and all packed-span checks are exact at 446 leaves.

## Matching leaf — func_80091080 (2026-09-01)

`func_80091080` is now a registered eight-word exact C leaf. It is a direct
frame-and-return wrapper around `func_80090F68`. The full rebuild, public
verifier, and all packed-span checks are exact at 447 leaves.

## Matching leaves — call-and-return-one wrappers (2026-09-01)

`func_800193B8` and `func_80019D24` now call their respective `void` callees
and return one as registered eight-word exact C leaves. The full rebuild,
public verifier, and all packed-span checks are exact at 449 leaves.

## Matching leaf — func_8007F7C8 (2026-09-01)

`func_8007F7C8` is now a registered eight-word exact C leaf. It forwards the
unsigned-byte result from `func_8007FC08`, preserving the required byte mask.
The full rebuild, public verifier, and all packed-span checks are exact at 450
leaves.

## Matching leaf — func_8007DE78 (2026-09-01)

`func_8007DE78` is now a registered ten-word exact C leaf. It calls
`func_8007E334` and then `func_8007E514` through a shared minimal frame. The
full rebuild, public verifier, and all packed-span checks are exact at 451
leaves.

## Matching leaf — func_8003E91C (2026-09-01)

`func_8003E91C` is now a registered ten-word exact C leaf. It calls
`func_80070D6C` followed by `func_80036F7C` through a minimal shared frame.
The full rebuild, public verifier, and all packed-span checks are exact at 452
leaves.

## Matching leaf — func_80085098 (2026-09-01)

`func_80085098` is now a registered ten-word exact C leaf. It passes zero to
`func_80085F44`, then clears `D_8009D24C`. The full rebuild, public verifier,
and all packed-span checks are exact at 453 leaves.

## Matching leaf — func_800850C0 (2026-09-01)

`func_800850C0` is now a registered thirteen-word exact C leaf. It sets
`D_8009D24C` to one, then registers `func_80085098` through `func_80085F44`.
The full rebuild, public verifier, and all packed-span checks are exact at 454
leaves.

## Matching leaves — func_800850F4 / func_80085134 (2026-09-01)

`func_800850F4` and `func_80085134` are now registered sixteen-word exact C
leaves. Both preserve their two arguments across `func_800850C0`, then forward
them to their respective dispatchers (`func_80085E54` and `func_80085DF4`).
The full rebuild, public verifier, and all packed-span checks are exact at 456
leaves.

## Matching leaves — func_8004C5DC / func_8004D9D8 (2026-09-01)

`func_8004C5DC` and `func_8004D9D8` are now registered eleven-word exact C
leaves. Each obtains a handle with `func_80062A34(1, constant)` and immediately
dispatches it through `func_80062F1C`; their constants are 19 and 39. The full
rebuild, public verifier, and all packed-span checks are exact at 458 leaves.

## Matching leaf — func_80076C10 (2026-09-01)

`func_80076C10` is now a registered nine-word exact C leaf. It forwards its
three arguments to `func_80076C34` as `(arg0, arg1, 0, arg2)`, including the
retail `$a3` move and zeroed delay-slot argument. The full rebuild, public
verifier, and all packed-span checks are exact at 459 leaves.

## Matching leaf — func_80077A00 (2026-09-01)

`func_80077A00` is now a registered ten-word exact C leaf. It registers
`func_80076EE4` with mode two through `func_80073CF4`; the function-pointer
argument and immediate delay slot are exact. The full rebuild, public verifier,
and all packed-span checks are exact at 460 leaves.

## Matching leaf — func_80080D34 (2026-09-01)

`func_80080D34` is now a registered ten-word exact C leaf. It forwards its
byte-typed first parameter and second word parameter to `func_8007EE84`, with
two zero trailing fields. The full rebuild, public verifier, and all packed-span
checks are exact at 461 leaves.

## Matching leaf — func_80017820 (2026-09-01)

`func_80017820` is now a registered eleven-word exact C leaf. It double-derefs
a pointer to obtain a signed halfword, calls `func_8003746C`, and returns one.
The full rebuild, public verifier, and all packed-span checks are exact at 462
leaves.

## Matching leaf — func_80018954 (2026-09-01)

`func_80018954` is now a registered ten-word exact C leaf. It passes the global
record pointer `D_8009D2F0` to `func_8002F7D8` and returns one. The full rebuild,
public verifier, and all packed-span checks are exact at 463 leaves.

## Matching leaf — func_800C7D2C (2026-09-01)

`func_800C7D2C` is now a registered ten-word exact C leaf. It forwards its
incoming first argument with the static buffer `D_800E0824` to `func_800C2414`
and returns zero. The full rebuild, public verifier, and all packed-span checks
are exact at 464 leaves.

## Matching leaf — func_800C8E70 (2026-09-01)

`func_800C8E70` is the matching fixed-buffer sibling using `D_800E09A0`.
All rebuild and verifier checks are exact at 465 leaves.

## Matching leaf — func_800C9B68 (2026-09-01)

`func_800C9B68` is the matching fixed-buffer sibling using `D_800E0A94`.
All rebuild and verifier checks are exact at 466 leaves.

## Matching leaf — func_800CA700 (2026-09-01)

`func_800CA700` is the matching fixed-buffer sibling using `D_800E0B84`.
All rebuild and verifier checks are exact at 467 leaves.

## Matching leaf — func_800CBF0C (2026-09-01)

`func_800CBF0C` is the matching fixed-buffer sibling using `D_800E0D08`.
All rebuild and verifier checks are exact at 468 leaves.

## Matching leaf — func_800CCEE8 (2026-09-01)

`func_800CCEE8` is the matching fixed-buffer sibling using `D_800E0E60`.
All rebuild and verifier checks are exact at 469 leaves.

## Matching leaf — func_800CD8C8 (2026-09-01)

`func_800CD8C8` is the matching fixed-buffer sibling using `D_800E0F28`.
All rebuild and verifier checks are exact at 470 leaves.

## Matching leaf — func_800CE144 (2026-09-01)

`func_800CE144` is the matching fixed-buffer sibling using `D_800E0FC0`.
All rebuild and verifier checks are exact at 471 leaves.

## Matching leaf — func_80071944 (2026-09-01)

`func_80071944` is an exact conditional pointer helper: when bit 3 of its
flags word is set it returns the offset-0xC field address, otherwise null.
All rebuild and verifier checks are exact at 472 leaves.

## Matching leaf — func_800719C4 (2026-09-01)

`func_800719C4` is its matching sibling, returning the offset-0x14 field
address when bit 3 of the flags word is set. All rebuild and verifier checks
are exact at 473 leaves.

## Matching leaf — func_80076150 (2026-09-01)

`func_80076150` is an exact no-global command-word builder. It combines the
`0xE1000000` base, two boolean-derived flag bits, and an `0x9FF` masked field.
All rebuild and verifier checks are exact at 474 leaves.

## Matching leaf — func_80018754 (2026-09-01)

`func_80018754` is now a registered eight-word exact C leaf. It sets bit 2
in `D_800A76C4` through a `$v1`-pinned global pointer and returns one. The
full rebuild, public verifier, and all packed-span checks are exact at 475
leaves.

## Matching leaf — func_80019618 (2026-09-01)

`func_80019618` is now a registered eight-word exact C leaf. It sets bit 13
in `D_800B0CD8` through a `$v1`-pinned global pointer and returns one. The
full rebuild, public verifier, and all packed-span checks are exact at 476
leaves.

## Matching leaf — func_80019638 (2026-09-01)

`func_80019638` is now a registered eight-word exact C leaf. It clears bit 13
in `D_800B0CD8`, with the global pointer pinned to `$v0` and the mask to
`$a0`, reproducing the retail register allocation. The full rebuild, public
verifier, and all packed-span checks are exact at 477 leaves.

## Matching leaf — func_800196A0 (2026-09-01)

`func_800196A0` is now a registered nine-word exact C leaf. It sets bit 14 in
the `+0x98` flag word of `D_8009D2F0`, keeping the state pointer in `$v1` to
reproduce retail loads. The full rebuild, public verifier, and all packed-span
checks are exact at 478 leaves.

## Matching leaf — func_800196C4 (2026-09-01)

`func_800196C4` is now a registered nine-word exact C leaf. It clears bit 14
in the `+0x98` flag word of `D_8009D2F0`, with the state pointer in `$v0` and
mask in `$a0` to match retail allocation. All checks are exact at 479 leaves.

## Matching leaf — func_8001967C (2026-09-01)

`func_8001967C` is now a registered nine-word exact C leaf. It clears bit 7
in the `+0x98` flag word of `D_8009D2F0`, keeping the state pointer in `$v0`
and mask in `$a0`. The full rebuild, public verifier, and packed-span checks
are exact at 480 leaves.

## Matching leaf — func_80019658 (2026-09-01)

`func_80019658` is now a registered nine-word exact C leaf. It sets bit 7 in
the `+0x98` flag word of `D_8009D2F0`, retaining the state pointer in `$v1`
for the retail load order. The full rebuild, public verifier, and packed-span
checks are exact at 481 leaves.

## Matching leaf — func_80019728 (2026-09-01)

`func_80019728` is now a registered eight-word exact C leaf. It sets bit 2
in `D_8009D2E8`; the compiler naturally reproduces retail's `$v0` load and
`$at`-addressed store. The full rebuild, public verifier, and packed-span
checks are exact at 482 leaves.

## Matching leaf — func_80019748 (2026-09-01)

`func_80019748` is now a registered eight-word exact C leaf. It clears bit 2
in `D_8009D2E8`, retaining retail's `$v0` global value and `$at` store
address. The full rebuild, public verifier, and packed-span checks are exact
at 483 leaves.

## Matching leaf — func_80019768 (2026-09-01)

`func_80019768` is now a registered 12-word exact C leaf. It forwards the
halfword reached through its pointer argument with `D_8009D2F0` to
`func_8001ACE0`, then returns one. The full rebuild, public verifier, and
packed-span checks are exact at 484 leaves.

## Matching leaf — func_80019798 (2026-09-01)

`func_80019798` is now a registered 14-word exact C leaf. It retains its
output pointer across `func_800392EC`, masks the result to one byte, writes it
through that pointer, and returns one. The full rebuild, public verifier, and
packed-span checks are exact at 485 leaves.

## Matching leaf — func_80019904 (2026-09-01)

`func_80019904` is now a registered nine-word exact C leaf. It clears bit 0
in the `+0x98` flag word of `D_8009D2F0`, with the state pointer pinned to
`$v0` and the mask to `$a0`. The full rebuild, public verifier, and packed-span
checks are exact at 486 leaves.

## Matching leaf — func_80019928 (2026-09-01)

`func_80019928` is now a registered nine-word exact C leaf. It sets bit 0 in
the `+0x98` flag word of `D_8009D2F0`, retaining the state pointer in `$v1`
to reproduce retail loads. The full rebuild, public verifier, and packed-span
checks are exact at 487 leaves.

## Matching leaf — func_8001994C (2026-09-01)

`func_8001994C` is now a registered 16-word exact C leaf. It dereferences four
pointer slots, forwards their words to `func_800676CC`, and returns one. The
full rebuild, public verifier, and packed-span checks are exact at 488 leaves.

## Matching leaf — func_8001998C (2026-09-01)

`func_8001998C` is now a registered 16-word exact C leaf. It dereferences four
pointer slots, forwards their words to `func_80067730`, and returns one. The
full rebuild, public verifier, and packed-span checks are exact at 489 leaves.

## Matching leaf — func_800199F8 (2026-09-01)

`func_800199F8` is now a registered nine-word exact C leaf. It clears bit 4
in the `+0x250` halfword state field of `D_8009D2F0`, retaining `$v1` for the
retail load order. The full rebuild, public verifier, and packed-span checks
are exact at 490 leaves.

## Matching leaf — func_80019A9C (2026-09-01)

`func_80019A9C` is now a registered nine-word exact C leaf. It clears bit 3
in the `+0x250` halfword state field of `D_8009D2F0`, retaining `$v1` for the
retail load order. The full rebuild, public verifier, and packed-span checks
are exact at 491 leaves.

## Matching leaf — func_80019AC0 (2026-09-01)

`func_80019AC0` is now a registered nine-word exact C leaf. It sets bit 10
in the `+0x98` state flag word of `D_8009D2F0`, retaining `$v1` for the retail
load order. The full rebuild, public verifier, and packed-span checks are
exact at 492 leaves.

## Matching leaf — func_80019AE4 (2026-09-01)

`func_80019AE4` is now a registered nine-word exact C leaf. It clears bit 10
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$v0` and
mask in `$a0` to match retail allocation. The full rebuild, public verifier,
and packed-span checks are exact at 493 leaves.

## Matching leaf — func_80019C28 (2026-09-01)

`func_80019C28` is now a registered nine-word exact C leaf. It sets bit 17
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$v0` and
mask in `$a0` to match retail allocation. The full rebuild, public verifier,
and packed-span checks are exact at 494 leaves.

## Matching leaf — func_80019C04 (2026-09-01)

`func_80019C04` is now a registered nine-word exact C leaf. It clears bit 17
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$a0` and
wide mask in `$v1` to match retail allocation. The full rebuild, public
verifier, and packed-span checks are exact at 495 leaves.

## Matching leaf — func_8001A32C (2026-09-01)

`func_8001A32C` is now a registered nine-word exact C leaf. It sets bit 1 in
the `+0x98` state flag word of `D_8009D2F0`, retaining `$v1` for the retail
load order. The full rebuild, public verifier, and packed-span checks are
exact at 496 leaves.

## Matching leaf — func_8001A350 (2026-09-01)

`func_8001A350` is now a registered nine-word exact C leaf. It clears bit 1
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$v0` and
mask in `$a0` to match retail allocation. The full rebuild, public verifier,
and packed-span checks are exact at 497 leaves.

## Matching leaf — func_8001A1F0 (2026-09-01)

`func_8001A1F0` is now a registered nine-word exact C leaf. It sets bit 24
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$v0` and
mask in `$a0` to match retail allocation. The full rebuild, public verifier,
and packed-span checks are exact at 498 leaves.

## Matching leaf — func_8001A2F0 (2026-09-01)

`func_8001A2F0` is now a registered 15-word exact C leaf. It counts set bits
using the retail `value &= value - 1` loop, stores the count through its second
pointer slot, and returns one. The full rebuild, public verifier, and
packed-span checks are exact at 499 leaves.

## Matching leaf — func_8001A1A8 (2026-09-01)

`func_8001A1A8` is now a registered 18-word exact C leaf. It retains its
descriptor across `func_8005186C`, writes the result through the descriptor's
second pointer slot, and returns one. The full rebuild, public verifier, and
packed-span checks are exact at 500 leaves.

## Matching leaf — func_80019D44 (2026-09-01)

`func_80019D44` is now a registered 16-word exact C leaf. It dereferences four
halfword pointer slots, forwards them to `func_80037454`, and returns one. The
full rebuild, public verifier, and packed-span checks are exact at 501 leaves.

## Matching leaf — func_80017D7C (2026-09-01)

`func_80017D7C` is now a registered eight-word exact C leaf. It sets bit 0 in
`D_8009D2E8`, retaining retail's `$v0` global value and `$at` store address.
The full rebuild, public verifier, and packed-span checks are exact at 502
leaves.

## Matching leaf — func_80017D5C (2026-09-01)

`func_80017D5C` is now a registered eight-word exact C leaf. It clears bit 0
in `D_8009D2E8`, retaining retail's `$v0` global value and `$at` store
address. The full rebuild, public verifier, and packed-span checks are exact
at 503 leaves.

## Matching leaf — func_80017D3C (2026-09-01)

`func_80017D3C` is now a registered eight-word exact C leaf. It transfers the
signed second halfword through the `$v0` load/store path into
`D_8009D2F0+0x224`; an empty constrained asm operand preserves the retail
signed-load choice. The full rebuild, public verifier, and packed-span checks
are exact at 504 leaves.

## Matching leaf — func_80017D18 (2026-09-01)

`func_80017D18` is now a registered nine-word exact C leaf. It clears the
low three bits of `D_800BCF88` and sets bit 7; pinning the global address in
`$v0` retains the retail address/value register sequence. The full rebuild,
public verifier, and packed-span checks are exact at 505 leaves.

## Matching leaf — func_80017CC4 (2026-09-01)

`func_80017CC4` is now a registered nine-word exact C leaf. It tests whether
the low three bits of `D_800BCF88` equal four and writes the Boolean result to
the caller's indirect result slot. The full rebuild, public verifier, and
packed-span checks are exact at 506 leaves.

## Matching leaves — func_80017C54 and func_80017C8C (2026-09-01)

`func_80017C54` and `func_80017C8C` are now registered fourteen-word exact C
leaves. Both forward signed, signed, and unsigned halfwords to `func_800661EC`;
their only behavioral difference is the fourth argument (zero versus eight).
The full rebuild, public verifier, and packed-span checks are exact at 508
leaves.

## Matching leaf — func_80017CE8 (2026-09-01)

`func_80017CE8` is now a registered twelve-word exact C leaf. It invokes
`func_800665A0` on the `D_8009D254+0x28` state subrecord with both remaining
arguments set to minus one. The full rebuild, public verifier, and packed-span
checks are exact at 509 leaves.

## Matching leaf — func_80017B34 (2026-09-01)

`func_80017B34` is now a registered sixteen-word exact C leaf. It caps an
incoming unsigned halfword by the byte limit at `D_8009D2F0+0xF`, stores it at
`+0x12`, and sets state flag `0x200`. The full rebuild, public verifier, and
packed-span checks are exact at 510 leaves.

## Matching leaves — func_80017A78, func_80017AA4, and func_80017AC0 (2026-09-01)

`func_80017A78`, `func_80017AA4`, and `func_80017AC0` are now registered
exact C leaves for the `D_8009D2E8` flag word: a caller-supplied mask clear,
a read to an indirect result slot, and a caller-supplied OR update. The full
rebuild, public verifier, and packed-span checks are exact at 513 leaves.

## Matching leaves — func_80017A24 and func_80017A50 (2026-09-01)

`func_80017A24` and `func_80017A50` are now registered exact C leaves. They
respectively write a selected source bit to the destination slot and set that
bit in the source word using the shared three-pointer argument layout. The
full rebuild, public verifier, and packed-span checks are exact at 515 leaves.

## Matching leaves — func_80017928, func_80017948, and func_80017968 (2026-09-01)

`func_80017928`, `func_80017948`, and `func_80017968` are now registered
eight-word exact C leaves. They read the byte at `D_8009D2F0+0xD`, write that
byte from an indirect input slot, and read the unsigned halfword at `+0x24`,
respectively. The full rebuild, public verifier, and packed-span checks are
exact at 518 leaves.

## Matching leaf — func_800179F8 (2026-09-01)

`func_800179F8` is now a registered eleven-word exact C leaf. It clears the
selected indexed bit in the source word using the shared source/bit pointer
layout. The full rebuild, public verifier, and packed-span checks are exact at
519 leaves.

## Matching leaves — func_800C2B10 and func_800C2B28 (2026-09-01)

`func_800C2B10` and `func_800C2B28` are now registered six-word exact C
leaves. They return indexed addresses in `D_800E2248` at base offsets `0x8`
and `0x48`; a compiler memory barrier preserves the retail shift/load/add
schedule. The full rebuild, public verifier, and packed-span checks are exact
at 521 leaves.

## Matching leaf — func_800C2B68 (2026-09-01)

`func_800C2B68` is now a registered ten-word exact C leaf. It checks whether
the high halfword at `D_800E2248+4` equals `0x0101`. The full rebuild, public
verifier, and packed-span checks are exact at 522 leaves.

## Matching leaf — func_8008C70C (2026-09-01)

`func_8008C70C` is now a registered five-word exact C leaf. It stores the
second input word as a halfword at `D_8009D2C8+0x56`, retaining the retail
return-delay-slot store. The full rebuild, public verifier, and packed-span
checks are exact at 523 leaves.

## Matching leaf — func_8008C16C (2026-09-01)

`func_8008C16C` is now a registered eight-word exact C leaf. It clears
`D_8009D220` and stores the signed input byte shifted into the high halfword of
`D_8009D2D0`; pinning the value in `$v0` retains retail's load-delay schedule.
The full rebuild, public verifier, and packed-span checks are exact at 524
leaves.

## Matching leaf — func_8008C270 (2026-09-01)

`func_8008C270` is now a registered eight-word exact C leaf. It is the
companion signed-byte setter: it clears `D_8009D21E` and writes the shifted
byte to `D_8009D2CC`, using the same retail `$v0` load-delay schedule. The full
rebuild, public verifier, and packed-span checks are exact at 525 leaves.

## Matching leaf — func_80084B20 (2026-09-01)

`func_80084B20` is now a registered eight-word exact C leaf. It returns either
the base of `D_800A5B70` or its `+0xF0` subregion according to bits 4–7 of the
argument. The full rebuild, public verifier, and packed-span checks are exact
at 526 leaves.

## Matching leaf — func_80084B44 (2026-09-01)

`func_80084B44` is now a registered thirteen-word exact C leaf. It initializes
the three callback slots at `D_8009B73C`, `D_8009B740`, and `D_8009B744`; the
existing store-delay-slot profile preserves retail's final `sw` in the `jr`
delay slot. The full rebuild, public verifier, and packed-span checks are
exact at 527 leaves.

## Matching leaf — func_80084AE8 (2026-09-01)

`func_80084AE8` is now a registered fourteen-word exact C leaf. It maps either
of two `D_800A5B70` entry addresses to slots `0x10` and `0x20`, returning
`0xFF` when neither matches. Pinning the entry, index, and slot locals to
retail's `$v1`, `$a1`, and `$a2` preserves the loop schedule. The full rebuild,
public verifier, and packed-span checks are exact at 528 leaves.

## Matching leaf — func_8006EBE4 (2026-09-01)

`func_8006EBE4` is now a registered nine-word exact C leaf. It returns the
signed halfword `D_800B0DBC` when byte flag `D_800B0DBA` is set, otherwise
`-1`. The natural conditional-return phrasing preserves the retail load-delay
and branch-delay scheduling. The full rebuild, public verifier, and
packed-span checks are exact at 529 leaves.

## Matching leaf — func_80090574 (2026-09-01)

`func_80090574` is now a registered ten-word exact C leaf. It consumes one
stream byte, sets the `0x900` flag at state offset `0xF4`, and writes the byte
as a halfword at `+0x10E`. A zero-code memory barrier after advancing the
stream pointer preserves retail's `$v0` reuse and load order. The full rebuild,
public verifier, and packed-span checks are exact at 530 leaves.

## Matching leaf — func_8001856C (2026-09-01)

`func_8001856C` is now a registered eleven-word exact C leaf. It clears six
word fields at offsets `0x68`, `0x6C`, `0x70`, `0x78`, `0x7C`, and `0x80` of
the `D_8009D2F0` state, then returns `1`. The full rebuild, public verifier,
and packed-span checks are exact at 531 leaves.

## Matching leaf — func_8007F960 (2026-09-01)

`func_8007F960` is now a registered eleven-word exact C leaf. It invokes the
nullable `D_800B8AB8` callback with the low byte of its argument. The full
rebuild and packed-span checks are exact at 532 leaves.

## Matching leaf — func_80016DF8 (2026-09-01)

`func_80016DF8` is now a registered nine-word exact C leaf. It sets
`0x20000000` in the `+0x98` state flags of `D_8009D2F0`, then returns `1`.
The full rebuild and packed-span checks are exact at 533 leaves.

## Matching leaf — func_80016E1C (2026-09-01)

`func_80016E1C` is now a registered nine-word exact C leaf. It clears
`0x20000000` in the `+0x98` state flags of `D_8009D2F0`, then returns `1`.
The full rebuild and packed-span checks are exact at 534 leaves.

## Matching leaf — func_800172BC (2026-09-01)

`func_800172BC` is now a registered nine-word exact C leaf. It sets bit
`0x10` in the `+0x98` flags of `D_8009D2F0` and returns `0`; pinning the state
pointer to retail `$v1` preserves the original allocation. The full rebuild
and packed-span checks are exact at 535 leaves.

## Matching leaf — func_800173F4 (2026-09-01)

`func_800173F4` is now a registered seven-word exact C leaf. It copies one
word from the source pointer at argument offset `+4` to the destination
pointer at `+0`, then returns `1`. The full rebuild and packed-span checks are
exact at 536 leaves.

## Matching leaf — func_800172FC (2026-09-01)

`func_800172FC` is now a registered eight-word exact C leaf. It sets bit
`0x10` in halfword `D_8009D300[4]` and returns `0`. The Era `-O2 -G8` profile
preserves its GP-relative pointer load and retail `addu` return-zero form. The
full rebuild and packed-span checks are exact at 537 leaves.

## Matching leaf — func_800172E0 (2026-09-01)

`func_800172E0` is now a registered seven-word exact C leaf. It reads an
unsigned halfword through its argument pointer, stores it as a word at
`D_8009D300 + 0x10`, and returns `0`. The Era `-O2 -G8` profile preserves the
GP-relative state-pointer load; full rebuild and packed-span checks are exact
at 538 leaves.

## Matching leaf — func_800176E0 (2026-09-01)

`func_800176E0` is now a registered seven-word exact C leaf. It reads the
halfword at `D_8009D300 + 0xA`, writes it as a word through the supplied
destination pointer, and returns `1`. Era `-O2 -G8` preserves its
GP-relative load and retail return form; full rebuild and packed-span checks
are exact at 539 leaves.

## Matching leaf — func_80019154 (2026-09-01)

`func_80019154` is now a registered seven-word exact C leaf. It copies the
global state value `D_8009D28C` through the destination pointer supplied in
its argument and returns `1`. The full rebuild and packed-span checks are
exact at 540 leaves.

## Matching leaf — func_80019298 (2026-09-01)

`func_80019298` is now a registered eight-word exact C leaf. It copies a word
through its argument-held pointer into the halfword field at
`D_8009D2F0 + 0x1E6`, then returns `1`. The state pointer is constrained to
retail `$v1`; full rebuild and packed-span checks are exact at 541 leaves.

## Matching leaf — func_800177AC (2026-09-01)

`func_800177AC` is now a registered seven-word exact C leaf. It copies the
word at `D_8009D300 + 0x14` through the supplied destination pointer and
returns `1`. Era `-O2 -G8` preserves the GP-relative pointer load; full
rebuild and packed-span checks are exact at 542 leaves.

## Matching leaf — func_8003E5F0 (2026-09-01)

`func_8003E5F0` is now a registered seven-word exact C leaf. It increments
`D_8003E60C`; constraining the address and value locals to `$a0` and `$a1`
preserves retail's load-delay schedule and return-delay-slot store. The full
rebuild and packed-span checks are exact at 543 leaves.

## Matching leaf — func_80089960 (2026-09-01)

`func_80089960` is now a registered eight-word exact C leaf. It sets bit
`0x100` in `D_8009D2C4`. The natural unsigned-global C form preserves the
retail load-delay and store sequence; full rebuild and packed-span checks are
exact at 544 leaves.

## Matching leaf — func_80090B30 (2026-09-01)

`func_80090B30` is now a registered 11-word exact C leaf. It consumes one
stream byte, stores either that byte plus one or the sentinel `0x101` at
`arg0 + 0xBA`, and advances the stream pointer. The default era `-O2 -G0`
profile preserves retail's `bnez`/increment delay-slot shape; full rebuild,
public verifier, and packed-span checks are exact at 545 leaves.

## Matching leaf — func_80090BA0 (2026-09-01)

`func_80090BA0` is now a registered 11-word exact C leaf and the direct twin
of `func_80090B30`: it consumes one stream byte, emits either byte-plus-one
or `0x101`, and stores it at `arg0 + 0xBC`. The default era `-O2 -G0` profile
again preserves the retail `bnez` increment-delay-slot scheduling; full
rebuild, public verifier, and packed-span checks are exact at 546 leaves.

## Matching leaves — func_80089B28 / func_80089CF0 (2026-09-01)

`func_80089B28` and `func_80089CF0` are now registered eight-word exact C
leaves. Both are byte-identical siblings of `func_80089960`, setting bit
`0x100` in `D_8009D2C4`; the natural unsigned-global form keeps the retail
load delay and `$at` store sequence. Full rebuild, public verifier, and all
packed-span checks are exact at 548 leaves.

## Matching leaf — func_8008F84C (2026-09-01)

`func_8008F84C` is now a registered seven-word exact C leaf. It consumes a
stream byte, advances the input pointer, and stores the byte at `arg0 + 0x7C`.
The natural unsigned-byte form preserves retail's pointer load delay and
return-delay-slot halfword store; full rebuild, public verifier, and packed
span checks are exact at 549 leaves.

## Matching leaf — func_8003E944 (2026-09-01)

`func_8003E944` is now a registered 12-word exact C leaf. It initializes the
fixed `D_800BE9A0` work buffer through `func_800844E4`, then calls
`func_80082534`. The natural two-call form preserves retail's frame and both
call delay slots; full rebuild, public verifier, and packed-span checks are
exact at 550 leaves.

## Matching leaf — func_80077C84 (2026-09-01)

`func_80077C84` is now a registered 11-word exact C leaf. It writes a GPU
draw-mode command packet, combining the `arg2` and `arg1` flags with the
masked `arg3` payload. Hard-register locals for the two independent command
words preserve the retail `$v1`/`$v0` schedule; full rebuild, public
verifier, and packed-span checks are exact at 551 leaves.

## Matching leaf — func_80017D9C (2026-09-01)

`func_80017D9C` is now a registered nine-word exact C leaf. It sets bit
`0x40` in the current state's flags field at `D_8009D2F0 + 0x98`, then returns
success. The established `$v1` state-pointer form preserves both retail load
delays and the return delay slot; full rebuild, public verifier, and packed
span checks are exact at 552 leaves.

## Matching leaf — func_80017DC0 (2026-09-01)

`func_80017DC0` is now a registered nine-word exact C leaf and the bit-clear
sibling of `func_80017D9C`: it clears `0x40` in the current state's flags
field at `D_8009D2F0 + 0x98`, then returns success. The `$v0` state pointer
and `$a0` mask preserve retail's load delays and return delay slot; full
rebuild, public verifier, and packed-span checks are exact at 553 leaves.

## Split cleanup — superseded generated asm units (2026-09-01)

`scripts/split_us.sh` now removes only git-ignored, hex-named generated asm
units that no longer belong to a YAML asm span or contain offsets now owned by
C. The build-time stale-overlap gate remains unchanged as a backstop. The
plan regression suite covers an asm-to-C carve; a detached scratch worktree
also proved that a previously generated `asm/disc1/33128.s` is removed by a
split with no manual cleanup. Real-tree rebuild and both verifiers remain
exact at 553 C leaves.

## Matching leaf — func_800392EC (2026-09-01)

`func_800392EC` is now a registered nine-word exact C leaf. It returns `1`
when `D_80091A1C` is zero, otherwise it returns `D_80091A1D`. The direct
byte-global conditional preserves the retail `beqz` return-constant delay
slot; full rebuild, public verifier, and packed-span checks are exact at 554
leaves.

## Matching leaf — func_8008B1D0 (2026-09-01)

`func_8008B1D0` is now a registered 11-word exact C leaf. It forwards the
two pointer fields at `arg0 + 4` and `arg0 + 8` to `func_8008A400`; retaining
the descriptor in `$v0` reproduces retail's frame, loads, and call sequence.
Full rebuild, public verifier, and packed-span checks are exact at 555 leaves.

## Matching leaf — func_80018B98 (2026-09-01)

`func_80018B98` is now a registered 12-word exact C leaf. It dereferences two
descriptor pointers from `arg0`, forwards their payload pointers to
`func_80065954`, and returns `1`. Explicit descriptor locals preserve
retail's `$v0`/`$v1` load order; full rebuild, public verifier, and packed-span
checks are exact at 556 leaves.

## Matching leaf — func_80018864 (2026-09-01)

`func_80018864` is now a registered 12-word exact C leaf. It loads two
descriptor payload pointers, forwards them to `func_8006F820` with a zero
middle argument, and returns `1`. Explicit locals retain retail's `$v0`/`$v1`
argument-load order and the zeroed `$a1` call delay slot; full rebuild, public
verifier, and packed-span checks are exact at 557 leaves.

## Matching leaf — func_80018894 (2026-09-01)

`func_80018894` is now a registered 12-word exact C leaf. It follows the same
two-stage descriptor load sequence as its adjacent sibling, forwarding the
payload pointers to `func_8006F820` with a one-valued middle argument and
returning `1`. Explicit first/second locals preserve the retail `$v1` cursor
and `$v0` payload intermediary; full rebuild, public verifier, and packed-span
checks are exact at 558 leaves.

## Matching leaf — func_800534CC (2026-09-01)

`func_800534CC` is now a registered six-word exact C leaf. It returns one
signed halfword from the GP-relative table pointer `D_8009D048`; era `-O2
-G8` reproduces the table load, scaled index, and halfword access. The full
rebuild, public verifier, and all packed-span checks are exact at 427 leaves.

## PE-B54K-AM — CdlReadS registration prefix (2026-08-31)

The 30-word production prefix of `func_80081314` now applies both mode bits
and registers exact guest callbacks `0x8007C214` (DMA channel 3) and
`0x800813E8` (CD callback exchange). The executable names command `0x1B`
`CdlReadS`; its 212-word low-level provider builds Pause/Setmode/Setloc/ReadS
queue records and remains untranslated. Registration invokes no callback.
The suite is 994/994 and strict production stops before queue issue at
`func_80081314_func_8007F0C8_cut`. Evidence:
`docs/evidence/pe-b54kam-read-registration/REPORT.md`.

## PE-B54K-AN — CdlReadS delivery audit (2026-08-31)

The registered CD callback enters a 583-word stream/ring state machine; its
35-word DMA3 callback changes producer-record status from the CD phase's `3`
to ready `2` before optional consumer notification. Retail also conditionally
calls that callback at the CD-handler tail under `D_800B89F4`, so the next
rung must recover the direct-tail versus DMA-dispatch rule. Direct
sector-to-ring synchronous completion would erase retail partial-record,
ring-full, DMA-in-flight, and callback ordering, so no such shortcut was
added. The next rung is a generic first-sector event contract with the exact
completion-selection rule. Evidence:
`docs/evidence/pe-b54kan-cdlreads-delivery-audit/REPORT.md`.

## PE-B54K-AO — stream completion selector (2026-08-31)

The executable/PE.IMG direct-writer census proves production
`D_800C0DB8 == 0`. That value makes the CD-handler tail skip its optional
direct call to `func_8007C214`; `D_800B89F4` remains one and suppresses CD
re-entry until the separately registered DMA3 callback publishes status 2,
notifies the consumer, and clears it. The next implementation contract can
therefore target the proven deferred-DMA production path. Evidence:
`docs/evidence/pe-b54kao-stream-completion-selector/REPORT.md`.

## PE-B54K-AP — first retail multiplexed stream sectors (2026-08-31)

Disc 1 LBA 189742, the first sector of authenticated `FMV001.STR`, is
Mode-2 Form-1 video: 2048 bytes plus EDC/ECC, with submode `0x48`. Video
chunks 0..6 are followed by a Mode-2 Form-2 XA sector (`0x64`, 2324 data
bytes), then chunks 7..8. The current disc API correctly exposes 2048 bytes
for ISO/Form-1 reads but discards the routing subheader and truncates Form-2
XA by 276 bytes, so it cannot faithfully back multiplexed `CdlReadS`. No
delivery shortcut was added; the next rung is a separate read-only Mode-2
stream-sector API with variable geometry. Evidence:
`docs/evidence/pe-b54kap-first-form2-sector/REPORT.md`.

## PE-B54K-AL — blocking CdlSetloc arm (2026-08-31)

The complete 26-word blocking command wrapper and the executable's own
command-name table prove the movie call is command 2 / `CdlSetloc`. The
complete movie CFG never reads its eight-byte stack response, so native
retains the exact four-byte location without inventing response bytes.
Unsupported commands, response consumers, absent disc, malformed BCD, and
out-of-range locations are inert. The native suite is 993/993 and strict
production stops before `func_80081314` at
`func_801924F8_801927A0_cut`. Evidence:
`docs/evidence/pe-b54kal-blocking-setloc/REPORT.md`.

## PE-B54K-AK — movie CD idle wait (2026-08-31)

The exact 8-word `CdReady`/queue-depth loop at `0x80192770..0x8019278F` is
now translated over the already-complete libcd providers. It waits for ready
state 1 and queue depth zero without writing either authority. Real Disc 1
and the independent synthetic movie fixture both retain ready/idle state,
all predecessor oracles pass, and strict production stops at
`func_801924F8_80192790_cut` before the unresolved CD command wrapper.
Evidence: `docs/evidence/pe-b54kak-cd-idle-wait/REPORT.md`.

## PE-B54K-AJ — movie stream-control initialization (2026-08-31)

The complete 33-word `func_8007C304` initializer and 7-word
`func_8007C544` setter are translated. Production supplies
`(1, signed record[+6], -1, 0, 0)`; the setter publishes the signed range and
the initializer resets only the proven callback/option/auxiliary state. No
stream frame or callback is consumed. Signed-value, option-bit, callback,
auxiliary, and halfword-width controls pass; the native suite is 992/992 and
strict production stops at the first following CD-ready call,
`func_801924F8_80192770_cut`. Evidence:
`docs/evidence/pe-b54kaj-stream-control/REPORT.md`.

## PE-B54K-AI — movie record-pool initializer (2026-08-31)

Retail entry `0x8007A214`, its worker `0x8007A244`, and the 13-word clear
helper `0x8007C444` are now translated completely. The production call
publishes `D_801D0DFC` as the pool base and `0x40` as its unsigned record
count, resets the proven associated globals, and clears only word zero of
each 32-byte record. No unproven SDK name is assigned. Synthetic footprint,
access-width, count-boundary, and zero-count controls pass; the native suite
is 991/991 and strict production stops at
`func_801924F8_80192750_cut`. Evidence:
`docs/evidence/pe-b54kai-record-pool/REPORT.md`.

## PE-B54K-AH — libpress `DecDCToutCallback` registration (2026-08-31)

The complete 9-word `func_8010C0D8` wrapper is now translated. Retail passes
the exact guest callback identity `0x80191DC8` at `0x80192738`; the wrapper
registers it through the complete DMA callback setter with channel 1. Focused
controls prove only slot 1 and its DICR enable change, with no callback
delivery or invented DMA completion. The native suite is 990/990 and strict
production stops at the later record-pool initializer. Evidence:
`docs/evidence/pe-b54kah-decdctoutcallback/REPORT.md`.

## PE-B54K-AG — generic MDEC reset/table substrate (2026-08-31)

The complete 60-word `func_8010C0FC` mode-zero/mode-one reset and its 36-word
DMA0 table-submit helper are now translated over a generic value-only MDEC
substrate. Retail's MMIO pointer table, DPCR `| 0x88`, DMA0 register values,
and both 32-word command blocks are authenticated. Real Disc 1 submits exact
quantization and scale payloads; the second submission remains pending rather
than being falsely completed. A synthetic payload control and mutation-free
invalid-mode boundary pass. Normal and fresh ASan/UBSan CTest pass, the native
suite is 989/989, and strict production stops at
`func_801924F8_80192730_cut`. Evidence:
`docs/evidence/pe-b54kag-mdec-reset/REPORT.md`.

## PE-B54K-AF — libpress `DecDCTReset` wrapper (2026-08-31)

The 38-sector payload `[0x039F,0x03C5)` is now authenticated as the retail
MDEC/libpress module. Its debug strings, 256-byte environment pair, and
channel-0/channel-1 callback wrappers establish the SDK family. The complete
13-word `func_8010BE3C` is `DecDCTReset`: mode zero calls the already-complete
`ResetCallback`, then all modes forward unchanged to `func_8010C0FC`.
Production now executes the call at `func_801924F8+0x230` and stops at that
60-word internal MDEC/DMA reset rather than treating it as a no-op. Mode-zero
and mode-one controls pass, normal and fresh ASan/UBSan CTest pass, and the
native suite is 988/988. Evidence:
`docs/evidence/pe-b54kaf-decdctreset/REPORT.md`.

## PE-B54K-AE — movie state setup (2026-08-31)

The authenticated `func_801924F8` prefix now covers 140 of 271 words through
`0x80192728`. The new 69-word block copies the returned `CdlLOC`, populates
four retail pointer fields, builds the two record-derived coordinate pairs,
selects one with the low byte of `D_800ACDDC`, and derives 16 versus 24 from
the movie-kind byte. The block contains no calls; native stops immediately
before retail's `jal 0x8010BE3C`. Real Disc 1 and a poisoned synthetic
buffer-1/kind-zero control both pass, as do normal and fresh ASan/UBSan CTest.
The native suite is 987/987 and strict execution stops at
`func_801924F8_80192728_cut`. Evidence:
`docs/evidence/pe-b54kae-1924f8-state/REPORT.md`.

## PE-B54K-AD — movie filename/search continuation (2026-08-31)

The authenticated `func_801924F8` prefix now covers 71 words through
`0x80192614`. Retail selects `\\FMV1` below record index 21 and `\\FMV2`
otherwise, appends the record suffix through the proven BIOS A(15h) `strcat`
trampoline, waits for the CD queue, and calls `DsSearchFile`. Real Disc 1
index 1 resolves `FMV001.STR;1` at LBA 189742; a synthetic index-21 control
independently proves the FMV2 branch without planting production state.
Normal and fresh ASan/UBSan CTest pass, the native suite is 986/986, and
strict real-disc execution stops honestly at
`func_801924F8_80192614_cut`. Evidence:
`docs/evidence/pe-b54kad-1924f8-filename/REPORT.md`.

## PE-B54K-AC — retail CD sector contract (2026-08-31)

The next `func_801924F8` continuation exposed and corrected an older host-unit
mismatch: retail passes sector counts through `func_8006E6A8` / `8006E6D4`
into `func_80080E34`, while the host provider had treated direct counts as
bytes. `func_8006E6D4` is now the single checked sector-to-`0x800`-byte
adaptation point; compensating shifts were removed from `func_8006E6A8` and
the `func_8006CDA4` state-7 cut. Real Disc 1 now loads the complete 133-sector
overlay `[03D2,0457)` into `[8018EFF0,801D17F0)`, verified by SHA/FNV and an
exclusive-end canary. Normal and fresh ASan/UBSan CTest pass; the 985-case
suite is unchanged and strict production still stops honestly at
`func_801924F8_80192584_cut`. Evidence:
`docs/evidence/pe-b54kac-sector-overlay/REPORT.md`.

## Grind-lane port complete — 275 matching C leaves (2026-08-21)

All remaining pe-continuous-decomp grind leaves are ported. After 29388
(below): `func_800293F4` (0x19BF4, 124w, era `-O2 -G8` +
`MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2E8` — new era_compile knob that
strips a symbol's sdata `.extern` so its RMW stays absolute),
`func_8002F76C` (0x1FF6C, 27w, `-O2 -G0`, tail-of-19DE4 carve, no
resume), and the writer trio `func_8002FA10`/`FAA4`/`FAD8`
(0x20210/0x202A4/0x202D8, `-O2 -G0`, head-of-20210 carve, resume
`202F8.s` 0xA3C). Each landed as one commit with a fresh in-container
EXACT SHA-1 build; evidence under `docs/evidence/func-800293F4/`,
`func-8002F76C/`, `func-8002FA10-FAA4-FAD8/`. Branch
`leaves/from-grind-20260821`, pushed to origin.

**Lanes merged (2026-08-21):** `leaves/from-grind-20260821` merged into
`grind/continuous-decomp` (309-commit divergence). Leaf-file conflicts
resolved to the leaves lane (its `build_us.sh`/`disc1.yaml` are
authoritative); grind's `pc_port/` and PE-BTL evidence carried over
untouched. Gates re-run on the merged tree: split `c: 275`, docker build
EXACT SHA-1, `verify_us.sh` EXACT MATCH. The grind lane's pre-merge
handoff narrative (native/PE-BTL state) is preserved at commit
`29fe11b:docs/ai_context/ACTIVE_HANDOFF.md`. PR toward `main` carries
the unified 275 story; deeper doc reconciliation can follow on `main`.

## PE-BTL147 — theater → Eve-entry dependency audit (2026-08-24)

Retail evidence is committed under `docs/evidence/pe-btl147-theater-eve-path/`.
The exhaustive 414-script scan finds the authentic `m0004i` → `m0005i` hops
and zero field-script `0x31` inbound hops to `m0360i`. Retail m0360i module 2
is the unique `persist[0] |= 4` writer; the native branch must not plant that
bit or a destination token. The current native code has no generic scene
scheduler that enters m0360i, so the next faithful rung is the event/scheduler
bridge, not another m0005i gate workaround. Current native suite: 985/985.

## PE-BTL148 — scheduler census needs retail artifact (2026-08-24)

The Disc 1 executable census is recorded at
`docs/evidence/pe-btl148-scheduler-census/REPORT.md`. All 1,011 field-script
`0x31` commands use immediate argument mode. The executable destination-state
writers are limited to ordinary immediate `0x31`, the two fixed computed
name-table handlers (`func_80015790`/`func_80015964`), system/death/menu
states, and save restoration. The computed tables omit `m0360i`, and the
package loader consumes only `D_8009D280`.

No scheduler implementation or m0360i special case is allowed yet. A retail
PCSX trace/save reaching the Day 2+ event, or the executable/overlay that
contains the missing writer, is required to close the provenance. The
standalone native frontier is separately
`PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192584_cut` after
B54K-AB completed the display-pair helper; this
does not change the scheduler evidence boundary.

## PE-BTL149 — runtime-loaded overlay / Disc 2 census (2026-08-24)

The prior Disc 1 executable census was extended to the PE.IMG ranges selected
by retail `D_8009315E..D_8009317A` and to Disc 2. Disc 2 `SLUS_006.68` and
PE.IMG are byte-identical to Disc 1, so they add no distinct code coverage.
The PE.IMG range `[0x0700,0x07B7)` contains a real state-driven chooser:
three direct stores to `D_8009D280` at raw offsets `0x383060`, `0x383230`,
and `0x383258`, with inputs including `D_800A7918`, `D_800A77FC`, and an
indirect `D_801ACA68`/`D_8019F034` dispatch. It constructs nearby computed
destination tokens but not `0xA8066048`; the full PE.IMG scan found zero raw
or `lui 0xA806`→`ori/addiu 0x6048` forms. Evidence and reproducible scanner:
`docs/evidence/pe-btl149-overlay-disc2-census/REPORT.md` and
`tools/research/pe_btl149_overlay_disc2_census.py`.

This closes the “Disc 2 may contain a different executable” hypothesis and
corrects BTL148's coverage boundary, but does not prove the indirect branch's
event input or m0360i selection. Do not implement a scheduler or special-case
m0360i. Remaining scheduler status: `SEMANTIC_IMPLEMENTATION=not_started`
and `SCHEDULER_PROVENANCE=NEEDS_ARTIFACT`. The independent native frontier is
now `func_801924F8` internal cut `0x80192584`; current suite `985/985`.

## PE-BTL150 — name-form search for m0360i (2026-08-24)

The retail alphabet at `D_800930B4` is
`0123456789abcdefghiklmnopqrstuvwxy` (no `j`). `func_8006E2D0` extracts six
5-bit fields at shifts `27,22,17,12,7,2`; `func_8006E3D4`, called by
`func_80015790` and `func_80015964`, is the inverse. Round-trip proof confirms
`0xA8066048` is exactly `m0360i`, so the BTL147 token identity was correct.

The executable and complete PE.IMG name-form scan found zero ASCII
`m0360i`/`M0360I` spellings and zero packed `0xA8066048` words in either byte
order. The generic `D_80093378` package table has the established numeric slot
359 (`D_80093378 + 359*8 = 0x80093EB0`), but that entry is package metadata and
does not provide a static name association. Evidence and scanner:
`docs/evidence/pe-btl150-name-search/REPORT.md` and
`tools/research/pe_btl150_name_search.py`.

BTL150 closes the wrong-token hypothesis and the available static name-form
lead. The unresolved boundary remains the runtime population/selection of the
indirect `D_801ACA68 -> D_8019F034` dispatch. Do not implement a scheduler or
special-case m0360i. Scheduler status remains
`SEMANTIC_IMPLEMENTATION=not_started` / `NEEDS_ARTIFACT`; the independent
native frontier is now `func_801924F8` internal cut `0x80192584`, with suite
`985/985`.

Capture guidance: watch `func_8006E3D4` callers and their six-byte inputs, then
the resulting `D_8009D280` write. Since `m0367i=0xA80663C8` and
`m0360i=0xA8066048` differ only in one 5-bit field, runtime field derivation
from a neighboring token is a concrete hypothesis to test; this is not a
native implementation claim.

## PE-B54K-B1 — func_80030894 through L4 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x80030C9C)`:
258 words total. B54K-B1 added the exact 118-word continuation
`[0x80030AC4,0x80030C9C)`, covering the bank-local fixed tile/G4/state/sprite
setup and the complete four-packet L4 loop. The first excluded instruction at
`0x80030C9C` initializes L5, so the named strict frontier is now
`func_80030894_L4_cut`.

The independent oracle verifies the exact executable SHA-1, whole-window
SHA-256, all seven `jal` sites in order, 39 selected retail words, the L4
back edge/bound, and the first excluded L5 word. Two focused tests verify all
fixed fields, the exact `4 * 28 = 0x70` L4 extent, dirty/repeat determinism,
and the unresolved-boundary stop. Normal and fresh ASan/UBSan suites pass
`930/930`; the sanitizer audit also fixed a pre-existing negative signed-shift
UB in GTE translation math at commit `469f13c`.

Evidence: `docs/evidence/pe-b54kb1-30894-l4/REPORT.md` and
`pc_port/tools/b54kb1_30894_l4_oracle.py`. Scheduler provenance remains
separately `NEEDS_ARTIFACT`: no destination token, m0360i special case, or
persist bit was added. The next available production rung is L5; the BTL151
packer capture remains the next scheduler rung.

## PE-B54K-B2 — func_80030894 through L5 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x80030D20)`:
291 words total. B54K-B2 adds the retail 33-word L5 continuation
`[0x80030C9C,0x80030D20)`. Its five packets use the machine-decoded address
`0x8009E1D0 + bank*140 + slot*28`, close exactly after `5*28 = 0x8C`
bytes, and stop before the post-L5 setup instruction at `0x80030D20`.

The independent oracle checks every one of the 33 retail words plus the first
excluded word, the sole `jal func_800370DC`, loop edge/bound, and extent. Two
tests verify all packet fields, complete dirty/repeat determinism, both end
sentinels, and the named `func_80030894_L5_cut`. Normal and rebuilt
ASan/UBSan suites pass `932/932` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kb2-30894-l5/REPORT.md` and
`pc_port/tools/b54kb2_30894_l5_oracle.py`. Scheduler provenance remains
`NEEDS_ARTIFACT`; the next artifact-free production rung begins with the
post-L5 fixed-group setup.

## PE-B54K-C — func_80030894 through L6 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x80030F6C)`:
438 words total. The 147-word C rung closes a coherent group of two G4
records, three shaded sprite records, and the three-tile L6 loop. Its exact
write set is six separate ranges; gap/end sentinels prevent a broad envelope
from hiding stray writes. L6 uses the three prologue bytes from
`D_8009CD90`, one replicated RGB value per tile.

The independent oracle verifies the whole retail-window hash, all nine calls
in order, 55 selected words, L6 edge/bound/extent, and the first excluded
`D_8009E460` materialization. Focused state and dirty/repeat tests pass;
normal and rebuilt ASan/UBSan suites are `934/934` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kc-30894-l6/REPORT.md` and
`pc_port/tools/b54kc_30894_l6_oracle.py`. The named production boundary is
`func_80030894_L6_cut`; scheduler provenance remains independently
`NEEDS_ARTIFACT`.

## PE-B54K-D — func_80030894 through L7 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x800310A4)`:
516 words total. The 78-word D rung builds one wrapped sprite, two direct
SetSprt records, one PolyF3 header, and the complete three-sprite L7 loop.
Its write set is five separate ranges; per-range end sentinels and poisoned
retail-untouched bytes guard against broad writes. L7 closes exactly after
`3*28 = 0x54` bytes.

The independent oracle verifies the whole retail-window hash, all five calls
in order, 52 selected words, L7 edge/bound/extent, and both cut-side boundary
words. Focused state and dirty/repeat tests pass; normal and rebuilt
ASan/UBSan suites are `936/936` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kd-30894-l7/REPORT.md` and
`pc_port/tools/b54kd_30894_l7_oracle.py`. The named production boundary is
`func_80030894_L7_cut`; the first excluded word initializes L8 at
`0x800310A4`. Scheduler provenance remains independently `NEEDS_ARTIFACT`.

## PE-B54K-E — func_80030894 through L8 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x80031110)`:
543 words total. The 27-word E rung closes the ten-sprite L8 loop at
`D_8009E500 + bank*280 + slot*28`. The exact bank-zero span is
`[0x8009E500,0x8009E618)`; each wrapped sprite receives RGB `0x80`, while
all twelve bytes at `+0x10..+0x1B` remain untouched.

The independent oracle compares all 27 words, verifies the sole static call,
L8 edge/bound/strides/extent, and both cut-side words. Focused full-span and
dirty/repeat tests pass; normal and rebuilt ASan/UBSan suites are `938/938`
with zero diagnostics.

Evidence: `docs/evidence/pe-b54ke-30894-l8/REPORT.md` and
`pc_port/tools/b54ke_30894_l8_oracle.py`. The named production boundary is
`func_80030894_L8_cut`; the first excluded word materializes
`D_8009E768` at `0x80031110`. Scheduler provenance remains independently
`NEEDS_ARTIFACT`.

## PE-B54K-F — func_80030894 through L9 (2026-08-30)

The production prefix now implements retail `[0x80030894,0x800311EC)`:
598 words total. The 55-word F rung builds one fixed wrapped sprite at
`D_8009E768 + bank*28` and the complete four-sprite L9 loop at
`D_8009E7A0 + bank*112 + slot*28`. Its two packet ranges remain separate
across the retail gap, and L9 closes exactly after `4*28 = 0x70` bytes.

The independent oracle compares all 55 words, verifies both static calls,
L9 edge/bound/strides/extent, and both cut-side words. Focused full-union and
dirty/repeat tests pass; normal and rebuilt ASan/UBSan suites are `940/940`
with zero diagnostics.

Evidence: `docs/evidence/pe-b54kf-30894-l9/REPORT.md` and
`pc_port/tools/b54kf_30894_l9_oracle.py`. The named production boundary is
`func_80030894_L9_cut`; the first excluded word materializes
`D_8009E730` at `0x800311EC`. Scheduler provenance remains independently
`NEEDS_ARTIFACT`.

## PE-B54K-G — func_80030894 through L10 (2026-08-30)

The production prefix now implements retail `[0x80030894,0x80031320)`:
675 words total. The 77-word G rung preserves the retail call-coupled group:
fixed wrapper at `D_8009E730`, CLUT computation `(0x130,0x1F9)->0x7E53`,
fixed wrapper at `D_8009E880`, and the complete two-sprite L10 loop at
`D_8009E8B8 + bank*56 + slot*28`.

The independent oracle compares all 77 words, verifies all four calls in
order, CLUT inputs/value, L10 edge/bound/strides/extent, and both cut-side
words. Focused full-union and dirty/repeat tests pass; normal and rebuilt
ASan/UBSan suites are `942/942` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kg-30894-l10/REPORT.md` and
`pc_port/tools/b54kg_30894_l10_oracle.py`. The named production boundary is
`func_80030894_L10_cut`; the first excluded word materializes
`D_8009E928` at `0x80031320`. Scheduler provenance remains independently
`NEEDS_ARTIFACT`.

## PE-B54K-H — func_80030894 through L11 (2026-08-30)

The production prefix now implements retail `[0x80030894,0x80031438)`:
745 words total. The 70-word H rung builds the fixed sprite at
`D_8009E928 + bank*28` and the complete thirteen-sprite L11 loop at
`D_8009E960 + bank*364 + slot*28`. L11 consumes patterned descriptors
`func_8005DADC(0x6A + slot)` and preserves the retail U/V/CLUT/width/height
field mapping while leaving every XY word untouched.

The independent oracle compares all 70 words, verifies all four static calls,
descriptor/TPage constants, L11 edge/bound/strides/extent, and both cut-side
words. Focused patterned-state and dirty/repeat tests pass; normal and rebuilt
ASan/UBSan suites are `944/944` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kh-30894-l11/REPORT.md` and
`pc_port/tools/b54kh_30894_l11_oracle.py`. The named production boundary is
`func_80030894_L11_cut`; the first excluded word at `0x80031438` begins the
final 43-word TPage/fixed-sprite/outer-loop epilogue. Scheduler provenance
remains independently `NEEDS_ARTIFACT`.

## PE-B54K-I — func_80030894 complete (2026-08-30)

The final 43-word window `[0x80031438,0x800314E4)` is now native, completing
all 788 words of `func_80030894`. It builds the `16 x 16` final sprite at
`D_8009EC38 + bank*28`, increments the bank, repeats the complete bank-local
body for banks 0 and 1 through the retail back edge to `0x80030910`, then
returns through `jr ra + nop`.

The independent oracle compares all 43 new words and hashes the full body,
verifies both final calls, final packet fields, outer-loop edge/bound/extent,
and next-function boundary. Focused tests sample every second-bank packet
family and dirty/repeat the complete two-bank E-region; normal and rebuilt
ASan/UBSan suites are `946/946` with zero diagnostics.

Evidence: `docs/evidence/pe-b54ki-30894-complete/REPORT.md` and
`pc_port/tools/b54ki_30894_complete_oracle.py`. No unresolved provider remains
inside `func_80030894`. B54K-J below supersedes this rung's caller-side
frontier. Scheduler provenance remains independently `NEEDS_ARTIFACT`.

## PE-B54K-J — D_800930F0 completion/reissue gate (2026-08-30)

The exact eight-word caller window `[0x8006B0B4,0x8006B0D4)` is now native.
It polls the F0 read after `func_80030894`, clears the provider's busy bits on
completion, returns only to the F0 issue on `-1`, and skips both `718D0` and
`func_80030894` on that reissue path. The implemented `func_8006AD40` prefix
now closes at `0x394` bytes / 229 words.

The independent oracle authenticates the executable and window hash,
compares all 8 words, decodes both back edges and the sole call, and checks
both boundary words. Two focused tests prove canonical completion and
repeat/no-duplicate behavior. Normal and freshly rebuilt ASan/UBSan suites
pass `948/948` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kj-6ad40-f0-wait/REPORT.md` and
`pc_port/tools/b54kj_6ad40_f0_wait_oracle.py`. The production frontier is now
superseded by B54K-K below. Scheduler provenance remains independently
`NEEDS_ARTIFACT`; no destination token, `m0360i` branch, or persistence bit
was added.

## PE-B54K-K — D_800930E0 issue, F0 lookups, and completion (2026-08-30)

The natural 38-word caller group `[0x8006B0D4,0x8006B16C)` is now native. It
issues the five-sector E0 range into `*(D_800B0CD8+0x16C)`, resolves three
retail keys from the completed F0 archive into `+0x11C/+0x120/+0x124` exactly
once, and then consumes the live E0 completion. Timeout reissues only E0;
positive polls do not replay the lookups. The implemented `func_8006AD40`
prefix now closes at `0x42C` bytes / 267 words.

The independent oracle authenticates and compares all 38 words, all five
calls, four control-flow edges, exact keys/stores, boundaries, and prefix
arithmetic. Focused tests use a 256-sector MODE2 fixture with a real on-disc
three-key F0 archive plus an empty negative control. Normal and freshly
rebuilt ASan/UBSan suites pass `950/950` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kk-6ad40-e0-group/REPORT.md` and
`pc_port/tools/b54kk_6ad40_e0_group_oracle.py`. The production frontier is
superseded by B54K-L below. Scheduler provenance remains independently
`NEEDS_ARTIFACT`; no destination token, `m0360i` branch, or persistence bit
was added.

## PE-B54K-L — first D_80093126 issue/walk/wait (2026-08-30)

The coherent 45-word group `[0x8006B16C,0x8006B220)` is now native. It issues
the two-sector `D_80093126={0x219,0x21B}` range into
`*(D_800B0CD8+0x188)`, walks the completed E0 archive at `+0x16C` with the
retail packed count/offset and 0x14-byte entry stride exactly once, then
consumes completion. Timeout reissues only the 3126 range; positive polls
skip the completed entry walk. The implemented prefix is now 0x4E0 bytes /
312 words.

The independent oracle authenticates and compares all 45 words, issue ABI,
entry loop, retry topology, both boundaries, and the poll branch's mandatory
delay-slot `lui`. Focused tests prove a two-entry archive, ordered data
addresses, exact two-sector transfer, and a zero-count negative. Normal and
freshly rebuilt ASan/UBSan suites pass `952/952` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kl-6ad40-3126-wait/REPORT.md` and
`pc_port/tools/b54kl_6ad40_3126_wait_oracle.py`. At that historical rung the
production frontier was `func_8006AD40_D_80093126_archive_cut` before retail
`0x8006B220`; B54K-M below has since completed the function.
Scheduler provenance remains independently `NEEDS_ARTIFACT`; no destination
token, `m0360i` branch, or persistence bit was added.

## PE-B54K-M readiness — historical pre-implementation audit (2026-08-30)

At this audit rung, `[0x8006B220,0x8006B35C)` was fully audited but not yet
implemented; the completion section below supersedes that disposition. The
independent oracle authenticates all 79 words, both
window hashes, the completed `+0x188` archive walk, all seven now-available
callees, display-env selection, exact state resets, bit-0 clear, normal
return, and the next-function boundary.

A temporary completion translation compiled, then was reverted after a full
trial produced `929/952`: all 23 failures were historical contracts that
intentionally require the current prefix frontier or pre-finalization repeat
behavior. The dedicated implementation rung must migrate those assertions to
full-function state checks, add positive/zero `+0x188` walks, the `0x40/0x80`
matrix, first-call-clear/second-call-guard behavior, stream-F1 whole-RAM
effects, and measure the caller's next strict frontier after its two DMA
checkpoints. Production remained at B54K-L and `952/952` at this audit rung.

Evidence:
`docs/evidence/pe-b54km-6ad40-completion-readiness/REPORT.md` and
`pc_port/tools/b54km_6ad40_completion_readiness.py`.

## PE-B54K-N — BIOS `FlushCache` adapter (2026-08-30)

`func_800726C4` is now a generic native platform provider. Retail is the
three-word BIOS A0(44h) `FlushCache` veneer, with 14 exact executable callers.
The host adapter is a deliberate no-op because native code has no emulated
R3000 instruction-cache authority. Direct strict execution and strict
`func_8006E834` integration pass without guest writes, bootstrap records, or
an unbalanced critical section. The suite is `954/954`.
The fresh ASan/UBSan suite is also `954/954` with zero diagnostics.

At B54K-N this did not move the B54K-L production frontier. B54K-M below has
since completed 6AD40 and measured `func_801909B4` after both DMA checkpoints,
confirming this static prediction.
Evidence: `docs/evidence/pe-b54kn-726c4-flushcache/REPORT.md` and
`pc_port/tools/b54kn_726c4_flushcache_oracle.py`.

## PE-B54K-O — `func_801909B4` overlay recovery (2026-08-30)

The canonical post-B54K-M boundary's retail bytes are now recovered. Retail
6E834 loads PE.IMG `[0x03D2,0x0457)` at `0x8018EFF0`, placing
`func_801909B4` at PE.IMG offset `0x1EA9C4`. Its exact range is
`[0x801909B4,0x801918F8)`, 0xF44 bytes / 977 words, with 70 direct calls to
32 targets and a normal return.

The audit found a prerequisite native discrepancy: `D_80093164` is currently
modeled as zero BSS and `D_80011614` uses bootstrap address `0x8010BD00`,
whereas retail rodata is `{0x03D2,0x0457}` and `0x8018EFF0`. Thus native
6E834 presently performs a zero-length read and does not load this overlay.
Do not claim natural 801909B4 entry until a dedicated authority/layout rung
fixes and tests those values. Production and suite remain B54K-L / `954/954`.
Evidence: `docs/evidence/pe-b54ko-1909b4-overlay-recovery/REPORT.md` and
`pc_port/tools/b54ko_1909b4_overlay_oracle.py`.

## PE-B54K-P — retail overlay authority handoff (2026-08-30)

Real-disc startup now adopts `D_80011614=0x8018EFF0` and
`D_80093164[0..3]={0x03D2,0x0457,0x04FC,0x0516}` from guest RAM only after
the boot EXE is authenticated and loaded. Range ordering and the complete
0x42800-byte destination are validated before publication. Bootstrap fixtures
retain their explicit safe defaults.

Normal and fresh ASan/UBSan suites pass `956/956`; a real-disc strict smoke
still reaches the unchanged B54K-L frontier. Evidence:
`docs/evidence/pe-b54kp-overlay-authority/REPORT.md` and
`pc_port/tools/b54kp_overlay_authority_oracle.py`.

## PE-B54K-M — complete func_8006AD40 (2026-08-30)

All 391 retail words of `func_8006AD40` are now implemented through the
normal `jr ra; nop` return. The final 79-word suffix walks the completed
`+0x188` archive, issues stream command F1, performs DrawSync/ResetGraph/
VSync/PutDispEnv/SetDispMask in retail order, applies the complete
`0x40`/`0x80` conditional-field matrix, and clears `D_800B0CD8` bit 0 last.

All 23 historical B54K-L frontier failures were migrated to full-function
contracts without deleting their earlier loop, poll, transfer, image-call,
DMA, and ordering assertions. Two new tests cover positive/zero final
archives and all four flag combinations. Normal and fresh ASan/UBSan suites
pass `958/958`; sanitizer diagnostics are zero.

The real-disc caller executes both explicit DMA checkpoints (`2/2` calls,
queries, and services; token 2 serviced). The next measured unbounded strict
frontier is `func_801909B4` from `func_8001220C`. Evidence:
`docs/evidence/pe-b54km-6ad40-complete/REPORT.md` and
`pc_port/tools/b54km_6ad40_completion_oracle.py`.

```text
FUNC_8006AD40=COMPLETE_NATIVE_TRANSLATION
PRODUCTION_REACHABILITY=blocked_at_func_801909B4
FUNC_801909B4_BYTES=STATICALLY_RECOVERED_NOT_IMPLEMENTED
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-Q — `func_801909B4` MoveImage prefix (2026-08-30)

Historical rung, superseded by B54K-R below.

The real-disc overlay now enters 149 translated retail words
`[0x801909B4,0x80190C08)`. The prefix copies two 0x5C-byte DRAWENV records
and two 0x14-byte DISPENV records into overlay storage, publishes six arena
pointers from authenticated `D_80011610`, executes translated
`func_8005E57C`, the positive path of `func_8005C1EC`, complete
`func_80042538`, and `SetDispMask(0)`, then captures the first unresolved
call.

The exact next provider is PsyQ `MoveImage` (`func_8007512C`) at
`0x80190C08`, with arguments `RECT{320,0,160,256}`, destination `(704,0)`.
The full RECT payload is recorded and the caller now honors a nested stop
before consuming the retained `-1` return. Strict real-disc execution proves
natural overlay entry and reports `func_8007512C` from `func_801909B4`.

Five focused tests, full normal, and fresh ASan/UBSan suites pass `962/962`
with zero sanitizer diagnostics. Evidence:
`docs/evidence/pe-b54kq-1909b4-moveimage-prefix/REPORT.md` and
`pc_port/tools/b54kq_1909b4_prefix_oracle.py`.

```text
FUNC_8006AD40=COMPLETE_NATIVE_TRANSLATION
FUNC_801909B4_PREFIX=149_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_8007512C_from_func_801909B4
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-R — generic MoveImage and display prefix (2026-08-30)

PsyQ `func_8007512C` is now a complete 46-word native translation. Retail
`D_80095744=0x80095704` proves that it loads `func_80076C34` from jump-table
offset `+8` and `func_80076B98` from `+0x18`; it does not route through
`func_80076C10`. The exact 18-word worker accepts only the authenticated
five-word GP0(80h) MoveImage packet. General DrawOTag linked lists remain a
named boundary.

The generic GPU authority now performs synchronous VRAM-to-VRAM copies with
retail coordinate/size masking, zero-as-maximum dimensions, both-axis wrap,
and the console-verified horizontal overlap direction. It has no overlay
special case and creates no synthetic DMA2 completion or callback.

`func_801909B4` now translates 240 words
`[0x801909B4,0x80190D74)`: the canonical `160x256` MoveImage, DrawSync/VSync
sequence, two draw/display environments, exact field stores, and black
ClearImage all execute before the one-time overlay-local call
`func_80190660`. Eight focused contracts, the independent oracle, full
normal suite, and fresh ASan/UBSan suite pass; total is `967/967` with zero
sanitizer diagnostics. Strict real-disc execution measures
`func_80190660 from func_801909B4` as the next provider.

Evidence:
`docs/evidence/pe-b54kr-moveimage-display-prefix/REPORT.md` and
`pc_port/tools/b54kr_moveimage_oracle.py`.

```text
FUNC_8007512C=COMPLETE_NATIVE_TRANSLATION
FUNC_80076B98=MOVEIMAGE_ONE_PACKET_SUBSET_ONLY
FUNC_801909B4_PREFIX=240_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80190660_from_func_801909B4
GENERAL_LINKED_LIST_DMA=UNSUPPORTED_NAMED_BOUNDARY
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-S — deterministic DrawSync DMA drain (2026-08-30)

The 26-word PsyQ `func_80074DC0` wrapper and execution-proven 79-word
`func_80077294` drain are now represented. Mode zero preserves retail queue,
DMA-busy, and GPU-ready tests; nonzero mode preserves the exact pending/status
returns. Between calls to retail wait helper `func_80077404`, the host admits
at most one already-active DMA token through the established
`PE_Port_ServiceDmaIrqCheckpoint` owner. Polling reads never evolve hardware,
and the translated pump remains the only guest-ring consumer.

The normal wait-poll half of `func_80077404` preserves deadline and poll-word
semantics. Its destructive timeout recovery and a state with no external
progress remain named boundaries. Four focused contracts prove idle/status,
direct DMA, a two-transfer queued drain, and the no-progress negative. Full
normal and fresh ASan/UBSan suites pass `971/971`.

Real-disc execution now records 27 checkpoint opportunities, 26 active-token
queries/services, then reaches the unchanged `func_80190660 from
func_801909B4` frontier. This supersedes B54K-M's historical 2/2
caller-checkpoint service trace; it does not change scheduler provenance.

Evidence: `docs/evidence/pe-b54ks-drawsync-drain/REPORT.md` and
`pc_port/tools/b54ks_drawsync_oracle.py`.

```text
FUNC_80074DC0=DRAWSYNC_WRAPPER_ADAPTED
FUNC_80077294=EXECUTION_PROVEN_PATHS_TRANSLATED
FUNC_80077404=NORMAL_POLL_TRANSLATED_TIMEOUT_RECOVERY_FENCED
DMA_CHECKPOINT_TOTAL=27_CALLS_26_ACTIVE_TOKENS
PRODUCTION_REACHABILITY=blocked_at_func_80190660_from_func_801909B4
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-T — `func_80190660` image/fade prefix (2026-08-30)

Function-hood is proven for the 213-word overlay-local `func_80190660`: one
exact-start call at `0x80190D74`, a canonical `jr ra; nop`, and real preceding
and following function boundaries. Its first 128 words
`[0x80190660,0x80190860)` now execute.

The prefix computes image records from `[0x80193278]=0x3BAC8` and anchor
`0x80193254`, yielding records `0x801CED1C` and `0x801CED50`. Their retail
RECTs `{0,480,16,1}` and `{512,256,64,64}` traverse LoadImage; DrawSync drains
the large transfer through one established checkpoint in the focused test.
It then builds both `E1000018/E1000019` draw-mode banks and paired SPRTs,
clears both environment `+0x6D` bytes, enables display, applies the exact
`old==0 ? 1 : 0` selector, and stores the selected environment pointer.

The new boundary is `jal func_80075358` at `0x80190860`. Native applies its
RGB delay-slot store and records only packet length 1 plus command word
`0xE1000018`; retail's uninitialized stack-tag bytes 0..2 and a native stack
pointer are excluded. Two focused contracts, retained B54K-R integration,
the independent oracle, full normal suite, and fresh ASan/UBSan pass
`973/973`. Real-disc framebuffer state is now 6 VSync, 4 DrawSync, 3
presentations, mask 1; the aggregate DMA checkpoint census remains 27/26.

Evidence: `docs/evidence/pe-b54kt-190660-drawprim-prefix/REPORT.md` and
`pc_port/tools/b54kt_190660_drawprim_prefix_oracle.py`.

```text
FUNC_80190660_PREFIX=128_WORDS_TRANSLATED
OVERLAY_IMAGE_RECORDS=TABLE_DERIVED_AND_EXECUTED
TRANSIENT_PACKET_POINTER=NOT_RETAINED
PRODUCTION_REACHABILITY=blocked_at_func_80075358_from_func_80190660
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-U — DrawPrim wrapper + GP0(E1h) command (2026-08-30)

Retail `func_80075358` is the 23-word DrawPrim wrapper
`[0x80075358,0x800753B4)`. It calls jump-table slot 15 / `func_80077294`
(DrawSync), re-reads the jump-table pointer, then calls slot 5 / the 16-word
`func_80076B58` worker with `packet+4` and `packet[3]`. The worker always
writes GP1(04h), DMA direction off, then writes exactly the requested GP0
words. Dirty indirect targets remain typed boundaries.

The GPU authority now implements generic GP0(E1h) draw-mode state. The first
overlay DrawPrim executes command `0xE1000018`, growing the authenticated
prefix to 130 words, `[0x80190660,0x80190868)`. The next boundary is the
second DrawPrim at `0x80190868`, whose complete four-word SPRT packet is
`64000000 00580020 78000000 00400100`. Its tpage/CLUT geometry lines up with
the two table-derived uploads from B54K-T; rasterization is not claimed yet.

Two new focused contracts, all retained B54K-T/B54K-R contracts, the
independent oracle, full normal suite, and fresh ASan/UBSan pass `975/975`.
Real-disc framebuffer state is 6 VSync, 5 DrawSync, 3 presentations, mask 1;
the aggregate DMA checkpoint census remains 27/26.

Evidence: `docs/evidence/pe-b54ku-drawprim-e1/REPORT.md` and
`pc_port/tools/b54ku_drawprim_e1_oracle.py`.

```text
FUNC_80075358=EXECUTION_PROVEN_PATH_TRANSLATED
FUNC_80076B58=COUNTED_COMMAND_WORD_PATH_TRANSLATED
GP0_E1=DRAW_MODE_STATE_IMPLEMENTED
FUNC_80190660_PREFIX=130_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80075358_sprite_at_80190868
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-V — GP0(64h) textured SPRT (2026-08-30)

The second DrawPrim now traverses the exact wrapper and a generic GP0(64h)
parser/raster path. The accepted subset is opaque, modulated, variable-size
4bpp: it sign-extends/clips destination coordinates, wraps eight-bit UV,
fetches packed nibbles and the packet-selected CLUT from VRAM, preserves zero
texture-color transparency and CLUT bit 15, and applies the retail 5-bit by
8-bit modulation rule. Unsupported depth/raw forms are mutation-free fences;
drawing-area/offset/mask state is not guessed.

Retail data proves the canonical packet is a 256x64 SPRT at `{32,88}` using
tpage `{512,256}` and CLUT `{0,480}`. The authenticated overlay prefix now
covers 183 words, `[0x80190660,0x8019093C)`, including the explicit DrawSync,
canonical optional-LoadImage bypass, VSync(0), and ResetGraph(1). PutDrawEnv
`func_80075424` at `0x8019093C` is next.

Two focused contracts, retained B54K-U/T/R contracts, the independent oracle,
the full normal suite, and fresh ASan/UBSan pass `977/977`. Real-disc
framebuffer telemetry is 7 VSync, 7 DrawSync, 3 presentations, mask 1; DMA
remains 27/26.

Evidence: `docs/evidence/pe-b54kv-textured-rectangle/REPORT.md` and
`pc_port/tools/b54kv_textured_rectangle_oracle.py`.

```text
GP0_64=OPAQUE_MODULATED_4BPP_VARIABLE_RECTANGLE_IMPLEMENTED
FUNC_80190660_PREFIX=183_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80075424_from_func_80190660
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-W — PutDrawEnv and first loop back-edge (2026-08-30)

The canonical PutDrawEnv path is translated from the complete 48-word retail
wrapper. It builds the terminal six-command DR_ENV node, dispatches it through
the existing `jtb[2]/jtb[6]` identities, then caches 0x5C bytes only after the
worker returns. The generic GPU authority now represents E2h..E6h texture
window, drawing area, offset, and mask state; the GP0(64h) raster consumes
those registers. Unsupported node shapes are rejected before GP1 mutation.

The B54K-W authenticated prefix was 192 words through PutDispEnv and the first
taken branch delay slot. B54K-X below supersedes that execution frontier while
this section remains authoritative for PutDrawEnv and GP0 environment state.

Evidence: `docs/evidence/pe-b54kw-putdrawenv/REPORT.md` and
`pc_port/tools/b54kw_putdrawenv_oracle.py`.

```text
PUTDRAWENV_CANONICAL_DR_ENV=IMPLEMENTED
GP0_E2_E6_ENVIRONMENT=IMPLEMENTED
FUNC_80190660_PREFIX=192_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80190660_loop_reentry_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=translate_func_80190660_480_frame_loop
```

## PE-B54K-X — complete overlay fade loop (2026-08-30)

Overlay-local `func_80190660` is complete: all 213 authenticated words and all
480 state-driven fade/display iterations execute. The four retail intensity
segments, parity packet banks, environment toggle, optional-upload gate,
DrawPrim/DrawSync/VSync/ResetGraph/PutDrawEnv/PutDispEnv order, final
SetDispMask(0), display-byte restore, and normal return are represented.

Canonical direct cardinality is 480 rectangles, 960 E1 commands, 480 of each
E2h..E6h environment command, 1441 DrawSync calls, 480 VSync calls, and 480
presentations. Two focused contracts include a nonzero-phase negative control.
All 981 normal tests, fresh ASan/UBSan, retained R/U/V/W oracles, and the new
independent oracle pass. Real-disc telemetry is 486 VSync, 1444 DrawSync, 483
presentations, mask 0; DMA remains 27/26. The exact next instruction is the
caller's saved-bit branch at `0x80190D7C`.

Evidence: `docs/evidence/pe-b54kx-overlay-loop/REPORT.md` and
`pc_port/tools/b54kx_overlay_loop_oracle.py`.

```text
FUNC_80190660=COMPLETE_213_WORDS
OVERLAY_FADE_LOOP=480_FRAMES_STATE_DRIVEN
PRODUCTION_REACHABILITY=blocked_at_func_801909B4_80190D7C_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_saved_bit_branch_at_80190D7C
```

## PE-B54K-Y — saved-bit arm and `func_80192CE8` prefix (2026-08-30)

The caller's authenticated `D_800B0DCD & 1` branch is now represented. Disc 1
takes the nonzero arm into `func_80192CE8(1)`; zero remains the exact
`0x80191120` structural cut. The first 69 of `func_80192CE8`'s 172 words are
translated through the initial state writes, display/reset calls, retail
issue/retry/poll loop, critical-section/cache sequence, and first
overlay-local call.

The table selects PE.IMG `[0x039F,0x03C5)`: 38 sectors into retail pointer
`0x8010BCF8`, payload SHA-256
`d0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5ba8d0b40`.
The next call is `func_80191FB8(1, &0x80122D00)`; its transient stack word is
recorded by value, never as a host pointer. The zero-arm negative control
proves no read-prefix state is touched. All 983 normal and fresh ASan/UBSan
tests pass; real-disc production stops exactly at `func_80191FB8` from
`func_80192CE8`, with DMA still 27/26.

Evidence: `docs/evidence/pe-b54ky-192ce8-prefix/REPORT.md` and
`pc_port/tools/b54ky_192ce8_prefix_oracle.py`.

```text
FUNC_80192CE8_PREFIX=69_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80191FB8_from_func_80192CE8
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_func_80191FB8
```

## PE-B54K-Z — complete `func_80191FB8` (2026-08-30)

Overlay-local `func_80191FB8` is complete: all 207 authenticated words,
one/two-source pointer layouts, `0x28`/`0xB8` environment copies, state bytes,
and both generic MoveImage calls execute. The `0x08000000` gate skips only
the second move; invalid count, null-member, and busy paths are mutation-free.
The transient caller-stack pointer list is consumed by value without assigning
a host pointer any guest identity.

All 985 normal and fresh ASan/UBSan tests pass. Production now stops at
`func_801924F8` from `func_80192CE8`; framebuffer and DMA telemetry remain
486/1445/483 and 27/26. Evidence:
`docs/evidence/pe-b54kz-191fb8-complete/REPORT.md` and
`pc_port/tools/b54kz_191fb8_oracle.py`.

```text
FUNC_80191FB8=COMPLETE_207_WORDS
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_from_func_80192CE8
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_func_801924F8
```

## PE-B54K-AA — enter `func_801924F8` (2026-08-30)

`func_801924F8` is authenticated as a real 271-word function with one exact
caller and a normal return. Its first 29 words are translated: index `<47`
selects a 20-byte record, publishes its pointer, and passes signed record byte
`+4` to the first overlay-local call. Index 47 is mutation-free. Production
now stops exactly at `func_801918F8(0, kind)` from `func_801924F8`; all 985
tests remain green. Evidence:
`docs/evidence/pe-b54kaa-1924f8-prefix/REPORT.md`.

```text
FUNC_801924F8=AUTHENTICATED_271_WORDS_PREFIX_29_WORDS
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192584_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_func_801918F8
```

## PE-B54K-AB — complete `func_801918F8` (2026-08-30)

The 155-word overlay display-pair initializer is complete. Four exact callers,
normal return, both 320-wide and 480-then-folded-to-320 paths, and all SDK
environment effects are proven. `func_801924F8` now executes both calls and
reaches exact cut `0x80192584`; 985/985 tests remain green. Evidence:
`docs/evidence/pe-b54kab-1918f8-complete/REPORT.md`.

```text
FUNC_801918F8=COMPLETE_155_WORDS
FUNC_801924F8=AUTHENTICATED_271_WORDS_PREFIX_35_WORDS
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192584_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=continue_func_801924F8_at_80192584
```

## func_800125E0 — descriptor spawn loop matching C (35 words)

**276 matching C leaves.** `src/func_800125E0.c` matches era `-O2 -G8`,
VRAM `0x800125E0` / file `0x2DE0` / size `0x8C`. DrawSync(0), then walk
`**D_8009CE04` (`lbu` count, two-byte descriptors from offset 1) calling
`func_80035038(desc, 0, 1)`. A local list pointer keeps the header in `$a0`
(retail gp-load carry). Pins hold `$s0=count` / `$s1=offset`; an unused
`int` with an empty m-constraint supplies the retail `vars=8` / frame
`0x28` and emits no instructions. Mid-`2D74` carve: prefix `0x6C`, C
`0x8C`, resume `2E6C.s` `0x5830`. Evidence:
`docs/evidence/func-800125E0/REPORT.md`.

## func_80012574 — parked non-exact relocation leaf

Three bounded attempts on `func_80012574` (file `0x2D74`, 27 words) produced
a size-correct candidate with the retail unsigned tag test, empty 8-byte
frame, and loop cursor form, but retained a four-byte count/sum register
allocation mismatch (`$v0`/`$a2` instead of retail `$v1`/`$v0`). The leaf
stays parked and must not be retried without explicit authorization.
Evidence: `docs/evidence/func-80012574/REPORT.md`. The next unmatched
head after 125E0 is `func_8001266C` (file `0x2E6C`, 37 words).

## func_80029388 — slot-table clear + record-init wrapper matching C (27 words)

**270 matching C leaves (now 275, see above).** `src/func_80029388.c` matches era `-O2 -G8` +
`MASPSX_THREE_WORD_SYMBOL_STORE=1`, VRAM `0x80029388` / file `0x19B88` /
size `0x6C`. jal 2F658, 7×220B `SlotRecord` in-use clear (2F9CC shape,
andi-FILLED back-branch slot), gp byte zeros D_8009D2A0/D_8009D2EC, jal
20EFC. Mid-`11718` carve: prefix 0x8470, C 0x6C, resume `19BF4.s` 0x63E4.
Evidence: `docs/evidence/func-80029388/REPORT.md`.

**Build environment note:** the `pe-mipsel-img` docker image was rebuilt
from `dev/mipsel/Dockerfile` (2026-08-21) — the stale image lacked
`python3` and aborted `build_us.sh` at the maspsx step; a prior agent then
hashed a stale candidate and committed a false match (dropped via
`git reset --hard`). `build_us.sh` now deletes `build/disc1.candidate.exe`
at start so a stale artifact can never pass for a fresh build. Run builds
as: `docker run --rm -v "$PWD:/workspace" -w /workspace --user
"$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh`.

## func_8005288C — return-zero stub matching C (2 words)

`src/func_8005288C.c` matches era `-O2 -G0`, VRAM `0x8005288C` / file
`0x4308C` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-8005288c/REPORT.md`.

## func_800CA7B0 — return-zero stub twin matching C (2 words)

`src/func_800CA7B0.c` matches era `-O2 -G0`, VRAM `0x800CA7B0` / file
`0xBAFB0` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800ca7b0/REPORT.md`.

## func_800CA7A8 — return-zero stub matching C (2 words)

`src/func_800CA7A8.c` matches era `-O2 -G0`, VRAM `0x800CA7A8` / file
`0xBAFA8` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800ca7a8/REPORT.md`.

## func_800C9C18 — return-zero stub twin matching C (2 words)

`src/func_800C9C18.c` matches era `-O2 -G0`, VRAM `0x800C9C18` / file
`0xBA418` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800c9c18/REPORT.md`.

## func_800C9C10 — return-zero stub matching C (2 words)

`src/func_800C9C10.c` matches era `-O2 -G0`, VRAM `0x800C9C10` / file
`0xBA410` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800c9c10/REPORT.md`.

## func_800CD978 — return-zero stub twin matching C (2 words)

`src/func_800CD978.c` matches era `-O2 -G0`, VRAM `0x800CD978` / file
`0xBE178` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800cd978/REPORT.md`.

## func_800CD970 — return-zero stub matching C (2 words)

`src/func_800CD970.c` matches era `-O2 -G0`, VRAM `0x800CD970` / file
`0xBE170` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800cd970/REPORT.md`.

## func_80018F0C — five-reader call wrapper matching C (18 words)

`src/func_80018F0C.c` matches era `-O2 -G0`, VRAM `0x80018F0C` / file
`0x970C` / size `0x48`. It forwards five unsigned-halfword reader values to
`func_80066BD8` and returns 1. Evidence:
`docs/evidence/func-80018f0c/REPORT.md`.

## func_80018F54 — D_800BCFEE bit-0x40 clearer matching C (8 words)

`src/func_80018F54.c` matches era `-O2 -G0`, VRAM `0x80018F54` / file
`0x9754` / size `0x20`. It clears bit `0x40` in `D_800BCFEE` and returns 1.
Evidence: `docs/evidence/func-80018f54/REPORT.md`.

## func_80018EE0 — unsigned-halfword reader wrapper twin matching C (11 words)

`src/func_80018EE0.c` matches era `-O2 -G0`, VRAM `0x80018EE0` / file
`0x96E0` / size `0x2C`. It forwards an unsigned halfword from its reader to
`func_80066C7C` and returns 1. Evidence:
`docs/evidence/func-80018ee0/REPORT.md`.

## func_80018EB4 — unsigned-halfword reader wrapper matching C (11 words)

`src/func_80018EB4.c` matches era `-O2 -G0`, VRAM `0x80018EB4` / file
`0x96B4` / size `0x2C`. It forwards an unsigned halfword from its reader to
`func_80066B60` and returns 1. Evidence:
`docs/evidence/func-80018eb4/REPORT.md`.

## func_80018E58 — one-reader call wrapper matching C (11 words)

`src/func_80018E58.c` matches era `-O2 -G0`, VRAM `0x80018E58` / file
`0x9658` / size `0x2C`. It forwards a dereferenced reader value to
`func_80066800` and returns 1. Evidence:
`docs/evidence/func-80018e58/REPORT.md`.

## func_80018D20 — complemented two-reader call wrapper matching C (12 words)

`src/func_80018D20.c` matches era `-O2 -G0`, VRAM `0x80018D20` / file
`0x9520` / size `0x30`. It forwards a dereferenced reader and the complement
of the second to `func_80065A9C`, then returns 1. Evidence:
`docs/evidence/func-80018d20/REPORT.md`.

## func_80018CF0 — two-reader call wrapper matching C (12 words)

`src/func_80018CF0.c` matches era `-O2 -G0`, VRAM `0x80018CF0` / file
`0x94F0` / size `0x30`. It forwards the two dereferenced reader values to
`func_80065A9C` and returns 1. Evidence:
`docs/evidence/func-80018cf0/REPORT.md`.

## func_80018CB8 — three-reader call wrapper matching C (14 words)

`src/func_80018CB8.c` matches era `-O2 -G0`, VRAM `0x80018CB8` / file
`0x94B8` / size `0x38`. It forwards three dereferenced reader values to
`func_80065A60` and returns 1. Evidence:
`docs/evidence/func-80018cb8/REPORT.md`.

## func_80018C88 — two-reader call wrapper matching C (12 words)

`src/func_80018C88.c` matches era `-O2 -G0`, VRAM `0x80018C88` / file
`0x9488` / size `0x30`. It forwards the two dereferenced reader values to
`func_800659F8` and returns 1. Evidence:
`docs/evidence/func-80018c88/REPORT.md`.

## func_80018C58 — two-reader call wrapper matching C (12 words)

`src/func_80018C58.c` matches era `-O2 -G0`, VRAM `0x80018C58` / file
`0x9458` / size `0x30`. It forwards the two dereferenced reader values to
`func_800659C8` and returns 1. Evidence:
`docs/evidence/func-80018c58/REPORT.md`.

## func_80018BEC — D_8009D2F0 flag-0x20 setter matching C (9 words)

`src/func_80018BEC.c` matches era `-O2 -G0`, VRAM
`0x80018BEC` / file `0x93EC` / size `0x24`. Evidence:
`docs/evidence/func-80018bec/REPORT.md`.

## func_80018BC8 — D_8009D2F0 flag-0x20 clearer matching C (9 words)

`src/func_80018BC8.c` matches era `-O2 -G0`, VRAM
`0x80018BC8` / file `0x93C8` / size `0x24`. It clears bit `0x20` in offset
`0x98` of `D_8009D2F0` and returns 1. Evidence:
`docs/evidence/func-80018bc8/REPORT.md`.

## func_80018B68 — two-reader call wrapper twin matching C (12 words)

`src/func_80018B68.c` matches era `-O2 -G0`, VRAM
`0x80018B68` / file `0x9368` / size `0x30`. It forwards two nested reader
values to `func_8006590C` then returns 1. Evidence:
`docs/evidence/func-80018b68/REPORT.md`.

## func_80018B00 — two-reader call wrapper matching C (12 words)

`src/func_80018B00.c` matches era `-O2 -G0`, VRAM `0x80018B00` / file
`0x9300` / size `0x30`. It forwards two nested reader values to
`func_80067678` then returns 1. Evidence: `docs/evidence/func-80018b00/REPORT.md`.

## func_800182E0 — D_8009D2F0 offset-0x20 reader commit matching C (8 words)

`src/func_800182E0.c` matches era `-O2 -G0`, VRAM `0x800182E0` / file
`0x8AE0` / size `0x20`. It stores the nested reader value at offset `0x20` of
`D_8009D2F0` and returns 1. Evidence: `docs/evidence/func-800182e0/REPORT.md`.

## func_800182A0 — D_800BCF88 bits-0xC0 clearer matching C (8 words)

`src/func_800182A0.c` matches era `-O2 -G0`, VRAM `0x800182A0` / file
`0x8AA0` / size `0x20`. It clears `0xC0` from `D_800BCF88` and returns 1.
Evidence: `docs/evidence/func-800182a0/REPORT.md`.

## func_800182C0 — D_800BCF88 bits-0xC0 setter matching C (8 words)

`src/func_800182C0.c` matches era `-O2 -G0`, VRAM
`0x800182C0` / file `0x8AC0` / size `0x20`. It ORs `0xC0` into
`D_800BCF88` and returns 1. Evidence: `docs/evidence/func-800182c0/REPORT.md`.

## func_80017FB0 — D_8009D1A0 dynamic bit clearer matching C (11 words)

`src/func_80017FB0.c` matches era `-O2 -G0`, VRAM `0x80017FB0` / file
`0x87B0` / size `0x2C`. It clears a nested reader mask from `D_8009D1A0` and
returns 1. Evidence: `docs/evidence/func-80017fb0/REPORT.md`.

## func_80017F88 — D_8009D1A0 dynamic bit setter matching C (10 words)

`src/func_80017F88.c` matches era `-O2 -G0`, VRAM `0x80017F88` / file
`0x8788` / size `0x28`. It ORs a nested reader mask into `D_8009D1A0` and
returns 1. Evidence: `docs/evidence/func-80017f88/REPORT.md`.

## func_80017F20 — D_8009D2F0 flag clearer matching C (9 words)

`src/func_80017F20.c` matches era `-O2 -G0`, VRAM `0x80017F20` / file
`0x8720` / size `0x24`. It clears bit `0x100` in the offset-`0x98` field.
Evidence: `docs/evidence/func-80017f20/REPORT.md`.

## func_80017EFC — D_8009D2F0 flag setter matching C (9 words)

`src/func_80017EFC.c` matches era `-O2 -G0`, VRAM `0x80017EFC` / file
`0x86FC` / size `0x24`. It sets bit `0x100` in the offset-`0x98` field and
returns 1. Evidence: `docs/evidence/func-80017efc/REPORT.md`.

## func_80017EA4 — reader result commit matching C (8 words)

`src/func_80017EA4.c` matches era `-O2 -G0`, VRAM `0x80017EA4` / file
`0x86A4` / size `0x20`. It commits the nested reader value to offset `0x1C`
of `D_8009D2F0` and returns 1. Evidence: `docs/evidence/func-80017ea4/REPORT.md`.

## func_8005184C — dynamic bit setter matching C (8 words)

`src/func_8005184C.c` matches era `-O2 -G0`, VRAM `0x8005184C` / file
`0x4204C` / size `0x20`. The `$v0` pointer and `$v1` mask pins reproduce
retail's address/mask allocation and store delay slot. Evidence:
`docs/evidence/func-8005184c/REPORT.md`.

## func_80033A2C — D_8009D244 byte-flag setter matching C (5 words)

`src/func_80033A2C.c` matches era `-O2 -G0`, VRAM `0x80033A2C` / file
`0x2422C` / size `0x14`; `2422C.s` now resumes at `24240.s`. Evidence:
`docs/evidence/func-80033a2c/REPORT.md`.

## func_80037140 — packet setup/submit twin matching C (25 words)

`src/func_80037140.c` matches era `-O2 -G0`, VRAM `0x80037140` / file
`0x27940` / size `0x64`. It is the 370DC wrapper twin with the `77C44` packet
configuration call; its full span is now C. Evidence:
`docs/evidence/func-80037140/REPORT.md`.

## func_800370DC — packet setup/submit wrapper matching C (25 words)

`src/func_800370DC.c` matches era `-O2 -G0`, VRAM `0x800370DC` / file
`0x278DC` / size `0x64`. It initializes a packet, configures the `+8` member,
submits it, and reports `-1` on error. `278BC.s` resumes at `27940.s`.
Evidence: `docs/evidence/func-800370dc/REPORT.md`.

## func_800370A8 — fixed-point quotient helper matching C (5 words)

`src/func_800370A8.c` matches era `-O2 -G0`: `sra; div; mflo; jr; sll`.
VRAM `0x800370A8` / file `0x278A8` / size `0x14`. `26C48.s` now resumes at
`278BC.s`; the full Docker rebuild is the exact target SHA-1 with 236 leaves.
Evidence: `docs/evidence/func-800370a8/REPORT.md`.

## func_800124F8 — boot-table clear leaf matching C (31 words)

`src/func_800124F8.c` matches byte-exact on era `-O2 -G8`. VRAM
`0x800124F8` / file `0x2CF8` / size `0x7C`. The leaf clears the
`D_8009D310` 72×11 work table, the `D_8009DF70` 16-word table, and the
gp-relative state fields using the retail pointer and delay-slot loop shape.
The former `2A0C.s` chunk is split at the leaf and resumes at `2D74.s`.
`scripts/build_us.sh` and `scripts/verify_us.sh` report **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with 235 leaves. Evidence:
`docs/evidence/func-800124f8/REPORT.md`.

## func_800305C8 — angle-wrap helper matching C (30 words)

`src/func_800305C8.c` matches byte-exact on era `-O2 -G0`. VRAM
`0x800305C8` / file `0x20DC8` / size `0x78`. The non-leaf uses a 0x18-byte
frame, preserves the second record pointer in `$s0`, calls `func_80079FB4`,
then performs the signed-i16 truncation and `+0xFFF` negative wrap shown by
the retail branch. `$v0`/`$v1` register pins preserve the exact allocation.
`scripts/build_us.sh` and `scripts/verify_us.sh` report **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with 234 leaves. The split resumes
at `20E40.s` for `func_80030640`. Evidence:
`docs/evidence/func-800305c8/REPORT.md`.

## func_80030584 — angle helper matching C (17 words)

`src/func_80030584.c` matches byte-exact on era `-O2 -G0`. VRAM
`0x80030584` / file `0x20D84` / size `0x44`. ratan2 of two `lh<<16`
vs `a1[0]`/`a1[2]`, then `+2048` as i16. Head of former `20D84.s`;
resume `20DC8.s` `0x78`. `scripts/build_us.sh` **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Evidence: `docs/evidence/func-80030584/REPORT.md`.

## func_8002F7D8 — 0x6F body create matching C (102 words)

`src/func_8002F7D8.c` matches byte-exact on era `-O2 -G0` +
`MASPSX_THREE_WORD_SYMBOL_STORE=1`. VRAM `0x8002F7D8` / file `0x1FFD8` /
size `0x198`. Both 216-byte copies are gcc aligned `Body216` block
moves: four `lw` `$v0/$v1/$a0/$a1`, four `sw`, `addiu` 0x10 in the
`bne` delay, 2-word tail. First-cut `dst[i]=src[i]` unrolls miss that
shape. Tail of 11718: prefix `0xE8C0`, C `0x198`, then existing 2F970.
`scripts/build_us.sh` **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Evidence: `docs/evidence/func-8002F7D8/REPORT.md`.

## func_80030534 — 2D distance helper matching C (20 words)

`src/func_80030534.c` matches byte-exact on era `-O2 -G0` + maspsx
`--aspsx-version=2.30`. VRAM `0x80030534` / file `0x20D34` / size `0x50`.
The leftover nop sits between the second `subu` and `mult` because
ASPSX ≥ 2.30 requires two instructions between `mflo` and the next
`mult` (`nop_mflo_mfhi`). 2.21 omits it. Mid-20210 carve: prefix `0xB24`,
C `0x50`, resume `20D84.s` `0xBC`. `scripts/build_us.sh` **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Evidence: `docs/evidence/func-80030534/REPORT.md`.

## func_80030640 — RNG gate matching C (40 words)

`src/func_80030640.c` matches byte-exact on era `-O2 -G0`. VRAM
`0x80030640` / file `0x20E40` / size `0xA0`. Mid-20210 carve: prefix
`0xC30`, C `0xA0`, resume `20EE0.s`. `lui $v1,1` is bit 16 (`0x10000`),
not `andi 1`. Second `D_8009D278` load is `$v1` because signed `%100`
clobbers `$a0`. `scripts/build_us.sh` **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Evidence: `docs/evidence/func-80030640/REPORT.md`.

## Current state

| Fact | Value | Derive |
| --- | --- | --- |
| Branch / tip | `phase5fm-main-barrier-revisit` @ 5FU-17EA4 | `git branch --show-current` / `git status --short` |
| Phase | **5FU-17EA4 / 241 exact leaves** (six nearby exact leaves added after 5FM; parked candidates remain untouched) | `scripts/verify_us.sh` summary + exact rebuild |
| Matching C leaves | **241** (non-integrated candidates: parked src/func_800698D4.c / func_8001220C.c / func_800725DC.c + IN-PROGRESS src/func_8006A9E4.c) | `grep -c ',\s*c,' configs/USA/disc1.yaml` |
| Yaml asm segments | **153** | `grep -c ',\s*asm\]' configs/USA/disc1.yaml` |
| Era leaf compiles | **83** | `grep -c '^era_compile \|^\w*=1 era_compile ' scripts/build_us.sh` |
| Target SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` | `scripts/build_us.sh` compare |
| Progress | https://blizz127.github.io/parasite-eve-progress/ | `scripts/publish_progress.sh` |

**Yaml `asm` segments are not remaining functions.** One segment can hold
dozens of glabels; do not subtract it from anything as a function count.

Oracle: bare `scripts/build_us.sh` exits 0 on exact SHA-1; `scripts/verify_us.sh`
reports Phase 5FU-17EA4 / 241. Disc images / `asm/` / `build/` / `tools/era/`
are git-ignored inputs — never commit them.

**Toolchain**

- Default leaves: GCC 14.2 in Distrobox `pe-mipsel` (Phase 4J flags; selective
  `-G 8` / `-fno-delayed-branch` / `-fno-tree-ter`).
- Era leaves (opt-in): `scripts/setup_era.sh` → `era_compile` =
  cpp → cc1 → maspsx → GNU as, typically `-O2 -G0` (some leaves `-O1 -G0`).
- Era maspsx: `ERA_ASPSX_VER=2.21` + `--dont-expand-li`. **Why:**
  `expand_load_immediate` turns positive small `li` into `ori`; ROM wants
  `addiu`. Defer `li` expansion to GNU as. Same config also preserves
  large-literal `lui;ori` (cc1 emits PSY-Q `li` high + `ori` low natively).
  Do **not** bump aspsx-version casually — that also flips `nop_at_expansion`
  / `addiu_at`.
- **Vendored maspsx LOCAL PATCH:** `tools/era/maspsx/maspsx/__init__.py` is
  repo-tracked (`.gitignore` negations; `setup_era.sh` re-clones upstream
  AROUND it, restores the tracked file from git if absent). Patch 1 =
  **sw-store delay-slot fill**, opt-in per `era_compile` line via env
  `MASPSX_FILL_STORE_DELAY_SLOT=1`: an absolute `sw $r,SYM` macro immediately
  before a bare `j $31` is emitted as `lui $at,%hi` / `j $31` /
  `sw $r,%lo($at)`. sw only — sb/sh macro stores and multi-store epilogues
  are ROM-proven to stay pre-jr with a nop slot (e.g. func_8003FFAC vs
  func_8007FBC0: identical C shape, different ROM scheduling — the original
  units were assembled with different ASPSX scheduling).
- Patch 2 (`f0b9155`) / Patch 3 (`439c244`): **three-word indexed symbolic
  store AND load expansion**, opt-in per leaf via `MASPSX_THREE_WORD_SYMBOL_STORE=1`.
  Standalone `op $r,SYMBOL($index)` uses the retail/ASPSX-2.30-shaped
  `lui $at,%hi` / `addu $at,$at,$index` / `op $r,%lo($at)` sequence,
  for stores (2) and standalone indexed symbolic loads (3: lb/lbu/lh/lhu/lw/lwl/lwr;
  `lwc2` stays outside, durable negative test). Compound semicolon lines retain the
  2.21 four-word expansion. Flag-off rebuild is the exact leaf-count retail SHA.
  Full 224-leaf regression (flag OFF and flag ON over the three existing 3W store
  leaves) both exact; 153 vendored tests; re-clone restore byte-identical.
- Maspsx stdin: closed with `</dev/null` in `era_compile` (non-TTY hang under
  agent sockets). Bare `scripts/build_us.sh` is fine.

## How to count (do not hand-maintain)

```bash
grep -c ',\s*c,' configs/USA/disc1.yaml            # C leaves
grep -c ',\s*asm\]' configs/USA/disc1.yaml          # yaml asm segments (NOT fn count)
grep -c '^era_compile \|^\w*=1 era_compile ' scripts/build_us.sh  # era leaf compiles
git log --oneline -1
```

**Do not** count `asm/disc1/*.s` from disk. That tree is git-ignored and
contains orphans, stale duplicates, and nop-pads. **Yaml is the source of truth.**
Known stale orphans (counter ignore-list): `2E7D0.s` (superseded by live
`2E7D8.s`) and `807C.s` (stale duplicate of live `2A0C.s`; unreferenced in
yaml). Keep both out of function scans.

**asm/ sync invariant:** `$at` family totals from
`tools/analysis/at_absolute_store_counter.py` hard-fail (no SUMMARY) when asm/
is missing units or still holds glabels for yaml C leaves. Re-split with
`scripts/split_us.sh` before planning off a family count. Leaf count stays
yaml-only and still works when asm/ is stale.

## Proven era fingerprints (evidence, not claims)

| Fingerprint | Status |
| --- | --- |
| `move` → `addu` in delay slot | Proven 5EA / 5EB / 5EC / 5ED |
| `$v0` / `$v1` allocation | Proven 5EC / 5ED (sb+ret0 reuse) |
| `li` const materialization (`addiu` not `ori`) | Proven 5EC via `--dont-expand-li` |
| `$at` absolute `sw` macro expansion | Proven by scratch probe; integrated exact in 5EE |
| Branch delay-slot constant hoist (`beqz` slot) | **PROVEN** (5EG-first-branch): era cc1 `-O1 -G 8` reproduces the retail schedule on `func_8004F448` word-for-word |
| Test-and-clear-return if/else (`bnez` + j-over) | **PROVEN, VOLUME** (5ER): era `-O2 -G0` matches the adjacent byte/word twins `func_80038D1C` / `func_80038D48` — shared address in `$v1`, `addu $v0,$zero,$zero` in the `bnez` slot, `addiu $v0,$zero,0xFF` in the unconditional-jump slot, then `sb`/`sw` clear. Direct-global C rebuilt the address and used a 12-word `beq` form; one natural explicit-pointer phrasing retry matched all 11 words without pinning |
| `$a0`-in/`$v0`-out + redundant double store | **PROVEN** (5EH): era `-O2 -G8` preserves both stores + `addu` return-0 on `func_800438C0`; GCC 14.2 `-O1` merges stores and emits `move` — **era required for value-returning leaves**; era+gp `-G8` first proven here |
| Non-leaf stack frame + `jal` | **PROVEN** (5EI; repeated as volume in 5EK): era matches the `func_800197D0` / `func_800197F0` void-callee twins — `addiu $sp,-0x18` / `sw $ra,0x10($sp)` / `jal`+nop / `lw $ra` / `addiu $v0,1` / `jr $ra` with the `addiu $sp,+0x18` teardown **in the `jr` delay slot**, word-exact; 197F0 uses `-O2 -G0` and adds no primitive |
| Outgoing `$a0` + `jal` after double dereference | **PROVEN** (5EJ-outgoing-arg): era `-O2 -G0` on `func_80019484(int **)` emits `lw $v0,0($a0)` / load-delay nop / `lw $a0,0($v0)` / `jal func_800438C0` + nop, then the proven return-1 frame teardown shape; all 11 words exact |
| Return-forwarded `$v0` + teardown-before-`jr` epilogue | **PROVEN** (5EL-return-forwarding): era `-O2 -G0` on `func_8007F7A8` emits the frame + `jal func_8007FCAC` + nop, forwards `$v0` untouched, then `lw $ra`; `addiu $sp,+0x18`; `jr $ra`; nop. Era reproduces this per-function schedule as well as 197D0/F0's opposite teardown-in-slot schedule |
| Straight-line boot pointer-layout scheduling | **PROVEN, COMPILER-CONSTRAINED C** (5EM-boot-6a8d4): era `-O2 -G0` matches all 68 words / 19 absolute pointer stores in retail order. Both the initial plain-local source and one retail-order retry allocate cursors to `$a0/$a1`, constants to `$v0/$v1`, and sink `D_800B0E28` past `D_800B0E2C/E30`. The exact fallback therefore uses the established explicit-register convention (`$v0/$v1` cursors, `$a0/$a1` constants); it is target-specific matching C, not portable natural C |
| Empty-asm scheduling barrier for materialization placement | **PROVEN (5FJ `func_8006E9A0`)**: era `-O2`'s pre-RA scheduler sank a callee-saved `$s2 = &SYM` lui/addiu pair past a `jal` (source position = retail words 29-30; cc1 put it after the first post-arena call); `asm volatile("" : : "r"(reg) : "memory")` right after the assignment pins the pair to retail position, emits no code, and the leaf matches 141/141. `-fno-schedule-insns` is NOT the lever (double materialization, breaks arena scheduling) |
| Paired register pins + `"=r":"0"` zero-code barrier for call-result home | **PROVEN (5FK `func_8006E834`)**: retail keeps a `$v1` backup of a call result across a range test and restores it to `$v0` for the equality tests; natural C makes cc1 coalesce the tests onto `$v1` and drop both restore copies (90/91). Pinning `register int t asm("$3")` (backup) and `register int rt asm("$2")` (test home) plus empty `asm volatile("" : "=r"(x) : "0"(x))` barriers (emit nothing; block copy-prop folding) restores the retail shape, and reorg threads the idempotent merge copy into the `beqz` delay slot — 91/91. Single-register pins alone (V1/V3) are inert; pinning only `$v0` without the decoupling barrier leaves the tests on `$v1` |
| Counting-loop back-edge scheduling | **PROVEN; VOLUME-ELIGIBLE** (5EN/5EP `func_8006A674` probe): era `-O2 -G0` puts pointer advances in all five retail back-branch delay slots — `bnez` up-counters (`$a0+4`, `$v1+2`, `$a1+8`) and `bgez` down-counters (`$a3-4`, `$v0-4`) — and preserves the final store in the `jr` delay slot. The leaf remains parked for unrelated constant-hoist scheduling; the loop primitive passed. |
| Natural counting loop in volume | **PROVEN, VOLUME** (5ES `func_8004BF08`): era `-O2 -G0` matches a natural pointer-walk loop over parallel signed `int[8]` arrays in all 14 words, with no pins or maspsx opt-in. Explicit initialization in retail order (`i`, first pointer, second pointer) plus `do/while` phrasing gives `$a1/$a0/$v1` allocation; the first pointer advances before the bound test and the second pointer advances in the backward `bnez` delay slot. The declaration-initialized `for` form was semantically correct but allocated the three live values differently. |
| Pure-register bit-serial loop in volume | **PROVEN, VOLUME** (5ET `func_8005186C`): era `-O2 -G0` matches all 15 words on the first natural-C try — no loads/stores, calls, or `$gp`; explicit-init `do/while`; the unconditional `result <<= 1` fills the forward `bnez` skip-branch delay slot, the `bgez` back-edge keeps a nop slot, and the return lands as `addu $v0,$a1,$zero` in the `jr` delay slot |
| Indexed global-array store/load expansion | **PROVEN, TOOL-SOLVED** (`f0b9155` stores; `439c244` loads): per-leaf `MASPSX_THREE_WORD_SYMBOL_STORE=1` reproduces `lui` / indexed `addu` / op `%lo` and removed the extra L3 word in `func_8006A674` (153→152 words). `439c244` extends the gate to standalone indexed symbolic LOADS (all seven widths; `lwc2` stays outside — durable negative test; compound lines retain the 4-word expansion). Default off is byte-identical. |
| `lui;ori` large-literal synthesis | **PROVEN** (capability probe): both bit15-clear and bit15-set; cc1 emits PSY-Q `li` high + `ori` low; ROM-exact under 2.21 + `--dont-expand-li` |
| Rotated/peeled loop idiom | **PROVEN SHAPE** (5EV `func_80052BCC`, leaf parked on unrelated allocation): write the first iteration explicitly, then `while (cond) { body }` → era `-O2 -G0` emits the rotated shape: `beq`-exit head, bottom-tested `bne` back-edge, pointer advance in both delay slots |
| Signed `char` vs 0xFF-range constant | **PROVEN SHAPE** (5EV `func_80052BCC`, same parked leaf): signed `char c` compared against `0xFF` emits the conversion `andi` on the compare path even after `lbu`; `unsigned char` does not. Typing controls the mask |
| Return-accumulator vs direct-return phrasing | **PROVEN (5FH `func_80037548`)**: a search loop with a default return value must hold the result in an ACCUMULATOR (`signed char result = 0; ... result = v; break; return result;`). Direct `return v;` on the match path makes cc1 emit a SEPARATE `addu $v0,$zero,$zero` default path before `jr` (28 words vs ROM's 27) — the accumulator keeps one `$a2` merge with the `sll/sra` sign-extension pair hoisted to the merged exit |
| `-fschedule-insns2` load-delay `li` hoist | **PROVEN, FIRST LEAF** (5EW `func_80052BCC`, era `-O1 -G0 -fschedule-insns2`): the post-allocation scheduler hoists an independent `li` above `sb`/`andi` into the `lbu` delay — the exact spot retail's ccpsx scheduled it. At plain `-O1` the same `li` emits after the `andi` (14/15). Paired phrasing: two `0xFF` consts of different modes (u8 head const dies at the guard → loop re-materializes into the freed `$v1`; `int` loop byte → mask-free raw `bne`); comparing the loop byte against a *variable* or both consts sharing a mode cross-jumps/CSE-shares head and loop |
| sched2 scope (negative result) | **NARROWED (5EY `func_8003E610`)**: `-fschedule-insns2` is NOT a universal retail fingerprint — it governs **store-adjacent `li`/`addiu` placement and load-delay hoists** only (52BCC head-`li`, 6A674's 21 order swaps). Straight-line `jal`-arg scheduling (`$a0` hoisted + `$a1` in slot for two-arg calls; `$a0` slot-filled single-arg; nop slot no-arg) is already correct at plain `-O2`. Do NOT flip sched2 into the era default |
| dbr_sched `$v0`-steal screening rule | **CHARACTERIZED (5FB `func_800698D4`, PARKED)**: a `beqz`/`beq` whose delay-slot steal candidate is a `$v0`-setter gets the fill when the branch target hits a `jal` immediately (kills `$v0`), but retail DECLINES the steal when the target is the return-computation block (`$v0` live to `jr $ra`) — our cc1 steals anyway. Screening rule: nop slot + `$v0`-constant load on fall-through + branch to a RETURN block → expect divergence; same pattern to a `jal`-adjacent block → matches. reorg.c liveness skew (ccpsx vs 2.7.2-psx), not source-expressible |
| Nested-if defeats range-test collapse | **PROVEN IDIOM (5FB `func_800698D4`)**: `v != 0 && v != -1` folds to `addiu $v0,$v0,1; sltiu $v0,$v0,2; bnez` under era `-O2` (range test, not retail's shape). Two nested `if`s keep the separate `beqz`/`beq` compares. -O1 keeps compares but flattens other structure |
| Frame-size arithmetic for struct locals | **PROVEN (5FB `func_800698D4`)**: size opaque locals from the frame, not the type's rounded size — DsSearchFile's CdlFILE local is `0x18` (pos 4 + size 4 + name 16): `0x10` args + `0x18` local + `$s0` + `$ra` = frame `0x30`. A `0x20` local emits frame `0x38` and fails at word 0 |
| Five-arg call (o32 stack arg) | **PROVEN, FIRST LEAF** (5FC `func_8006E834`; leaf integrated exact in 5FK): the 5th argument emits `sw $v0,0x10($sp)` in the `jal`'s delay slot — plain C `f(a,b,c,d,e)` with an immediate 5th arg, era `-O2 -G0`, worked first try. `sb $v0,0x29($sp)` (struct byte field) also lands in a `jal` slot |
| Frame decomposition before writing | **PROVEN METHOD (5FB/5FC)**: decompose the frame BEFORE choosing local sizes — `args + locals + saves + pad = frame` must be exact (5FB: CdlFILE `0x18` not `0x20`; 5FC: args `0x18` + env `0x18` + local30 `0x8` + regs `0xC` + pad `0x4` = `0x48`, byte field lands at `env[0x11]` = `0x29($sp)`). Wrong local size fails at word 0 |
| Aggregate element type as addressing-mode lever | **PROVEN (5FD `func_8002F9CC`)**: for an indexed store into a symbol array, declaring the real aggregate element (`SlotRecord D_800A5D58[]`, `arr[i].field = 0`) makes cc1 emit the standalone indexed symbolic store (`sw $0,SYM($3)`) at plain `-O2` — flat `arr[i*55] = 0` instead hoists `la $5,SYM` out of the loop (invariant under `-O2`/`-O1`/`-O1 -fschedule-insns2`; an addressing choice, not scheduling). With the symbol store present, `MASPSX_THREE_WORD_SYMBOL_STORE=1` passes it to GNU as for retail's 3-word `lui $at / addu / sw %lo($at)` form. Also: a lone symbol materialization is NOT an `-O1` signal — the `-O1` lever is for *repeated* constant/address materialization |
| `-O1` per-use constant materialization — SELECTION RULE | **PREDICTIVE (three leaves)**: if ROM materializes the same constant/address more than once, try `-O1` FIRST. `-O2`'s shared hoist runs through the hardwired `optimize>1` path (not flag-reachable); `-O1` re-materializes per use. 6A674 (discovered: per-use `-1`), 6A5BC (applied: `$s0=1` twice), 3E680 (predicted from five per-store `lui`s with a shared `0x8009` high half retail didn't CSE) |
| Return-use readiness of asm callees | **VALIDATED (5EZ `func_8006A5BC`)**: a caller may USE a still-asm callee's return and stay matchable when the use is a **raw full-width compare** (`beq $v0,$s0`, no mask/sign-extend) or a **bare store** (`sh $v0`). Both are codegen-determined regardless of the callee's true return type, so `int f(void)` externs suffice. Extends the 5EY rule (immediates-only args, returns ignored) |
| Fn-ptr arg to still-asm callee | **PROVEN, FIRST LEAF** (5FA `func_8003E680`): `f(func_8003E91C)` emits `lui $a0,%hi(sym)` / `addiu $a0,$a0,%lo(sym)` with R_MIPS_HI16/LO16 relocs against a same-segment TEXT symbol; the linker resolves it exactly like a data symbol. Declare `extern void g(void);` and pass the bare name |
| Unsigned loop-bound compare | **PROVEN (5FA `func_8003E680`)**: ROM `sltiu` (unsigned) vs cc1's `slt` for `int i < const` — declare the counter `unsigned int`. One-word type-driven fix, no flag involvement |
| Two-word `lui/addiu` zero from C | **PROVEN (5FN `func_800725DC` probe)**: no C zero spelling emits `lui/addiu` (ten forms probed, all `move`; a constant-0 loop bound deletes the whole loop at `-O2` AND `-O1`). The address expression `(int)SYM - BASE` compiles to `la $r,SYM+(0-BASE)` → `R_MIPS_HI16/LO16` with addend; when `SYM==BASE` the final words are `lui 0x0000 / addiu 0x0000` and the loop body STAYS ALIVE. The only known source-expressible origin for a baked two-word zero |
| No-args-area frame via asm call | **PROVEN (5FN `func_800725DC` probe)**: era cc1 reserves the 16-byte o32 outgoing-args area for EVERY C call form (direct/indexed-pointer/pinned-pointer all `args=16`). An inline-asm `jalr` (counter decrement as tied `"=r"/"0"` operand in the delay slot) is not a CALL insn → `.frame args=0`, frame = saves only. Diagnostic for retail frames smaller than saves+16 |
| Era prologue save order is fixed descending | **CHARACTERIZED (5FN)**: multi-`$s` prologue saves emit `$ra`-first / offsets top-down under `-O2`, `-O1`, `-fno-schedule-insns`, `-fschedule-insns2`, `-G8` alike — flag-invariant. PE1's matched/asm units are all descending or slot-interleaved; only the 0x800725xx SDK-runtime unit is contiguous-ascending (per-TU toolchain skew; see `per-tu-725dc`) |

All four fingerprints from the original 5EA era claim are now proven in bytes.
The “~290 era-blocked functions” figure remains an **ESTIMATE**, not a countdown.

## Known-open families

- **sb+ret0:** **done** in 5ED (family closed).
- **`$at` absolute-store population:** counter committed
  (`tools/analysis/at_absolute_store_counter.py`). The historical integration
  inventory was **18 pre-jr** / 14 delay-slot / 5 sb-sh; the current yaml-live
  population is **0 pre-jr** / **0 delay-slot** / 5 sb-sh. Weak-int policy **NO**.
  - **Pinned by 5EG-readers:**
    - `D_8009D240` = `unsigned short *`, `D_8009D260` = `unsigned char *`
      via `func_8008AB1C` (era `-O1 -G0`).
    - `D_800A1870` = `void (*)(void)` via `func_80042B6C` (era `-O2 -G0`).
  - **Integrated:** `func_80085728`; 5EI readers-typed trio; 5EJ `D_8009D28C`
    int-state (4); 5EK `D_8009D270` unsigned flags (2); **5EF all 14
    delay-slot `sw` members**. The pilot `func_8007FBC0` plus the remaining 13
    typed leaves are integrated exact. Current leaf count **217**.
  - **Delay-slot shape: FAMILY CLOSED (5EF).** Vendored maspsx LOCAL PATCH
    (`MASPSX_FILL_STORE_DELAY_SLOT=1`) fills the `j $31` slot with the trailing
    absolute `sw`. Pilot gate exact + objdump-probed (`3C01800A 03E00008
    AC2436A0`). The remaining 13 members now have per-global typing evidence,
    and all 14 members pass the full exact-match gate; see
    `docs/ai_context/PHASE5EF_TYPING.md`.
  - **sb-sh-five: RECLASSIFIED — never tool-blocked.** ROM words show sb/sh
    macro stores stay **pre-jr with a nop slot** (func_80033A2C sb,
    func_800C6ED8/C6EE8 sh, func_800C6EC0 dual-sh; func_8001A374 has a
    cc1-filled `li` slot). Current maspsx already emits that shape; the patch
    deliberately does not touch sb/sh. Remaining work is typing + integration,
    toolchain-independent.
  - **Still open (typing):** remaining opaque-word (`D_800A1868` other writers).
- **`lui;ori`:** **CAPABILITY-VERIFIED** — not a blocker. Constant-heavy
  computational functions (mult/div/mask, e.g. ÷100 via `0x51EB851F`) are
  approachable as a **separate future phase**; synthesis itself is solved.
- **gp arena loop `func_80055724`:** **PARKED-SCHEDULING** (branch
  `phase5eu-gp-loop-55724`; closest candidate stashed as `park phase5eu
  func_80055724 while-form 13-15`). Empty 8-byte frame **solved** (cc1 2.7.2
  `vars=8` home slots, natural). Blocker: three-way scheduling tension —
  while-form keeps frame+regs but hoists the cursor load above the `blez`
  guard (13/15); if+for keeps frame+regs but duplicates the guard and steals
  the prologue into its slot; if+do/while gets word order but `vars=0` and
  flipped regs. era `-O1 -G8` output is **byte-identical** to `-O2` for both
  leading phrasings — no per-function `-O` support from this leaf. Residual is
  scheduling, not proven allocation. Detail: `docs/ai_context/parked_blockers.json`.
- **disc mount `func_800698D4`:** **PARKED-SCHEDULING** (branch
  `phase5fb-boot-698d4`; closest candidate stashed as `park phase5fb
  func_800698D4 nested-ifs 140-141 (search3 beqz-slot residual)`). Disc
  identification/mount — clears the mount flag, verifies drive ready, searches
  for `\FMV1\PEDISC01.IDF;1` / `\PE.IMG;1` / `\FMV2\PEDISC02.IDF;1` via
  `DsSearchFile`, records via `func_80080C48` → `D_800B0DD8` + `D_800B0DCD`
  flag bits. 140/141 words; everything exact except ONE delay-slot steal:
  search #3's `beqz` (`0x5A24C`) — retail nop, ours steals `addiu $v0,$zero,-1`.
  Mechanism is the dbr_sched `$v0`-liveness screening rule (fingerprint table);
  not source-expressible. Banked idioms: nested-if defeats range-test collapse;
  CdlFILE local is `0x18` not `0x20` (frame arithmetic). Detail:
  `docs/ai_context/parked_blockers.json` (`boot-698d4-dbr-sched`).
- **post-mount loader `func_8006E834`:** **RESOLVED — INTEGRATED (5FK,
  91/91 exact).** The 5FC call-result register-home residual (`$v0`+two
  restores vs `$v1`) WAS source-expressible after all: paired hard-register
  pins ($v1 backup / $v0 test home) + two zero-code `"=r":"0"` barriers
  (fingerprint table). Historical park evidence preserved in
  `docs/ai_context/parked_blockers.json` (`boot-6e834-register-home`,
  status INTEGRATED).
- **flag-clear loop `func_800374E8`:** **PARKED-ALLOCATION, register COLORING**
  (branch `phase5ff-374e8`; candidate stashed as `park phase5ff func_800374E8
  (register-coloring skew; structure correct)`). Flag-clear loop over 4 x 56-byte
  records at `D_800BCEA8` — **RECORD TYPE ESTABLISHED** (durable deliverable;
  propagates to `func_80037548`): +0x00 `unsigned char` (lbu/sb), +0x0C
  `unsigned int` flags (lw/sw; bit 0x02000000 cleared here), +0x10 `signed short`
  (lh/sh); extent closes EXACTLY at +0xE0 = 4 x 56. **STRUCTURE CORRECT**: 5FD
  aggregate-subscript rule (no `rec` pointer) + the landed load gate (`439c244`)
  produce retail's 3-word indexed-symbolic shape (no `la` hoist, correct DAG and
  scheduling). **RESIDUAL — register coloring only**: era cc1 assigns
  mask->`$v0`/chain->`$v1`/value->`$v0`; ROM is mask->`$v1`/chain->`$v0`/value->`$v1`.
  Five phrasings x two loop forms x ladder rungs are ALL byte-identical —
  invariant under phrasing. Same class as 6E834's call-result home:
  hard-register-assignment skew. This leaf MOTIVATED the maspsx load-gate patch.
  Detail: `docs/ai_context/parked_blockers.json` (`register-coloring-374e8`).
  **TWIN FALSIFIED (5FH)**: `func_80037548` was probed and MATCHES 27/27
  (accumulator shape) — no coloring skew. Refined rule: coloring skew is
  LIVE-VALUE-PRESSURE DEPENDENT (374E8: mask+chain+value all live;
  37548: needle/accumulator/index in $a0/$a2/$a1 leave $v0/$v1 free), not
  per-table. Predict skew only when 3+ scratch values compete.
- **sentinel walk `func_80062CE4`:** **PARKED-SCHEDULING, loop-LAYOUT**
  (do/while form stashed as `park func_80062CE4 (loop-layout scheduling;
  do/while lever proven source-invariant)`). Sentinel validate-and-consume
  over the D_8009D154 list: if D_8009D160 (pending) is still linked, promote
  it to D_8009D15C (confirmed); clear D_8009D160 either way. 12/18; PROVEN
  source-invariant — both while-form and do/while produce BYTE-IDENTICAL
  output; ROM has sentinel-at-top->advance->null-back-edge. cc1
  canonicalizes loop body order before block layout. SIXTH skew instance.
  CARVE CORRECTION: spimdisasm 0x5C label OVERSHOOTS — active span 0x48;
  trailing 5 words are func_80062Fxx prologue. Postmortem: dual gp-four
  filters could not catch loop-layout skew (no pre-compile tell known).
  Detail: `docs/ai_context/parked_blockers.json` (`loop-layout-62ce4`).

- **SDK-runtime runner `func_800725DC` + twin `func_8007264C`:** **PARKED-PER-TU-TOOLCHAIN**
  (candidate preserved at `src/func_800725DC.c`, PARKED, semantically complete;
  detail: `docs/ai_context/parked_blockers.json` `per-tu-725dc`). One-shot
  callback runner over the empty fn-ptr table at `jtbl_80010000` (main's
  first callee). **THREE BANKED LEVERS** (durable, probe-verified): (1) the
  retail two-word zero count (`lui/addiu` of 0) is unreachable from any C
  zero spelling — the address expression `(int)jtbl_80010000 - 0x80010000`
  emits `la SYM+0x7FFF0000` whose hi/lo relocs resolve to 0000/0000 and KEEP
  the loop body alive at -O2; (2) an inline-asm `jalr` call (decrement as
  tied operand in the slot) yields `.frame args=0` — the no-args-area frame;
  (3) the loop body + flag load/store shape is word-exact on era `-O2 -G0`
  with pins. **RESIDUAL — three coupled per-TU mechanisms, flag-invariant**
  (-O2/-O1/-fno-schedule-insns/-fschedule-insns2/-G8 ladder): ascending
  contiguous prologue saves (era's base order is fixed descending; PE1's
  other units are descending/slot-interleaved), `li→ori` expansion (retail
  slot constant is `ori`, era pipeline yields `addiu`), and the
  no-args-area frame model. **Third per-TU datapoint** — the 0x800725xx
  SDK-runtime unit was built with a different ccpsx/aspsx configuration.

- **boot-read `func_8006A9E4` (215w):** **IN PROGRESS (5FO checkpoint d10)** —
  candidate preserved at `src/func_8006A9E4.c` (d10: frame EXACT 0x30,
  all zone/copy structure word-count-exact, **216 words (+1)**; sole
  structural extra = zone-1 gate steal, 698D4-class; NOT integrated).
  Key levers: `__builtin_memcpy(dst,src,16)` = retail's lwl/lwr+swl/swr
  block shape; `called→$17` pin = retail's zone-2 register split;
  do/while = no trip guard; sentinel VARIABLE fixes the zone-1/3 steal
  but ripples zone 2 (next: pin combo). Main's post-init
  boot-read: ClearImage rect setup, FOUR table-driven retry-read zones over
  the u16 boundary pairs at `D_800930DC[0]/[1]/[4]/[5]` (issue `func_8006E6A8`,
  poll `func_8006E7E8` through a goto-gate: `flag=1; poll: if (flag==-1)
  goto restart; flag=poll(); if (flag) goto poll;` — reproduces retail's
  dead-edge outer loop), zone-2 one-shot `func_800527C8`, 0x10A50-byte
  alignment-split copy to `D_800E2858` (unaligned = lwl/lwr blocks), two
  `func_8006E498` decode calls (data[0x144] = SECOND call's return),
  `func_80087090(D_800B0E6C,1)`, 0x1400-byte copy to data[0x130].
  Resume from `src/func_8006A9E4.c` + the REMAINING-DELTAS list in its header.

- **CC1 PROVENANCE INVESTIGATION — COMPLETE (NULL RESULT):** **no closer community build exists.**
  Four-phase read-only investigation (Phases 1–4) into the era toolchain's cc1, the retail PE1 compiler
  (ccpsx), and whether a closer community build is obtainable. Blinded two GCC MIPS-backend mechanisms
  across the 2.7→2.8 version boundary (loop-body layout via `62CE4`, dbr_sched `$v0`-liveness via `698D4`);
  both survived REORGED (the `reorg.c` rewrite in 2.8 produced identical steal-vs-decline decisions).
  The six parks reflect GCC 2.x MIPS-backend ARCHITECTURE DECISIONS, not version-local divergences.
  FORK: (i) cc1 source patch (the maspsx model one layer deeper — the 698D4 liveness check is scoped)
  or (ii) accept the six residuals as structurally-correct-C with one-word compiler-decision deltas.
  Full report: `docs/ai_context/cc1_investigation.md`. Pipeline reconstructible from the report's
  candidate hashes and `git show stash@{N}^3:path` recovery procedure.

- **`main` (`func_8001220C`, 187 words):** **PARKED-SCHEDULING, WITH COMPLETE CANDIDATE**
  (candidate preserved at stash; five drafting iterations on scratch /tmp/mainvN.c).
  The boot keystone: init sequence, 20-call-site mount/read/dispatch loop, volume gate,
  A8-code three-way state switch. ~180 words match at opcode/position. DURABLE DELIVERABLES:
  the 9-word scratchpad stack handoff (sp → 0x1F8003FC, jal 8019234C, restore) is
  BYTE-EXACT as fenced inline asm with full caller-saved clobbers — the fenced-exception
  mechanism (register-pinning precedent) is validated for when main integrates. Role map
  pinned: $s0 data ptr (D_800B0CD8), $s1 dispatch, $s2 flagbyte (+0xF5), $s3 state_val
  (0xA9400048), $s4 bitmask (0x100000). All 20 externs typed (69B08 int, 1909B4→6E9A0
  raw-flow chain). RESIDUAL — ONE mechanism, proven scheduler-driven by an
  init-placement lever test (draft 4 declared bitmask at top, draft 5 moved init after two
  calls; cc1 kept the li at the same position and the $s-save interleaving identical —
  source cannot express the difference): prologue save-batching + invariant-constant
  placement, cc1 ordering pass vs ccpsx. Fifth scheduling-family instance.
  NOTE: with 6E834 resolved in 5FK, main and 698D4 are the TWO remaining
  boot-chain parks — the cc1 archaeology still directly gates boot-to-black.
  Detail: `docs/ai_context/parked_blockers.json` (`main-prologue-scheduling`).

- **ccpsx-vs-2.7.2 SKEW SET — four distinct mechanisms:** (1) the
  allocation/scheduling family (`6A674`/`55724`/`52BCC`; two recovered via
  `-O1`), (2) dbr_sched `$v0`-liveness slot-steal (`698D4`), (3) call-result
  register home (`6E834`) — **RESOLVED 5FK via paired register pins +
  zero-code barriers; NOT unreachable from C**, (4) register coloring /
  pseudo-numbering (`374E8`). (4) remains register-ASSIGNMENT skew; (2)
  remains a liveness-screening skew.
  SIX instances documented (four scheduling, two register-assignment); 6E834
  left the set in 5FK. TWO boot-chain functions remain parked (`main`,
  `698D4`) — the cc1 question still directly gates boot-to-black under plan A.
  Do not chase mid-leaf.
- **PARKED-ALLOCATION/SCHEDULING family:** cc1 2.7 register
  allocation/scheduling decisions that natural C cannot steer and `-O` level
  does not change. **FAMILY INVESTIGATED (read-only, accepted): NO SINGLE
  KNOB.** All residuals are present in cc1's **raw** output, pre-maspsx
  (maspsx does only `move`→`addu`, delay-slot nops, the 2.21 indexed-store
  expansion — no reordering/renaming), so a maspsx patch cannot fix any of
  them; the `addiu_at` template does not apply. Pass attribution
  (flag-probed) and current status:
  - `52BCC`: **MATCHED (5EW, leaf 218)** — the `-O1`→`-O2` flip required
    exactly `-fexpensive-optimizations` + `-fschedule-insns2` (regclass +
    post-alloc scheduler; bisection-proven minimal pair). Retried at era
    `-O1 -G0 -fschedule-insns2`: two-const-mode phrasing (u8 head const dies
    at the guard → loop const re-materializes into `$v1`; `int` loop byte →
    mask-free raw `bne`) + sched2 hoisting the head `li` into the `lbu`
    delay = all 15 words exact. First `-fschedule-insns2` leaf.
  - `55724`: pre-reorg RTL emission order (C statement order); NOT
    `dbr_sched` (`-fno-delayed-branch` doesn't move it), `-O`-invariant.
    Retail *sank* the p-load below the guard; 2.7.2-psx has no pass that
    sinks loads past conditional branches. **No lever** — constrained-C or
    acceptance. (Still parked; see entry above.)
  - `6A674`: **MATCHED (5EX, leaf 219)** — the `-O`-sensitive constant
    materialization runs through a hardwired `optimize>1` path (not
    flag-reachable), so `-O1` is the only lever; at `-O1 -G0` the residual
    shrank 45→21 (all pure `li`/`addiu`-before-store order swaps), and
    `-fschedule-insns2` closed them to **0/152** with the 5EP pins intact.
  - **`-fschedule-insns2` is a GENERAL RETAIL FINGERPRINT** (two independent
    leaves, 22 positions): retail's ccpsx ran post-allocation scheduling;
    our default doesn't. Try sched2 early on future scheduling-position
    residuals. Hypothesis to test later (carefully; current leaves match
    without it): sched2 may belong in the era default flag set.
  Evidence: scratch compiles `/tmp/fam_inv` + `/tmp/o1` (session-recorded).
- Complex `$gp` / GTE / BIOS / mult-div / large non-leaves: still open; not
  inventoried here. Path forward is matching real logic, not harvesting
  trivial setters.

## Boot Rung 1 — COMPLETE, climbing `main`'s call chain

```text
main -> func_8006A5BC ✓ exact C (5EZ, leaf 221)   # boot init, VSync waits
     -> func_8006A64C ✓ exact C -> { func_8006A8D4 ✓ exact C,
                                     func_8006A674 ✓ exact C (5EX, leaf 219) }
     -> func_8003E610 ✓ exact C (5EY, leaf 220)   # display/graphics bring-up
     -> func_8003E680 ✓ exact C (5FA, leaf 222)   # subsystem-init dispatcher
```

- `func_8003E680` is **MATCHED (5FA)**: era `-O1 -G0`, all 53 words exact.
  Zero five state globals (Stage-0 reader types: D1C4/D280 unsigned compares,
  D1A0 flags, D250 opaque, CDDC `int` index), 2000-pass poll loop with `i++`
  in the `jal` delay slot (ROM `sltiu` → `unsigned int` counter — the only
  phrasing fix needed), callback registration, ~11 subsystem inits. **First
  fn-ptr-to-asm-callee arg**: `func_80073D24(func_8003E91C)` →
  `lui $a0,%hi` / `addiu $a0,$a0,%lo` with R_MIPS_HI16/LO16 against the
  same-segment text symbol; links exactly. `-O1` predicted by the selection
  rule (five per-store `lui`s, no CSE). Segment-head carve of 2EE80:
  C `0xD4`, resume `2EF54.s` `0x1858`. Next candidates: `func_800698D4`
  (159L, disc mount w/ SDK `DsSearchFile`), `func_8003F3C4` (245L).

- `func_8006A5BC` is **MATCHED (5EZ)**: era `-O1 -G0`, all 36 words exact on
  the first attempt. Four setup calls, two structurally identical
  `while (f() != 1) VSync(0);` loops (VSync = `func_80073A44`, SDK), then
  `func_8007F7A8()`'s return stored to `D_800B0DD4` (`unsigned short`, typed
  by its `lhu` reader). `-O1` reproduces retail's per-use `$s0=1`
  materialization — in `func_80086FF8`'s delay slot AND re-materialized
  between the loops (the 6A674 `-O1` lever, third leaf). Return-use safety:
  both loop conditions compare `$v0` raw (full 32-bit `beq`, no
  mask/sign-extend), so asm callees declared `int(void)` are codegen-safe.
  Identical loop bodies did NOT cross-jump. Mid-55430 carve: prefix
  `0x598C`, C `0x90`, then the three existing boot C leaves — **four
  contiguous C carves, no asm between**. Next candidates up the chain:
  `func_8003E680` (56L, state zeroing + 2000-pass poll + callback
  registration), `func_800698D4` (159L, disc mount w/ SDK `DsSearchFile`).

- `func_8003E610` is **MATCHED (5EY)**: era `-O2 -G0`, all 28 words byte-exact
  on the **first** attempt — no sched2, no pins. Straight-line dispatcher of
  ten calls with immediate args (`0x140`/`0xE0` = 320x224 display res), no
  branches/loops/`$gp`/globals; plain `-O2` reproduces ccpsx's mixed
  arg-load/delay-slot placement exactly. All ten callees are extern-declared
  with call-site-determined signatures (immediate args, no returns used —
  callee bodies don't affect codegen; one already C: `func_80080CC8`).
  Mid-2E7D8 carve: prefix `0x638`, C `0x70`, resume `2EE80.s` `0x192C`.
  Readiness ranking of `main`'s remaining callees (size + callee C/SDK
  coverage) put it first; next candidates in order: `func_8006A5BC` (42L,
  two wait loops + one `sh` global), `func_8003E680` (56L, state zeroing +
  2000-pass poll loop + callback registration).

- `func_8006A674` is **MATCHED (5EX)**: era `-O1 -G0 -fschedule-insns2` +
  `MASPSX_THREE_WORD_SYMBOL_STORE=1`, all 152 words byte-exact. `-O1` gives
  retail's per-use `-1` materialization (the `-O2` shared hoist is hardwired
  `optimize>1`, not flag-reachable); sched2 places every `li`/`addiu` before
  its adjacent store (21 order swaps). The six semantic pins from the 5EP
  bounded candidate are load-bearing (dropping all six → 46 mismatches).
  Mid-55430 carve fills the 6A64C/6A8D4 gap exactly (0x260); the three boot
  C carves are contiguous.
- `func_8006A64C` matches all 10 words on era `-O2 -G0`: two sequential
  `void(void)` calls, teardown before `jr`, and a nop delay slot. Both
  `R_MIPS_26` relocations resolve at link time; matching a caller requires a
  known callee signature, not that every callee already be C.

## Standing policy

1. **PROBE BEFORE GRIND.** The two biggest unblocks (maspsx stdin hang;
   `expand_load_immediate` forcing `ori`) were short diagnostics, not
   integrations. When a family is blocked, diagnose before more members.
2. **Homogeneous families may be batched.** Risk lives in the first member.
3. **`asm/` is not a source of truth for counts.** Use `configs/USA/disc1.yaml`.
4. **Commit messages are not evidence.** A claim is proven when a gate is green
   and the leaf is objdump-probed (not SHA alone on carves).
5. **No weak-int cheat:** do **not** invent width a narrower store contradicts
   (e.g. `sh`/`sb` → `int`). Distinct from **opaque-word** typing (consistent
   32-bit `sw`/`lw` everywhere) — that is a separate lead ruling, currently
   open under `TYPING-POLICY` in `parked_blockers.json`.
6. **Width-only setters are triaged in `parked_blockers.json`.**
   `READY-FROM-READER` (src reader already *types* it), `BLOCKED-ON-READER`
   (undecompiled reader not yet proven to be a mere use-site),
   `TYPING-POLICY` (opaque 32-bit word; use-site found, no narrowing possible),
   or `DECISION-BLOCKED` (write-only; no reader). A use-site is not a type-site
   (`func_800405A4` lesson). Re-check after every reader phase.
   `5EF-delay-slot` **CLOSED** (14/14 integrated); `sb-sh-five` reclassified
   typing-only.
7. **Register pinning is an evidence-backed fallback, not a shortcut.** Use it
   only after natural C and a retail-order phrasing retry prove that the
   residual is register **allocation**, not statement order. Pins must have
   semantic names and a source comment recording the allocation proof
   (`func_8006A8D4` exact; `func_8006A674` bounded parked example).

## Resolved blockers

- **Phase 5I** delay-slot (`move`/`or` vs `addu`): **SOLVED in 5EC** by era.
- **Maspsx non-TTY hang:** **SOLVED** (`</dev/null` in `era_compile`).
- **`lui;ori` large-literal synthesis:** **CAPABILITY-VERIFIED** (scratch probe;
  both sign cases; no flag change).
- **5EF delay-slot (sw in `j $31` slot):** **CLOSED in 5EF** by the
  vendored maspsx LOCAL PATCH (`MASPSX_FILL_STORE_DELAY_SLOT=1`). Key evidence:
  `func_8003FFAC` vs `func_8007FBC0` — identical C, different ROM scheduling
  (pre-jr+nop vs in-slot) ⇒ original units assembled under different ASPSX
  scheduling; behavior is opt-in per leaf. All 14 members are integrated exact;
  sb/sh never fill (ROM-proven).

## History (append-only, truncated)

| Phase | **224 exact leaves** (tools: maspsx load gate `439c244` on main; parked: 698D4/6E834/374E8) | `scripts/verify_us.sh` summary + exact rebuild |
| --- | --- | --- |
| 4I–4J | 0→1 path | Exact asm rebuild; GCC 14.2 first leaf |
| 5B–5CW | →98 | Empty stubs, getters, store/setter batch |
| 5CX–5DB | →103 | Countdown memset/memcpy (`$2`/`$3` pins) |
| 5DC–5DJ | →156 | `$gp` small-data (`_gp`+`-G 8`); `-fno-tree-ter` |
| 5EA | 157 | Era dual-toolchain; return-0 `addu` |
| 5EB | 161 | Return-0 twins via mid-segment holes |
| 5EC | 163 | sb+ret0; `--dont-expand-li`; 5I dead |
| 5ED | 170 | sb+ret0 batch harvest (family closed) |
| 5EE | 171 | `$at` absolute-`sw` integrated pilot; delay-slot shapes blocked |
| 5EG-readers | 173 | Type-pinning readers `func_8008AB1C` / `func_80042B6C`; `D_800A1870` decl fix |
| 5EG-setter | 174 | `func_80085728` dual-store; first reader-recoverable pre-jr setter |
| 5EH-opaque-word | 182 | u32 opaque-word ruling; 8 A182x setters (`42BD8`…`42C64`) |
| 5EI-ready-from-reader | 185 | READY-FROM-READER setters `42910`/`42B38`/`42B50` |
| 5EJ-d8009d28c-state | 189 | `D_8009D28C` int-state setters `17FDC`/`17FF0`/`192B8`/`192C8` |
| 5EK-d8009d270-bitwise | 191 | `D_8009D270` unsigned flags setters `87198`/`87414` |
| lui-ori probe | 191 | Large-literal `lui;ori` CAPABILITY-VERIFIED (docs only) |
| 5EF-pilot | 192 | Vendored maspsx LOCAL PATCH (sw delay-slot fill); `func_8007FBC0` integrated |
| 5EF | 205 | Remaining 13 delay-slot `sw` members typed and integrated; family closed 14/14 |
| 5EG-first-branch | 206 | First branchy leaf `func_8004F448`; era cc1 `-O1 -G 8` hoists const into `beqz` delay slot word-exact (branch scheduling capability proven) |
| 5EH-arg-return | 207 | First value-returning leaf `func_800438C0` on era path: `-O2 -G8` preserves double store, `addu` return-0, era+gp proven; GCC 14.2 store-merge + `move` documented as $CC-path limits |
| 5EI-first-nonleaf | 208 | First non-leaf `func_800197D0` on era `-O2 -G8`: frame (`addiu $sp,∓0x18`, `sw/lw $ra,0x10($sp)`) + `jal func_800375B4`; teardown `addiu $sp,+0x18` lands **in the `jr` delay slot** word-exact |
| 5EJ-outgoing-arg | 209 | `func_80019484(int **)` on era `-O2 -G0`: double-dereference load schedule sets outgoing `$a0` before `jal func_800438C0`; load-delay nop, jal nop, frame, and teardown-in-`jr`-slot all word-exact |
| 5EK-volume-197f0 | 210 | First post-probe volume leaf: `func_800197F0` on era `-O2 -G0` transfers the proven 197D0 frame + void `jal` + return-1 + teardown-in-`jr`-slot shape word-exact; no new primitive |
| 5EL-return-forwarding | 211 | `func_8007F7A8` on era `-O2 -G0` forwards `func_8007FCAC`'s `$v0` untouched and reproduces retail's opposite epilogue schedule: teardown before `jr`, nop in the delay slot; all eight words exact |
| 5EM-boot-6a8d4 | 212 | First Rung-1 boot leaf: `func_8006A8D4` on era `-O2 -G0` lays out boot memory regions with 19 ordered absolute pointer stores; register-pinned byte cursors reproduce all 68 retail words exactly after two plain-local phrasings fail the retail register allocation/store schedule. Compiler-constrained, target-specific C is documented in source |
| maspsx indexed-store | 212 | Toolchain patch `f0b9155`: default-off `MASPSX_THREE_WORD_SYMBOL_STORE=1` opt-in adds the three-word symbol+register store form; exact 212-leaf regression, 148 tests, and live re-clone durability passed |
| 5EN/5EP-loop-probe | 212 | `func_8006A674` proves five `bnez`/`bgez` loop back-edge delay slots plus store-in-`jr`-slot; L2 and late allocation deltas cleared, but the leaf is parked with a 45-word `$v1` constant-hoist residual and no 213 claim |
| 5EQ-boot-6a64c | 213 | Boot wrapper `func_8006A64C` on era `-O2 -G0`: calls matched-C `func_8006A8D4` then live-asm `func_8006A674`, both proven `void(void)`; both `R_MIPS_26` relocations resolve and teardown-before-`jr` + nop-slot matches all 10 words |
| 5ER-d1c-d48 | 215 | Adjacent byte/word test-and-clear-return twins `func_80038D1C` / `func_80038D48` on era `-O2 -G0`; explicit pointer reuse gives retail `bnez` + j-over delay-slot returns and `sb`/`sw` clears, all 11 words each exact after one natural phrasing retry |
| 5ES-loop-4bf08 | 216 | First loop-as-volume leaf: natural explicit-init pointer walk in `func_8004BF08` clears two parallel `int[8]` arrays; era `-O2 -G0` reproduces all 14 words, including the split pointer advances and backward-`bnez` delay slot, with no pinning or tool flag |
| 5ET-loop-5186c | 217 | Loop-as-volume repeats: pure-register 16-pass bit-serial loop `func_8005186C` on era `-O2 -G0`, all 15 words on the first natural-C try; unconditional `result <<= 1` fills the forward `bnez` skip slot, nop `bgez` back-edge; mid-4204C carve (prefix 0x20, C 0x3C, resume 420A8.s 0x5A0) |
| 5EU/5EV parks | 217 | `func_80055724` (p-load hoist; `-O1`≡`-O2`) and `func_80052BCC` (rotated-loop `$v0`/`$v1` role swap, 13/15) parked as the **PARKED-ALLOCATION/SCHEDULING family** (with `6A674`): cc1 global allocation/scheduling choices natural C can't steer. Banked idioms: rotated loop = explicit first iteration + `while`; signed `char` vs `0xFF` emits the `andi`. Docs only, no carve |
| family diagnosis | 217 | Read-only investigation: **NO SINGLE KNOB**. All three residuals are in cc1 raw output (maspsx can't fix any). `55724` = pre-reorg emission order, no lever; `52BCC` = regclass+sched2 pair (`-fexpensive-optimizations`+`-fschedule-insns2`), `-O1` shows retail loop roles — retry at `-O1`; `6A674` = hardwired `optimize>1`, only lever `-O1` (untested). Toolchain-patch hypothesis closed; per-leaf `-O1` is the route |
| 5EW-52bcc-o1 | 218 | `func_80052BCC` MATCHED: era `-O1 -G0 -fschedule-insns2` (first sched2 leaf) + two-const-mode phrasing (u8 head const dies at guard → loop reload into `$v1`; `int` loop byte → raw `bne`); sched2 hoists head `li` into the `lbu` delay like ccpsx. All 15 words exact; mid-42FC8 carve (prefix 0x404, C 0x3C, resume 43408.s 0x2A8). Also fixed a latent pipefail/SIGPIPE flake in toolchain detection (`grep -q` → `grep … >/dev/null`) |
| 5EX-6a674-o1 | 219 | `func_8006A674` MATCHED after three parked attempts: era `-O1 -G0 -fschedule-insns2` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`, all 152 words + relocs exact with the 5EP semantic pins (load-bearing; dropping → 46 mismatches). `-O1` = per-use `-1` materialization; sched2 = `li`/`addiu`-before-store placement (21 fixes) — **sched2 confirmed as a general retail fingerprint**. Boot Rung 1 complete (`main → 6A64C ✓ → {6A8D4 ✓, 6A674 ✓}`); mid-55430 gap filled exactly (0x260), three contiguous C carves |
| 5EY-boot-3e610 | 220 | Boot display/graphics bring-up `func_8003E610` on era `-O2 -G0` — all 28 words exact on the **first** attempt, no sched2/pins: straight-line dispatcher, ten calls with immediate args (`0x140`/`0xE0` = 320x224), callees extern-declared with call-site-determined signatures. Readiness ranking of `main`'s callees (callee C/SDK coverage, not raw size) picked it; next up the chain: `func_8006A5BC`, `func_8003E680`. Mid-2E7D8 carve (prefix 0x638, C 0x70, resume 2EE80.s 0x192C) |
| 5EZ-boot-6a5bc | 221 | Boot init `func_8006A5BC` on era `-O1 -G0`, all 36 words exact first attempt: four setup calls, two identical `while (f() != 1) VSync(0);` loops (no cross-jump), `7F7A8()` return → `D_800B0DD4` (`unsigned short` via `lhu` reader). `-O1` per-use `$s0=1` materialization (delay-slot + between-loops re-materialization) — third `-O1`-lever leaf; return-use confirmed codegen-safe (raw `$v0` `beq`, no mask). Mid-55430 carve extends the boot block backward: **four contiguous C carves** (prefix 0x598C, C 0x90, then 6A64C/6A674/6A8D4) |
| 5FA-boot-3e680 | 222 | Boot subsystem-init dispatcher `func_8003E680` on era `-O1 -G0`, all 53 words exact: zero 5 globals (Stage-0 reader types), 2000-pass poll loop (`i++` in `jal` slot; `unsigned int` counter for ROM `sltiu` — the only phrasing fix), callback registration + ~11 inits. **First fn-ptr-to-asm-callee arg** (`&func_8003E91C` via R_MIPS_HI16/LO16 against a text symbol). `-O1` predicted by the per-use selection rule (five per-store `lui`s, no CSE). Fingerprint table banks: `-O1` selection rule, return-use readiness, sched2 scope narrowing, fn-ptr arg, unsigned loop compare. Segment-head carve of 2EE80 (C 0xD4, resume 2EF54.s 0x1858) |
| 5FB park | 222 | `func_800698D4` (disc mount, 141 words) PARKED-SCHEDULING at 140/141: nested-if phrasing defeats gcc's range-test collapse (`v!=0 && v!=-1` → `addiu`+`sltiu`+`bnez`), everything exact except one dbr_sched delay-slot steal at search #3's `beqz` — retail declines a `$v0`-setter steal when the branch target is the return block (`$v0` live to `jr`); ours steals. Screening rule + CdlFILE `0x18` frame note banked; candidate stashed; no carve, no leaf claim |
| 5FC park | 222 | `func_8006E834` (post-mount loader + display env, 91 words) PARKED-ALLOCATION at 89/91: five-arg call PROVEN (5th arg `sw $v0,0x10($sp)` in `jal` slot, first try); frame decomposition method banked; retail folds the range test in this unit (per-TU datapoint vs 698D4). Residual: call-result register home (`$v0`+restores vs `$v1`), not source-expressible. Third ccpsx-vs-2.7.2 skew mechanism recorded; candidate stashed; no carve, no claim |
| 5FD-table-2f9cc | 223 | Table clear `func_8002F9CC` (17 words) on era `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`: zero the in-use flag of all 7×220-byte records at `D_800A5D58` (record typed from the `func_8002F7D8` reader; extent `0x604` = 7×220). Key finding: aggregate element type is an addressing-mode lever — `arr[i].field = 0` keeps the symbolic indexed store; flat `arr[i*55] = 0` hoists `la` (flag-invariant). `unsigned char` counter (`andi 0xFF` masks), `sltiu` bound, stride 220B/55W (not 196B/49W). Mid-11718 carve (prefix 0xEAB4, C 0x44, resume 20210.s 0x4010) |
| 5FE-table-2f970 | 224 | Table twin `func_8002F970` (23 words) on era `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`: pointer-match search-and-clear over the 2F9CC table (`SlotRecord` typing inherited unchanged); `*p == D_800A5D58[i].body` → clear `inUse`, then `*p = 0` with the `sw` in the `jr` delay slot (5EN pattern). `$a3` body-base hoist = the aggregate lever producing (not preventing) a hoist; back-branch slot FILLED vs 2F9CC's nop — slot fill is per-shape, not per-table. One phrasing fix: operand order in the compare (`body == *p`) for `bne $v0,$v1`. Object-level `%lo` difference on the hoisted base (`D_800A5D58+4` vs `D_800A5D5C`) resolves to identical bytes at link. Contiguous carve with 2F9CC (prefix 0xEA58, C 0x5C, C 0x44, resume 20210.s) |
| 5FF-maspsx-loads | 224 | Toolchain patch `439c244`: `MASPSX_THREE_WORD_SYMBOL_STORE` extended from stores to standalone indexed symbolic LOADS (lb/lbu/lh/lhu/lw/lwl/lwr) under addiu_at — pass-through emits the ASPSX 2.30 three-word lui/addu/op-%lo form; compound lines retain legacy; `lwc2` stays outside (durable negative). Store path untouched; one gate, existing name. Full gate: flag-OFF 224 exact SHA; flag-ON 224 exact SHA (6A674/2F9CC/2F970 unchanged under the extended meaning); 153 vendored tests (was 148, +5 load); re-clone restores all three tracked files byte-identical. `func_800374E8` (which motivated the patch) PARKED — register-coloring residual (structure correct; see Known-open families + parked_blockers.json). 224 unchanged, no carve.
| 5FG-363f4 | 225 | Search-and-clear `func_800363F4` (21 words / 0x54 @ 0x26BF4): 16-entry D_800A7624 scan, clear key on match, break. era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 — FIRST leaf exercising the load gate; the probe EXPOSED the 439c244 bug (GNU as uses the DESTINATION reg as temp for lw; ROM uses $at), fixed at 5dac87e. 21/21 words; mid-2422C carve (prefix 0x29C8, C 0x54, resume 26C48.s 0xD5C); full 225 build EXACT SHA.
| 5FI-62a34 | 227 | 2-key node-list search `func_80062A34` on era `-O2 -G8` (gp head); `&&` short-circuit matches two-target block layout |
| 5FJ-6e9a0 | 228 | Boot display init + pointer arena + ClearOTagR poll loop + dispatch exit `func_8006E9A0` (141 words / 0x234 @ 0x5F1A0) on era `-O2 -G0`, **141/141 exact**: 6A8D4 arena pins (`$v0/$v1` cursors, `$a0/$a1` consts) reused verbatim; NEW pins `$s0`=saved_arg / `$s2`=&D_800B0E38 (natural allocation swaps them and sinks the materialization past a call); empty-asm barrier holds the `$s2` lui/addiu at retail words 29-30. Tail carve of 5B1E4: asm prefix 0x3FBC, C 0x234, no resume (6EBD4 C sibling follows). P3 types: D_800BCE80/D_800BCFEE/D_800B0DC6 `unsigned char` (opaque DISP_ENV addr, lbu poll, sb); arena globals + D_80011614 `unsigned char *`; D_8009D280 `unsigned int`, D_8009CDDC `int` (3E680). Full 228 build EXACT SHA. **Uncommitted on branch `phase5fj-6e9a0`** |
| 5FK-6e834 | 229 | Post-mount image loader + display env setup `func_8006E834` (91 words / 0x16C @ 0x5F034) on era `-O2 -G0`, **91/91 exact** — the 5FC-parked call-result register-home residual resolved by the 5FJ control technique: paired pins `register int t asm("$3")` ($v1 backup across the range test) + `register int rt asm("$2")` ($v0 equality-test home) and two zero-code `asm volatile("" : "=r"(x) : "0"(x))` barriers (block copy-prop folding, emit nothing); reorg threads the idempotent restore copy into the `beqz` delay slot, reproducing retail's two restores. Bounded matrix: natural V0 = historical 90-word residual; single-pin V1/V3 inert; paired pins without the rt barrier leave tests on $v1. Mid-5B1E4 carve: asm prefix 0x3E50, C 0x16C, then 6E9A0 C — 6E834→6E9A0→6EBD4 now contiguous C. Full 229 build EXACT SHA. **Uncommitted on branch `phase5fk-6e834-pin-revisit`** |
| 5FL-698d4-revisit | 229 | **PARKED** — Bounded pin/barrier revisit of `func_800698D4` (disc mount, 141w) following the 5FK proof. V0-V8 barrier matrix tested: empty `asm volatile("")` and 5FK-style value barriers prevent the delay-slot steal at searches #3/#4 but add scheduling-boundary overhead bloating to 144 words (+3 vs retail). Residual is pure instruction-scheduling (delay-slot fill), not register allocation; the 5FK control family cannot resolve without unacceptable overhead. sltiu fix confirmed. Candidate preserved at `src/func_800698D4.c` (PARKED, 140/141). Production unchanged at 229. **Uncommitted on branch `phase5fl-698d4-barrier-revisit`** |
| 5FM-main-revisit | 229 | **PARKED** — Bounded revisit of `func_8001220C` (main, 187w). V0 baseline (era -O2 -G0) produces 188 words with 150/187 word-level mismatches across all 7 zones: prologue save-batching order fundamentally differs, $s2/$s3 register assignment is swapped (state_val→$s2 vs retail $s3; flagbyte→$s4 vs retail $s2), invariant bitmask 0x100000 materialization point diverges, and the skew cascades through the entire dispatch loop. Scratchpad stack-handoff atom remains byte-exact. The bounded hard-register/barrier family from 5FJ/5FK cannot address pervasive global register-allocation skew of this scope; the V0-V6 matrix was not executed because the baseline already exceeds what localized barriers control. Candidate preserved at `src/func_8001220C.c` (PARKED, semantically complete, 20 callee declarations verified). Production unchanged at 229. Fresh Docker build confirms EXACT SHA-1. **Uncommitted on branch `phase5fm-main-barrier-revisit`** |
| 5FN-725dc | 229 | **PARKED-PER-TU-TOOLCHAIN** — bounded campaign on `func_800725DC` (main's first callee, 28w) + twin `func_8007264C` (26w): one-shot callback runner over the EMPTY fn-ptr table at `jtbl_80010000`. THREE LEVERS BANKED (probe-verified): the retail `lui/addiu`-zero count is unreachable from any C zero spelling (10 forms probed) but `(int)jtbl_80010000 - 0x80010000` → `la SYM+0x7FFF0000` resolves to 0000/0000 at link and keeps the loop alive at -O2; an inline-asm `jalr` call (tied decrement in the slot) gives the retail no-args-area frame (`.frame args=0`); the loop body + flag load/store shape is word-exact on era `-O2 -G0` with pins. RESIDUAL is three coupled per-TU mechanisms, flag-invariant across the ladder: ascending contiguous prologue saves (era fixed descending; all other PE1 units descending/interleaved), `li→ori` expansion, no-args-area frame model — the 0x800725xx SDK-runtime unit used a different ccpsx/aspsx config (third per-TU datapoint). Candidate preserved at `src/func_800725DC.c`. Production unchanged at 229 |
| 5FO-6a9e4 | 229 | **IN PROGRESS, CHECKPOINT d10** — `func_8006A9E4` (main's boot-read, 215w): twelve-draft campaign, **216 words (+1)**, frame EXACT 0x30, all zone/copy structure word-count-exact. NEW PROVEN LEVERS: `__builtin_memcpy(dst,src,16)` on char* emits EXACTLY retail's lwl/lwr×4 + swl/swr×4 block shape; `called→asm("$17")` pin flips zone 2 to retail's called=$s1/sentinel2=$s4; do/while copies kill the trip guard. NEGATIVE RESULTS banked: packed struct → byte-wise synthesis (+119w), aligned(1) ignored, `-fno-strength-reduce` regresses (208/221w). Sole structural extra = zone-1 gate steal (698D4-class); remaining deltas are copy-loop register/offset encodings + zone-3/4 gate li extras. ROUND-3 matrix (d13-d15) brackets the solution: block-scoped sentinel pins make ALL gates word-exact but bloat copies (+10) — gates-vs-copies register-pressure trade-off; next: narrow sentinel lifetimes or pin copy cursors. Candidate at `src/func_8006A9E4.c` (NOT integrated) |
| 5FH-twin-37548 | 226 | Record-field lookup twin `func_80037548` (27 words / 0x6C @ 0x27D48): scan 4 x 56-byte D_800BCEA8 records for short needle at +0x10, return signed byte0 (+0x00) on match else 0. era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 (lh/lbu indexed pair through the 5dac87e $at gate). Twin-hypothesis FALSIFIED: matches 27/27, no coloring skew — live-value-pressure rule refined (see register-coloring-374e8 parked entry). Accumulator phrasing banked (fingerprint table). Mid-27C6C carve: prefix 0xDC, C 0x6C, resume 27DB4 (existing sibling); full 226 build EXACT SHA; packed-span byte-exact.

Detail and leaf-by-leaf narrative: git history + wiki
([Current Status](https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki/Current-Status)).
PC port remains out of scope. Redump.org cross-check still open (non-blocking).
| WIN1-win32 | 241 | Native port builds on Windows 10/11 x64 (MSVC/MinGW): new `pc_port/platform/host_window_win32.c` Win32-GDI backend implementing the `host_window.h` interface (same pad bits, 60 Hz poll, top-down BGRA StretchDIBits), CMake picks backend by `WIN32`, `dl` link + GCC sanitizer flags guarded `NOT WIN32`/`NOT MSVC`, `__builtin_memcpy`→`memcpy` (52C6C), CUE sibling lookup also splits on `\`. Zero VLAs (`-Werror=vla` clean), strict-decl syntax gate clean, no compound literals/unions. Guide: `pc_port/docs/WINDOWS.md`. Linux suite unchanged: 1028 run / 1010 passed / 1 pre-existing disc-gated B54KY fail / 17 skipped. Windows-side compile NOT yet run here (no Windows toolchain in sandbox) — needs one `cmake --build` on a Windows box to close. |
| UBUNTU-native | 241 | Ubuntu 26.04 LTS, GCC 15.2, CMake 4.2.3, clean out-of-tree build in /tmp: configure + full build green, `parasite-eve-port --headless --bootstrap-disc --max-frames 1` exits 0 with byte-correct 320x240 P6 PPM (230415 B). ctest: only pre-existing disc-gated `B54KY_192CE8` fails (`missing local/pe_disc1.path`); all else passes. Deps: `build-essential cmake` (+ `libx11-6` for windowed). Guide: `pc_port/docs/UBUNTU.md`. |
| UBUNTU-retail-ledger | 241 | Retail-accuracy ledger `pc_port/docs/RETAIL_ACCURACY.md`: proven disc-free = suite green (modulo disc-gated B54KY), run determinism (PPM+trace `cmp`-clean across runs), 0 adapted/unsupported on frame-1 path, strict frontier mapped (7F72C from 698D4; frame-2 abort is the honest trap). Gap named: 6 scripted BOOTSTRAP_RET providers, 2 bounded adaptations (scratchpad handoff, disc-wait limit), oracle expect-sides + real-disc path blocked on absent image, capture harness for item 5 nonexistent. No divergence found in anything runnable; no code changed. |
| UBUNTU-retail-correction | 241 | Ledger corrected: the six mount providers were already translated in `platform/pe_libcd.c` (asm-verified); only the `--bootstrap-disc` fixture branch scripts them. Real mount path proven by the 698D4 family (`PE_TEST_FILTER=698D4`: 7 passed, 0 failed — mount, both disc bits, no-disc, fixture, archive seed). No new code needed; ledger + this row are the delta. |
| CDQ1-stream-issue | 241 | CdlReadS queue rung: translated 7E6B0 (ring alloc) + 80950 (copy/clear) + 7C214 (DMA-completion cb, B0CC8 chain proven dead via 7C304) + 7C394 (record math, signed-mult fix) + 7F0C8 (130-word queue issue, jump-table arms mechanically proven identical, stack-garbage canonicalization documented) + full 81314 (both arms incl. silent restore). Boundary moved 81314-cut -> F394 completion_selector (lanes idle, completion pending); 7C564 state machine + pump named next. 8 CDQ1 tests; 4 test sites + 11 py pins migrated (b54kan audit still passes verbatim intent). Suite: 1036/1036 WITH local Disc 1 image (1018 + B54KY-gated + 17 skipped without). Real-disc run: 483 presents, zero stubs before the selector. |

Latest retry update: INV16 all297 original cases PASS, normal and ASan/UBSan
CTest2/2 each,1180groups (44.81/54.13s), /tmp/pe-inv16-{tests,san-tests}.log.
INV15 helper61732 timed out on pistol menu; manual ordinary selection of
Medicine1/Use succeeded atframe26425:38→45HP,particle/ring image20frameslater
visuallyverified. Evidence battle-medicine-progress-inv15.json andPNG/RAM/RGB.
Firing helper64960 produced7shots but Aya naturally died; no scriptedexit.
Mode3phase4 stalls withAyabusy1 because2B29Cphase3 omitted3C5D8+fadeflags.
INV17 source restores those2fades at timer0; not built/tested yet.
Current GDB60961 still runs INV15 atdefeat; no guest writes or held keys.
PE page remains deferred;24250 is also partial, do not wire fieldPE as working.

INV17 fade transition verified against24 full original executions (state
ranges only; presentation packets remain outside this bounded patch).
Normal and ASan/UBSan CTest2/2 each,1181groups (41.18/49.46s). Logs
/tmp/pe-inv17-{oracle,filtered,tests,san-tests}.log. GDB60961 killed cleanly.
Fresh visible GDB84034,DISPLAY:10.0 window46137345, /tmp/pe-inv17-live.gdb/log.
Opening helper5458 acceptedAya; ordinary master92859 now plays stage/stairs,
waits firsthit, selects Medicine1 using observed listcursor, and immediately
starts firing. /tmp/pe-inv17-visible-replay.py; /tmp/pe-inv17-replay.log.
No native source builds currentlyrunning; noheldkeys outsidehelper.

INV17 fresh message20 capturevisually confirmsall3performers/backdrop/stairs:
docs/evidence/pe-aya-visible-dialogue/performance-inv17.png (+RAM/RGB).

DIST-B1 (2026-09-05): first private Banshee distribution of the native port.
Release build `pc_port/build-dist` (Release, 1.6MB, libc-only): 1181 run /
1160 passed / 20 skipped / 1 pre-existing env fail (`B54KY_192CE8`, missing
`local/pe_disc1.path` — identical on dev Debug build). New
`pc_port/package_runtime.sh` stages `runtime/{parasite-eve-port wrapper,
bin/parasite-eve-port, data/disc1.bin, build-info.json}` and emits
`dist/parasite-eve-PE-INV17-7821e7c3-linux-x64.tar.zst` (277MiB,
sha256 dcd3784072c2dc97039dc33b7b0efbbee4f95edcb0cf0b92b52012f8c3f01715;
binary f9c80367…; disc 7f20fce9… 495531120B). Launcher side already wired
(provider `banshee.parasite-eve`, Store product, Settings channel URL,
installer + BuildId-driven UPDATE, `ParasiteEveDistributionTests`); missing
pieces staged at /tmp/pe-publish-parasite-eve-channel.sh and
/tmp/pe-PARASITE_EVE_PRIVATE_DISTRIBUTION.md for the banshee repo (publish
to private bucket `games/parasite-eve/`, signed channel URL into Settings).
R2 publish DONE 2026-09-05: package at
`banshee-preservation-private/games/parasite-eve/builds/PE-INV17-7821e7c3/`
(290624037 B verified remote = local); channel `channels/dev.json` +
`channels/dev-PE-INV17-7821e7c3.json` carry size/sha256/objectKey +
7-day signed package URL (ranged GET 206 proven); signed channel URL in
/tmp/pe-chan/channel-url.txt (paste into Settings › Updates; never commit).
Prior head was B53I-B1-76d8cffc48b4, so this publishes as an UPDATE
(BuildId change drives HasUpdateAvailable; previous retained for rollback).
Launcher PE section needed no code change (provider/Store/Settings/
installer already committed); `dotnet test` unrunnable in sandbox
(MSBuild named-pipe IPC denied) — tests verified by reading only.
Publish script + distribution doc staged at /tmp (banshee repo is
sandbox read-only here): /tmp/pe-publish-parasite-eve-channel.sh →
`banshee-realm-client/scripts/`; /tmp/pe-PARASITE_EVE_PRIVATE_DISTRIBUTION.md
→ `banshee-realm-client/docs/`. Next: paste URL, INSTALL from launcher to
prove end-to-end.
Re-verified 2026-09-05: R2 head is PE-INV17-7821e7c3 (channels/dev.json +
pinned dev-PE-INV17-7821e7c3.json; package 290624037 B present); launcher
HEAD (0.9.51 line) ships the PE section (Store offers parasite-eve,
provider + Settings URL field + install/update flow committed) so no client
release is needed — the pushed update IS the channel head. Fresh 7-day
channel URL minted to /tmp/pe-chan/channel-url.txt. Banshee-repo file
placement still blocked (read-only FS from here): publish script + doc
remain staged at /tmp (see paths above).

INV18/19 update: native200PEapplicationcases and161menu cases PASS.
INV19 firstlinkfailedduplicate55610 (alreadycompletein5DCEC_port); removednew
copy. Main-menu referencefixture nowexplicitlyenablesPE(bit1),previous3Dmask
selectedEquipment. Testsregenerated/tmp/pe-inv19-oracle2.log andpass
/tmp/pe-inv18-filtered2.log,/tmp/pe-inv19-filtered2.log. Fullnormal+san CTest
running /tmp/pe-inv19-{tests,san-tests}.log.1183groups expected.
Fresh visible GDB64979 atDISPLAY:10.0/window46137345 (INV19 binary),
/tmp/pe-inv19-live.gdb/log. Ordinarymaster59749 opening/name→fullfight;
/tmp/pe-inv19-replay.log. Willstopaftermessage60 sofieldPEcanbetested.
GDBcaptures51770beforeand2presentframesafterabilityuse (read-only).

INV19 fullnormal+ASan/UBSan CTestPASS2/2each1183groups49.88/59.20s.
Visible replay59749 acceptedname/renderedstage, but oldaxis-onlynavigation
failedaftercameraanglechange nearstairs. Interruptedhelperandranfull-vector
/tmp/pe-visible-inv19-finish.py(session44736). ThewindowthenreceivedHOST_QUIT
at12944,fieldauditorium. Inputhelpersstopped; GDB64979 continued/quitcleanly.
No PE ability was visiblyused. FieldPEhelper31728 endedonHOST_QUIT. Noheldkeys,
no gamewindowcurrentlyopen. Do notreporta crashorcompletedINV19fight.
INV20 source nowreplaces22394 withfullPEbattlecommandlifecycle and20D50
recovery innew22394_port.c;142referencecases generated /tmp/pe-inv20-oracle.log.
Header/CMake/tests wired; normal+sanbuildsrunning /tmp/pe-inv20-{build,san-build}.log,
sessions recordedintoolmessages. NO C/header/CMakeeditsuntilbuildsfinish.
24A3C liberationpresentation remainspartial; originalcasecoveragewilltest
selectedstates butdoesnotproveallabilityeffectcallbacks areimplemented.
