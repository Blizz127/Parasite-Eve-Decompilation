# ACTIVE HANDOFF

Single source of truth for the current working state. Any agent or human
picking up this project reads this first and updates it after every
meaningful change.

## Current phase

**Phase 2 — splat config authored, toolchain installed locally, dry-run
guardrails pass; first real split not yet executed.** Phase 1 is complete
locally; only the official redump cross-check remains open
(non-blocking). (Branch: `main`.)

## What exists right now

- Working extraction pipeline: `scripts/extract_us.sh [1|2|all]` drives
  three stdlib-only Python tools — `tools/extract/psxiso.py` (ISO9660
  reader for raw MODE2/2352 images), `tools/analysis/psxexe_info.py`
  (PS-X EXE header dump), `tools/verify/hashfile.py` (CRC-32/MD5/SHA-1).
- Both discs processed end-to-end with the **fixed** script; all results
  recorded in `docs/disc_info.md`. Extracted output lives only under
  `build/extracted/` (git-ignored).
- The extract script's fail-loudly defect is fixed: every step now carries
  explicit `|| return 1` (immune to `set -e` suppression in the `||`
  caller context) and the per-disc output dir is wiped at run start so
  stale results can't masquerade as success. Failure tests re-run
  2026-07-04: garbage image → exit 1 with ERROR (previously exit 0 with
  stale "OK"), missing images → exit 1, cue-without-bin → exit 1.
- `configs/USA/disc1.yaml` now contains a conservative minimal initial
  splat config built from verified Phase 1 values;
  `configs/USA/disc2.yaml` is a documented byte-identical disc 2
  pointer/alias, not a separate active config. `scripts/split_us.sh` has
  tested fail-loudly gates but no split has been run. No generated asm,
  symbols, C, matching build, or PC-port work exists. `scripts/setup_env.sh`
  is now implemented (pinned `.venv/` install); `verify_us.sh` is still a
  placeholder.

## What is verified

- Disc 1 (SLUS-00662) and disc 2 (SLUS-00668) local dumps: image and EXE
  hashes, PS-X EXE headers, SYSTEM.CNF boot lines, full ISO9660 listings
  (25 and 31 files) — see `docs/disc_info.md` for values and commands.
- **The two boot executables are byte-identical** (SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` on both) — one EXE target
  covers both discs. Entry with evidence in
  `docs/reverse_engineering_notes.md`.
- **`PE.IMG` is byte-identical across both discs** (SHA-1
  `146c0ce7308bf9fdc2ba5a84230e198db0663f3b`, verified 2026-07-04): the
  discs differ only in FMV/XA streams and volume metadata. The decomp
  target is one EXE plus one archive. Archive format unexplored.
- Both image dumps are corroborated by an independent third-party catalog
  with identical hashes (GitHub `portforge/portforge-mediaitems`, found
  via code search for our SHA-1s; recorded in docs/disc_info.md). Not an
  authoritative redump verification.

## Phase 2 state (this branch)

- `configs/USA/disc1.yaml` is now a real minimal splat config: header +
  one conservative `asm` segment covering the whole loadable image
  (0x800–0x1EE800 at vram 0x80010000). Every number in it is a verified
  Phase 1 fact; no internal boundaries or symbols are claimed.
- `configs/USA/disc2.yaml` is a documented pointer to disc 1 (the EXEs
  are byte-identical), deliberately not a duplicate config.
- `scripts/setup_env.sh` is now implemented: creates a git-ignored
  `.venv/` and installs the pinned `splat64[mips]==0.41.0` (latest PyPI
  release as of 2026-07-05, when the pin was decided — see "Open
  decisions" below, now closed). Idempotent; has repo-root and git-repo
  sanity guards.
- `scripts/split_us.sh` gained a `--check` dry-run mode (verifies repo
  root, config, extracted EXE presence + SHA-1, splat availability via
  `.venv/bin/splat` first then `PATH`, and that every output path is
  `git check-ignore`d — prints what a real run would generate, invokes
  nothing). A real run now also diffs `git status` before/after and
  fails loudly if the split produced anything not git-ignored. Verified
  2026-07-07: `scripts/setup_env.sh` created `.venv/` with splat64
  0.41.0; `scripts/split_us.sh --check` passes all six gates on this
  machine (no split invoked, git status clean); `bash -n` clean on all
  four `scripts/*.sh`.
- `.gitignore` gained `undefined_funcs_auto.txt` / `undefined_syms_auto.txt`
  (splat's auto-generated symbol/function lists, written to repo root).
- `docs/splitting.md` documents the canonical target, verified values,
  unknowns, the pinned toolchain, the dry-run workflow, and the
  no-matching-claims policy.

## What is NOT verified

- Whether either dump matches redump.org itself (unreachable again on
  2026-07-04 retry, ECONNREFUSED; disc 1 page is
  http://redump.org/disc/116/). Non-blocking.
- Everything about the EXE's internals: text/data/bss boundaries, symbols,
  functions, overlays — no split output has been generated or inspected.
- The toolchain: `compiler: GCC` in the config is splat boilerplate, not a
  verified compiler identification (Phase 5+ fingerprinting).

## Next concrete step

1. Run `scripts/split_us.sh` for real (all prerequisites verified
   2026-07-07 via `--check`), then sanity-check the first disassembly
   around pc0 `0x80072534` for plausible MIPS prologue code.
2. When redump.org is reachable, record the official cross-check in
   `docs/disc_info.md`.

## Open decisions

- License for original tooling/docs (see `docs/legal.md`).
- ~~Exact splat version / Python toolchain pinning~~ — decided
  2026-07-05: `splat64[mips]==0.41.0`, owned by `scripts/setup_env.sh`.
  Revisit only with a deliberate, recorded reason to bump.

## Rules reminder (never violate)

- No game data, images, extracted files, or SDK material in git.
- No invented decompiled C; no matching claims without the verify harness.
- Update this file after every meaningful change.

## Changelog

- 2026-07-07: Phase 2 smoke test: `scripts/setup_env.sh` installed
  splat64 0.41.0 into `.venv/`; `scripts/split_us.sh --check` passed all
  gates (repo root, config, EXE SHA-1, splat, gitignore coverage). No
  split invoked; no asm/symbols/C generated or committed; git status
  clean.
- 2026-07-05: Phase 2 guardrails: implemented `scripts/setup_env.sh`
  (pinned `.venv/` install of `splat64[mips]==0.41.0`); added `--check`
  dry-run mode plus a post-split `git status` diff check to
  `scripts/split_us.sh`; added splat's root-level auto-generated symbol
  list files to `.gitignore`; documented the toolchain and dry-run
  workflow in `docs/splitting.md`. No split executed (splat still not
  installed on this machine); no asm/symbols/C generated or committed.
- 2026-07-04: Phase 2 started: authored minimal splat config for
  SLUS_006.62 (verified values only, single conservative segment),
  disc2.yaml documented as pointer to disc 1, split_us.sh implemented
  with tested fail-loudly gates, docs/splitting.md added. No split run
  yet (splat not installed); no asm generated or committed.
- 2026-07-04: Closed the PE.IMG hypothesis — verified byte-identical
  across discs (SHA-1 146c0ce7...). Redump retry still ECONNREFUSED;
  recorded independent third-party hash corroboration for both dumps.
- 2026-07-04: Fixed extract_us.sh fail-loudly blocker (explicit per-step
  error handling + fresh output dir per run; failure paths re-tested).
  Disc 2 verification pass recorded in docs/disc_info.md. First verified
  RE observation: both boot EXEs byte-identical. Redump cross-checks
  still pending for both discs.
- 2026-07-04: Phase 1 disc 1 verification: implemented extract_us.sh +
  psxiso.py/psxexe_info.py/hashfile.py, extracted and hashed SLUS_006.62
  locally, recorded all disc 1 facts in docs/disc_info.md. Redump
  cross-check pending (site unreachable). Disc 2 pending.
- 2026-07-04: Phase 0 scaffold created (structure, docs, placeholders).
