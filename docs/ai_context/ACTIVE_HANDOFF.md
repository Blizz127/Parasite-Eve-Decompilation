# ACTIVE HANDOFF

Single source of truth for current working state. Read this first; update after
every meaningful change. Prefer shortening over accruing.

## Current state

| Fact | Value | Derive |
| --- | --- | --- |
| Branch / tip | `phase5ed-sbret0-batch` @ `a6036a0` + dirty | `git log --oneline -1` / `git status` |
| Phase | **5ED** | `scripts/verify_us.sh` summary |
| Matching C leaves | **170** | `grep -c ',\s*c,' configs/USA/disc1.yaml` |
| Yaml asm segments | **127** | `grep -c ',\s*asm\]' configs/USA/disc1.yaml` |
| Era leaf compiles | **14** | `grep -c '^era_compile ' scripts/build_us.sh` |
| Target SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` | `scripts/build_us.sh` compare |
| Progress | https://blizz127.github.io/parasite-eve-progress/ | `scripts/publish_progress.sh` |

**127 is yaml `asm` segments, not remaining functions.** One segment can hold
dozens of glabels; do not subtract it from anything as a function count.

Oracle: bare `scripts/build_us.sh` exits 0 on exact SHA-1; `scripts/verify_us.sh`
reports Phase 5ED / 170. Disc images / `asm/` / `build/` / `tools/era/` are
git-ignored inputs — never commit them.

**Toolchain**

- Default leaves: GCC 14.2 in Distrobox `pe-mipsel` (Phase 4J flags; selective
  `-G 8` / `-fno-delayed-branch` / `-fno-tree-ter`).
- Era leaves (opt-in): `scripts/setup_era.sh` → `era_compile` =
  cpp → cc1 → maspsx → GNU as, typically `-O2 -G0`.
- Era maspsx: `ERA_ASPSX_VER=2.21` + `--dont-expand-li`. **Why:**
  `expand_load_immediate` turns positive small `li` into `ori`; ROM wants
  `addiu`. Defer `li` expansion to GNU as. Do **not** bump aspsx-version
  casually — that also flips `nop_at_expansion` / `addiu_at`.
- Maspsx stdin: closed with `</dev/null` in `era_compile` (non-TTY hang under
  agent sockets). Bare `scripts/build_us.sh` is fine.

## How to count (do not hand-maintain)

```bash
grep -c ',\s*c,' configs/USA/disc1.yaml            # C leaves
grep -c ',\s*asm\]' configs/USA/disc1.yaml          # yaml asm segments (NOT fn count)
grep -c '^era_compile ' scripts/build_us.sh         # era leaf compiles
git log --oneline -1
```

**Do not** count `asm/disc1/*.s` from disk. That tree is git-ignored and
contains orphans, stale duplicates, and nop-pads. **Yaml is the source of truth.**

## Proven era fingerprints (evidence, not claims)

| Fingerprint | Status |
| --- | --- |
| `move` → `addu` in delay slot | Proven 5EA / 5EB / 5EC / 5ED |
| `$v0` / `$v1` allocation | Proven 5EC / 5ED (sb+ret0 reuse) |
| `li` const materialization (`addiu` not `ori`) | Proven 5EC via `--dont-expand-li` |
| `lui;ori` / `$at` address macros | **NOT YET PROVEN** — next real test |

The “~290 era-blocked functions” figure from the 5EA commit message is an
**ESTIMATE**, partially supported by fingerprints above — **not** a verified
inventory. Do not treat it as a countdown.

## Known-open families

- **sb+ret0:** **done** in 5ED (seven remaining harvested; family closed).
- **`$at` setters:** raw-grep size often quoted ~113 — **UNVERIFIED** (includes
  orphans/dups if taken from disk). Shape typically
  `lui $at,%hi; …; sw …` — era/`lui;ori` probe still open.
- Complex `$gp` / GTE / BIOS / mult-div / large non-leaves: still open; not
  inventoried here.

## Standing policy

1. **PROBE BEFORE GRIND.** The two biggest unblocks (maspsx stdin hang;
   `expand_load_immediate` forcing `ori`) were short diagnostics, not
   integrations. When a family is blocked, diagnose before more members.
2. **Homogeneous families may be batched.** Risk lives in the first member.
3. **`asm/` is not a source of truth for counts.** Use `configs/USA/disc1.yaml`.
4. **Commit messages are not evidence.** A claim is proven when a gate is green
   and the leaf is objdump-probed (not SHA alone on carves).

## Resolved blockers

- **Phase 5I** delay-slot (`move`/`or` vs `addu`): **SOLVED in 5EC** by era.
- **Maspsx non-TTY hang:** **SOLVED** (`</dev/null` in `era_compile`).

## History (append-only, truncated)

| Phase | Leaves | What it proved |
| --- | --- | --- |
| 4I–4J | 0→1 path | Exact asm rebuild; GCC 14.2 first leaf |
| 5B–5CW | →98 | Empty stubs, getters, store/setter batch |
| 5CX–5DB | →103 | Countdown memset/memcpy (`$2`/`$3` pins) |
| 5DC–5DJ | →156 | `$gp` small-data (`_gp`+`-G 8`); `-fno-tree-ter` |
| 5EA | 157 | Era dual-toolchain; return-0 `addu` |
| 5EB | 161 | Return-0 twins via mid-segment holes |
| 5EC | 163 | sb+ret0; `--dont-expand-li`; 5I dead |
| 5ED | 170 | sb+ret0 batch harvest (family closed) |

Detail and leaf-by-leaf narrative: git history + wiki
([Current Status](https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki/Current-Status)).
PC port remains out of scope. Redump.org cross-check still open (non-blocking).
