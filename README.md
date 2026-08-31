# Parasite Eve Decompilation

A work-in-progress reverse engineering, matching decompilation, and native
runtime research project for the original Sony PlayStation version of
**Parasite Eve** (Square, 1998).

Initial target:

- Region: USA / NTSC-U
- Disc 1: `SLUS-00662`
- Disc 2: `SLUS-00668`

## Project status

**Matching decomp:** **380 exact matching C leaves**, plus **24
`ACCEPTED-RESIDUAL` leaves** whose semantic C and compiler-decision residuals
are documented but are not counted as matching C. Authoritative count:
`grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml`.
Disc 1 EXE rebuilds byte-for-byte via `scripts/build_us.sh`
(SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`).
See [`docs/acceptance/MATCHING_RESIDUAL_POLICY.md`](docs/acceptance/MATCHING_RESIDUAL_POLICY.md).
`func_8001F814` remains assembly (`NONMATCHING_C`); native battle ports
of that body are not matching decomp leaves.

**Native / PC-port research:** in-tree under `pc_port/`. This is a
host-safe guest-RAM runtime with translated boot/battle leaves, oracles,
and a test suite. It is **not** a complete playable game, **not** a
complete battle teardown, and **not** first-Eve-boss complete.

**UE5:** a separate consumer repo. Gameplay semantics live here.

See [`docs/project_plan.md`](docs/project_plan.md) for the roadmap and
[`docs/ai_context/ACTIVE_HANDOFF.md`](docs/ai_context/ACTIVE_HANDOFF.md)
for the current working state. Public progress:
https://blizz127.github.io/parasite-eve-progress/

## Repository roles

| Repo | Role |
| --- | --- |
| [Blizz127/Parasite-Eve-Decompilation](https://github.com/Blizz127/Parasite-Eve-Decompilation) | Retail / matching-decomp / native gameplay authority |
| [Blizz127/parasite-eve-ue5](https://github.com/Blizz127/parasite-eve-ue5) | Presentation consumer of promoted native behavior |

Do not add Unreal Engine source here. Do not duplicate gameplay
research into the UE5 tree.

## Repository layout

```text
configs/USA/     Splat/spimdisasm split configs (per disc)
docs/            Project documentation and research notes
docs/ai_context/ Handoff state for AI-assisted sessions
docs/acceptance/ Day-1 / fidelity / UE-native contract
include/         C headers (as decompilation progresses)
rom/image/       User-supplied disc images — NEVER committed
scripts/         Reproducible extract/split/verify/build entry points
src/             Matching decompiled C leaves only
pc_port/         Native PC-port / battle-runtime research
asm/, assets/, build/   Generated locally; ignored by git
```

## Getting started — matching rebuild

You must provide your own legally obtained copies of the game discs.

```bash
# 1. Place Disc 1 bin+cue under rom/image/
scripts/setup_env.sh
scripts/setup_era.sh
scripts/extract_us.sh 1
scripts/split_us.sh
scripts/verify_us.sh
# 2. Matching rebuild (needs mipsel-linux-gnu-{as,gcc} or docker)
docker run --rm -v "$PWD":/workspace -w /workspace pe-mipsel:trixie \
  bash scripts/build_us.sh
```

`scripts/build_us.sh` is a match only when the rebuilt candidate EXE
SHA-1 equals retail `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Compilation alone is not a match.

Count matching C leaves from yaml, not from `src/` or native ports:

```bash
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
```

## Getting started — native runtime

```bash
cmake -S pc_port -B pc_port/build
cmake --build pc_port/build -j"$(nproc)"
./pc_port/build/pe-native-tests
```

Sanitizer build (ASan + UBSan):

```bash
cmake -S pc_port -B pc_port/build-san -DPE_PORT_SANITIZERS=ON
cmake --build pc_port/build-san -j"$(nproc)"
./pc_port/build-san/pe-native-tests
```

Battle oracles live in that same suite (`PE_TEST_FILTER=BTL120`, etc.).
They do not claim a complete battle system.

Disc-backed native oracles read a local path file
`local/pe_disc1.path` (git-ignored) or `PE_DISC1_BIN`.

## Project principles

1. Every phase must be reproducible from scripts and configs in this repo.
2. Every claim (matching status, disc layout, symbols) must be backed by a
   command, checksum, symbol map, disassembly, or documented observation.
3. No game images, extracted game data, or proprietary SDK files are ever
   committed.
4. Small commits with exact, descriptive names.
5. Native C ports are not matching decomp leaves.

## Legal

This repository does not contain game images, copyrighted assets, extracted
game files, or proprietary SDK files. Users must provide their own legally
obtained copy of the game. See [`docs/legal.md`](docs/legal.md).
