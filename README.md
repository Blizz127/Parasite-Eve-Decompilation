# Parasite Eve Decompilation

A work-in-progress reverse engineering and decompilation project for the
original Sony PlayStation version of **Parasite Eve** (Square, 1998).

Initial target:

- Region: USA / NTSC-U
- Disc 1: `SLUS-00662`
- Disc 2: `SLUS-00668`

## Project status

**Current phase: Phase 5CW — 98 matching C leaves** on branch
`phase5ae-2a0c-hole-aware`. Disc 1 EXE rebuilds byte-for-byte via
`scripts/build_us.sh` (SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`).
Phase 1 local verification is complete; the official redump.org cross-check
remains open (non-blocking). PC port is out of scope.

**Progress dashboard:** https://blizz127.github.io/parasite-eve-progress/

See [`docs/project_plan.md`](docs/project_plan.md) for the roadmap and
[`docs/ai_context/ACTIVE_HANDOFF.md`](docs/ai_context/ACTIVE_HANDOFF.md) for
the current working state. Public mirror:
[GitHub Wiki](https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki).

Regenerate/publish the dashboard:

```bash
python3 tools/progress/generate_progress.py
python3 tools/progress/render_dashboard.py
scripts/publish_progress.sh
```

## Goals

1. Document the original executable and disc layout.
2. Build reproducible extraction and splitting tools.
3. Create a matching/rebuildable decompilation workflow.
4. Gradually replace assembly with readable C.
5. Eventually explore native runtime / PC-port experiments once enough
   systems are understood.

## Repository layout

```text
configs/USA/     Splat/spimdisasm split configs (per disc)
docs/            Project documentation and research notes
docs/ai_context/ Handoff state for AI-assisted sessions
include/         C headers (as decompilation progresses)
rom/image/       User-supplied disc images — NEVER committed
scripts/         Reproducible extract/split/verify entry points
src/main/        Decompiled C source (matching only)
tools/           Extraction, analysis, and verification tooling
asm/, assets/, build/   Generated locally; ignored by git
```

## Getting started

You must provide your own legally obtained copies of the game discs. Place
disc images under `rom/image/` (git-ignored). Tooling and setup scripts are
placeholders until Phase 1.

## Project principles

1. Every phase must be reproducible from scripts and configs in this repo.
2. Every claim (matching status, disc layout, symbols) must be backed by a
   command, checksum, symbol map, disassembly, or documented observation.
3. No game images, extracted game data, or proprietary SDK files are ever
   committed.
4. Small commits with exact, descriptive names.

## Legal

This repository does not contain game images, copyrighted assets, extracted
game files, or proprietary SDK files. Users must provide their own legally
obtained copy of the game. See [`docs/legal.md`](docs/legal.md).
