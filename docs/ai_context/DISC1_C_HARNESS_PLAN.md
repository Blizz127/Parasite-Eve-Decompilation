# Disc 1 C/Matching Harness Design Audit (Phase 4D)

**Date:** 2026-07-08 (post 5c18fb3 "Record Phase 5 C conversion blocker")

**Goal of this audit:** Design the *smallest honest* C/matching harness so that a future Phase 5 attempt on `func_80090C38` (the recommended first target from `DISC1_FIRST_DECOMP_TARGETS.md`) can be done without faking progress.

**Rules observed in this pass:**
- Docs-only.
- No harness implementation.
- No C files added.
- No script edits.
- No config changes.
- No split boundary edits.
- Generated output (asm/, linkers/) read-only; nothing committed.
- No PC-port work.

## Repo state

- Branch: `phase5-disc1-first-c-leaf`
- HEAD: 5c18fb3 (the blocker commit)
- Working tree: clean
- Confirmed split map (unchanged):
  ```
  [0x800,     rodata]  prefix jump tables + strings
  [0x2A0C,    asm]     main text from func_8001220C
  [0x818A0,   rodata]  mid-image data island
  [0xB2AF8,   asm]     tail code from func_800C22F8
  ```
- `scripts/split_us.sh --check`: passes
- Generated output status: present locally (asm/disc1/2A0C.s, B2AF8.s, linkers/disc1.ld, etc.). All git-ignored. Never committed in this audit.
- Current blocker (from Phase 5 attempt): `verify_us.sh` is a non-functional placeholder. No rebuild path exists. `src/main/` is empty except `.gitkeep`.

Sources of truth read:
- `docs/ai_context/ACTIVE_HANDOFF.md`
- `docs/ai_context/DISC1_FUNCTION_INVENTORY.md`
- `docs/ai_context/DISC1_CALL_ANCHOR_MAP.md`
- `docs/ai_context/DISC1_FIRST_DECOMP_TARGETS.md`
- `scripts/split_us.sh`
- `scripts/verify_us.sh`
- `configs/USA/disc1.yaml`
- `CLAUDE.md` / `docs/project_plan.md`

## Current blocker summary

**Update after Phase 4E:** `scripts/verify_us.sh` is no longer a placeholder
for *split sanity*. Rebuild/matching remains the open blocker (Phase 4F).

Still open from the Phase 5 attempt:
- `scripts/verify_us.sh`: Phase 4E only — no assemble/link/compare yet.
- `src/main/`: only `.gitkeep` (0 bytes of real content).
- `configs/USA/disc1.yaml`: `src_path: src` is declared but unused. All subsegments are asm/rodata only. `compiler: GCC` is explicitly "standard splat boilerplate" — not a verified PSX C toolchain.
- No top-level build system (no Makefile, no rules for assembling split asm or compiling C).
- Linker script (`linkers/disc1.ld`) exists (generated) and references `build/asm/disc1/*.s.o` objects. No C object support yet.
- No object-level or binary comparison target.
- No documented way to extract/compile a *single* function while keeping the rest of the split intact.
- Per `CLAUDE.md` hard rule #1: "Until that harness exists, do not add code under `src/` at all."

## Current script inventory

- `scripts/extract_us.sh`: Phase 1, solid. Extracts EXE, verifies hashes (via tools), outputs to `build/extracted/` (ignored). Good foundation.
- `scripts/setup_env.sh`: Creates `.venv/`, pins `splat64[mips]==0.41.0`. Idempotent, root guards. Works.
- `scripts/split_us.sh`: Excellent. `--check` mode, root/config/EXE-hash/gitignore guards, runs splat, enforces no-committable outputs. Only produces study artifacts.
- `scripts/verify_us.sh`: **Phase 4E implemented** (split-artifact sanity). Rebuild + SHA-1 compare still Phase 4F.
- `tools/`: Python only (psxiso.py, psxexe_info.py, hashfile.py, etc.). No build tools.

No other scripts under `scripts/`.

## Current config/build inventory

- `configs/USA/disc1.yaml`:
  - `src_path: src` present (unused).
  - Segments: header + main (sub: rodata at 0x800, asm at 0x2A0C, rodata at 0x818A0, asm at 0xB2AF8).
  - No `c_files` or mixed asm+C segments.
  - `ld_script_path: linkers/disc1.ld`
  - `compiler: GCC` (boilerplate, see comments).
- `configs/USA/disc2.yaml`: Just points to disc1 (EXEs identical).
- `linkers/disc1.ld` (generated, read-only):
  - Sections: .header, .main (with explicit .text from 2A0C.s.o + B2AF8.s.o, .data, .rodata from data/*.rodata.s.o + 2A0C/B2AF8, .bss).
  - Uses `build/asm/disc1/... .o` objects.
  - AT() for ROM positioning, SUBALIGN(16).
  - No provision for C objects yet (e.g. no `build/src/*.o`).
- `build/`: Only `extracted/disc1/SLUS_006.62`. No objects, no intermediate build dir.
- `src/`: `main/` with only `.gitkeep`.
- `include/`: Some .inc files (likely from splat runs; tracked .gitkeep).
- No Makefile, no build/ rules, no toolchain wrapper.
- No `splat` "undefined_syms" or "symbol_addrs" for C yet.
- Toolchain: Expected "Psy-Q-era gcc" (per docs). Not vendored (correctly, per rules). No `mipsel-linux-gnu-` or equivalent in PATH observed in this env.

Splat 0.41.0 already generates usable `ld_script_path` and asm subsegments. It does *not* handle C compilation or mixed rebuilds.

## Required minimum harness pieces

To move from "blocker" to "can safely try one C function":

**For Phase 4E (minimal verification harness — no rebuild claims):**
- Make `scripts/verify_us.sh` actually run and report useful status.
- Always verify:
  - EXE hash (already in split check, but centralize).
  - `split_us.sh --check` passes.
  - Generated files exist and are gitignored.
  - Config boundaries match expectations.
  - splat version pinned.
- Explicitly report "Rebuild/matching not implemented yet" and exit non-zero for matching checks.
- No assembly or linking required.

**For Phase 4F (actual build/matching harness — enables Phase 5):**
- Way to assemble the split asm files into .o (splat or gas + mips toolchain).
- Minimal C compilation for one .c file (correct flags for PSX, no optimization that breaks matching).
- Link using the generated `linkers/disc1.ld` (or a C-aware variant) + objects from asm + the one C object.
- Produce a candidate EXE.
- Compare SHA-1 (or better: object diff or section diff) against original.
- Support "single function" mode: replace only the target function's object.
- Document exact compiler/flags once fingerprinted (Phase 5 work).
- Handle includes, undefined syms for the C side.

## Recommended file layout (minimal)

```
scripts/
  verify_us.sh          # upgraded (4E then 4F)
  build_us.sh           # new helper for 4F (or fold into verify)
build/
  asm/disc1/            # .o from asm (gitignored)
  src/                  # .o from C (gitignored)
  disc1.elf or .exe     # final candidate (gitignored)
src/
  main/
    80090C38.c          # future (not now)
include/
  (game types, sdk stubs as needed later)
configs/USA/
  disc1.yaml            # will gain C subsegment later
linkers/
  disc1.ld              # existing (generated)
tools/
  (add build helpers only if needed, keep Python)
```

Keep everything under `build/` gitignored. Never commit objects or rebuilt EXEs.

## Recommended first implementation phase

**Phase 4E (minimal verification harness) — IMPLEMENTED 2026-07-08.**

`scripts/verify_us.sh` now:

1. Checks repo root + git.
2. Confirms `configs/USA/disc1.yaml` contains the parked Phase 3 markers
   (`[0x800, rodata]`, `[0x2A0C, asm]`, `[0x818A0, rodata]`, `[0xB2AF8, asm]`).
3. Verifies EXE presence + SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
4. Runs `scripts/split_us.sh --check` and reports its output.
5. Confirms pinned splat64 `0.41.0` (prefer `.venv/bin/pip show splat64`).
6. Confirms expected generated files exist:
   `asm/disc1/header.s`, `2A0C.s`, `B2AF8.s`, `data/800.rodata.s`,
   `data/818A0.rodata.s`, `linkers/disc1.ld`.
7. Confirms split output paths are git-ignored (same set as `split_us.sh`).

Summary always includes:
`Rebuild/matching harness: NOT IMPLEMENTED YET (see Phase 4F …)`.

Exit 0 iff all checks pass; exit 1 on any real problem; exit 2 on usage.
No assembly, link, C, or matching logic.

Only after 4E is useful and committed should Phase 4F begin.

**Phase 4F (build/matching) later:**
- Add assembly step (use splat or `mipsel-linux-gnu-as` on the .s files to produce the .o under build/asm/).
- Add minimal C compile step (once a toolchain is documented).
- Extend linker or create a C-aware variant.
- Add comparison logic in verify (SHA-1 of rebuilt vs original, or better diffs).
- Support targeting a single function (e.g. via symbol replacement or partial link).

## What NOT to implement yet

- Any C source or stubs.
- Any Makefile.
- Any changes to `configs/USA/disc1.yaml` (no C subsegments).
- Any changes to `split_us.sh`.
- Full matching claims.
- Toolchain installation or vendoring (document only).
- Anything under `src/`.
- PC-port experiments.

## Risks / unknowns

- Exact Psy-Q gcc version + flags that produced the original binary (fingerprinting needed in Phase 5).
- Whether the current linker script (which hardcodes asm .o paths) can be extended without breaking.
- How to handle the mid-image rodata island + BSS when mixing C.
- Whether splat's current output (with "nonmatching" labels etc.) will assemble cleanly.
- Runtime BSS zero-fill expectations (D_8009CDF8 etc.) — must not be treated as ROM data.
- Size of the minimal harness: keep it tiny so one leaf function (func_80090C38, 5 instructions) can be the first real test.
- Over-engineering risk: do not build a full decomp build system on day 1.

## Phase 4E status

**Done (2026-07-08).** Commit message target:
`Implement Phase 4E minimal verification harness`.

Evidence on this worktree (no local extract/venv/split artifacts):
- `bash -n scripts/verify_us.sh` clean
- `./scripts/verify_us.sh` exits 1 with 9 expected FAILs (missing EXE,
  split --check, splat, six artifact paths) and still prints the Phase 4F
  "NOT IMPLEMENTED" banner
- Config boundary markers + gitignore gates pass without local data

On a fully provisioned machine (extract + setup_env + split), the same
script should exit 0 with "Verification (Phase 4E): split artifacts OK."

## Exact proposed next implementation prompt (Phase 4F)

```
Parasite Eve decomp Phase 4F: first build/matching harness pieces.

Start only after Phase 4E is committed and verify_us.sh is the split sanity gate.

Read first:
- docs/ai_context/ACTIVE_HANDOFF.md
- docs/ai_context/DISC1_C_HARNESS_PLAN.md
- scripts/verify_us.sh
- scripts/split_us.sh
- configs/USA/disc1.yaml
- linkers/disc1.ld (generated, read-only study)

Goal:
Add the smallest possible rebuild path that can assemble current split asm
and (later) one C leaf — without claiming a match until compare logic works.

Rules:
1. Do not add C under src/ until assemble+link of pure asm succeeds.
2. Do not claim matching until SHA-1 (or better object/section diff) is wired.
3. Keep build outputs under build/ and git-ignored.
4. Document toolchain requirements; do not vendor proprietary Psy-Q.
5. Prefer extending verify_us.sh or a new build_us.sh over a sprawling Makefile
   on day 1 — stay minimal.

Do not jump to Phase 5 C conversion until 4F can rebuild pure-asm and compare.
```

Phase 4D audit (docs-only design) produced this plan. Phase 4E implements
the minimal verify path described above. Phase 4F remains unimplemented.