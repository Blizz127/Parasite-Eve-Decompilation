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

From the Phase 5 blocker commit and inspections:
- `scripts/verify_us.sh`: always exits 1 with "Phase 0 placeholder" message. No actual rebuild or comparison logic.
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
- `scripts/verify_us.sh`: **Pure placeholder**. 10 lines. Always fails. Intended for Phase 4 rebuild + SHA-1 compare vs recorded original.
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

**Phase 4E (minimal verification harness) — do this first, before any C:**

1. Rewrite `scripts/verify_us.sh` to:
   - Run all the checks that `split_us.sh --check` does (or call it).
   - Verify EXE SHA-1.
   - Confirm `asm/`, `linkers/`, etc. exist and are gitignored.
   - Confirm expected files from current split (2A0C.s, B2AF8.s, data/*.rodata.s, header.s, disc1.ld).
   - Print clear summary:
     "Verification (Phase 4E): split artifacts OK. Rebuild/matching harness NOT IMPLEMENTED YET."
   - Exit 0 for "artifacts good", non-zero only for real problems.
2. Update `ACTIVE_HANDOFF.md` and this plan with results.
3. Make `verify_us.sh` the single source of "is the split state sane?"
4. Do **not** attempt to assemble or link anything.

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

## Exact proposed next implementation prompt

```
Parasite Eve decomp Phase 4E: minimal verification harness.

Start only after this plan (DISC1_C_HARNESS_PLAN.md) is committed.

Read first:
- docs/ai_context/ACTIVE_HANDOFF.md
- docs/ai_context/DISC1_C_HARNESS_PLAN.md
- scripts/verify_us.sh (current placeholder)
- scripts/split_us.sh
- configs/USA/disc1.yaml

Goal:
Make scripts/verify_us.sh actually useful as a Phase 4E "split artifacts are sane" checker. Do not implement any rebuild, assembly, C compilation, or matching logic yet.

Rules:
1. Edit only scripts/verify_us.sh and docs (ACTIVE_HANDOFF.md + this plan if needed).
2. Do not touch configs, src/, Makefiles, or generated output.
3. Do not claim any matching or rebuild capability.
4. Keep it minimal: reuse existing checks from split_us.sh --check where possible.
5. The script must clearly state that rebuild/matching is not implemented.
6. No generated asm/linker/assets commits.

Implementation (minimum):
- Make verify_us.sh run the equivalent of split_us.sh --check.
- Verify EXE hash.
- Verify presence of expected generated files (asm/disc1/2A0C.s, B2AF8.s, data/*.rodata.s, header.s, linkers/disc1.ld).
- Verify they are gitignored.
- Verify splat version.
- Print a clear summary and "Rebuild/matching harness: NOT IMPLEMENTED YET (see Phase 4F)".
- Exit 0 if artifacts good, 1 on problems.
- Update handoff with exact behavior.

Deliverable:
- Working (but limited) verify_us.sh
- Updated ACTIVE_HANDOFF.md
- Commit: "Implement Phase 4E minimal verification harness"

After this, a future Phase 4F can add the actual build pieces. Do not jump ahead.
```

This plan is the complete output of the Phase 4D audit. All evidence came from direct inspection of scripts, configs, generated artifacts (read-only), source tree, and project docs. No implementation performed.