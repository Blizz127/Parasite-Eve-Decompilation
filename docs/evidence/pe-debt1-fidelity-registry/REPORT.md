# PE-DEBT1 — cross-lane fidelity debt registry

```text
PE-DEBT1 SUCCESS — CROSS-LANE FIDELITY DEBT REGISTRY ESTABLISHED
```

Evidence only. No repair. No promotion. No reclassification of a
proven result as debt. No production runtime change. No push.

```text
authorities
  PE-SYS0     c799bbb
  PE-PST0     6f51289
  PE-TXT0     1d47df3
  PE-BTL0     de959cf
  PE-RD5-F9   891159a
  PE-RD5-X    29c095a
  PE-RD5-C2   1e7f0df
  PE-RD6-A    65446c8
  PE-RD6-B    a0baebc
  PE-RD6-R    f7e8ab5
  PE-RD7-R    ee552bd
  PE-UE0      45e6cd8
  PE-AUD1-D   40b7c3f
disc_sha256 = 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
exe_sha256  = 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
```

This rung consolidates every currently evidenced outcome-faithful
shim, stand-in, approximation, and parked blocker across the Day 1
research, UE0 native field VM, AUD, TXT, PST, BTL, and this
checkout's matching/native tree. Companion files:

| File | Role |
|---|---|
| `DEBT_REGISTRY.csv` | one row per live entry |
| `PROMOTION_TRIGGERS.md` | rung-or-system flip conditions |
| `CROSS_LANE_COLLISIONS.md` | same mechanism, disagreeing shims |

Classes used here are the three requested by this rung:

```text
NONBLOCKING_FIDELITY   proven outcome; intermediate machinery or
                       presentation is approximate
PARKED_BLOCKER         missing or unlocated behavior that a later
                       named rung cannot ship without
UNKNOWN_LABEL          handler or bytes proven; gameplay name is not
```

SYS0 `PROVEN_OUTCOME` maps here to `NONBLOCKING_FIDELITY`. SYS0
`PRESENTATION_ONLY` residuals that are still live shims also map
here to `NONBLOCKING_FIDELITY`. SYS0 `RESEARCH_REQUIRED` rows are
included only when a report already names the gap; they are not
invented.

## 1. Scope and hard negatives

Included: labeled stand-ins, outcome-faithful machines, short
models of a larger retail bank, unresolved labels on proven
handlers, and parked blockers named by an accepted report.

Excluded (not debt):

- Proven results (PST0 512-word `D_800A77F0`; persist[1] =
  `entrance_selector`; RD7-R dest contract; TXT0 letter map;
  RD5-C2/`OP_0x22` close; RD5-F9 "no over-gating"; CAM-B / VIS-B
  / VIS-C freeze).
- Matching-decomp scheduling parks in
  `docs/ai_context/parked_blockers.json` (compiler residuals, not
  runtime fidelity).
- Phase 6E host SDK / GPU provider shims on the boot-oracle path
  (`pc_port/docs/shim_inventory.md`). That lane is not the Day 1
  playable runtime.
- Historical shims that current floors no longer run: the 8-word
  persist vector and `persist[0x4A]==0` treated as 9. UE0 uses 512
  words. RD6-A writes exact 9 / 0x12 / 0x18. PST0 records those
  as retired risks, not live implementations.
- ATB / damage / PE as invented battle features. BTL0 parks them
  out of contract; they are not a current stand-in.

## 2. Known entries (requested) and the rest

### 2.1 Requested seven

| id | lane / commit | class | one-line |
|---|---|---|---|
| DEBT-FID1-001 | UE0 `45e6cd8` (allowed RD3-A) | NONBLOCKING_FIDELITY | `0x85`/`0x9C` hard-cut |
| DEBT-FID1-002 | UE0 `45e6cd8` + RD7-R `ee552bd` | NONBLOCKING_FIDELITY | **merged** view-0 / no-`0x82` |
| DEBT-FID1-003 | RD6-A `65446c8` / RD6-R `f7e8ab5` | NONBLOCKING_FIDELITY | init-table bytes ambiguous |
| DEBT-FID1-004 | RD5-X `29c095a` / RD6-A `65446c8` | NONBLOCKING_FIDELITY | mailbox outcome-faithful |
| DEBT-FID1-005 | RD7-R `ee552bd` | UNKNOWN_LABEL | `0x9B` byte triplet |
| DEBT-FID1-006 | TXT0 `1d47df3` | PARKED_BLOCKER | font atlas pixels |
| DEBT-FID1-007 | BTL0 `de959cf` | PARKED_BLOCKER | `D_8009D28C==7` setter |

### 2.2 Merge: UE0 lobby view 0 and RD7-R auto-apply

These are the same open question: **what selects the current
52-byte view when opcode `0x82` (`func_80066800`) is absent?**

- Retail lobby: RD3-A allowed "stay on record 1" while the
  per-frame picker among records 1 and 7 is undecoded. RD6-A
  hard-gates the init-table OR-2 *outcome* onto records 1 and 7
  without proving the 16-byte entry layout (DEBT-FID1-003).
- UE0 implements the stand-in as **always decode view index 0**
  (`pe_render.cpp` → `decode_view_record(..., 0, ...)`).
- RD7-R proves m0377i first view is the authored 2D layers.
  Authored 52-byte view 0 (H=307 + MATRIX) is **not** applied.
  There is no `0x82`. Unknown listed: whether the engine ever
  auto-applies view 0 without `0x82`.

One registry row (DEBT-FID1-002). The 16-byte init-table *layout*
stays a separate row (DEBT-FID1-003) because that is a byte
schema gap, not the apply-or-not question.

### 2.3 Additional live entries

Audio (AUD1-A / AUD1-D `40b7c3f`): dry type-5 reverb; linear
sample interpolation; bank-0 fallback articulations; incomplete
SFX/BGM mix; `0x199 0x88` payload unlabeled; PT1 package still
has no Event-3 so that packaged lobby stays silent; speaker
verification pending.

Text: `0x4B` unmapped glyph; processed pad mask `0x100` unnamed;
m0372i research reel keeps `current_message` for the full
`OP_0x02` hold after retail `0xF9` already closed (RD5-F9).

Field stand-ins still in the Python research runtime: 180-frame
linear `0xB8` walk (RD4-B); one-frame dest `0x3F` restore
(RD3-B); analytic rsin/rcos at the frozen heading (RD2M).

Persist / save: Python hop fixtures still allocate
`persist = [0] * 0x4B` (indices 0..0x4A only); `persist[0x19]`
writer unknown; day-transition reset unknown; save header / CRC
span / card block unclosed; this checkout's `pe_save.c` is
libcard bring-up only.

Route parks: FMV003 recorded not played; opening FMV file
unmapped; m0001i curb-to-sidewalk writer still TENTATIVE; Day 1
boss identity RESEARCH_REQUIRED; no battle runtime (BTL1 is
handoff only).

RD7-R residuals that are labels, not machines: `0xED`
`actor+0x27D`; `0x41` `actor+0x98` bit `0x40`; type-2/3/4 names;
`0x04` thread-walk body.

## 3. Native-tree occupancy

"Native tree" here is **this checkout**
(`phase6e-b-provider-frontier`): matching `src/` plus
`pc_port/`. UE0 lives in a sibling worktree and is not compiled
here.

Three registry rows have a live artifact in this tree:

| id | artifact | what it is |
|---|---|---|
| DEBT-FID1-004 | `pc_port/game/boot/func_8006536C_port.c` | zeros `D_800A3180` (28×12); `func_800653B8` / `65400` / `12700` untranslated |
| DEBT-FID1-007 | `src/func_80017FDC.c` / `17FF0.c` / `192B8.c` / `192C8.c` | matching setters store 5 / 6 / 0 / 8; none stores 7 |
| DEBT-FID1-019 | `pc_port/platform/pe_save.c` | libcard save-manager bring-up; not `D_800A77F0` serialization |

UE0 (not this tree) additionally hosts DEBT-FID1-001 and
DEBT-FID1-002 as compiled C++.

## 4. Collisions (summary)

Full write-up: `CROSS_LANE_COLLISIONS.md`.

1. **Mailbox (highest risk).** Python RD5-X/RD6-A is a payload-byte
   outcome machine. This checkout only clears the retail table.
   UE0 does not implement `0x1C`/`0x1F` (fail-closed). Three
   treatments of `D_800A3180` / `func_800653B8`.
2. **`0x85`/`0x9C`.** UE0 hard-cuts. RD6-A waits 30 mode-2 ticks
   then gates on `(D_800BCFEE & 3) < 2`. Same opcodes.
3. **Persist width.** Python hop fixtures are 75 words. UE0 and
   this checkout's `func_80034F10` are 512 words. Harmless on the
   current prefix; live the moment m0005i / BTL1 writes
   `persist[0x50]` / `[0x54]` / `[0x64]`.

View-0 is a merge, not a fourth live collision. It becomes a
collision if UE0 later loads m0377i with always-view-0, which
RD7-R already forbids.

## 5. Highest-risk entry

**DEBT-FID1-004** (mailbox / task).

BTL0 proved the first Day 1 fight is reached from m0004i mailbox
3/4 volumes, not the RD6-A north hop. SYS0 already rules that
outcome-faithful mailbox **cannot** promote to Native/UE Parity
or Day1 Accepted once battle or save share the tables. The three
live treatments will disagree on that rung first.

## 6. Files

```text
docs/evidence/pe-debt1-fidelity-registry/REPORT.md
docs/evidence/pe-debt1-fidelity-registry/DEBT_REGISTRY.csv
docs/evidence/pe-debt1-fidelity-registry/PROMOTION_TRIGGERS.md
docs/evidence/pe-debt1-fidelity-registry/CROSS_LANE_COLLISIONS.md
```

---

```text
entry_count=33
nonblocking_fidelity_count=15
parked_blocker_count=9
unknown_label_count=9

entries_in_native_tree=3
cross_lane_collisions=3
merged_duplicates=1

highest_risk_entry=DEBT-FID1-004
highest_risk_reason=three live treatments of D_800A3180/func_800653B8; BTL1 mailbox 3/4 is the first Day 1 combat path; SYS0 already names the flip to BLOCKER when battle or save share task state

registry_ready=YES

hard_blockers=
unknowns=
warnings=do_not_repair_or_promote_in_this_rung; do_not_reopen_pt1_visual_freeze; persist_8slot_and_0_means_9_are_retired_not_live; matching_parks_and_6e_sdk_shims_are_out_of_scope; m0377i_dest_and_txt0_letter_map_are_proven_not_debt

SUCCESS

PE-DEBT1 SUCCESS — CROSS-LANE FIDELITY DEBT REGISTRY ESTABLISHED
```
