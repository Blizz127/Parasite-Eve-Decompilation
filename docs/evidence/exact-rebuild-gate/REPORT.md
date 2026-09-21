# Exact-rebuild gate — re-proof and operationalization

Status (2026-09-11, session cont. 5): the exact packed rebuild was **re-proven
on the current live tip** and the sequence was **institutionalized** as a
single repeatable command, `scripts/exact_rebuild.sh`, documented in
`docs/ai_context/EXACT_REBUILD_GATE.md`.

Two independent PASSes were captured:

**A. Live tree (the strongest result).**

```
EXACT_REBUILD_GATE=PASS plan=8f7e8b3f394b98e9027230412b23a9abaf4e30833ec9246b9c87e89ca892216f spans=[606 c, 295 asm, 2 rodata] sha1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

| | value |
|---|---|
| YAML SHA-256 | `e403face61e178c3dbffd4e3b4630cbf4a2dd3d9c3cca443006077c9bfbc751a` |
| plan SHA-256 (full) | `8f7e8b3f394b98e9027230412b23a9abaf4e30833ec9246b9c87e89ca892216f` |
| spans | 903 = 606 c + 295 asm + 2 rodata |
| both SHA-1s | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` (`orig` and `cand`) |
| `RESULT` | `EXACT MATCH` |
| `verify_us.sh` | `VERIFY_US=PASS`, 7/7 |
| race handling | split ignore-check raced once; `--allow-split-race` continued, and the YAML hash was identical at all four checkpoints |

The candidate on disk after the run (`build/disc1.candidate.exe`) hashes to
retail, independently confirming the packed image.

**B. Frozen snapshot (601 c), for cross-checking the snapshot recipe.**

```
EXACT_REBUILD_GATE=PASS plan=9f77a5bb7b77b11c2b28da6f822fd295fae9ba48ed189b64c5fef6e76af6c581 spans=[601 c, 296 asm, 2 rodata]
```

| | value |
|---|---|
| YAML SHA-256 | `d4dbb5a9c8122c3f6c9e6117ab15483bb8fdfdb88d44faf4cb4a5b37f85bb0d7` |
| plan SHA-256 (full) | `9f77a5bb7b77b11c2b28da6f822fd295fae9ba48ed189b64c5fef6e76af6c581` |
| spans | 899 = 601 c + 296 asm + 2 rodata |

The tree advanced ~30 leaves during this session (578 → 606 c), so a live-tree
proof has to win a quiet moment; when it does not, the frozen snapshot below is
the fallback. The gate prints the plan SHA-256 and pins the YAML hash through
all three steps, so the proven state is unambiguous either way.

### Snapshot recipe (when live is being edited)

```bash
SNAP=/tmp/pe-gate-snap
rm -rf "$SNAP"; mkdir -p "$SNAP"
# rsync everything except the huge/generated dirs; then copy (not symlink,
# because disc1_build.py resolves __file__ and a tools symlink would point the
# build at the live tree) tools/ and the extracted retail image.
```

The one non-obvious requirement: `tools/` and `.venv/` must be **real copies or
absolute-path symlinks resolved correctly**, because
`tools/build/disc1_build.py` sets `ROOT = Path(__file__).resolve().parents[2]`;
a relative symlink is followed and the build reads the *live* YAML while the
snapshot's splat writes the snapshot's `asm/`. `setup_mipsel_host.sh`'s shims
are relocation-safe, so a copied `tools/mipsel-host/` works correctly provided
the copied `usr/` tree travels with it.

## Race attempts and outcomes

| # | where | outcome |
|---|---|---|
| 1 | live `split` | exit 1, `split created files git does not ignore` (`src/func_8006DED4.c`, `src/func_8006DF50.c`) — sibling-created mid-run; splat completed, plan `bae734bb73c1` |
| 2 | live `split` | exit 1, same guard (`docs/evidence/func-8001A680/`, `src/func_8006DED4/`) — YAML advanced to 581 c mid-run |
| 3 | live `split` ×4 | exit 1 each; YAML hash changed during attempts 1–3; **attempt 4 was clean** (`6dc94541…`, plan `aa86e168edd3`, 583 c) |
| 4 | live `build` | **exit 0, `RESULT: EXACT MATCH`** on 583 c (hash stable through build) |
| 5 | live `verify` | exit 1 at gate 3: `YAML C sources are not tracked: src/func_8006DB9C.c, src/func_8006DBE0.c` — staging gap, **not** a build failure |
| 6 | live gate loop ×8 | exit 1 each, all `split race` (siblings creating `src/func_80075*.c` and `docs/evidence/func-80075*/` at 18:00, 18:03, 18:04, 18:05) |
| 7 | snapshot gate (stale) | exit 1, genuine link mismatch `func_80074D28` — see below |
| 8 | snapshot gate (refreshed) | **exit 0, `EXACT_REBUILD_GATE=PASS`** (601 c, plan `9f77a5bb7b77`) |
| 9 | live gate (clean window) | **exit 0, `EXACT_REBUILD_GATE=PASS`** (606 c, plan `8f7e8b3f394b`, YAML `e403face…` stable) |

No mismatch originating in a file this lane owns was observed; every live
failure was the documented split race or a staging gap, and the one genuine
link mismatch was a snapshot of a leaf that its owner fixed minutes later.

## Newly exposed link-level leaf failure (owner: matching worker)

The stale snapshot (captured 18:03, before the owner's 18:12 fix) is itself the
proof the gate detects real defects. Detail:

- **Leaf**: `func_80074D28`, `src/func_80074D28.c`
- **YAML span**: `[0x65528, 0x655C0)`, 38 words
- **Profile**: `era_o2_g0_fill_epilogue_delay_slot` (`-O2 -G0` +
  `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`)
- **Build failure**: `NON-MATCH: first byte at 0x65574: candidate=0xFF
  retail=0x6A` (candidate SHA-1 `d2323b67…`)
- **Object-level check**: clean bar the 9 expected relocation fields
  (`MISMATCHES=9`) — so the object-level check alone would have missed it
- **Linked divergence** (16 words), the two swapped branch delay slots:

```text
            candidate (wrong)          retail
0x80074d70  bnez  s0, +0x10             bnez  s0, +0x10
0x80074d74  li    a1,-1                 addiu a0,s1,0x6A   <- delay slot
0x80074d78  addiu a0,s1,0x6A            li    a1,-1
...
0x80074d84  lui   v0,0x8009             lui   a0,0x300
0x80074d88  lw    v0,0(v0)              lui   v0,0x8009
0x80074d8c  bnez  s0, +0x10             lw    v0,0(v0)
0x80074d90  lui   a0,0x300              bnez  s0, +0x10
0x80074d94  ori   a0,a0,1               ori   a0,a0,1
0x80074d98  lw    v0,16(v0)             lui   a0,0x300
0x80074d9c  ...                         lw    v0,16(v0)
```

- **Root cause** (fixed by the owner at 18:12): the leaf's ternary operand
  order was `(s0 == 0 ? 0x3000001 : 0x3000000)`; the retail-matching order is
  `(s0 ? 0x3000000 : 0x3000001)`. Operand order is load-bearing — the
  `beqz`/delay-slot materialization shape depends on it (documented lever #3 in
  `docs/evidence/func-80074D28/REPORT.md`).
- **Status**: resolved in the live tree; the refreshed snapshot (601 c) is
  exact. Reported here as the worked example for the gate's failure taxonomy,
  not as an outstanding defect.

## The gate

`scripts/exact_rebuild.sh` — drivers on top of the existing entry points, no
semantics changed:

- Preflights the toolchain (via the same `find_toolchain()` resolver as
  `disc1_build.py`), the retail input + splat + ignore coverage (via
  `split_us.sh --check`), and the expected SHA-1, all **before** any expensive
  work.
- Runs split → build → verify, pinning the YAML SHA-256 at each step.
- Prints `EXACT_REBUILD_GATE=PASS plan=<64-hex> spans=[…] sha1=…` on success.
- Exit 0 PASS / 1 FAIL (step or race) / 2 ENV (missing prerequisite with the
  exact remedy).
- `--allow-split-race` continues past the documented split ignore-check race
  (used only when a sibling is mid-edit); strict by default.

### Proofs

| proof | command | observed |
|---|---|---|
| PASS | `scripts/exact_rebuild.sh` (snapshot) | `EXACT_REBUILD_GATE=PASS plan=9f77a5bb…` |
| loud ENV fail (toolchain) | `mv tools/mipsel-host{,.hidden}; scripts/exact_rebuild.sh` | `EXACT_REBUILD_GATE=ENV mipsel toolchain unavailable: …` + `remedy: scripts/setup_mipsel_host.sh`, exit 2 |
| loud ENV fail (splat) | `mv .venv/bin/splat{,.hidden}; scripts/exact_rebuild.sh` | `EXACT_REBUILD_GATE=ENV split prerequisites failed: ERROR: splat not found …`, exit 2 |
| real mismatch | snapshot that predated the owner's fix | `EXACT_REBUILD_GATE=FAIL build … owner=c func_80074D28 …`, exit 1 |

The toolchain and splat were restored immediately after each failure test and
the live tree verified intact (`tools/` is a real directory, `SLUS_006.62`
present, candidate SHA-1 still retail).

## Whole-tree gates (live, this session)

| gate | result |
|---|---|
| `scripts/exact_rebuild.sh` (snapshot) | `EXACT_REBUILD_GATE=PASS plan=9f77a5bb7b77…` |
| `scripts/exact_rebuild.sh` (live) | `EXACT_REBUILD_GATE=PASS plan=8f7e8b3f394b…` |
| `python3 tools/build/test_disc1_plan.py` | 8 tests OK |
| `python3 tools/build/disc1_plan.py --check` | `903 spans (606 c, 295 asm, 2 rodata), plan=8f7e8b3f394b` at the proof; the matching worker keeps adding leaves afterwards |
| `scripts/verify_us.sh --public` | `PUBLIC_VERIFY=PASS` at 588 c (plan `8af66b433ce5`); a mid-write profiles file transiently raised `unknown profile 'era_o2_g8_vm_dispatch'`, self-resolved on retry |
| `scripts/setup_mipsel_host.sh` re-run | `OK mipsel-host toolchain already installed` (no-op, exit 0) |
| pinned downloads | all 10 Debian trixie URLs resolve; all 10 SHA-256 match the script's pins |

## Fresh-clone usability audit

`scripts/setup_mipsel_host.sh` is sufficient to go from a clean checkout to a
passing exact rebuild, given the other pinned prerequisites:

1. `scripts/setup_env.sh` — `.venv` + pinned `splat64[mips]==0.41.0`.
2. `scripts/setup_era.sh` — era `gcc-2.7.2-psx` + vendored patched maspsx
   (fetches `decompals/old-gcc` 0.17 and `mkst/maspsx`, restores the tracked
   patched files from git).
3. `scripts/setup_mipsel_host.sh` — rootless GCC 14.2.0 / binutils 2.44 from
   Debian trixie into `tools/mipsel-host/`.
4. A user-supplied retail disc 1 image at `rom/image/`, then
   `scripts/extract_us.sh 1` → `build/extracted/disc1/SLUS_006.62`.

Verified this session: `tools/mipsel-host/.debs/` cache hashes match every pin,
the shims report `gcc-14 (Debian 14.2.0-13) 14.2.0` / `GNU as, ld 2.44`, and
the idempotent re-run is a no-op. `find_toolchain()` falls through host `PATH`
(empty here) → repo-local `tools/mipsel-host/bin` → distrobox → hard error
naming the remedy, so no `PATH` setup is required.

Gaps a new contributor could hit (all pre-existing, none blocking the gate):

- **`cmake` is not on `PATH` in this environment** (native `pc_port/` suite only;
  it resolves via `/tmp/pe-tools/env.sh`). Not needed for the exact rebuild.
- **`maspsx` is a network clone plus repo-tracked local patches.** If
  `tools/era/maspsx` is re-cloned and git is unavailable, `setup_era.sh` prints
  `WARNING: … local patches NOT restored`; the linked build then needs the
  patch-1/2/3 env knobs, so the gate would fail. The warning is loud but the
  script exits 0.
- **`extract_us.sh` requires a real disc image**; the gate preflights this and
  exits 2 with the remedy rather than failing deep inside the build.
- **Race with concurrent editors** is the dominant live failure mode; the
  documentation now records the frozen-snapshot recipe.

## What is proven / what is not

Proven: the registered tree at plan `9f77a5bb7b77` (601 c + 296 asm + 2 rodata)
round-trips **byte-exact** to the retail `SLUS_006.62` via the committed
sequence, on the rootless mipsel toolchain. The gate automates and enforces
that proof with a pinned plan SHA-256 and a `PASS`/`FAIL`/`ENV` taxonomy, and
its failure path is demonstrated both for missing prerequisites and for a
genuine link-level leaf mismatch.

Not proven / not complete: this is **recompilation exactness for the covered
spans**, not a 1:1 decompile of the whole boot→end-of-day-2 path. 298 spans
(296 asm + 2 rodata) remain reassembled rather than lifted to matching C, and
the native `pc_port/` runtime is incomplete where its evidence says so. An
exact rebuild is a necessary, not sufficient, milestone toward the persistent
goal.

---

# Session cont. 6 — continuous-matching usability + route coverage

Two follow-ups, both in the build-tooling / measurement lane. The gate work in
the previous section was not redone.

## 1. Gate usable while matching runs continuously

`scripts/exact_rebuild.sh` now:

- **Classifies the live split trip** instead of blanket-failing. Entries are
  parsed from the split's ERROR block; if none lies under a split
  `OUTPUT_PATH` (the list is read from `split_us.sh` itself — single source of
  truth), the trip is benign and the gate continues **without any opt-in flag**.
  A real violation (generated output no longer git-ignored) still FAILs, as
  does `--strict-split`. This removes the previous `--allow-split-race`
  requirement (kept as a no-op alias).
- **Pins the plan's inputs, not the whole tree.** The YAML SHA-256 is checked
  at five checkpoints (start / after split / after build / after verify / final,
  the last taken after the plan hash is printed) and a digest of `disc1.yaml` +
  `disc1_build_profiles.json` + every `c`-span source is checked before build,
  after build, and after verify. A sibling's new, not-yet-registered `src/*.c`
  draft is deliberately excluded — it is not an input to this build and cannot
  invalidate it. (An earlier iteration hashed every `src/func_*.c` and produced
  a false FAIL on exactly such a draft; the scoped digest fixes that.)
- **Prints a self-describing PASS line**: plan SHA-256, YAML SHA-256, span
  counts, **both** SHA-1s, and both checkpoint groups. A PASS cannot be misread
  as covering a later state.
- Emits the `ROUTE_COVERAGE=` line (informational; PASS semantics unchanged),
  and labels a verify staging gap (`YAML C sources are not tracked`) as such
  instead of a generic failure.

### Proofs (session cont. 6)

| proof | where | command | observed |
|---|---|---|---|
| **PASS** | live tree | `scripts/exact_rebuild.sh` | `EXACT_REBUILD_GATE=PASS plan=32b6fe6d9681… yaml=fcb3e8b7… spans=[610 c, 298 asm, 2 rodata] sha1_orig=452fb033… sha1_cand=452fb033…`, `VERIFY_US=PASS`, exit 0 |
| benign race, no flag | live tree | same run | split tripped on `?? docs/ai_context/ROUTE_COVERAGE.md` (the doc this lane had just written); gate printed `WARN split git-status trip … no split output path involved`, continued, all input checkpoints equal |
| benign race, sibling `src` | live tree | many later runs | tripped on `A src/func_*.c` / `?? docs/evidence/*` repeatedly; continued each time; only failed afterwards when a real unsoundness or leaf defect was present |
| **ENV** (loud) | sandbox copy | `mv <sandbox>/tools/mipsel-host{,.hidden}; <sandbox>/scripts/exact_rebuild.sh` | `EXACT_REBUILD_GATE=ENV mipsel toolchain unavailable: …` + `remedy: scripts/setup_mipsel_host.sh`, exit 2 |
| **real FAIL** | sandbox copy | reintroduce the `func_80074D28` ternary operand-order defect | `ERROR: NON-MATCH: first byte at 0x65593: candidate=0x16 retail=0x12; owner=c func_80074D28 [0x65528,0x655C0) profile=era_o2_g0_fill_epilogue_delay_slot`, `EXACT_REBUILD_GATE=FAIL build …`, exit 1 |
| **unsound-proof FAIL** | live tree | (observed, not injected) | `EXACT_REBUILD_GATE=FAIL race: YAML changed during the split (splat read a moving plan)`, exit 1 (runs 4, 6, 10, 15) |
| **verify race** | live tree | (observed, not injected) | attempt 11: split ok, **build `RESULT: EXACT MATCH`**, then verify `missing split-generated source(s): asm/disc1/68664.s` — a sibling split ran mid-run and clobbered this run's generated asm; labeled `verify: concurrent split race`, exit 1 |
| **real FAIL, sibling leaf** | live tree | (observed, not injected) | run 5: benign trip on `AM src/func_8005DB8C.c`, split ok; build then FAILed `NON-MATCH: first byte at 0x663FC … owner=c func_80075B84 [0x66384,0x66404) profile=era_o2_g0` — a matching defect in a sibling-owned leaf mid-batch, not this lane |

The live outcomes are the intended behaviour under continuous matching: the
*ignore* trip was benign and did not fail the gate; the gate failed only on a
real unsoundness (moving plan), a concurrent verify race, or a real leaf defect.

### Proofs (session cont. 7 — scoring the two hardenings)

The two unscored hardenings were re-proven this session. The **live tree could
not be scored** because a sibling-owned YAML/object size inconsistency
(`func_80077D30` span `0x94` vs compiled `.text 0x90`, see §"Sibling defect"
below) fails `build_us.sh` before the summary code. The **frozen-snapshot
fallback** was used and PASSed, scoring all five checkpoints and the route line:

```
EXACT_REBUILD_GATE=PASS plan=2bedd04d5cb5006838cb1d8fb479faac80db6bf137cd659fc8d808fadf655af8 yaml=3a6fde001ab6672458252d0ae0e9bb58f51ca64aa7c7d31a0c869e67d380d476 spans=[637 c, 311 asm, 2 rodata] sha1_orig=452fb033f2eaa4b18aa20a5bca60b8125af3a37b sha1_cand=452fb033f2eaa4b18aa20a5bca60b8125af3a37b sha1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
ROUTE_COVERAGE=plan=2bedd04d… funcs=316/979 c_words=5592 nonc_funcs=47 nonc_words=147 asm_funcs=616 asm_words=71954 asm_words_unknown=0 nonmatchable=present tierA=734 tierB=248 tierB_unresolved=0
```

| proof | where | command | observed |
|---|---|---|---|
| **PASS (both hardenings)** | frozen snapshot `/tmp/pe-snap-s3` | `scripts/exact_rebuild.sh` | PASS line above, `VERIFY_US=PASS`, exit 0; all five YAML checkpoints equal (`start=split=build=verify=final`), all three input digests equal; the fifth `final` checkpoint and the `ROUTE_COVERAGE=` line both printed |
| **verify-race label fires** | frozen snapshot | delete one generated `asm/disc1/*.s` after split, before verify | `ERROR: missing split-generated source(s); run scripts/split_us.sh: asm/disc1/11718.s` → `EXACT_REBUILD_GATE=FAIL verify: concurrent split race — …; re-run`, exit 1 |
| **ENV (loud), incidental** | frozen snapshot | run before `CLAUDE.md`/`.gitignore` were copied in | `EXACT_REBUILD_GATE=ENV not a Parasite-Eve-Decompilation root: /tmp/pe-snap-s3`, exit 2; also `ENV split prerequisites failed: splat not found`, exit 2 |

**Defect found and fixed in this lane:** the verify-race label grepped
`missing split-generated source`, but the other generator path
(`tools/build/disc1_verify.py::local_generated_checks`) emits
`missing split source(s):`. The label therefore missed that path. The grep is
now the common prefix `missing split`, and both producers are covered — proven
by the live-induced race above.

**Sibling defect (reported, not fixed — `configs/USA/disc1.yaml` is
sibling-owned):** `func_80077D30`'s YAML span is `0x94` bytes (`0x68530` →
next edge `0x685C4`), but the retail function ends at `0x685C0` (the two words
at `0x685B8`/`0x685BC` are the `jr $ra`/`nop` epilogue; `0x685C0` is zero
padding; `func_80077DC4` starts at `0x685C4`). The era compile of
`src/func_80077D30.c` emits `.text = 0x90`, so `trim_elf_section_pad.py` aborts
`ERROR: target size 0x94 > current 0x90` and `build_us.sh` fails. Remedy for
the owner: insert `- [0x685C0, asm]` before the `func_80077DC4` edge (leaving
the function's own span `0x90`) or otherwise align the span with the object.
The same inconsistency is why the live PASS could not be scored.

**Sibling defect 2 (reported, not fixed):** during this session the live
`configs/USA/disc1.yaml` became **syntactically invalid** — line 1749 is a
hard-wrapped comment continuation (` cursor by`) starting at column 0, which
`splat`'s YAML parser rejects (`did not find expected key … line 1753`). This
is why the final live attempts fail at `[1/3] split_us.sh` with
`split_us.sh exit 1`. (`tools/build/disc1_plan.py` parses the file with a
tolerant regex, so its `--check` still prints a plan — only the splat split
fails.) Remedy: rejoin the wrapped comment into a single `#` line.

**Live attempts this session (all `FAIL`, none sound):** one reached `build` and
failed on the `func_80077D30` size defect; one failed the split-window YAML
checkpoint (`race: YAML changed during the split` — the moving-plan classifier
working as designed); two failed at `split_us.sh exit 1` on the syntax defect 2
above; several earlier attempts were blocked the same way. Every failure was a
sibling tree-state or plan defect, never an unsound PASS — the gate refused to
certify a moving or inconsistent tree.

**Toolchain hiding was copy-based.** The ENV proof ran against `/tmp/pe-gate-sandbox`
(a full self-contained copy of the tree including its own `tools/mipsel-host/`),
so the **live** `tools/mipsel-host/` was never moved and never unavailable to
the sibling worker — zero downtime. Both the sandbox and the live copies were
confirmed present immediately afterwards.

The failure-mode work used the sandbox `/tmp/pe-gate-sandbox`: a copy-based
snapshot (real `tools/`, `.venv/`, `configs/`, `src/`, `asm/`, `docs/`, and the
extracted retail image) with its own `git init`, so its `split`/`build`/`verify`
do not race the live tree. Snapshot coherence matters: `configs/USA/disc1.yaml`
is copied **before** `src/`, so a sibling edit landing between the two copies
only adds an extra, unreferenced `src/` file, which the plan ignores. A snapshot
taken the other way around (or twice, minutes apart) fails with a real
`NON-MATCH` on a leaf the sibling fixed in between — which is itself a
demonstration of the gate working, but not the proof intended here.

## 2. Route-coverage tool (`tools/analysis/route_coverage.py`)

Measures the actual objective — the boot → end-of-Day-2 path — instead of the
disc-wide span count. Evidence-based route definition (root = initial PC
`func_80072534`; Tier A = direct `jal` closure with matched-C body references;
Tier B = `D_800910A0[word & 0x1FFF]` text targets read from the retail image).
A function is matched C iff named by a `c` span. Unresolved targets are
reported, never dropped.

Current output at plan `9806a40a7cbc…` (three buckets; non-C classification
consumed from the matching worker's `docs/evidence/non-c-matchable/MANIFEST.json`):

```
matched C:              316 / 979 functions, 5,593 words  (7.2%)
provably non-C:          47 functions, 147 words  (0.2%)
real remaining asm:     616 functions, 71,954 words  (92.6%)
tierA=734 tierB=248 tierB_unresolved=0 asm_words_unknown=0 nonmatchable=present
```

Every run appends an idempotent, append-only entry to
`docs/generated/ROUTE_COVERAGE_HISTORY.md` (default on; `--no-history` opts
out). The timestamp is derived from the plan SHA-256 rather than the wall
clock, so re-running at one plan is byte-identical while different plans get
distinct entries; history-write failures never break the report.

Top remaining-asm queue (on-path fan-in): `func_80079FB4` (19), `func_80073A44`
(17), `func_8003708C` (16), `func_8006DE80` (15), `func_8008CBA8` (12),
`func_80067CBC` (11). Full record in `docs/ai_context/ROUTE_COVERAGE.md`; the
tool supersedes the stale hand-written `BOOT_TO_DAY2_COVERAGE.md` snapshot
(which is now retained read-only and owned by a sibling).

## 3. Adversarial leaf audit (session cont. 7)

Eleven recently registered leaves were independently re-derived from the retail
disassembly (`mipsel-linux-gnu-objdump` over the extracted EXE at the YAML VMA)
and checked for control-flow, comparison polarity/operand order, constants,
signedness, loop bounds, pointer lifetimes, and global types. Every referenced
`D_*`/`func_*` symbol was resolved by its embedded address and confirmed to be
the address the retail instruction actually touches. `profile_necessity.py`
and the link-level `era_link_check.py` were run for each.

| leaf | verdict | evidence |
|---|---|---|
| `func_80077DC4` | **CONFIRMED** | retail `bgez`/`negu`/`andi 0xfff`; branches `0x801`/`0x401`/`0xC01`; tables `lh` at `D_8009589C`/`-D_8009509C`/`-D_8009589C`/`D_8009409C` all match the C. `LINK_EXACT`; profile PASS |
| `func_80077CF4` | **CONFIRMED** | `bltz`→`negu`+`andi 0xfff`→`jal 77D30`→`negu $v0`; C's `-func_80077D30((-a0)&0xFFF)` is exact. `LINK_EXACT`; profile PASS |
| `func_80077D30` | **CONFIRMED (C)** | C bounds/table/sign select the four quarter tables exactly as retail; `LINK_EXACT` at both `0x90` and `0x94`; profile PASS. (Its *YAML span* is the sibling defect above, not the C) |
| `func_80052E30` | **CONFIRMED** | `beqz a0` / `beqz` on `D_8009D04C`; then `D_8009D048=D_8009D04C`, `D_8009D058=&D_800A1F84`, `D_8009D064=4`, `D_8009D050=old D_8009D054` — matches C. Else `&D_800C0E48`, `func_80052F70()`, `&D_8009D05C`, `2` — matches. `LINK_EXACT` (forced-absolute profile); profile PASS |
| `func_8006F044` | **CONFIRMED** | six `sb -1` to `D_800B0DB5..B2`; `D_800B0CD8 &= 0xFF0F`; two read stages (`D_8009315E`/`D_80093166`, dest `D_8001160C`/`D_80011610`); poll clears `0xFEFFBFFF` on `r∈{-1,0}`; trio `72714/726C4/72724` twice. `LINK_EXACT`; profile PASS |
| `func_8006F2C4` | **CONFIRMED** | `sltiu 0x16`/`sltiu 0xB` splits at `D_800942E4`/`E8` (`lui`+`lw` = pointer globals); `lbu +1 == 0x72`; 7-word `D_800E10A0` clear; `& 0xFFFEFFFF`; `sb 00 FF FF FF` + 2 word clears. `LINK_EXACT`; profile PASS |
| `func_8006F820` | **CONFIRMED** | handler byte clamp `0x55`; null → `-0xF`; `bnez a1` (mode≠0) reads `p[0]` into `*arg`, `sltiu 6` gates the low-byte store; returns `p[0]`. `LINK_EXACT`; profile PASS |
| `func_8006F8EC` | **CONFIRMED** | `(p[0]-1)<2` guard; `>=0xC0`→`-0x11`; `D_800942E0[h]` null→`-0x12`; `lw +0xC` fn-ptr; `bnez v0`→`jalr` else `-1`. `LINK_EXACT`; profile PASS |
| `func_8006F224` | **CONFIRMED** | `sltiu 0xC0`→`-1`; `(id-0x46)<0xF` selects `D_800942E8` (slot `i+0xB`); else `D_800942E4` (slot `i`); first zero in-use byte; `0xB` probe. `LINK_EXACT`; profile PASS |
| `func_8006F6D4` | **CONFIRMED** | `D_800942E0[h]+8` fn-ptr (guard `-1`, call); `a1==1 && a2==0` writes `p[2]`/`p[3]`/`*(p+4)` to `*a3/*a4/*a5`; 6th/7th args from caller frame (`48/52(sp)`) per the 6-arg C signature. `LINK_EXACT`; profile PASS |
| `func_8006DB9C` / `func_8006DBE0` | **CONFIRMED** | signed-byte pair search at `D_800B0CD8+0xDC/0xDE`; `DB9C` returns the pair's `+1` byte, `DBE0` the index; both `-1` on miss. `LINK_EXACT`; profile PASS |

No `DEFECT` and no `UNCERTAIN` verdicts in this sample. All 11 are
object-level and link-level exact at the retail VMA, and all recorded profile
assignments are load-bearing (`PROFILE_NECESSITY=PASS` for each).

## What is proven / what is not (session cont. 7)

Proven: the reworked gate proves the retail round-trip self-consistently on a
frozen snapshot (all five YAML checkpoints and all three input digests equal),
and its verified behaviour classes — PASS, ENV (loud, exit 2), real
build-mismatch FAIL, benign-race auto-continue, and the verify-race label — are
each demonstrated. The route tool reproducibly measures the on-path split in
three buckets and records an append-only history. Eleven recently registered
leaves were independently re-derived and all 11 are `CONFIRMED`.

Not proven / not complete: (a) the **live-tree** PASS for this revision — the
shared tree is blocked by the sibling `func_80077D30` span defect and continuous
edits, so the evidence is the frozen-snapshot PASS with the stated plan hash;
(b) the boot → end-of-Day-2 goal. Only **≈7.2 % of the on-path code words** are
matched C, 147 words are provably non-C-matchable, and ~71,954 real asm words
remain reassembled; several route identities are still `RESEARCH_REQUIRED`; the
native `pc_port/` runtime is incomplete where its evidence says so. An exact
rebuild remains necessary, not sufficient.

# Session cont. 2026-09-11 — fast preflight validator

## Live-tree defect state (checked first, read-only)

The two sibling-owned `configs/USA/disc1.yaml` defects were **still present** at
the current tip (`plan=e584072c97f9`, 981 spans; later `991 spans (670 c)`):

1. **YAML syntactically invalid** — line 1779 held a hard-wrapped comment
   continuation at column 0 (`cursor by`), so `yaml.load(..., SafeLoader)`
   aborts with `expected <block end>, but found '<scalar>'`; splat would fail at
   `split_us.sh`.
2. **`func_80077D30` span `0x94`** (`0x68530`→`0x685C4`) while the era toolchain
   emits `.text = 0x90`, so `trim_elf_section_pad.py` aborts with
   `target size 0x94 > current 0x90`.

The deep preflight additionally exposed **three more** real defects that the
historical reports had not named:

3. **`func_80076B58` span `0x88`** (`0x67358`→`0x673E0`) but the era toolchain
   emits `.text = 0x40`. The carve swallowed `func_80076B98` (real code: 17
   words writing `D_80095854`/`D_80095858`/`D_8009585C`/`D_80095860` and
   returning via `jr ra`).
4. **`func_800906B4` span `0x68`** (`0x80EB4`→`0x80F1C`) but `.text = 0x30`.
   Swallowed `func_800906E4` (real code: `D_800B2A0C[...]` byte load, `+0x38`
   clear `& 0xFFFFFDFF`, `+0xF4` OR `0x4400`, `sh` to `+0x116`).
5. **`configs/USA/disc1_build_profiles.json`** assigned `func_800556E8` and
   `func_80058E08` to **two** profiles (`era_o2_g8` and the new
   `era_o2_g8_symbol_at_temp`); it also left `func_80058E08` stale at one point.
   The duplicate makes `--check` runnable but `profile_necessity` FAIL and the
   deep check abort. Hypothesis confirmed: assigning both to
   `era_o2_g8_symbol_at_temp` makes them compile to the full span (`PASS`).

All five are printed with the offending span/file/line and the exact remedy.

## The validator (`tools/build/disc1_preflight.py`)

Fast mode (default): `yaml-syntax` (same loader as splat, with caret),
`yaml-sha1`, `geometry` (gaps/overlaps/duplicate VMA/ascending/edges/kind and
VRAM-name agreement), `c-source`, `profile-ref`. Runtime **<1 s**.

Deep mode (`--deep`, **on by default in the gate**): compiles every `c` leaf
with its exact per-leaf profile and compares emitted `.text` to the declared
span. ~**6–8 s** for ~500–670 leaves, 12-way parallel. Two finding classes:

- `deep-size` — span exceeds compiled `.text` (the `0x94`-vs-`0x90` class);
  prints the exact replacement edge, e.g.
  `end the span at 0x685C0 (move the following span/edge up 0x4)`.
- `deep-pad` — compiled `.text` exceeds the span and the tail is **non-zero**
  (the carve swallowed real code).

If the era/mipsel tooling is missing, deep mode skips with a `WARN` rather than
misclassifying an ENV problem as a config FAIL.

Wired into `scripts/exact_rebuild.sh` as step 4, before `split_us.sh`; it can
only add an early FAIL. `--preflight-fast` / `EXACT_REBUILD_GATE_PREFLIGHT=fast`
skip the deep check; `=off` skips the whole preflight.

### Five seeded-failure proofs

Sandbox `/tmp/pp/root` (symlinked `tools`/`include`/`src`, synthetic configs):

| # | seeded defect | finding |
|---|---|---|
| i | hard-wrapped comment continuation (`raw continuation` at col 0) | `yaml-syntax …:25:1: expected '<document start>' …` + caret + remedy |
| ii | `func_80077D30` span `0x94` vs compiled `0x90` | `deep-size … exceeds compiled .text 0x90 by 0x4; trim_elf_section_pad.py will abort with target size 0x94 > current 0x90` → `end the span at 0x685C0 (move the following span/edge up 0x4)` |
| iii | duplicated VMA `0x8744` | `duplicate VMA … (first declared on line 139)` + `offset 0x8744 does not ascend past line 139` + VRAM-name mismatch |
| iv | `c` span with no `src/` file | `c-source … has no src/func_DEAD0000.c` + remedy |
| v | assignment to unknown profile / stale symbol | `unknown profile 'era_no_such_profile'` and `stale assignment … -> 'func_DEADBEEF'; no such YAML C span` |

Unit regression suite: `tools/build/test_disc1_preflight.py` (8 tests, all OK);
usable alongside `tools/build/test_disc1_plan.py` (8 tests, OK).

### Current-tree results

- **Live tree**: `disc1_preflight: FAIL 4 finding(s)` in **8.5 s** — the
  `yaml-syntax` blocker plus the three `deep-size` over-carves. The gate stops
  at `EXACT_REBUILD_GATE=FAIL preflight: invalid build inputs` in 8.5 s instead
  of after a ~30-minute split/build cycle.
- **Repaired frozen snapshot** (`/tmp/pe-score`): `PASS (deep, 670 c / 322 asm /
  2 rodata)`.

### Frozen-snapshot gate PASS (with preflight enabled)

The snapshot repaired all five defects (three `asm` spans inserted at
`0x67398`/`0x685C0`/`0x80EE4`, the wrapped comment joined, the two patch-5
leaves moved to `era_o2_g8_symbol_at_temp`) and then ran the full gate:

```
EXACT_REBUILD_GATE=PASS plan=45ccaeaf68890ccae2495c4000021be660604485ceea7941139c37ae1408e3ab yaml=103ac05028dbae08f3266ff71a544b443926ec0c5eff9c9574c81ea43033a5d7 spans=[670 c, 322 asm, 2 rodata] sha1_orig=452fb033f2eaa4b18aa20a5bca60b8125af3a37b sha1_cand=452fb033f2eaa4b18aa20a5bca60b8125af3a37b sha1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
ROUTE_COVERAGE=plan=45ccaeaf… funcs=326/979 c_words=5728 nonc_funcs=47 nonc_words=147 asm_funcs=606 asm_words=71818 asm_words_unknown=0 nonmatchable=present tierA=734 tierB=248 tierB_unresolved=0
```

`preflight: ok (disc1_preflight: PASS (deep, 670 c / 322 asm / 2 rodata))` was
printed before step 1; all five YAML checkpoints and all three input digests
were equal; `orig`/`cand` both equal retail.

### Behaviour-class re-proof (with deep preflight default)

- **PASS** — repaired snapshot above, exit 0.
- **ENV** — a sandbox `PATH` without the toolchain printed
  `EXACT_REBUILD_GATE=ENV mipsel toolchain unavailable …`, exit 2. The live
  `tools/mipsel-host/` was never moved (hidden via a stripped `PATH` in a copy).
- **real FAIL** — a same-size semantic edit to `src/func_80076B58.c`
  (`0x4000000`→`0x4000001`) passed the fast preflight and then failed the build:
  `ERROR: build/src/func_80076B58.c.o .text: bytes beyond 0x40 are not all zero`,
  `EXACT_REBUILD_GATE=FAIL build_us.sh exit != 0`, exit 1, in **3m29s**. This
  confirms the preflight does not mask genuine mismatches and quantifies the
  cost the deep preflight front-runs.

## Route coverage (this session)

Live plan `830424c3b7e2…` (674 c / 321 asm / 2 rodata) measured and appended as
history entry 6; re-running at the same plan is idempotent (no new entry) and
preserves the prior bytes (append-only verified by `cmp` on the prefix). The
`provably non-C` bucket grew to 76 functions / 5,414 words because the matching
worker's `MANIFEST.json` now carries the `handwritten-gte-wrapper` class (103
entries); the tool consumed it read-only. Current three-bucket split:
**matched C** 326 funcs / 5,750 words (7.4%), **provably non-C** 76 / 5,414
(7.0%), **real remaining asm** 569 / 66,291 (85.6%).

## What is proven / what is not (this session)

Proven: the fast+deep preflight detects all five historical defect classes with
actionable line-level messages, passes a valid tree, and is wired before the
expensive stages without weakening PASS semantics; a repaired frozen snapshot
round-trips to retail with the preflight enabled; the ENV and real-FAIL paths
still behave; route coverage is recorded append-only at the live plan hash.

Not proven / not complete: (a) the **live-tree** PASS — the shared tree is still
blocked by the four sibling span defects plus the YAML syntax error, so the
evidence is the frozen-snapshot PASS labelled with its own plan hash; (b) the
boot → end-of-Day-2 goal. An exact rebuild remains necessary, not sufficient.

# Session cont. 2026-09-11 — full-image masked-match sweep

Motivated by a real finding: the matching worker's three span-size defects
included one **masked false match** — `func_800906B4` declared `0x68` while its
C compiled to `0x30`; the surplus span swallowed the next real function and hid
a register-allocation mismatch (`$a1`/`$v1` where retail uses `$v0`/`$v1`).

## New tool: `tools/analysis/verify_matched_leaves.py`

One command over the whole image: for every `c` span it compiles the leaf with
its recorded profile, compiles/links it at the retail VMA, and compares the
retail EXE word-for-word over the **declared** span, while separately enforcing
`compiled .text >= declared span` with a fully-zero tail and a `jr $ra`
terminator. Race-aware (configs + `src/*.c` fingerprinted before/after;
`VERIFY_SWEEP=RACE`, exit 1, if anything moved) and environment-safe (strips
leaked `MASPSX_*`/`ERA_*` and reports it). ~75 s at ~735 leaves.

## Proven run

```
plan SHA-256:  60916571ea3558972fb9d6c05de8e7ed6e085abafa7a89ddd4176d822c87f474
yaml SHA-256:  581845e18f96b05623945d234c0c2ee8a945de5729a573f1f8a54051305b06cc
spans:         1064 (735 c, 327 asm, 2 rodata)
VERIFY_SWEEP=PASS leaves=735
```

Later in the same session the tip advanced to plan `818ab96a9b4d…`
(739 c, YAML `b93dfa2ec809…`) and the sweep was re-run clean:
`VERIFY_SWEEP=PASS leaves=739`. All 739 matched `c` spans pass size, tail,
link, and terminator. Raw table: `docs/evidence/verify-mask/report.json`;
analysis: `docs/evidence/verify-mask/REPORT.md`. Companion at the tip:
`PROFILE_NECESSITY=PASS` (575/575 era leaves clean, 0 hard defect, 0 redundant).

## The masking hole, proved

`tools/analysis/era_link_check.py` (used by the sibling `check_leaf.sh`)
compares only `min(linked, size)` words and prints `LINK_EXACT` when
`size > len(linked)`. Two proofs:

- `func_800176B8` inflated to `0x1000` → `LINK_EXACT`; sweep → FAIL (7 mismatches).
- clean 16-aligned era leaf `func_800172FC` (compiled `0x20`, `era_o2_g8`) with
  its span inflated to `0x60` by deleting the next edge:
  `era_link_check.py 0x60` → `LINK_EXACT`; sweep →
  `FAIL [size,link] span=0x60 compiled=0x20` (16 mismatches).

The old tool only catches an inflated span when the surplus lies inside the
compiled object (non-zero word) or past a 16-aligned compiled size; for leaves
whose compiled size is 4/8/12 mod 16 it hides behind gas padding.

## Live finding

An early sweep at plan `ab0a6dfeb321…` (702 c) caught `func_80086464`: declared
`0x68`, actual file `0x34` (`D_800BCD80 = 0x10; D_800BCD84 = a0; func_8008CBA8();`),
swallowing `func_80086498`. The owner split the span; it now `LINK_EXACT`s. No
other masked match found.

## Masking-risk classes

- (a) `span > compiled`: detected (size check; deep preflight; trim at build).
- (b) `span == compiled` masked otherwise: non-zero tail, alignment word,
  different-size registration → all detected. A swallowed *smaller* adjacent
  function that folds into the compiled object is only **narrowed** (terminator
  check: 0 of 735 spans have an interior `jr $ra`; all end in `jr $ra`), not
  fully excluded — that residual needs per-leaf register-semantics review.

## Environment hazard guarded

A leaked `export MASPSX_SYMBOL_AT_TEMP=1` changes codegen
(`func_80042770` fails without it, at its own profile that sets it). The sweep
strips `MASPSX_*`/`ERA_*` and reports it; `scripts/exact_rebuild.sh` does the
same before any stage and pins the repo-local toolchain onto
`PATH`/`LD_LIBRARY_PATH`.

## Gate wiring

`scripts/exact_rebuild.sh --sweep` runs the sweep only *after* the PASS, so it
can never turn a FAIL into a PASS; default is off and PASS semantics are
unchanged. Route coverage at the tip: 345/979 functions matched C, 6,048 words;
76 / 5,414 provably non-C; 558 / 66,231 real remaining asm — appended as the
latest history entry (append-only, plan-hash timestamp; idempotent).
