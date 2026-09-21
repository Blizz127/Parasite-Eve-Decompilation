# Masked-match audit — independent full-image re-verification of every matched `c` span

Owner lane: verification/gates. Sibling-owned inputs (`src/`,
`configs/USA/disc1.yaml`, `disc1_build_profiles.json`) are read-only here; any
defect found in them is reported for the owner, never fixed.

## Why

The matching worker fixed three span-size defects that **deep** preflight
caught. One was a **masked false match**: `func_800906B4` declared `0x68` while
its C compiled to `0x30`; the surplus span swallowed the next real function and
*hid* a register-allocation mismatch (`$a1`/`$v1` where retail uses `$v0`/`$v1`).
The matched-C count briefly included a leaf that was not retail-exact.

This report independently re-verifies the whole image, quantifies the masking
risk, and records a repeatable sweep so the claim "all N verified at plan X" is
reproducible rather than asserted.

## The sweep tool

`tools/analysis/verify_matched_leaves.py` — one command over the whole image.
For every `c` span in the plan it checks:

| check | what it enforces | class |
|---|---|---|
| **build** | the leaf compiles with its exact recorded profile/flags/knobs | — |
| **size** | compiled `.text` ≥ declared span **and** every byte past the span is zero (see masking analysis) | a + b |
| **tail** | no non-zero byte past the declared span | b |
| **link** | the object, linked at its retail VMA with every undefined symbol bound to its address-named retail value, matches the retail EXE word-for-word over the whole declared span, with zero non-zero pad past it | — |
| **terminator** | the span's last instruction is a bare `jr $ra` or a tail `j` (a genuine boundary) | b |

Properties:

- **Fail-loud**: exit 0 iff every leaf passes; each failure prints the leaf, the
  declared vs compiled size, and the first mismatched words (`address: ROM x LNK y`).
- **Run-wide provenance**: it reads the YAML + profiles, builds the plan, re-reads
  and retries until the bytes are stable, then fingerprints every referenced
  `src/*.c` before and after compilation. A config change or a `src/` edit
  mid-sweep yields `VERIFY_SWEEP=RACE` (exit 1), never a PASS.
- **Environment guard**: it strips leaked `MASPSX_*` / `ERA_*` from its own
  environment and reports this, because `disc1_build.compile_era` copies
  `os.environ` and layers leaf knobs on top — a stray export would silently
  change unrelated leaves (verified below).
- **Idempotent**: compiles into the ignored `build/verify-sweep/` scratch dir;
  re-running produces the same result. The whole image is ~75 s at 735 leaves
  (~12-way parallel).

### Reproduce

```
python3 tools/analysis/verify_matched_leaves.py
python3 tools/analysis/verify_matched_leaves.py --json docs/evidence/verify-mask/report.json
python3 tools/analysis/verify_matched_leaves.py --only func_800906B4
scripts/exact_rebuild.sh --sweep          # opt-in, only after a PASS
```

`report.json` (schema v1) carries the plan SHA-256, YAML SHA-256, profiles
SHA-256, a `src/` fingerprint, the span counts, and the per-leaf table.

## Proven run

The first full sweep ran at the mandate's starting state — plan
`8a90f1e110e7…`, **695 c** (YAML `dd685a10297e…`) — and reported
`695 c leaves, 695 OK, 0 failing — PASS`. The tip then advanced under an active
sibling matching worker; the latest clean run is:

```
plan SHA-256:  818ab96a9b4db35a54337ecc3dce2c805d526a0d9945e9d98c8b6bb857852d7a
yaml SHA-256:  b93dfa2ec80947f7c2e324c24ad8707eece7ef77a36f06ea098396da65a561a2
spans:         1069 (739 c, 328 asm, 2 rodata)
sources_stable: true
result:        VERIFY_SWEEP=PASS leaves=739
```

All **739** matched `c` spans pass every check: compiled size ≥ declared span
with a fully-zero tail, `LINK_EXACT` at the retail VMA (0 word mismatches, 0
non-zero pad), and a `jr $ra` terminator. Raw per-leaf table:
`docs/evidence/verify-mask/report.json`. The mandate's **695** spans were all
verified PASS at `8a90f1e110e7…`; earlier clean passes in this session also
covered plan `60916571ea35…` / 735 c and plan `40d2381ca1ea…` / 734 c. The tree
is extended by a sibling matching worker throughout, so the sweep is re-run at
the current plan and its provenance is pinned to the plan/YAML hashes it
actually verified.

Companion checks in the same lane at the same tip:

- `tools/build/disc1_preflight.py` → `PASS (fast, 695 c …)` / deep PASS.
- `tools/analysis/profile_necessity.py` → `575/575 era leaves clean; 0 hard
  defect(s); 0 redundant` (`PROFILE_NECESSITY=PASS`). Non-era (modern) leaves
  are reported unverifiable by that tool and are covered by the sweep's
  size/link checks instead.
- Every `c` span has a `src/*.c` (the plan builder refuses a `c` span without
  one; the sweep additionally fails if a source vanishes).

## Masking-risk analysis

Two classes, exactly as the mandate framed them.

### Class (a): `span > compiled size`

The historic `func_800906B4` shape. Caught by the sweep's **size** check, and by
deep preflight, and by `trim_elf_section_pad.py` at build time.

### Class (b): `span == compiled size` but still masked

Sub-cases and whether the sweep detects them:

- **Non-zero tail pad** (`compiled > span` with a non-zero word past the span):
  detected by **size/tail**. Deep preflight also flags it. **0 leaves** at the
  proven plan (padding is exactly the gas 16-byte alignment pad: 210 leaves pad
  0, 171 pad 4, 192 pad 8, 166 pad 12; max pad 12 < 16).
- **Span includes a following `nop`/alignment word the C does not produce**:
  this is the same defect as class (a) (compiled < span, or the extra word is
  non-zero) and is detected.
- **Registration verified at a different size than declared**: detected,
  because the sweep compares the *declared* span against the *compiled* object
  and against retail at that same span.
- **Swallowing a *smaller* adjacent function** (the span grows but the compiled
  object grows to match, e.g. an inliner or a shared tail folded into the
  object): `compiled >= span` and retail words can transiently agree for the
  whole span. This is *not* distinguishable from a legitimately larger function
  by the sweep's `size`/`link` checks alone. It is narrowed by the
  **terminator** check (a swallowed function would leave an interior `jr $ra`;
  **0** of 739 spans have one) and by the fact that **all 739 spans end in
  `jr $ra`**. It is not fully closable without the matching worker's
  register-allocation review per leaf.

### The blindness this fixes (proved)

`tools/analysis/era_link_check.py` (used by the sibling
`tools/analysis/check_leaf.sh`) compares only `n = min(len(rom), len(linked),
size)` words and only rejects trailing pad when `len(linked) > size`. When
`size > len(linked)` it compares the shorter of the two and prints
`LINK_EXACT`. Proofs:

1. `func_800176B8` at an inflated size `0x1000` → `LINK_EXACT` from
   `era_link_check.py`, while the sweep says
   `FAIL [size,link] func_800176B8 span=0x1000 compiled=0x30` (7 mismatches).
2. A clean 16-aligned era leaf, `func_800172FC` (compiled `0x20`, profile
   `era_o2_g8`), with the next leaf's edge removed so its span becomes `0x60`
   (swallowing the 0x20 `func_8001731C`):
   - `era_link_check.py … 0x60` → `linked .text 32 bytes, target 0x60, word
     mismatches=0, nonzero_pad=0` → **`LINK_EXACT`**;
   - the sweep → `FAIL [size,link] func_800172FC span=0x60 compiled=0x20` with
     16 word mismatches.

The old tool can only catch an inflated span when the surplus is (i) within the
compiled object with a non-zero trailing word or (ii) past a 16-byte-aligned
compiled size. For era leaves whose compiled size is 4/8/12 mod 16 the inflated
case hides behind gas padding. The sweep's explicit `compiled >= span` guard
closes it for every leaf.

### Live finding: one transient masked match (already fixed by the owner)

An early sweep run at plan `ab0a6dfeb321…` (702 c, YAML `7c6089b6267f…`) caught
a **real** masked match:

- `func_80086464` — declared span `0x68`, but the file is a 0x34-byte function
  (`D_800BCD80 = 0x10; D_800BCD84 = a0; func_8008CBA8();`). The span swallowed
  the adjacent `func_80086498`; at the true `0x34` it is `LINK_EXACT`. The
  owner split the span immediately (`0x76C64`→`0x76C98`, `0x76C98`→`0x76CCC`),
  and it now passes.

This is the same defect class as `func_800906B4` and was found by the sweep
within one plan revision. No *other* masked match has been found since.

## Environment hazard (found and guarded)

A leaked `export MASPSX_SYMBOL_AT_TEMP=1` from a persistent shell changes
codegen: `func_80042770` (profile `era_o2_g0_symbol_at_temp`, which *does* set
the knob) fails without it, but a **different** `era_o2_g0` leaf that also
happens to be symbol-store-shaped would be built with the wrong shape. The
sweep's `sanitize_environment` strips `MASPSX_*`/`ERA_*` and reports it; the
gate (`scripts/exact_rebuild.sh`) does the same before any stage and also pins
`tools/mipsel-host/{bin,usr/lib/x86_64-linux-gnu}` onto `PATH`/`LD_LIBRARY_PATH`.

## What is proven / not proven

Proven:

- At plan `818ab96a9b4d…` / YAML `b93dfa2ec809…`, all **739** matched `c` spans
  are size-exact (zero tail pad), `LINK_EXACT` at the retail VMA, and end in a
  genuine `jr $ra`, by one independent command whose provenance is recorded in
  `report.json`.
- The old per-leaf check (`era_link_check.py`) is blind to `span > compiled`
  (two constructed proofs above); the sweep is not.
- The masked-match class is real and recurring (`func_800906B4`,
  `func_80086464`), and the sweep catches it.
- No leaf is withdrawn at the proven plan; 0 of 739 fail.

Not proven:

- The deep "swallowed a smaller adjacent function with a matching compiled
  object" sub-case of class (b) cannot be fully excluded by size/link alone; it
  is only narrowed by the terminator check and needs the owner's per-leaf
  register-semantics review.
- The sweep proves byte-exactness at declared sizes, **not** source-level 1:1
  retail semantics; the matching worker's review remains authoritative for
  that.
- The full boot→end-of-day-2 objective is far from complete (route coverage
  below).

## Route coverage at this tip

`tools/analysis/route_coverage.py` (three buckets), entry appended to
`docs/generated/ROUTE_COVERAGE_HISTORY.md`:

```
plan=818ab96a9b4db35a54337ecc3dce2c805d526a0d9945e9d98c8b6bb857852d7a
funcs=345/979 c_words=6048 nonc_funcs=76 nonc_words=5414 asm_funcs=558
asm_words=66231 asm_words_unknown=0 nonmatchable=present
```

History is append-only (verified: same-plan reruns are byte-identical; a new
plan appends without touching prior entries; the timestamp is derived from the
plan hash, not the wall clock).

---

## Session cont. 2026-09-11 (later) — the blindness is no longer default-reachable

The team's enforced per-leaf routine (`tools/analysis/check_leaf.sh`) calls
`tools/analysis/era_link_check.py`, which is blind to `span > compiled` (it reads
`min(linked, size)` words). The strong check is now the **enforced default**, not
an opt-in.

### What changed

1. **New shared primitive module** `tools/analysis/leaf_strong_check.py` — one
   definition of "verified": `size` (compiled `.text` covers the span with a
   fully-zero tail), `link` (word-exact at the retail VMA over the whole declared
   span), `terminator` (ends on a bare `jr $ra` or a tail `j`), and **`interior`**
   (no `jr $ra` before the terminal one — a swallowed adjacent function leaves
   one). It also exposes a standalone CLI:
   `python3 tools/analysis/leaf_strong_check.py func_XXXXXXXX`.
2. **`disc1_preflight.py --deep` now runs the strong check**, not just the span
   size. New finding classes: `deep-link`, `deep-boundary`,
   `deep-interior-return` (alongside `deep-size` / `deep-pad`). Because
   `check_leaf.sh` already invokes `disc1_preflight.py --deep --only "$NAME"`,
   the sibling helper is **automatically covered with no edit to it**.
   The deep preflight's scratch dir is now unique per process
   (`tempfile.mkdtemp` under `build/`), so a concurrent sibling
   `check_leaf.sh --deep` can no longer wipe this run's objects (a real race
   observed during this session as spurious `deep-build: object not produced`).
3. **`scripts/exact_rebuild.sh` runs the full-image strong sweep by default**
   (`--sweep` is now the default; `--no-sweep` / `EXACT_REBUILD_GATE_SWEEP=off`
   is the explicit, loud opt-out). The sweep runs **before** the
   `EXACT_REBUILD_GATE=PASS` line is printed, so no PASS can be emitted for a
   state whose spans were not all strongly verified. The strong checks only ever
   add a FAIL, so PASS semantics are unchanged.
4. **`verify_matched_leaves.py`** now uses the same shared primitives (single
   definition of "verified") and reports `interior_jr_ra`.

### Proof the enforced default catches the mask

Both seeded inflated spans FAIL on the **default** `--deep` path (no `--sweep`):

`func_800176B8` inflated to `0x1000`:

```
disc1_preflight: FAIL [deep-size] func_800176B8: declared span 0x1060 (0x7EB8->0x8F18) exceeds compiled .text 0x30 by 0x1030; trim_elf_section_pad.py will abort with `target size 0x1060 > current 0x30`
disc1_preflight: FAIL [deep-link] func_800176B8: object does not match retail at its retail VMA over the declared span (1038 word mismatch(es), pad_nonzero=0)
disc1_preflight: FAIL [deep-interior-return] func_800176B8: span 0x1060 contains 60 interior `jr $ra` before the terminal one (offsets 0x20, 0x3C, 0xA4, 0xEC, 0x108, 0x160)
```

`func_800172FC` inflated to `0x60` (compiled `0x20`, 16-aligned — the hard case
where the surplus hides entirely inside gas padding, which `era_link_check.py`
prints `LINK_EXACT` for):

```
disc1_preflight: FAIL [deep-size] func_800172FC: declared span 0x60 (0x7AFC->0x7B5C) exceeds compiled .text 0x20 by 0x40; trim_elf_section_pad.py will abort with `target size 0x60 > current 0x20`
disc1_preflight: FAIL [deep-link] func_800172FC: object does not match retail at its retail VMA over the declared span (16 word mismatch(es), pad_nonzero=0)
disc1_preflight: FAIL [deep-interior-return] func_800172FC: span 0x60 contains 1 interior `jr $ra` before the terminal one (offsets 0x18)
```

### Live catch (concurrent mid-carve defect)

Running the gate on the live tree, the strong preflight stopped it in **~1 s**:

```
disc1_preflight: FAIL [deep-size] func_8005257C: declared span 0x214 (0x42D7C->0x42F90) exceeds compiled .text 0x20 by 0x1F4
  remedy: end the span at 0x42D9C (move the following span/edge up 0x1F4)
disc1_preflight: FAIL [deep-link] func_8005257C: 127 word mismatch(es) …
disc1_preflight: FAIL [deep-interior-return] func_8005257C: span 0x214 contains 7 interior `jr $ra` before the terminal one (offsets 0x10, 0x68, 0xB0, 0xF8, 0x140, 0x188)
EXACT_REBUILD_GATE=FAIL preflight: invalid build inputs (fix the findings above before split/build)
```

This is a matching worker's in-progress carve (`func_8005257C`'s C compiles to
`0x20`; the span is `0x214`, swallowing ~7 functions). Without the strong check
it would have reached `trim_elf_section_pad.py` after a ~30-minute split/build
cycle — the exact failure mode the preflight exists to front-run. The
full-image sweep independently flagged the same leaf (`FAIL [size,link,interior]
func_8005257C span=0x214 compiled=0x20`), so the two paths agree.

### The safe command

`check_leaf.sh` already runs the strong path, so **no change to it is required**;
the recommended one-line (cosmetic) improvement is its final echo:

```bash
echo "== [$NAME] OK (strong: link exact + size exact + terminator + no interior return) =="
```

The enforced command set is:

```bash
python3 tools/build/disc1_preflight.py --deep --only func_XXXXXXXX   # per leaf
python3 tools/build/disc1_preflight.py --deep                        # whole batch
scripts/exact_rebuild.sh                                            # full proof + strong sweep, default on
```

### Semantics audit

31 additional on-path leaves were independently re-derived from the retail
disassembly (all CONFIRMED; 0 DEFECT, 0 UNCERTAIN) — see
`docs/evidence/verify-mask/SEMANTICS_AUDIT.md`. Cumulative: 42 leaves
semantics-confirmed; the remainder are byte-confirmed only.

### Fresh numbers (this session)

Clean-tip result (live tree; all five YAML checkpoints pinned to one hash, so the
proof cannot straddle two states):

```
[1/3] split  ok (YAML sha256 3181378d8517bc8b99cc699e316bf0fe9c87333f65fe36d0bae4ca799b139435)
[2/3] build  ok orig=452fb033f2eaa4b18aa20a5bca60b8125af3a37b cand=452fb033f2eaa4b18aa20a5bca60b8125af3a37b RESULT=EXACT_MATCH
[3/3] verify ok VERIFY_US=PASS
[sweep] tools/analysis/verify_matched_leaves.py (strong, default)
        ok VERIFY_SWEEP=PASS leaves=750 plan=7ede9199b81788a73dbee0b3146477637109aa4822456e7de2763ad7968d029b yaml=3181378d8517bc8b99cc699e316bf0fe9c87333f65fe36d0bae4ca799b139435
EXACT_REBUILD_GATE=PASS plan=7ede9199b81788a73dbee0b3146477637109aa4822456e7de2763ad7968d029b yaml=3181378d8517bc8b99cc699e316bf0fe9c87333f65fe36d0bae4ca799b139435 spans=[750 c, 330 asm, 2 rodata] sha1_orig=452fb033… sha1_cand=452fb033… sha1=452fb033…
ROUTE_COVERAGE=plan=7ede9199b817… funcs=348/981 c_words=6112 nonc_funcs=76 nonc_words=5414 asm_funcs=557 asm_words=66191 …
```

- Deep preflight (strong, whole tree): `disc1_preflight: PASS (deep, 750 c /
  330 asm / 2 rodata)`, 0 findings.
- Full-image strong sweep: `750/750` exact, `sources_stable: true`; artifact
  `docs/evidence/verify-mask/report.json` at plan `7ede9199b817…`.
- `profile_necessity`: `586/586 era leaves clean; 0 hard defect(s); 0 redundant`
  (`PROFILE_NECESSITY=PASS`).
- Route coverage: `funcs=348/981 c_words=6112 nonc_funcs=76 nonc_words=5414
  asm_funcs=557 asm_words=66191`; history appended and idempotent.
- Mid-session the gate also produced a genuine **FAIL** at a moving state: the
  strong preflight refused the sibling's in-progress `func_8005257C` over-carve
  (declared `0x214`, compiled `0x20`, 7 interior `jr $ra`) in ~1 s and the
  independent sweep flagged the same leaf — the owner then landed the fixed
  `0x18` carve and the next run passed. Two independent paths agreeing on a real
  defect is the strongest evidence the enforcement works.

