# CLAUDE.md — AI agent guidance for this repository

This is a PS1 decompilation research project for **Parasite Eve** (USA,
NTSC-U: SLUS-00662 / SLUS-00668). It is structured like a serious matching
decomp (compare: Xenogears decomp, PE2 decomp), not a PC port. Read
`docs/project_plan.md` for the phase roadmap and
`docs/ai_context/ACTIVE_HANDOFF.md` for the current working state **before
doing anything**.

## Hard rules

1. **Never invent decompiled C.** Only add C that is verified against the
   original binary by the checksum/rebuild harness (Phase 4+). Until that
   harness exists, do not add code under `src/` at all.
2. **Never claim matching progress** unless verified by checksum/build
   tooling, with the exact command recorded.
3. **Never commit game data**: no ISO/BIN/CUE/CHD images, no extracted files,
   no assets, no proprietary Psy-Q/PsyQ SDK files. `rom/image/`, `assets/`,
   `asm/`, and `build/` are git-ignored on purpose — keep them that way.
4. **Every claim needs evidence**: a command, checksum, symbol map,
   disassembly excerpt, or documented observation. Record it in `docs/`.
5. **Every phase must be reproducible** via scripts in `scripts/` and
   configs in `configs/`. User-supplied disc images live under `rom/image/`
   locally and are inputs, never outputs.

## Working conventions

- Update `docs/ai_context/ACTIVE_HANDOFF.md` after every meaningful change:
  what was done, what was verified, what the next concrete step is.
- Prefer small commits with exact, descriptive messages.
- The Parasite Eve 2 decomp (GabeRealB/parasite-eve-2-decomp) is a
  **structural reference only** — study its layout and tooling flow, but do
  not copy source or configs blindly; PE1 is a different binary.
- Scripts must be idempotent and fail loudly (`set -euo pipefail`).
- Python tooling goes in `tools/`; keep it dependency-light and pinned.

## Current phase

**Phase 2 — initial Splat/spimdisasm infrastructure.** Phase 1 local
verification (both discs, extraction, hashes) is complete and corroborated;
the official redump.org cross-check remains open and non-blocking. No
disassembly output, matching build, decompiled C, or PC port exists yet.
`docs/ai_context/ACTIVE_HANDOFF.md` has the exact current state and
`docs/splitting.md` the split target and policy.
