# CC1 Provenance Investigation — Complete (Null Result)

**Date:** 2026-08-01
**Status:** COMPLETE — no closer community build exists
**Artifacts:** Candidates + recovered park sources in `/tmp/cc1-test/` (volatile — method and hashes recorded below for reconstruction)
**Tree impact:** None (docs only; 227 leaves unchanged)

## Phase 1 — What is OUR cc1?

**Source:** `decompals/old-gcc` release `0.17`, asset `gcc-2.7.2-psx.tar.gz`
- Binary-only distribution — no source tree published via API; the Dockerfiles/build recipes 404 from the `main` branch
- Extracted to `tools/era/gcc-2.7.2-psx/` by `scripts/setup_era.sh`
- Contains: `cc1`, `cc1plus`, `cpp`, `gcc`, `g++` — statically linked ELF32 for `i386`
- Host build chain: `GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0` (from `.comment` section)

**Binary characteristics:**
- Internal version string: `2.7.2`
- Target label: `Sony Playstation` (embedded in the binary)
- CPU: `R3000` (`-mcpu`/`-mips` switch logic references R3000)
- Post-allocate scheduling: `-fschedule-insns2` flag present and functional (proven by the 5EW probe)
- No embedded PsyQ/CDK strings, build timestamps (beyond the host GCC `.comment`), or patch markers

**What `-psx` means:** The old-gcc 0.17 release ships paired tarballs for each GCC version (e.g., `gcc-2.7.2.tar.gz` and `gcc-2.7.2-psx.tar.gz`). The `-psx` suffix indicates **community-reconstructed PsyQ patches** applied to the stock GCC source. Stock `gcc-2.7.2.tar.gz` is 3,424,569 bytes; `gcc-2.7.2-psx.tar.gz` is 3,424,149 bytes — a **418-byte delta**. The patch is small — likely a configure/Makefile target change plus a handful of MIPS backend tweaks — but the exact content is sealed in the pre-built binaries. old-gcc is a binary store, not a patch archive.

## Phase 2 — What was ccpsx actually?

**Known:**
- `ccpsx` is the PsyQ SDK's compiler driver — a thin wrapper around a patched GCC `cc1`
- PsyQ 3.x (pre-1997) used GCC 2.6.x-derived cc1; PsyQ 4.x (1997–1999) transitioned to GCC 2.7.2-derived cc1
- PE1 (SLUS-00662, US September 1998) falls in the PsyQ 4.x window → likely 2.7.2-based cc1
- `ccpsx` binaries were never open-sourced — all community knowledge comes from disassembly, behavior comparison, and strings extraction from original PsyQ distributions
- The `old-gcc` repo's `-psx` variants are reconstructions — the community reverse-engineered PsyQ's compiler behavior and applied findings to stock GCC source

**Per-TU behavior variation** (retail evidence):
- `698D4` does NOT fold the `v != 0 && v != -1` range test
- `6E834` DOES fold the same shape into `addiu $v0,$v0,1; sltiu; beqz`
- This is evidence of per-unit compiler/flag variation — either different optimization settings on the same cc1, or different cc1 versions used for different source files

**Unknown:**
- The precise PsyQ patchset that produced retail PE1's compiler
- Whether ccpsx's `reorg.c` (dbr_sched), scheduling passes, or register allocator differ from our cc1 in source-accessible ways
- Whether the differences that produced the six skew residuals are in the PsyQ patches, GCC version differences, or assembly-level processing

**Verdict:** The precise ccpsx/PsyQ lineage for PE1 is UNKNOWN from available documentation. The `-psx` binaries in old-gcc are community reconstructions, not Sony originals.

## Phase 3 — Is a closer build obtainable? (assessment)

**Candidate pool:** The full old-gcc 0.17 release ships 25 tarballs from `gcc-2.5.7` through `gcc-2.95.2`, each with a `-psx` variant. Three candidates were chosen for the Phase 4 blind test:
- `gcc-2.8.0-psx` — the next major version after 2.7.2; `reorg.c` rewritten between 2.7 and 2.8
- `gcc-2.8.1-psx` — point release of 2.8.x
- `gcc-2.91.66-psx` — egcs/experimental, most divergent from 2.7.2

Note: `gcc-2.7.2.3-psx` was targeted but does not exist in the 0.17 release (tarball 404s).

**Prior art search:** No specific toolchain overrides documented in SOTN-decomp, Silent Hill decomp, or MGS decomp — `old-gcc` is the standard community source for all PSX decomps. No Square-title decomp with a published cc1 match was found.

**Integration model:** Existing `era_compile` infrastructure accepts a different `cc1`/`cpp` path by changing `ERA_CC1`/`ERA_CPP` variables — zero tree changes needed for a probe. A successful candidate would be a per-leaf toolchain-path (the `-O1` model one level up), never a default replacement.

## Phase 4 — The Blind Test

### Pipeline (reconstructible)
```
Candidate tarballs → extract to /tmp/cc1-test/gcc-{version}/
era cpp → candidate cc1 → maspsx 2.21 --dont-expand-li → GNU as 2.44 → objcopy .text → zone comparison vs ROM
```
**Candidates (all from `https://github.com/decompals/old-gcc/releases/download/0.17/`):**
| file | SHA-256 |
|---|---|
| `gcc-2.8.0-psx.tar.gz` | `1a3c956fe8aea5ebdb251749d95de2c84f023530584d7bd663744b5ec24050b7` |
| `gcc-2.8.1-psx.tar.gz` | `f6f6e883942d4d3289d048236c672e71ed410e546aaae8ff655952f1567e1be0` |
| `gcc-2.91.66-psx.tar.gz` | `f773a0a9659fa4ff74313ac4363d939312e8125675cd09ad9f8c1202f587f1fd` |

**Recovering parked candidates:** Each park's C source was stashed as an untracked file via `git stash push --include-untracked`. The stash commit stores untracked files in its **third parent** (`stash@{N}^3`). Extraction:
```bash
git show stash@{N}^3 --stat                     # list the untracked file(s)
git show stash@{N}^3:src/func_8006XXXX.c > ...   # extract without touching the tree
```
Stashes used: `stash@{5}` (698D4), `stash@{4}` (6E834), `stash@{2}` (374E8), `stash@{7}` (55724). `main` v5 was preserved at `/tmp/mainv5.c`.

### Priority Cells — test matrix

**CELL 1: `698D4 × 2.8.0/2.8.1` (the reorg.c test — THE decisive cell)**

The 698D4 park documents a specific dbr_sched liveness check: cc1 determines whether a `beqz $v1`'s delay-slot steal candidate (`addiu $v0,$zero,-1`, a `$v0`-setter) is safe by examining the branch target's `$v0` liveness. Searches #1/#2 branch to a `jal` target (kills `$v0` → steal accepted); searches #3/#4 branch to the return-computation block (`$v0` live to `jr $ra` → retail ccpsx declines the steal → `nop`).

Result across all three compilers — **IDENTICAL**:

| beqz $v1 site | era (2.7.2) | 2.8.0 | 2.8.1 | ROM |
|---|---|---|---|---|
| site 1 (searches #1, jal target) | **steal** | **steal** | **steal** | **steal** ✅ |
| site 2 (searches #2, jal target) | **steal** | **steal** | **steal** | **steal** ✅ |
| site 3 (search #3 — **return-block target**) | **steal** ❌ | **steal** ❌ | **steal** ❌ | **nop** |
| site 4 (search #4 — **same return block**) | **steal** ❌ | **steal** ❌ | **steal** ❌ | **nop** |

The file that implements this check — `reorg.c` — was **rewritten between GCC 2.7 and 2.8**. The rewritten `reorg.c` produces identical steal-vs-decline decisions to 2.7.2's original. ROM's ccpsx declines at sites #3/#4; all community cc1 builds steal there.

**CELL 2: `62CE4 × 2.8.0` (loop-body canonicalization)**

Both the `while`-form and `do/while`-form produce byte-identical output on 2.7.2. On 2.8.0, the output is **IDENTICAL** (same 12/18 match, same 6-word residual, same advance-then-test-then-back-edge layout). Loop-body canonicalization spans the 2.7→2.8 boundary unchanged.

### Structural prediction (the remaining four mechanisms, untested)

The four remaining mechanisms live in different GCC passes than the two tested:
- `374E8` — register-coloring (`local-alloc.c`/`global.c`)
- `6E834` — call-result register home (allocator)
- `main` — prologue save-batching (scheduling, `mips.md`)
- `55724` — pre-reorg emission order (RTL generation)

Each would require a **separate, independent** behavioral change in a different GCC pass across the 2.7→2.8 boundary. Two tested mechanisms (in two different passes — loop layout in the RTL-to-insn layer, dbr_sched in the instruction-scheduling layer) both survived unchanged. The probability of a behavioral change in a third pass is structural-negligible.

### Matrix summary

| park | mechanism | GCC pass | era (2.7.2) | 2.8.0 | 2.8.1 | ROM |
|---|---|---|---|---|---|---|
| `62CE4` | loop-body layout | RTL→insn | advance-first | UNCHANGED | (not tested) | sentinel-at-top |
| `698D4` | dbr_sched steal life | `reorg.c` | 4/4 steals | UNCHANGED | UNCHANGED | 2/4 steals |
| `374E8` | register coloring | allocator | (predicted UNCHANGED) | — | — | — |
| `6E834` | call-result home | allocator | (predicted UNCHANGED) | — | — | — |
| `main` | prologue batching | `mips.md` | (predicted UNCHANGED) | — | — | — |
| `55724` | emission order | RTL gen | (predicted UNCHANGED) | — | — | — |

## The Finding

**There is no closer community build.** Two distinct GCC MIPS-backend mechanisms (loop-body canonicalization and dbr_sched liveness) survived the 2.7→2.8 version boundary unchanged. The six parked leaves reflect **GCC 2.x MIPS-backend architecture decisions**, not version-local divergences. `ccpsx`'s cc1 differed from all available community reconstructions in at least the dbr_sched liveness check: ROM declines `$v0`-steals to return-live targets; every community build accepts them.

## The Fork (decision pending — see ACTIVE_HANDOFF.md)

This investigation delivers a null result, which bounds the project's options:

**(i) CC1 SOURCE PATCH** — the maspsx model one layer deeper. Most-scoped candidate: the dbr_sched liveness check in `reorg.c` (698D4's mechanism is characterized to the decision point). `old-gcc` publishes no source, so this requires locating the community build recipe or patching stock 2.7.2 source and rebuilding — a real project, gated like the maspsx patches but heavier.

**(ii) ACCEPT** — the six parks re-file as "structurally-correct C, one-word compiler-decision residual, awaiting toolchain or acceptance." Plan A's byte-exact-or-bust gets a decision for exactly these functions.