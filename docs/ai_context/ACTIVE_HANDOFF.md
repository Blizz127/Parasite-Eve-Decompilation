# ACTIVE HANDOFF

Single source of truth for the current working state. Any agent or human
picking up this project reads this first and updates it after every
meaningful change.

## Current phase

**Phase 3 — prefix rodata + mid-image data island split at file 0x818A0
(VRAM 0x800910A0); pc0 still sane after re-split.** Phase 1 complete
locally; only the official redump cross-check remains open (non-blocking).
(Branch: `phase3-disc1-boundary-audit`.)

## What exists right now

- Working extraction pipeline: `scripts/extract_us.sh [1|2|all]` drives
  three stdlib-only Python tools — `tools/extract/psxiso.py` (ISO9660
  reader for raw MODE2/2352 images), `tools/analysis/psxexe_info.py`
  (PS-X EXE header dump), `tools/verify/hashfile.py` (CRC-32/MD5/SHA-1).
- Both discs processed end-to-end with the **fixed** script; all results
  recorded in `docs/disc_info.md`. Extracted output lives only under
  `build/extracted/` (git-ignored).
- The extract script's fail-loudly defect is fixed: every step now carries
  explicit `|| return 1` (immune to `set -e` suppression in the `||`
  caller context) and the per-disc output dir is wiped at run start so
  stale results can't masquerade as success. Failure tests re-run
  2026-07-04: garbage image → exit 1 with ERROR (previously exit 0 with
  stale "OK"), missing images → exit 1, cue-without-bin → exit 1.
- `configs/USA/disc1.yaml` now contains a conservative minimal initial
  splat config built from verified Phase 1 values;
  `configs/USA/disc2.yaml` is a documented byte-identical disc 2
  pointer/alias, not a separate active config. **First real split run
  completed 2026-07-07** — local study artifacts under `asm/disc1/`,
  `linkers/disc1.ld`, `include/*.inc`, `undefined_*_auto.txt` (all
  git-ignored, never committed). No matching build, decompiled C, or
  PC-port work exists. `scripts/setup_env.sh` is implemented (pinned
  `.venv/` install); `verify_us.sh` is still a placeholder.

## What is verified

- Disc 1 (SLUS-00662) and disc 2 (SLUS-00668) local dumps: image and EXE
  hashes, PS-X EXE headers, SYSTEM.CNF boot lines, full ISO9660 listings
  (25 and 31 files) — see `docs/disc_info.md` for values and commands.
- **The two boot executables are byte-identical** (SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` on both) — one EXE target
  covers both discs. Entry with evidence in
  `docs/reverse_engineering_notes.md`.
- **`PE.IMG` is byte-identical across both discs** (SHA-1
  `146c0ce7308bf9fdc2ba5a84230e198db0663f3b`, verified 2026-07-04): the
  discs differ only in FMV/XA streams and volume metadata. The decomp
  target is one EXE plus one archive. Archive format unexplored.
- Both image dumps are corroborated by an independent third-party catalog
  with identical hashes (GitHub `portforge/portforge-mediaitems`, found
  via code search for our SHA-1s; recorded in docs/disc_info.md). Not an
  authoritative redump verification.

## Phase 2 state (this branch)

- `configs/USA/disc1.yaml` is now a real minimal splat config: header +
  one conservative `asm` segment covering the whole loadable image
  (0x800–0x1EE800 at vram 0x80010000). Every number in it is a verified
  Phase 1 fact; no internal boundaries or symbols are claimed.
- `configs/USA/disc2.yaml` is a documented pointer to disc 1 (the EXEs
  are byte-identical), deliberately not a duplicate config.
- `scripts/setup_env.sh` is now implemented: creates a git-ignored
  `.venv/` and installs the pinned `splat64[mips]==0.41.0` (latest PyPI
  release as of 2026-07-05, when the pin was decided — see "Open
  decisions" below, now closed). Idempotent; has repo-root and git-repo
  sanity guards.
- `scripts/split_us.sh` gained a `--check` dry-run mode (verifies repo
  root, config, extracted EXE presence + SHA-1, splat availability via
  `.venv/bin/splat` first then `PATH`, and that every output path is
  `git check-ignore`d — prints what a real run would generate, invokes
  nothing). A real run now also diffs `git status` before/after and
  fails loudly if the split produced anything not git-ignored. Verified
  2026-07-07: `scripts/setup_env.sh` created `.venv/` with splat64
  0.41.0; `scripts/split_us.sh --check` passes all six gates on this
  machine (no split invoked, git status clean); `bash -n` clean on all
  four `scripts/*.sh`.
- `.gitignore` covers splat output: `asm/`, `linkers/`, `assets/`,
  `.splache`, `undefined_*_auto.txt`, and splat boilerplate under
  `include/*.inc` / `include/*.h` (tracked `include/.gitkeep` preserved).
- `docs/splitting.md` documents the canonical target, verified values,
  unknowns, the pinned toolchain, the dry-run workflow, and the
  no-matching-claims policy.

### First split output (2026-07-07, local only)

Command: `scripts/split_us.sh` on branch `phase2-first-real-split`.
splat 0.41.0 reported **Split 2 MB (100.00%)** — header 2 KB, main asm
2 MB. Generated and git-ignored:

| Path | Present | Notes |
| --- | --- | --- |
| `asm/disc1/header.s` | yes | PS-X EXE header; pc0 `0x80072534`, t_addr `0x80010000` match Phase 1 |
| `asm/disc1/800.s` | yes | ~529k lines; single monolithic asm subsegment (expected) |
| `linkers/disc1.ld` | yes | `.main` at VRAM `0x80010000`; text/data/rodata/bss sections from one object |
| `undefined_syms_auto.txt` | yes | ~1438 auto data labels (`D_*`) |
| `undefined_funcs_auto.txt` | yes | ~468 auto func labels; many addresses outside load range (data-as-code noise) |
| `include/{macro,labels,gte_macros}.inc`, `include_asm.h` | yes | splat boilerplate copied on split |
| `assets/disc1/` | no | no asset subsegments in config — expected |
| `.splache` | no | not created by this splat 0.41.0 run — expected |

**pc0 sanity (`0x80072534`, file offset `0x62D34` in `800.s`):** plausible
Psy-Q-style C runtime startup — zero-fill loop over a RAM range, stack
pointer loaded from `D_8009CD70`/`D_8009CD74`, `$gp` set, `jal` to
auto-labeled `func_800726B4` then `func_8001220C`, ending in `break 0,1`.
Not a bare `j`/`jal` trampoline; consistent with crt0 calling into main.
**Image start (`0x80010000`):** jump table (`jtbl_80010000` with `.word`
targets in `0x80012xxx`) — plausible dispatch table, not random opcodes.

**Known limitations (expected, not bugs):** one asm file disassembles the
entire loadable image; data regions appear as instructions, `nonmatching`
labels, and out-of-range `func_*` entries in `undefined_funcs_auto.txt`.
No text/data/bss boundaries or real symbol names claimed.

**Guardrail fix on same date:** first run exited 1 because splat writes
boilerplate into tracked `include/` (alongside `include/.gitkeep`). Extended
`.gitignore` and `scripts/split_us.sh` `OUTPUT_PATHS`; re-run passes
post-split `git status` check.

## What is NOT verified

- Whether either dump matches redump.org itself (unreachable again on
  2026-07-04 retry, ECONNREFUSED; disc 1 page is
  http://redump.org/disc/116/). Non-blocking.
- Text/data/bss boundaries inside the image — **prefix + one mid-image boundary
  applied** (rodata 0x800–0x2A0B, asm 0x2A0C–0x8189F, rodata from 0x818A0);
  tail code after the data island (from ~0x800C22F8) is still folded into
  rodata until a resume-asm boundary is added. See Phase 3 boundary audit.
- Real symbol/function names — only splat auto-labels (`func_*`, `D_*`).
- The toolchain: `compiler: GCC` in the config is splat boilerplate, not a
  verified compiler identification (Phase 5+ fingerprinting).

## Next concrete step

1. **Phase 3 continued:** add the **resume-asm** boundary at file `0xB2AF8`
   (VRAM `0x800C22F8`) to close the mid-image data island opened at
   `0x818A0` — evidence: 8-byte zero padding then `addiu $sp,$sp,-0x18`
   prologue and sustained valid MIPS through `0x800C3xxx`. One boundary per
   commit; re-split and re-check pc0 each time. Optionally refine prefix
   rodata (`0xEE4`…`0x1E44`) or nested splits inside `818A0` (`0x93CCC`
   suggested by splat 0.41.0). Do not invent symbol names.
2. When redump.org is reachable, record the official cross-check in
   `docs/disc_info.md`.

### Phase 3 boundary audit (2026-07-07)

Evidence table for required anchors (from `asm/disc1/800.s` pre-split and
raw EXE bytes):

| Address | File offset | Location | Evidence | Classification | Confidence | Action |
| --- | --- | --- | --- | --- | --- | --- |
| `0x80010000` | `0x800` | `jtbl_80010000` | 62+ pointer words/256 B block; targets in `0x80012xxx` | jump table / rodata | high | Split as rodata (done) |
| `0x800119CC` | `0x21CC` | after 4 B nop | ASCII `none`, `CdlReadS`, `CdlSeekP`, … (Psy-Q CDL API names) | rodata / strings | high | Include in prefix rodata (done) |
| `0x80012000` | `0x2800` | mid-prefix | ASCII `file: searching...\n` | rodata / strings | high | Include in prefix rodata (done) |
| `0x8001211C` | `0x291C` | `jtbl_8001211C` | Trailing pointer table before code | jump table / rodata | high | Include in prefix rodata (done) |
| `0x8001220C` | `0x2A0C` | `func_8001220C` | First `addiu $sp,$sp,-0x28` prologue in scan; crt0 `jal` target; only prior `glabel` is 4 B `nop` at `0x800119C8` | text | high | **Asm subsegment start (applied)** |
| `0x80072534` | `0x62D34` | pc0 / `func_80072534` | BSS zero-fill loop, stack init, `$gp` setup, `jal func_800726B4`, `jal func_8001220C`, `break 0,1` | text (crt0) | high (already verified) | Do not split; anchor only |
| `D_8009CDF8` (`0x8009CDF8`) | `0x8D5F8` | crt0 `lui`/`addiu` | File bytes all zero; label is **runtime BSS zero-fill start**, not a ROM section edge | bss (runtime) | high it's NOT a ROM boundary | **Do not change yet** |
| `D_800C20C8` (`0x800C20C8`) | `0xB28C8` | crt0 `lui`/`addiu` | File holds `error : service thread…` string data; label is **runtime zero-fill end**, spans RAM not a single ROM section | data + runtime bss span | high it's NOT a ROM boundary | **Do not change yet** |
| `D_8009CD70` / `D_8009CD74` | `0x8D570` / `0x8D574` | crt0 stack setup | Initialized words/strings in ROM (`m0290i`, …); used as heap/stack bounds at runtime | data | medium | Defer; needs deeper data-map pass |

**Config changes applied:** `configs/USA/disc1.yaml` main subsegments now
`[0x800, rodata]` + `[0x2A0C, asm]` + `[0x818A0, rodata]`. Re-split
2026-07-08: splat 0.41.0 OK; output `asm/disc1/data/800.rodata.s` (prefix),
`asm/disc1/2A0C.s` (~149k lines, text through `func_80091080`),
`asm/disc1/data/818A0.rodata.s` (~412k lines, mid-image data island).
pc0 `func_80072534` unchanged/sane in `2A0C.s` (BSS zero-fill, stack init,
`jal func_800726B4`, `jal func_8001220C`, `break 0,1`). Splat still warns
prefix rodata nested splits (`0xEE4`…`0x1E44`) and suggests `0x93CCC`
inside `818A0` — deferred.

### Phase 3 next-boundary audit (2026-07-08)

Read-only audit inside former monolithic `2A0C.s` (raw SLUS_006.62 bytes +
generated asm). Sustained valid MIPS from `0x8001220C` through
`func_80091080` at `0x8009109C`; no early 64 KiB zero/ASCII islands.

| Address | File offset | Observed bytes/words | Class | Confidence | Evidence | Action |
| --- | --- | --- | --- | --- | --- | --- |
| `0x80073C54` | `0x64454` | `0x21017350`, `0x004237AD` between epilogue and `func_80073C5C` | embedded tramp/data | medium | 8 B gap; `jal` targets skip over it; not a section edge | Defer |
| `0x800910A0` | `0x818A0` | `0x80017294`, `0x800172BC`, … func pointers | rodata / pointer table | **high** | `func_80091080` epilogue at `0x8009109C`; `lw` from `0x80017230`; table ends `enddlabel D_800910A0` at `0x80091648` | **Applied** (`[0x818A0, rodata]`) |
| `0x80091464` | `0x81C64` | ASCII/glyph-like `,,,,L,,,,`, bitmap words | rodata | high | Inside `818A0` island; misdecoded as code before split | Included in `818A0` |
| `0x800917E0` | `0x81FE0` | `>?@Am0295i` repeated | rodata / heap tags | high | Matches Psy-Q `m0290i` family in crt0 stack setup | Included in `818A0` |
| `0x800930B4` | `0x838B4` | `0123456789abcdefghiklmnoprstuvwy` | rodata / charset | high | Pure ASCII in raw bytes | Included in `818A0` |
| `0x8009458C` | `0x84D8C` | Sony copyright string | rodata | high | Known Psy-Q SDK string | Included in `818A0` |
| `0x8009CB70` | `0x8D370` | 128-word pointer table to `0x8008Fxxx` | rodata | high | Dense `0x800xxxxx` pointers; `.word D_800910A0` xref at `0x8009454C` | Included in `818A0` |
| `0x800C20C8` | `0xB28C8` | `error : service thread not found` | data (also BSS runtime) | high NOT ROM edge | crt0 `lui`/`addiu` zero-fill end; ROM holds string | Do not split |
| `0x800C22F8` | `0xB2AF8` | zeros then `0x27BDFFE8` (`addiu $sp,$sp,-0x18`) | text (code resume) | high | First strong prologue after padding; sustained MIPS follows | **Next:** `[0xB2AF8, asm]` |
| `0x80072534` | `0x62D34` | crt0 startup | text | high (anchor) | pc0; do not use as split start | Anchor only |

## Open decisions

- License for original tooling/docs (see `docs/legal.md`).
- ~~Exact splat version / Python toolchain pinning~~ — decided
  2026-07-05: `splat64[mips]==0.41.0`, owned by `scripts/setup_env.sh`.
  Revisit only with a deliberate, recorded reason to bump.

## Rules reminder (never violate)

- No game data, images, extracted files, or SDK material in git.
- No invented decompiled C; no matching claims without the verify harness.
- Update this file after every meaningful change.

## Changelog

- 2026-07-08: **Phase 3 mid-image boundary:** audited `2A0C.s` monolith; applied
  `[0x818A0, rodata]` after `func_80091080` (`D_800910A0` func-pointer table).
  Re-split OK; pc0 sane; `2A0C.s` shrinks to ~149k lines, new
  `818A0.rodata.s` ~412k lines (git-ignored). Next: resume asm at `0xB2AF8`.
- 2026-07-07: **Phase 3 boundary audit + first split:** documented anchor
  evidence table; applied rodata/asm boundary at file `0x2A0C` (VRAM
  `0x8001220C`); re-split OK, pc0 sane, output git-ignored. No symbols/C
  committed.
- 2026-07-07: **First real split:** `scripts/split_us.sh` on
  `phase2-first-real-split` — splat 0.41.0 split 2 MB (100%); pc0
  `0x80072534` sanity OK (crt0-style startup). Fixed guardrail gap:
  splat `include/*.inc`/`*.h` now git-ignored and in `OUTPUT_PATHS`.
  Re-run passes post-split git check. No asm/symbols/C committed.
- 2026-07-07: Phase 2 smoke test: `scripts/setup_env.sh` installed
  splat64 0.41.0 into `.venv/`; `scripts/split_us.sh --check` passed all
  gates (repo root, config, EXE SHA-1, splat, gitignore coverage). No
  split invoked; no asm/symbols/C generated or committed; git status
  clean.
- 2026-07-05: Phase 2 guardrails: implemented `scripts/setup_env.sh`
  (pinned `.venv/` install of `splat64[mips]==0.41.0`); added `--check`
  dry-run mode plus a post-split `git status` diff check to
  `scripts/split_us.sh`; added splat's root-level auto-generated symbol
  list files to `.gitignore`; documented the toolchain and dry-run
  workflow in `docs/splitting.md`. No split executed (splat still not
  installed on this machine); no asm/symbols/C generated or committed.
- 2026-07-04: Phase 2 started: authored minimal splat config for
  SLUS_006.62 (verified values only, single conservative segment),
  disc2.yaml documented as pointer to disc 1, split_us.sh implemented
  with tested fail-loudly gates, docs/splitting.md added. No split run
  yet (splat not installed); no asm generated or committed.
- 2026-07-04: Closed the PE.IMG hypothesis — verified byte-identical
  across discs (SHA-1 146c0ce7...). Redump retry still ECONNREFUSED;
  recorded independent third-party hash corroboration for both dumps.
- 2026-07-04: Fixed extract_us.sh fail-loudly blocker (explicit per-step
  error handling + fresh output dir per run; failure paths re-tested).
  Disc 2 verification pass recorded in docs/disc_info.md. First verified
  RE observation: both boot EXEs byte-identical. Redump cross-checks
  still pending for both discs.
- 2026-07-04: Phase 1 disc 1 verification: implemented extract_us.sh +
  psxiso.py/psxexe_info.py/hashfile.py, extracted and hashed SLUS_006.62
  locally, recorded all disc 1 facts in docs/disc_info.md. Redump
  cross-check pending (site unreachable). Disc 2 pending.
- 2026-07-04: Phase 0 scaffold created (structure, docs, placeholders).
