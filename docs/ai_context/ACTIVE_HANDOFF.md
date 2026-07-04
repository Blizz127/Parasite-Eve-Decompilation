# ACTIVE HANDOFF

Single source of truth for the current working state. Any agent or human
picking up this project reads this first and updates it after every
meaningful change.

## Current phase

**Phase 1 — disc verification and extraction. Disc 1 DONE, disc 2 pending.**
(Branch: `phase1-disc-verification`.)

## What exists right now

- Working extraction pipeline: `scripts/extract_us.sh [1|2|all]` drives
  three stdlib-only Python tools — `tools/extract/psxiso.py` (ISO9660
  reader for raw MODE2/2352 images), `tools/analysis/psxexe_info.py`
  (PS-X EXE header dump), `tools/verify/hashfile.py` (CRC-32/MD5/SHA-1).
- Disc 1 processed end-to-end; all results recorded in
  `docs/disc_info.md`. Extracted output lives only under
  `build/extracted/disc1/` (git-ignored).
- No disassembly, splat configs, symbols, or C exist. `split_us.sh`,
  `verify_us.sh`, `setup_env.sh`, and `configs/USA/*.yaml` are still
  placeholders.

## What is verified

- Disc 1 (SLUS-00662) local dump: image and EXE hashes, PS-X EXE header
  (pc0 `0x80072534`, t_addr `0x80010000`, t_size `0x1EE000`), SYSTEM.CNF
  boot line, and the full 25-file ISO9660 listing — see
  `docs/disc_info.md` for values and commands.
- Structural fact: almost all disc-1 game data is inside one packed
  archive, `PE.IMG` (206,213,120 bytes at LBA 1013). Format unexplored.

## What is NOT verified

- Whether the disc 1 dump matches redump (redump.org unreachable
  2026-07-04, ECONNREFUSED; disc page is http://redump.org/disc/116/).
- Disc 2: its image is now present under `rom/image/` but its results are
  deliberately NOT recorded yet (this pass was scoped to disc 1 only). A
  preliminary local run of `scripts/extract_us.sh 2` suggests
  `SLUS_006.68` is byte-identical to `SLUS_006.62` — re-run and record
  formally in the disc 2 pass before treating that as fact.
- Anything about the EXE's internals (no disassembly yet — Phase 2+).

## Next concrete step

1. Disc 2 pass: run `scripts/extract_us.sh 2`, fill in the disc 2 section
   of `docs/disc_info.md` (including the identical-EXE check and the
   disc 2 filesystem listing).
2. Re-attempt the redump cross-check for both discs and record it.
3. Only after both discs are recorded: start Phase 2 (splat config for
   `SLUS_006.62`).

## Open decisions

- License for original tooling/docs (see `docs/legal.md`).
- Exact splat version / Python toolchain pinning (decide in Phase 2;
  `scripts/setup_env.sh` will own installation).

## Rules reminder (never violate)

- No game data, images, extracted files, or SDK material in git.
- No invented decompiled C; no matching claims without the verify harness.
- Update this file after every meaningful change.

## Changelog

- 2026-07-04: Phase 1 disc 1 verification: implemented extract_us.sh +
  psxiso.py/psxexe_info.py/hashfile.py, extracted and hashed SLUS_006.62
  locally, recorded all disc 1 facts in docs/disc_info.md. Redump
  cross-check pending (site unreachable). Disc 2 pending.
- 2026-07-04: Phase 0 scaffold created (structure, docs, placeholders).
