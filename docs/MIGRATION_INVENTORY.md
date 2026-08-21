# Parasite Eve UE5 — Phase 0 migration inventory

Generated 2026-08-17. Read-only discovery. No remotes changed, no history rewritten,
no worktrees discarded.

Target (empty, private): https://github.com/Blizz127/parasite-eve-ue5  
Confirmed via `gh repo view`: `isEmpty=true`, `isPrivate=true`, `diskUsage=0`.

## Do not lose

These trees have local-only or dirty state. Do not reset / restore / stash / clean
/ delete / force-push / discard worktrees.

| Tree | Risk |
|---|---|
| `/home/blizz/dev/parasite-eve-port-black` | 142 commits ahead of `origin/phase6e-b-provider-frontier`; 171 dirty/untracked paths; origin is **not GitHub** |
| Decomp worktrees under `/home/blizz/dev/parasite-eve-port-black-worktrees/` | 51 registered worktrees; several dirty; many branches have **no upstream** |
| `/home/blizz/dev/pe-worktrees/pe-decomp-btl` | local `battle-decomp-ch1` (unpushed matching C) |
| `/home/blizz/dev/pe-b54a-audit`, `pe-b53id-audit`, `pe-b54c-audit` | loose audit notes / extra git |

## Authoritative UE5 project

**Root:** `/home/blizz/dev/parasite-eve-port-black-worktrees/pe-ue1-native-presentation-wrapper/ue5/ParasiteEve`

| Field | Value |
|---|---|
| `.uproject` | `ParasiteEve.uproject` |
| Engine | **UE 5.8** (`EngineAssociation: 5.8`) |
| Git repo | `/home/blizz/Projects/Parasite-Eve-Decompilation` (worktree) |
| Branch | `feature/pe-ue1-native-presentation-wrapper` @ `bb9d4f3` |
| Upstream | **none** (not on GitHub `origin`) |
| Dirty | clean |
| Role | Presentation wrapper: Unreal owns lifetime, tick, pixels, input only |

Companion native runtime (same commit): `.../pe-ue1-native-presentation-wrapper/native/`  
(`pe_runtime`, field VM, pad, disc bootstrap, tests/fixtures).

`origin/main` of the decomp repo (`7467308`) does **not** contain `ue5/`.

### Other UE5 checkouts (same `.uproject`, older/thinner)

All live as decomp worktrees. Do not treat as a second project.

| Worktree | Branch | HEAD | Notes |
|---|---|---|---|
| `pe-ue1-native-presentation-wrapper` | `feature/pe-ue1-native-presentation-wrapper` | `bb9d4f3` | **canonical** — HUD + player controller |
| `pe-vis3-retail-body-prims` | `research/pe-vis3-retail-body-prims` | `950e75a` | 46 dirty; newer native/battle notes; thinner UE module |
| `pe-ue0-native-field-bootstrap` | `feature/pe-ue0-native-field-bootstrap` | `491caef` | 7 dirty; UE0 bootstrap |
| `pe-pt2-a-native-window` | `feature/pe-pt2-a-native-window` | `1f76370` | native window playtest |
| `pe-vis1g-ortho-debug` | `feature/pe-vis1g-ortho-debug` | `19455b5` | debug |
| `pe-anim0*` / `pe-anim0b-*` | feature/* | various | anim experiments; some dirty |

Main decomp checkout `/home/blizz/Projects/Parasite-Eve-Decompilation` is
`research/pe-rd5b-m0004i` @ `1b8a223` and has **no `ue5/` directory**.

## Git repositories

### 1. Decomp (matching C + native field + UE wrapper)

- Path: `/home/blizz/Projects/Parasite-Eve-Decompilation`
- Remote: `https://github.com/Blizz127/Parasite-Eve-Decompilation.git` (**public**)
- Default on GitHub: `main` @ `7467308`
- Local HEAD: `research/pe-rd5b-m0004i` @ `1b8a223`
- Worktrees: **51** (listed by `git worktree list`)
- Unpushed (sample): `battle-decomp-ch1`, `research/pe-vis3-retail-body-prims`,
  `feature/pe-ue1-native-presentation-wrapper`, `feature/pe-anim0-subobject-assembly-anim1`, …
- Dirty worktrees: `pe-anim0-subobject-assembly` (32), `pe-vis3-retail-body-prims` (46),
  `pe-ue0-native-field-bootstrap` (7), `pe-anim0b-collapse-guard` (4),
  `pe-rd2m-field-movement-core` (1), `pe-rd3r-m0003i-lobby` (2)

This repo already has a retail-safe `.gitignore` (`*.bin`, `rom/image/*`,
`local/retail_disc/`, `assets/*`). **Keep it as the matching/decomp authority.**
Do not vendor it into `parasite-eve-ue5`.

### 2. Port-black (current battle / BTL series)

- Path: `/home/blizz/dev/parasite-eve-port-black`
- HEAD: `phase6e-b-provider-frontier` @ `bca1e02` — `PE-BTL96`
- Remote `origin`: `ovh-dev:/home/blizz/dev/parasite-eve` (path **missing locally**)
- Ahead of that origin: **142 commits**
- Dirty/untracked: **171** paths (evidence CSVs, BTL oracles, `local/*.py`, `pc_port/`)
- **No `ue5/`**
- **No merge-base** with the decomp repo (separate histories)
- Object count: 431 vs decomp 324

This is the live battle-research workspace (`docs/ai_context/ACTIVE_HANDOFF.md`
starts at PE-BTL96). Evidence packs under `docs/evidence/pe-btl*` and
`docs/evidence/pe-battle-*` must be preserved.

`pc_port/` here is a native battle experiment, **not** the UE5 production runtime.
Import contracts/oracles/docs; do not make UE5 depend on compiling `pc_port`.

### 3. Progress dashboard

- `/home/blizz/Projects/parasite-eve-progress`
- Remote: `https://github.com/Blizz127/parasite-eve-progress.git` (public, `gh-pages`)
- Clean @ `fc9a7ec`

### 4. Wiki mirror

- `/home/blizz/Projects/Parasite-Eve-Decompilation.wiki`
- Remote implied GitHub wiki. Separate history. Keep separate.

### 5. Non-canonical / do not import as the UE project

| Path | Why |
|---|---|
| `/home/blizz/Games/banshee-library/games/parasite-eve` | Banshee game library / `parasite-eve-port` binary |
| `/home/blizz/Projects/workbench/samples/parasite-eve` | Workbench sample seeder |
| `/home/blizz/dev/pe-b54*-audit` | Loose audit scratch |

## Project-owned content that should reach GitHub (UE5 repo)

From **UE1 worktree** (decomp history, branch `feature/pe-ue1-native-presentation-wrapper`):

- `ue5/ParasiteEve/` (`.uproject`, `Config/`, `Source/`, empty `Content/.gitkeep`)
- `native/` runtime + `native/tests/`
- project `docs/`, `tools/`, `scripts/`, `tests/` that are not retail payloads
- evidence already committed on that branch

From **port-black** (after classification, no retail bytes):

- `docs/evidence/pe-battle-*`, `pe-btl*`, `pe-ch1-*`, destination lifecycle CSVs
- `docs/ai_context/ACTIVE_HANDOFF.md`
- repository-safe oracles under `pc_port/tools/pe_btl*_oracle.py` (scripts only)
- `docs/MIGRATION_INVENTORY.md` (this file)

## Licensed retail — do not commit

| Item | Path | Class |
|---|---|---|
| Disc 1 image | `Parasite-Eve-Decompilation/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin` | LICENSED_RETAIL |
| Disc 1 cue | same dir `.cue` | LICENSED_RETAIL |
| Disc 2 | `.../Disc 2/*.bin` | LICENSED_RETAIL |
| Disc 1 SHA-256 | `7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4` | matches goal bootstrap hash |

`local/pe_disc1.path` on port-black points at the decomp Disc 1 `.bin`.

Also ignore: `rom/`, `assets/` extracts, `asm/`, `build/`, `local/retail_disc/`,
`local/btl0/` dumps if they contain payloads, STR/FMV, PE.IMG, SLUS binaries.

## Large-file audit (>25 MiB, skipped rom/build/Intermediate/DDC)

No project-source files over 25 MiB were found in:

- port-black
- decomp main checkout (excluding `rom/` / `build/` / `asm/`)
- UE1 and vis3 worktrees (excluding skipped dirs)
- parasite-eve-progress

The 473 MiB Disc 1 `.bin` is under `rom/image/` (already gitignored).

## `.gitignore` gaps for a UE5-first repo

Decomp/port-black ignore retail images and extract/build, but **do not** yet
cover Unreal generated trees:

- `Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, `.vs/`, `.idea/`

port-black does **not** ignore `local/` (many untracked research scripts).

## Current playable / battle status (as found)

- **Field playable:** native field on decomp feature branches (UE0/UE1 wrap the native runtime). UE5 is a thin 5.8 wrapper, not a full reimplementation.
- **Battle:** active on port-black `phase6e-b-provider-frontier` (PE-BTL96). Not hosted inside the UE1 wrapper yet.
- **Matching decomp:** public GitHub `Parasite-Eve-Decompilation` `main`; local research branches ahead.

## Recommended import plan (not executed)

1. Keep `Parasite-Eve-Decompilation` on GitHub as matching/C authority. Push or document unpushed decomp branches **without** discarding dirty worktrees.
2. Preserve port-black history (142 local commits + 171 dirty files) before any remote rewrite. Attach a backup remote if needed; do not force-push.
3. Create `parasite-eve-ue5` history from UE1 (`bb9d4f3`) filtered to `ue5/` + `native/` + repo-safe docs/tools, **or** a new root that copies those trees without nested `.git`.
4. Selectively import port-black `docs/evidence/pe-btl*` and oracles after a retail/secret scan.
5. Add UE `.gitignore`, README/docs set, `ci.yml` (repo-safe tests), `ue-build.yml` (self-hosted `linux,x64,parasite-eve-ue5`).
6. Disc bootstrap: user-supplied image, verify SHA-256 above, generate ignored assets. Never upload the `.bin`.

## UE1 import classification (Phase 0b)

Secret scan (`AKIA*`, `ghp_`, `github_pat_`, PEM/SSH private keys) over UE1,
port-black, and decomp (excluding `.git` / `rom` / `build`): **no hits**.
No tracked `.bin`/`.cue`/`.iso` on the UE1 branch.

| Class | Decision | What |
|---|---|---|
| PROJECT_SOURCE | INCLUDE | `ue5/ParasiteEve/**` (16 tracked files), `native/**` (41 files) |
| PROJECT_SOURCE | INCLUDE | `native/tests/fixtures/*.csv`, `*.sha256` |
| DOCS | INCLUDE | `docs/**/*.md`, `docs/**/*.csv` (hashes/offsets/contracts) |
| LICENSED_DERIVED | **EXCLUDE from first push** | 183 tracked PNG/PPM evidence renders, **94.1 MiB** (retail pixels) |
| LICENSED_RETAIL | EXCLUDE | `rom/`, Disc 1/2 `.bin`/`.cue` (Disc 1 SHA matches goal) |
| GENERATED | EXCLUDE | `build/`, Unreal `Binaries/Intermediate/Saved/DerivedDataCache` |
| TEMPORARY | EXCLUDE | port-black `local/*.py` dumps until individually reviewed |
| PC_PORT | DO NOT VENDOR as UE runtime | port-black `pc_port/` — import oracles/docs only |

UE1 `PeFieldGameMode` mounts disc via env `PE_DISC1_BIN` and fail-closes if unset.
Native CTest `pe_pt2_headless_smoke` is labeled `retail_disc;playable`.
Hosted CI must run only fixture tests (`pe-ue0-tests` / `tests/test_ue1_wrapper.py`
if they stay disc-free).

## Acceptance (Phase 0 only)

| Check | Status |
|---|---|
| Target repo empty + private | YES |
| UE5 root identified | YES — UE1 worktree, UE 5.8 |
| Retail disc located, not staged | YES — SHA matches goal |
| Histories preserved (no reset/force) | YES |
| GitHub push | NO (later phase) |
| CI / runner / fresh clone | NO (later phase) |
