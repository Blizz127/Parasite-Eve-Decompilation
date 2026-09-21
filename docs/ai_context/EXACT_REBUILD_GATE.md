# EXACT_REBUILD_GATE — repeatable exact packed-rebuild proof

This is the operational gate for "does the committed YAML + `src/` tree still
reassemble to the retail USA disc 1 executable, byte for byte?". Run it after
every matching batch (a batch of new `c` spans landing in
`configs/USA/disc1.yaml` + `src/`).

- One command: `scripts/exact_rebuild.sh`
- Underlying sequence (unchanged semantics): `scripts/split_us.sh` →
  `scripts/build_us.sh` → `scripts/verify_us.sh`
- Retail target: `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` (`SLUS_006.62`)

**Safe to run while matching runs continuously.** The gate distinguishes the
benign form of the live split race from a real problem:

| observed | classification | result |
|---|---|---|
| A sibling creates `src/func_*.c` / `docs/evidence/*` between the split's `git status` snapshots (not a split output path) | **benign** | continue, print the tripped entries; the YAML/inputs pins then decide whether the plan actually moved |
| A file appears under a split `OUTPUT_PATH` (`asm/disc1`, `linkers/disc1.ld`, …) | **real ignore-rule violation** | FAIL, exit 1 |
| The YAML (or the compiled `src/` inputs) changes mid-run | **unsound proof** | FAIL, exit 1 |
| Any split trip you want to be fatal | `--strict-split` | FAIL, exit 1 |

The benign case needs **no opt-in flag**. `--allow-split-race` is kept as a
no-op legacy alias. The proof can never straddle two tree states: the YAML
SHA-256 is pinned at four checkpoints and the plan's compiled inputs
(`disc1.yaml` + build profiles + every `c`-span source) are hashed from before
the build through the end of verify.

Build-environment facts (rootless `tools/mipsel-host/` toolchain, package pins,
toolchain resolution order) live in `docs/ai_context/TOOLCHAIN_REBUILD.md`.

## Fast preflight (runs before the expensive stages)

`tools/build/disc1_preflight.py` validates the build inputs in seconds and runs
automatically as step 4 of the gate, **before** `split_us.sh` and
`build_us.sh`. It can only *add* an early FAIL; it never relaxes a check, so the
PASS semantics below are unchanged.

| check | what it catches | cost |
|---|---|---|
| `yaml-syntax` | parses `configs/USA/disc1.yaml` with the exact consumer loader (splat uses `yaml.load(..., Loader=yaml.SafeLoader)`) and prints the offending line + caret; catches a hard-wrapped comment continuation or bad indentation | instant |
| `yaml-sha1` | missing/`malformed` top-level `sha1:` | instant |
| `geometry` | span gaps/overlaps, duplicate VMAs, non-ascending offsets, wrong first/last edge, unsupported kinds, and a `c` symbol whose name does not encode its own mapped VRAM | instant |
| `c-source` | a `c` span with no `src/<name>.c` | instant |
| `profile-ref` | unknown profile in an assignment, stale assignment (no such YAML `c` span), a symbol assigned by two profiles, empty assignment list, undefined `default_profile` | instant |
| `deep-size` / `deep-pad` | **compiles every `c` leaf** with its exact per-leaf profile and compares the emitted `.text` to the declared span: a span larger than the compiled leaf is the historical `0x94`-vs-`0x90` defect that aborted `trim_elf_section_pad.py`; a span smaller than a leaf with a **non-zero** tail means the carve swallowed real code | ~7 s for ~500 leaves (parallel) |
| `deep-link` | links each leaf at its **retail VMA** (undefined symbols bound to their address-named values) and compares the retail EXE word-for-word over the whole declared span. This is the check the weak per-leaf helper cannot make: `era_link_check.py` reads only `min(linked, size)` words, so an oversized span prints `LINK_EXACT` | included |
| `deep-boundary` | the span must end on a real function boundary — a bare `jr $ra` (`0x03E00008`) with or without its delay-slot word, or a tail `j` | included |
| `deep-interior-return` | the span must contain **no** `jr $ra` before the terminal one. A span that swallows an adjacent function leaves interior returns; this closes the class-(b) masking sub-case as far as is possible without register-semantics review | included |

The `--deep` check is the **strong per-leaf check**: `deep-size` +
`deep-pad` + `deep-link` + `deep-boundary` + `deep-interior-return`, i.e. the
same primitives (`tools/analysis/leaf_strong_check.py`) the full-image sweep
uses, so there is a single definition of "verified".

The deep check is **on by default** in the gate (the compile is ~7 s, versus the
~30-minute split/build cycle it front-runs). `--preflight-fast` /
`EXACT_REBUILD_GATE_PREFLIGHT=fast` skips it; `EXACT_REBUILD_GATE_PREFLIGHT=off`
skips the whole preflight. If the era/mipsel tooling is absent the deep check
skips with a `WARN` (the build step remains the authority on prerequisites and
still exits `ENV`).

Standalone:

```bash
python3 tools/build/disc1_preflight.py            # fast structural checks
python3 tools/build/disc1_preflight.py --deep     # + span-size compile of every leaf
python3 tools/build/disc1_preflight.py --deep --only func_80077D30
```

Exit codes: `0 PASS`, `1 FAIL` (each finding prints the span/file/line and the
exact remedy). Proven against five seeded defects (syntax, `0x94`-vs-`0x90`
size, overlap, missing `src/`, bad profile reference) and against a valid tree —
see `docs/evidence/exact-rebuild-gate/REPORT.md`.

## Full-image sweep (independent re-verification of every matched `c` span)

`tools/analysis/verify_matched_leaves.py` answers "are the N counted leaves
*all* genuinely exact at their declared sizes?" in one command. It compiles each
leaf with its recorded profile, links it at the retail VMA, and compares
word-for-word over the **whole declared span**, while separately enforcing
`compiled .text >= declared span`, a fully-zero tail, a real function
terminator, and **no interior `jr $ra`**.

That last guard matters: the sibling per-leaf helper (`check_leaf.sh` →
`era_link_check.py`) compares only `min(linked, size)` words and can print
`LINK_EXACT` for a span larger than the compiled object — the exact mask behind
`func_800906B4` (`0x68` declared, `0x30` compiled) and `func_80086464`
(`0x68` vs `0x34`, caught by this sweep mid-session). See
`docs/evidence/verify-mask/REPORT.md`.

```bash
python3 tools/analysis/verify_matched_leaves.py                       # whole image, exit 0 iff all pass
python3 tools/analysis/verify_matched_leaves.py --json docs/evidence/verify-mask/report.json
python3 tools/analysis/verify_matched_leaves.py --only func_800906B4  # one leaf, full detail
scripts/exact_rebuild.sh             # strong sweep runs by DEFAULT after PASS
scripts/exact_rebuild.sh --no-sweep  # explicit, LOUD opt-out
```

The sweep is race-aware (it refuses PASS if the YAML, profiles, or any
referenced `src/*.c` changed mid-run) and strips leaked `MASPSX_*`/`ERA_*` knobs
from its environment. Cost: ~75 s at ~735 leaves.

**The strong sweep is ON by default** in `scripts/exact_rebuild.sh`: it runs
only *after* the gate's own checks pass, so it can never turn a FAIL into a
PASS, and a sweep failure exits 1 **before** the `EXACT_REBUILD_GATE=PASS` line
is printed — a PASS can therefore never be misread as covering an unverified
span. `--no-sweep` (or `EXACT_REBUILD_GATE_SWEEP=off`) opts out with a loud
`WARN` that states the weak check cannot catch an oversized span, so relying on
the weak check is a deliberate, visible choice. `--sweep` is kept as an explicit
(now redundant) request.

## The enforced safe path (use this)

**The only command a person needs to be safe before declaring a leaf or batch
matched is the strong deep preflight — which is already wired into both the
per-leaf helper and the gate:**

```bash
# per leaf (what check_leaf.sh already invokes for you):
python3 tools/build/disc1_preflight.py --deep --only func_XXXXXXXX

# whole batch / before declaring a batch done:
python3 tools/build/disc1_preflight.py --deep        # strong, all c spans

# full proof (runs the strong check again on every span by default):
scripts/exact_rebuild.sh
```

`check_leaf.sh` already calls
`python3 tools/build/disc1_preflight.py --deep --only "$NAME"` as its second
step, and `--deep` is now the **strong** check (size + link + terminator +
interior-return). **No change to `check_leaf.sh` is required** for the stronger
coverage — it inherits it automatically. The only *optional* improvement for the
owner is to make its own documentation/echo match the new scope, e.g. change the
closing line

```bash
echo "== [$NAME] OK (link exact + span size exact) =="
```

to

```bash
echo "== [$NAME] OK (strong: link exact + size exact + terminator + no interior return) =="
```

which is cosmetic (the enforcement is already there). No functional one-liner is
needed; the enforcement lives in `disc1_preflight.py --deep`, which
`check_leaf.sh` already runs.

### Proof the enforced default catches the mask

Two seeded inflated spans, on the **default** `--deep` path (no `--sweep`
needed). Both previously printed `LINK_EXACT` from `era_link_check.py`:

`func_800176B8` inflated to `0x1000` (next surviving edge at `0x8EB8`):

```
disc1_preflight: FAIL 3 finding(s) — fix before running the expensive split/build
disc1_preflight: FAIL [deep-size] func_800176B8: declared span 0x1060 (0x7EB8->0x8F18) exceeds compiled .text 0x30 by 0x1030; trim_elf_section_pad.py will abort with `target size 0x1060 > current 0x30`
disc1_preflight: FAIL [deep-link] func_800176B8: object does not match retail at its retail VMA over the declared span (1038 word mismatch(es), pad_nonzero=0); first: 0x800176e0: ROM 8f820590 LNK 00000000, 0x800176e4: ROM 8c830000 LNK 00000000
disc1_preflight: FAIL [deep-interior-return] func_800176B8: span 0x1060 contains 60 interior `jr $ra` before the terminal one (offsets 0x20, 0x3C, 0xA4, 0xEC, 0x108, 0x160); the span likely swallows an adjacent function
```

`func_800172FC` inflated to `0x60` (the deliberately hard case: compiled `0x20`,
a 16-aligned size, so the surplus hides entirely inside gas padding):

```
disc1_preflight: FAIL 3 finding(s) — fix before running the expensive split/build
disc1_preflight: FAIL [deep-size] func_800172FC: declared span 0x60 (0x7AFC->0x7B5C) exceeds compiled .text 0x20 by 0x40; trim_elf_section_pad.py will abort with `target size 0x60 > current 0x20`
disc1_preflight: FAIL [deep-link] func_800172FC: object does not match retail at its retail VMA over the declared span (16 word mismatch(es), pad_nonzero=0); first:
disc1_preflight: FAIL [deep-interior-return] func_800172FC: span 0x60 contains 1 interior `jr $ra` before the terminal one (offsets 0x18); the span likely swallows an adjacent function
```

The same `func_800172FC` at those two sizes through the **weak** helper
(`era_link_check.py`, what `check_leaf.sh`'s old path printed), for contrast —
the inflated `0x60` span is indistinguishable from the true `0x20` one:

```
$ python3 tools/analysis/era_link_check.py src/func_800172FC.c 0x800172FC 0x20 -O2 -G8
linked .text 32 bytes, target 0x20, word mismatches=0, nonzero_pad=0
LINK_EXACT
$ python3 tools/analysis/era_link_check.py src/func_800172FC.c 0x800172FC 0x60 -O2 -G8
linked .text 32 bytes, target 0x60, word mismatches=0, nonzero_pad=0
LINK_EXACT                     # <-- phantom match: 0x40 bytes of code are unaccounted for
```

(Same file/leaf at its true span, strong path:
`LEAF_STRONG=EXACT func_800172FC span=0x20 vram=0x800172FC compiled=0x20 failures=[none]`.)

(Live, the new default caught the sibling's mid-carve `func_8005257C` —
declared `0x214`, compiled `0x20`, 7 interior `jr $ra` — in ~1 s and stopped the
gate at `EXACT_REBUILD_GATE=FAIL preflight: invalid build inputs` instead of
after a ~30-minute split/build until `trim_elf_section_pad.py` aborted.)

The gate's PASS semantics are **unchanged**: the strong checks only ever add a
FAIL. Two properties are intentionally preserved:

- `deep-*` findings in the preflight run *before* the split, so they can only
  turn an imminent FAIL into an earlier FAIL.
- The post-PASS sweep runs *after* the gate's own checks, so it cannot make a
  FAIL pass; and it now runs **before** the `EXACT_REBUILD_GATE=PASS` line is
  emitted, so a `PASS` line is only ever printed for a state whose spans were
  all strongly verified.

## Run the gate

```bash
# from a clean checkout, once:
scripts/setup_env.sh          # pinned splat (Python venv, .venv/bin/splat)
scripts/setup_era.sh          # era gcc-2.7.2-psx + vendored maspsx
scripts/setup_mipsel_host.sh  # rootless GCC 14.2.0 / binutils 2.44
# and a retail disc 1 image extracted by scripts/extract_us.sh 1

# then, after every matching batch:
scripts/exact_rebuild.sh
```

The driver takes no configuration and needs no `PATH` setup: `build_us.sh`
resolves the toolchain host-`PATH` → repo-local → distrobox. Prepend
`tools/mipsel-host/bin` to `PATH` yourself only if you want the gate's own
`find_toolchain()` to report `host PATH` instead of the repo-local shim dir.

### Expected output (PASS)

```
== exact rebuild gate ==
  root:      /path/to/Parasite-Eve-Decompilation
  toolchain: repo-local tools/mipsel-host/bin
  expected:  SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b

  preflight: ok (disc1_preflight: PASS (deep, 670 c / 322 asm / 2 rodata))
[1/3] scripts/split_us.sh
      ok (YAML sha256 <hash>)
[2/3] scripts/build_us.sh
      ok orig=452fb033... cand=452fb033... RESULT=EXACT_MATCH
[3/3] scripts/verify_us.sh
      ok VERIFY_US=PASS

  plan:      45ccaeaf68890ccae2495c4000021be660604485ceea7941139c37ae1408e3ab
  yaml:      103ac05028dbae08f3266ff71a544b443926ec0c5eff9c9574c81ea43033a5d7
  spans:     670 c, 322 asm, 2 rodata
  checkpoints: yaml start=<hash> split=<hash> build=<hash> verify=<hash> final=<hash>
               inputs split=<hash> build=<hash> verify=<hash>
  route:     plan=45ccaeaf… funcs=326/979 c_words=5728 nonc_funcs=47 nonc_words=147 asm_funcs=606 asm_words=71818 …
EXACT_REBUILD_GATE=PASS plan=45ccaeaf6889… yaml=103ac05028db… spans=[670 c, 322 asm, 2 rodata] sha1_orig=452fb033… sha1_cand=452fb033… sha1=452fb033…
ROUTE_COVERAGE=plan=45ccaeaf… funcs=326/979 c_words=5728 nonc_funcs=47 nonc_words=147 asm_funcs=606 asm_words=71818 asm_words_unknown=0 nonmatchable=present tierA=734 tierB=248 tierB_unresolved=0
```

(Counts in the example are the last frozen-snapshot proof, below; they change as
leaves are added.)

The final `EXACT_REBUILD_GATE=PASS …` line is the machine-greppable result and
is **self-describing**: it carries the plan SHA-256, the YAML SHA-256, the span
counts, and both SHA-1s, so a PASS can never be misread as covering a later
state. The `checkpoints:` block shows the five YAML hashes
(start/split/build/verify/final) and the three input hashes, all of which must
be equal within their group — that is the self-consistency proof. The final
YAML checkpoint is taken *after* the plan SHA-256 is printed, so the printed
plan can never belong to a later state than the proof.

The `ROUTE_COVERAGE=` line (informational only; see
`docs/ai_context/ROUTE_COVERAGE.md`) reports how much of the boot → end-of-Day-2
path is matched C. It never affects PASS/FAIL.

### Provenance of this revision

The live PASS recorded below (`plan=32b6fe6d9681…`, `610 c`) was scored by the
revision that added the benign-race classifier, the scoped input digest, the
self-describing summary, and the route line. Later hardenings: a **fifth** YAML
checkpoint taken after the plan hash is printed, the `verify: concurrent split
race` label, and the **fast preflight** wired in before the expensive stages.

**Preflight scored (session cont. 2026-09-11).** The live tip cannot currently
complete `build` because the sibling-owned YAML/profiles carry four span-size
defects and one hard-wrapped comment that makes the YAML **syntactically
invalid** (line 1779, ` cursor by` at column 0). The preflight catches all of
them in **8.5 s** instead of after a ~30-minute cycle:

```
disc1_preflight: FAIL 4 finding(s)
disc1_preflight: FAIL [yaml-syntax] configs/USA/disc1.yaml:1779:2 … (splat uses yaml.SafeLoader)
disc1_preflight: FAIL [deep-size] func_80076B58: declared span 0x88 (0x67358->0x673E0) exceeds compiled .text 0x40 by 0x48
disc1_preflight: FAIL [deep-size] func_80077D30: declared span 0x94 (0x68530->0x685C4) exceeds compiled .text 0x90 by 0x4
disc1_preflight: FAIL [deep-size] func_800906B4: declared span 0x68 (0x80EB4->0x80F1C) exceeds compiled .text 0x30 by 0x38
EXACT_REBUILD_GATE=FAIL preflight: invalid build inputs (fix the findings above before split/build)
```

The **frozen-snapshot fallback** then repaired those defects (three `asm`
spans closing the over-carved leaves, the wrapped comment joined, and the
`func_800556E8`/`func_80058E08` patch-5 assignment moved off `era_o2_g8` to
`era_o2_g8_symbol_at_temp`) and scored a clean PASS with the preflight enabled:

```
EXACT_REBUILD_GATE=PASS plan=45ccaeaf68890ccae2495c4000021be660604485ceea7941139c37ae1408e3ab yaml=103ac05028dbae08f3266ff71a544b443926ec0c5eff9c9574c81ea43033a5d7 spans=[670 c, 322 asm, 2 rodata] sha1_orig=452fb033… sha1_cand=452fb033… sha1=452fb033…
ROUTE_COVERAGE=plan=45ccaeaf… funcs=326/979 c_words=5728 nonc_funcs=47 nonc_words=147 asm_funcs=606 asm_words=71818 asm_words_unknown=0 nonmatchable=present tierA=734 tierB=248 tierB_unresolved=0
```

All five YAML checkpoints were equal (`start=split=build=verify=final`) and all
three input digests were equal. Earlier in this lane, the fifth checkpoint and
the `verify`-race label were scored on a frozen snapshot: deleting one generated
`asm/disc1/*.s` after split made verify fail with
`missing split-generated source(s): asm/disc1/11718.s` and the gate printed
`EXACT_REBUILD_GATE=FAIL verify: concurrent split race — …; re-run`, exit 1.
The label previously grepped only `missing split-generated source`, but
`tools/build/disc1_verify.py` emits `missing split source(s)`; the grep is now
the common prefix `missing split`, covering both producers.

**Full-image masked-match sweep (session cont. 2026-09-11).** Added
`tools/analysis/verify_matched_leaves.py` (see "Full-image sweep" above), the
opt-in `--sweep` gate flag, and an **environment guard** that strips leaked
`MASPSX_*`/`ERA_*` knobs from the gate's shell before any stage (a leaked
`export MASPSX_SYMBOL_AT_TEMP=1` silently changes codegen). Validated live: a
run printed `preflight: ok`, classified a sibling's mid-split file as a benign
race, then `build … RESULT=EXACT_MATCH` (both SHA-1s `452fb033…`) and
`verify … VERIFY_US=PASS`, and finally failed **loudly and correctly** with
`EXACT_REBUILD_GATE=FAIL race: YAML changed during the run` because a sibling
edited the YAML after verify — the strict "no mixed-tree proof" semantics are
intact. The `--sweep` flag runs only after a PASS, so PASS semantics are
unchanged. Full detail: `docs/evidence/verify-mask/REPORT.md`.

## Exit codes / failure modes

| exit | prefix | meaning | what to do |
|---|---|---|---|
| 0 | `EXACT_REBUILD_GATE=PASS` | clean exact rebuild | record the `plan=` hash + counts in the report below |
| 1 | `EXACT_REBUILD_GATE=FAIL preflight: invalid build inputs` | `tools/build/disc1_preflight.py` found a syntax/geometry/profile/span-size defect | fix the printed finding (span/file/line + remedy); this fires in seconds, before any split |
| 1 | `EXACT_REBUILD_GATE=FAIL race: YAML changed during the split` | splat read a moving plan | re-run |
| 1 | `EXACT_REBUILD_GATE=FAIL race: compiled sources changed` | a `c`-span source (or the YAML/profiles) changed during build/verify | re-run |
| 1 | `EXACT_REBUILD_GATE=FAIL real ignore-rule violation` | a split output path appeared un-ignored | fix `.gitignore`, then re-check |
| 1 | `EXACT_REBUILD_GATE=FAIL build …` | `build_us.sh` failed or did not print `RESULT: EXACT MATCH` | read the point-of-failure `NON-MATCH` line (names the owner span + profile) |
| 1 | `EXACT_REBUILD_GATE=FAIL verify: concurrent split race …` | a sibling `split_us.sh` ran during verify and this run's generated asm went missing | re-run |
| 1 | `EXACT_REBUILD_GATE=FAIL verify …` | `verify_us.sh` failed | see `ERROR:` above; common causes below |
| 2 | `EXACT_REBUILD_GATE=ENV …` | a prerequisite is missing (toolchain / splat / retail image / config) | run the named remedy |

### Distinguishing the common failure classes

1. **Toolchain missing** (exit 2, `mipsel toolchain unavailable`). The gate
   resolves the toolchain exactly as `disc1_build.py::find_toolchain()` does,
   so the message is the same and it names the remedy:
   `scripts/setup_mipsel_host.sh`. No split is attempted. Prove it **without
   touching the shared toolchain** by hiding the copy in a sandbox snapshot
   (`mv <sandbox>/tools/mipsel-host{,.hidden}; <sandbox>/scripts/exact_rebuild.sh`),
   not the live directory.

2. **Benign race with a sibling matching worker** (NOT a failure). The gate
   prints `WARN split git-status trip: sibling file(s) appeared mid-run`, lists
   the entries, notes that no split output path was involved, and continues.
   The YAML/inputs checkpoints then prove the plan did not move. (Historically
   this printed `ERROR` and required `--allow-split-race`.)

3. **Genuine link-level mismatch** (exit 1, `build …`). `build_us.sh` prints
   `NON-MATCH: first byte at 0x… owner=… profile=…` naming the owning span.
   This is a real matching defect in a registered leaf. Do **not** patch the
   leaf to force a pass; hand the owner span + the point-of-failure address to
   the worker who owns it. The historic examples were object-level matches that
   only failed once relocation/scheduling resolution ran (a `D_<wrongaddr>`
   symbol name; an address materialization hoisted ahead of a loop; a ternary
   operand order).

4. **Verify-only failure** (exit 1, `verify …`). `verify_us.sh` gate 3,
   `Tracked C sources …`, fails with `YAML C sources are not tracked:
   src/…` when the YAML references a leaf whose `src/func_*.c` has not been
   `git add`ed yet. That is a **staging gap**, not a build failure — the build
   already proved the byte-exact link, and the gate now labels it as such. The
   fix belongs to the leaf's owner (`git add` the source, its evidence report,
   and the YAML entry together). Other verify gates fail on stale generated
   status (`run python3 tools/build/disc1_plan.py --write-status`) or a
   disposition bijection mismatch.

## Underlying commands (if you need them individually)

```bash
scripts/split_us.sh              # splat; ~90 s; prints plan line + span counts
scripts/build_us.sh              # ~130 s; ends `RESULT: EXACT MATCH`
scripts/verify_us.sh             # 7 gates; ends `VERIFY_US=PASS`
scripts/verify_us.sh --public    # 4 artifact-independent gates; PUBLIC_VERIFY=PASS
python3 tools/build/disc1_preflight.py        # fast structural preflight (<1 s)
python3 tools/build/disc1_preflight.py --deep  # + span-size compile of every leaf (~7 s)
python3 tools/analysis/verify_matched_leaves.py  # full-image masked-match sweep (~75 s)
python3 tools/build/disc1_plan.py --check   # one-line plan summary
python3 tools/build/test_disc1_plan.py      # 8 unit tests
```

## Currently proven tip

The table is the record of the last clean self-consistent gate run. Re-run and
re-record it every time leaves are added — **the plan SHA-256 changes with every
YAML edit, so a recorded hash without a fresh run is stale by definition.**

| date | plan SHA-256 (full) | spans | both SHA-1s | result |
|---|---|---|---|---|
| 2026-09-11 | `74acf19a28ffd98c4018b9cd3cc9eee70441ad3054eaccc27b1281b19491c096` | 751 c, 330 asm, 2 rodata | `452fb033…` | **PASS (live tree, strong sweep default; 751/751 swept exact; preflight `PASS (deep, 751 c / 330 asm / 2 rodata)`) — YAML `a1a1b23b60b3…`, inputs `8cad094ff6a9…`, all five checkpoints pinned; includes the `func_8005257C` `0x214`→`0x18` trim (7 interior `jr $ra` had been masked)** |
| 2026-09-11 | `7ede9199b81788a73dbee0b3146477637109aa4822456e7de2763ad7968d029b` | 750 c, 330 asm, 2 rodata | `452fb033…` | **PASS (live tree, strong sweep default; 750/750 swept exact; preflight `PASS (deep, 750 c / 330 asm / 2 rodata)`) — YAML `3181378d8517…`, inputs `a35ef8c0fc61…`, all five checkpoints pinned** |
| 2026-09-11 | — (blocked, superseded minutes later) | 750 c, 329 asm, 2 rodata | — | FAIL: preflight `deep-size`/`deep-link`/`deep-interior-return` on sibling's mid-carve `func_8005257C` (declared `0x214`, compiled `0x20`, 7 interior `jr $ra`); the gate refused before split/build and the strong sweep independently agreed. The owner landed the fixed `0x18` carve (§`func_8005257C` now `[0x42D7C,c]` + `[0x42D94,asm]`) and the next run PASSed. |
| 2026-09-11 | `818ab96a9b4db35a54337ecc3dce2c805d526a0d9945e9d98c8b6bb857852d7a` | 739 c, 328 asm, 2 rodata | `452fb033…` | **PASS (live tree, `--sweep` enabled; 739/739 swept exact)** |
| 2026-09-11 | `45ccaeaf68890ccae2495c4000021be660604485ceea7941139c37ae1408e3ab` | 670 c, 322 asm, 2 rodata | `452fb033…` | **PASS (frozen snapshot; 4 sibling defects repaired; preflight enabled)** |
| 2026-09-11 | `32b6fe6d9681fcdcf33c44d2bf5a157073ad30d1e1478b5a99b43b53ab46aa3a` | 610 c, 298 asm, 2 rodata | `452fb033…` | **PASS (live tree, benign race auto-continued)** |
| 2026-09-11 | `8f7e8b3f394b98e9027230412b23a9abaf4e30833ec9246b9c87e89ca892216f` | 606 c, 295 asm, 2 rodata | `452fb033…` | PASS (live tree) |

YAML SHA-256 for the newest run: `b93dfa2ec80947f7c2e324c24ad8707eece7ef77a36f06ea098396da65a561a2`,
stable across split, build, verify, and the post-PASS sweep; the plan's compiled
inputs hashed identically before build, after build, and after verify. The
`--sweep` run scored 739/739 matched leaves exact at this plan. (An earlier
frozen-snapshot PASS at plan `45ccaeaf…`, 670 c, is also recorded in the report.)

A live-tree run of this gate is now winnable even while sibling matching
workers edit the tree, because the benign split race is auto-classified and the
YAML/inputs pins fail loudly if — and only if — the plan actually moved. When a
sibling rewrites `configs/USA/disc1.yaml` during the split or a `c`-span source
during the build, the gate still (correctly) FAILs and must be re-run; for a
fully reproducible proof against a fixed state, a frozen snapshot of
`configs/` + `src/` remains the fallback (see
`docs/evidence/exact-rebuild-gate/REPORT.md` for the snapshot recipe).

### Case study: a real link mismatch caught by the gate

On 2026-09-11 the gate caught `func_80074D28` (`profile=era_o2_g0_fill_epilogue_delay_slot`)
at `0x65574`: the object-level check reported only the 9 usual relocation fields,
but the **linked** leaf had two branch delay slots scheduled in the wrong order:

```text
0x80074d70  bnez  s0, 0x80074D84
0x80074d74  li    a1,-1      <- retail: addiu a0,s1,0x6A (the branch's real arg)
0x80074d78  addiu a0,s1,0x6A <- retail: li a1,-1
```

and the mirrored swap at `0x80074d84`/`0x80074d88`. Root cause: a ternary
operand-order mistake in the leaf (`s0 == 0 ? 0x3000001 : 0x3000000`) versus the
retail-matching order (`s0 ? 0x3000000 : 0x3000001`). This is exactly the
failure class the gate exists to catch: **object-level `.text` equality is not
sufficient; the link resolves relocation/scheduling that can still diverge.**
Hand such a leaf to its owner with the failing VMA + owner span; do not patch it
yourself.

Notes on interpreting an exact rebuild:

- **Proven by a PASS**: the registered tree round-trips to retail. Every
  registered `c` span and every `asm`/`rodata` span contributes the retail
  bytes, and the linked, packed image hashes to retail.
- **Not proven by a PASS**: that the whole boot→end-of-day-2 path is lifted to
  matching C. Spans still registered as `asm` are reassembled from the split,
  not decompiled. See `docs/ai_context/ROUTE_COVERAGE.md` for the measured
  on-path coverage (the gate prints its `ROUTE_COVERAGE=` line too). An exact
  rebuild is a necessary, not sufficient, milestone.

## Why this file exists separately

`docs/ai_context/TOOLCHAIN_REBUILD.md` owns the *environment* (how the rootless
mipsel toolchain is installed and pinned). This file owns the *gate* (the single
repeatable command, its expected output, and its failure taxonomy) so no human
has to remember the split→build→verify sequence after each matching batch.
