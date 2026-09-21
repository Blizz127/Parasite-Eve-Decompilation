# Plan: 4-hour native-port continuation (CD stream + executed-path C)

Paste into the prompt:

```
/goal Continue the native PC port for ~4 hours: close remaining live-boot CD/DMA/stream gaps, grow executed-path native C on the disc-1 boot→Day-2 route, keep two-disc identity honest, and never treat matching-C parks or a MIPS interpreter as progress.
```

## Goal kind
code-change

## Objective (one sentence)
A disc-1 native port that can stream from the user-supplied BIN without B0CD0/overrun hangs, with more of the **executed** boot→Day-2 path in `pc_port/` C, two coverage numbers kept honest, and Disc 2 treated as a real second image.

## Acceptance criteria
1. **CD/stream is still the first blocker.** No regression of DAY2-158: unread sector + `D_800B0CD0` holds; idle MDEC output DMA retries `func_8007C564` and BFRDs; clear unread sector still overruns. If a new live-boot CD/DMA3/FIFO gap is found, it is fixed with a test in `pc_port/tests/` (not a comment-only patch).
2. **Executed-path native C moved or an honest stop.** Either (a) at least one on-route function that was only asm/stub is now a real `pc_port/game/{boot,decomp}/func_*_port.c` translation used by the boot/field path, **or** (b) the remaining top fan-in holes are documented as non-C (GTE/syscall/jr-t2) or as a named port residual with a test. Matching YAML `c` without a native TU does **not** count.
3. **Two numbers in the log.** `python3 tools/analysis/route_coverage.py --quiet --no-history` still prints both `funcs=`/`c_words=` (rebuild matching C) and `native_c=`/`native_c_words=` (executed-path native C), with `disc=1`. Native C count must not drop.
4. **Disc 1 gameplay baseline is the regression pin.** `docs/generated/DISC1_GAMEPLAY_BASELINE.md` stays the contract (`pe-route-boot-day2-tests` → `m0027i`). The harness still rejects `PE_Disc_BootKind==2`. If `PE_DISC1_BIN` or `local/pe_disc1.path` exists, run the harness (or a shorter frame cap if 62k is too long) and record the stop/story/token in `{SCRATCH}/route_baseline.log`. If the disc is absent, record that skip — do not invent a pass.
5. **No game data in git.** No ISO/BIN/CUE/CHD, no extracted `SLUS_*`, no `asm/` / `rom/image/` / Psy-Q. `docs/legal.md` holds.
6. **No interpreter/recompiler** added to boot sooner.

## Verification plan
1. gating: `pc_port/build/pe-native-tests DAY2_cd_sector_device` and `DAY2_cd_dma` (filter still runs the suite; required: those two names PASS). Capture `{SCRATCH}/cd_sector.log` and `{SCRATCH}/cd_dma.log`.
2. gating: `python3 tools/analysis/route_coverage.py --quiet --no-history` → `{SCRATCH}/route_coverage.txt`. Required keys: `disc=1`, `native_c=`, `funcs=`. Compare native_c to the handoff snapshot (258/979 at start of this goal); must not decrease.
3. gating: If a new native leaf was added, it has a test or is on the route (name it in ACTIVE_HANDOFF). `grep` the new `func_*_port.c` from CMake/`pe_field_runtime` so it actually links.
4. gating: Disc-1 baseline: `{SCRATCH}/route_baseline.log` either contains a harness run (token/story/stop) or `SKIP no disc1 image`. Harness source still rejects boot kind 2.
5. evidence: Update `docs/ai_context/ACTIVE_HANDOFF.md` first section: what shipped, commands, native_c numbers, next CD/stream hole. Do not claim matching-C progress unless `check_leaf.sh` + SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Non-goals
- 100% matching decomp; YAML `c` carves of parked `func_8006AD40` / `func_80069B08` / `func_8001220C` residuals.
- M34 win, Day-2 `persist[74]=0x80`, or pixel-perfect FMV.
- Inventing a disc-2 route file without a driven path.
- MIPS interpreter or dynarec.
- Committing game data.

## Assumed scope
- `pc_port/platform/pe_cdreg.c`, `pe_disc.c`, `pe_mdec.c`, stream ports under `pc_port/game/boot/`
- `pc_port/tests/test_cd_*.h`, `test_movie_*.h`, `test_route_boot_day2.c`, `test_native.c`
- `tools/analysis/route_coverage.py`, `docs/generated/DISC1_GAMEPLAY_BASELINE.md`
- `docs/ai_context/ACTIVE_HANDOFF.md`, `DAY2_CD_SECTOR_DEVICE.md`
- User disc via `PE_DISC1_BIN` / `local/pe_disc1.path` when present

## Implementation approach (work order, stay on this list)
1. Re-run CD tests; if red, fix before anything else.
2. Next CD hole from `DAY2_CD_SECTOR_DEVICE.md` and live stops: DMA3/FIFO, default device attach, movie player `121C04` wait loops, `HostFB_StreamTick` vs field CD reads. One gap + one test per step.
3. Translate the highest-fan-in **executed-path** remaining native hole (`route_coverage.py --top` intersected with functions the port actually calls). Correct C + differential test; byte-match only CD/DMA/GPU/GTE/memory.
4. Two-disc: keep `PE_Disc_SetActive` as swap; if field code searches `PEDISC02.IDF`, prove it against the in-memory fixture (already `disc_findfile_disc2_idf`) and do not pretend Disc 1 `PE.IMG` is Disc 2.
5. If ~4h is up: stop on a green CD pair + updated handoff + coverage line, even if native_c only moved by one leaf.

## Task checklist
- [ ] Confirm DAY2-158 tests still PASS (`DAY2_cd_sector_device`, `DAY2_cd_dma`).
- [ ] Pick the next live CD/DMA/stream stop from evidence or a failing boot; fix with a test.
- [ ] Land at least one executed-path native C leaf **or** document why the top remaining are non-C/port-residual.
- [ ] Re-print `route_coverage.py --quiet --no-history`; native_c did not drop.
- [ ] Disc-1 baseline file + harness disc-2 reject still true; run harness if disc present.
- [ ] Update ACTIVE_HANDOFF; no game data; no interpreter.

## Risks / Contradictions
- An older `/goal` asked for 100% matching C starting at bootstrap. **This goal overrides that:** the port is the deliverable. Matching parks stay parked.
- `pe-native-tests <filter>` still executes every `TEST()` function (filter only skips bodies). Budget time; do not wait on a 62k-frame route run unless the disc is present and CD tests are already green.
- Sibling matching workers may edit `disc1.yaml`/`src/`; do not fight them unless they break the native link.

## Time box
About **four hours** of agent work. Prefer a smaller green increment over an unfinished megaleaf. When the time box is exhausted, ship whatever of the checklist is done and leave a single next command in the handoff.
