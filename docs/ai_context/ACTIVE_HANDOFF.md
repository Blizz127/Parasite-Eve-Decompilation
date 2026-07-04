# ACTIVE HANDOFF

Single source of truth for the current working state. Any agent or human
picking up this project reads this first and updates it after every
meaningful change.

## Current phase

**Phase 0 — scaffold. COMPLETE as of this commit.**

## What exists right now

- Repository scaffold: docs, ignore rules, placeholder configs and scripts.
- No disc has been verified. No executable has been extracted. No
  disassembly, symbols, or C exist. All configs/scripts are non-functional
  placeholders that fail loudly when run.

## What is verified

Nothing. There are no verified facts about the binaries yet. Do not trust
any claim about PE1 internals that is not recorded with evidence in
`docs/reverse_engineering_notes.md` or `docs/disc_info.md`.

## Next concrete step (Phase 1)

1. Obtain user-supplied NTSC-U disc images, place under `rom/image/`
   (git-ignored).
2. Implement `scripts/extract_us.sh`: verify image hashes against redump,
   extract `SLUS_006.62` / `SLUS_006.68`, record all hashes and PS-X EXE
   header fields in `docs/disc_info.md`.
3. Record the ISO9660 file listing for both discs in `docs/disc_info.md`.

## Open decisions

- License for original tooling/docs (see `docs/legal.md`).
- Exact splat version / Python toolchain pinning (decide in Phase 2;
  `scripts/setup_env.sh` will own installation).

## Rules reminder (never violate)

- No game data, images, extracted files, or SDK material in git.
- No invented decompiled C; no matching claims without the verify harness.
- Update this file after every meaningful change.

## Changelog

- 2026-07-04: Phase 0 scaffold created (structure, docs, placeholders).
