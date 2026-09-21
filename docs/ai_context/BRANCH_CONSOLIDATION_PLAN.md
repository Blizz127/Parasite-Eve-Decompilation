# Remote branch consolidation plan (2026-09-21)

**Status: scouted and classified. No branch has been merged yet.**

The remote carries **39 branches** on top of a snapshot-style history — `main` had a
single commit (`2cd7293a`) until `040f8fc3` today, which is why every branch looks
"hundreds of commits ahead" while their *content* is largely already in main.

Method: `git diff --name-status main origin/<branch>` and count the `A` (added =
exists in the branch, absent from main) entries. "ADDED" is the raw insert line
count of the same diff and overstates unique work, because it also counts main's
newer content as context on the branch side; **UNIQUE (file count) is the honest
column.**

## Tier 1 — contained: nothing to merge (5)

Their content is already in main; 0 added files.

| branch | verdict |
| --- | --- |
| `sync/laptop-phase6e-b` | contained — safe to archive/delete |
| `phase6e-b-provider-frontier` | contained — safe to archive/delete |
| `verify/post-merge-main` | contained — safe to archive/delete |
| `sync/vps-phase6e-b` | contained — safe to archive/delete |
| `phase5fm-main-barrier-revisit` | contained — safe to archive/delete |

## Tier 2 — the `agent/pb-*` family is ONE workstream, not five (5 → 1)

`pb-dispatch`, `pb-multiunit`, `pb-small`, `pb-19de4`, `pb-120d8` each show ~550
unique files, but they overlap **545–553 of ~550** with each other, and a sampled
shared file has an identical md5 in all of them. They are parallel copies of the
same agent session.

- **Single merge candidate:** `agent/pb-multiunit` (571 unique files — the superset).
- The other four need no separate merge; verify each is a subset before discarding.

## Tier 3 — substantial unique work (2)

| branch | unique files | character |
| --- | --- | --- |
| `cursor/cd-sector-backpressure-6f51` | 798 | 309 docs, 297 `pc_port`, 161 `src` — the largest real delta; **this is the branch checked out on `alienware-bazzite`** (`21a24a82`) |
| `wip/laptop-portblack-20260821` | 177 | 110 are `local/` (machine-local scratch), 41 docs, 24 `pc_port`, plus `.qoder/settings.local.json` and `.cursor/debug-*.log` — the editor-junk files are candidates for `.gitignore`, not for merging |

## Tier 4 — near-contained deltas (≤19 unique files each)

~20 branches, mostly the 2026-09-09 `cursor/*-6f51` and `dig/*` families. Each is a
handful of files, so they are cheap to inspect individually. Largest of them:
`dig/b0cd0-catchup-miss` (19), `dig/b0cd0-no-bfrd` (18), `dig/cd-sector-overrun` (17),
`cursor/movie-autonomous-stream-6f51` (16), `dig/mdec-decode-busy` (16),
`dig/909b4-early-title-cut` (15), `grind/continuous-decomp` (14),
`docs-github-wiki-mirror` (13), `dig/91dc8-decdctout` (13).

## Recommended order

1. **Tier 1** — record the five contained branches (done above); nothing to do.
2. **Tier 3 first, not Tier 2** — `cursor/cd-sector-backpressure-6f51` is the largest
   real delta *and* is the branch a live machine is sitting on, so consolidating it
   retires the most risk per unit effort. Do it as a `git merge --no-commit` into a
   scratch branch and review the 798 files by directory, not as a blind merge.
3. **Tier 2** — merge `agent/pb-multiunit` once, after confirming the other four are
   subsets.
4. **Tier 4** — batch the small deltas; most should be cherry-pickable by path.
5. **Tier 3 laptop branch last** — strip `local/`, `.qoder/`, `.cursor/` first, then
   merge only the docs/`pc_port` parts.

## Why nothing was merged yet

A merge of these branches into main is not a fast-forward: they hang off the old
snapshot and main has since absorbed overlapping content by other routes. Every one
of them will conflict, and the conflicts are *semantic* (which copy of a doc is
current) rather than mechanical — that is a per-file judgement call, which is why
this is a plan rather than a result. The safe first execution step is Tier 1
(already verified) plus a scratch-branch trial merge of `cd-sector-backpressure-6f51`.

## Machine note

`alienware-bazzite` is checked out on `cursor/cd-sector-backpressure-6f51`; the vps
clone `~/dev/parasite-eve` is stale on `2cd7293a` and should `git pull` before any
of this is compared against it. `matts-macbook` (offline) and `macserver` (key
refused) have not been read.

---

## RESULT 2026-09-21: Tier 3 #1 merged to main — `cursor/cd-sector-backpressure-6f51`

**Status: DONE and gate-passed.** `main` fast-forwarded to `8e0dc06b`.

The branch turned out to be a **parallel matching-decomp line**, not a doc delta:
its YAML registered 910 c spans against main's 810, and the two row sets were
almost disjoint. The merge is therefore a **span-level union**, not a file harvest.

| | c spans | asm spans | funcs | c_words |
| --- | ---: | ---: | ---: | ---: |
| main before | 810 | 347 | 377/980 | 6,834 |
| **main after** | **964** | **401** | **521/1153** | **11,156** |

`EXACT_REBUILD_GATE=PASS` — `sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`VERIFY_SWEEP=PASS leaves=964`, `plan=5dcdb1d3d30d…`. Port suite unchanged at 1405/1405.

### What went wrong on the way (all four attempts are in the history)

1. **A blind file harvest breaks invariants.** Checking out the branch's 798
   "unique" files made `gen_decomp_ports --verify` report stale TUs, left all 161
   `src/*.c` unregistered in main's YAML, and wired 288 of 289 port files nowhere —
   because the branch's *YAML rows* were the missing half. Reverted.
2. **"Same address → main wins" silently dropped 36 leaves.** 58 rows share an
   address with a different *kind*; in ~50 of them main says `asm` and the branch
   says `c`. Keeping main's row discarded the branch's decompiled carve, which is
   what made the build assemble dispatch tables the C leaves also emit.
   Fix: rank `rodata > c > asm` at equal addresses.
3. **The rebuild gate caught what preflight only warned about.** The first union
   passed preflight with a WARN; the gate hard-failed on
   `ERROR: .rodata 0x20 != pool table 0x1C for jtbl_80010080`. Excluding that one
   leaf just moved the failure to `jtbl_80010AC8` — same class.
4. **The dispatch-fold family is a real incompatibility**, not a profile bug.
   `func_80012E7C`, `func_8002FE78`, `func_8003010C`, `func_8004AE1C`,
   `func_80051CC4`, `func_800C3238` each build under a per-leaf
   `MASPSX_DISPATCH_FOLD=jtbl_X` profile and each fails the pool-table check
   whether or not the profile is applied. They are **excluded** from the merge and
   remain on the branch — this is the open follow-up.

### Next for this branch

- Resolve the jump-table/pool-table carve for the 6 dispatch leaves (they are worth
  ~6 more c spans, and `func_8004AE1C` is also the field-menu input tree's root in
  the port, so it is worth having).
- Then re-run the union including them; expect 970 c spans.
