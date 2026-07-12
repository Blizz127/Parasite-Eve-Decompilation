# ACTIVE HANDOFF

Single source of truth for the current working state. Any agent or human
picking up this project reads this first and updates it after every
meaningful change.

## Current phase

**Phase 5BC — `func_800822AC` integrated (fifty-second matching C leaf)**
(branch `phase5ae-2a0c-hole-aware`, derived from accepted Phase 5BB commit
`e9f46c1`). Fifty-two matching C leaves. Another **32-bit global getter**
(`lui/lw/jr/nop`, returns `D_8009B70C` as `int`); **plain -O1 matches**.
Mid-`71150.s` carve.
**Parked:** `func_8003DFD0` return-0 stub (5I-class `move` vs `addu` in
delay slot — GCC 14.2 emits `00001025` not `21100000`, same blocker as
`800C7DC4`) + sb-stub / accessor families + setter `func_8003FFAC`.

Oracle: `scripts/build_us.sh` exits 0 with exact SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` (fifty-two leaves).

**Provenance:** clean target-only derivation from accepted commit `e9f46c1`
(Phase 5BB / 51 leaves). `git stash list` is empty. The extracted 16-byte
5BC scratch-linked code slice and production probe both match raw ROM bytes
at file `0x72AAC`. The scratch linker inserted four leading alignment bytes;
production trim/link placement is independently exact at `0x72AAC`.
`verify_us.sh` edits the current-phase arm in place; the next historical arm
remains 5AQ, matching committed precedent.

Scope reset: accidental broad commit `c62e642` was discarded by resetting to
its clean parent `9bb3099`; isolated Phase 5AU was then accepted and committed
as `c3a8424`, Phase 5AV as `7323079`, Phase 5AW as `7902dd2`, Phase 5AX
as `27a6ba2`, Phase 5AY as `ffbff11`, and Phase 5AZ as `86e4a48`. The current
accepted chain continues with Phase 5BA as `79cdc7e`; Phase 5BB is isolated
and committed as `e9f46c1`. Phase 5BC is isolated to the standard five-file
leaf scope.

`README.md` and `CLAUDE.md` still contain pre-commit Phase 5AX checkpoint
wording from `27a6ba2`; they are intentionally left untouched in this
five-file leaf scope. This handoff is authoritative for the live state.

Solid-state config (`configs/USA/disc1.yaml`):

```text
[0x800,     rodata]
[0x2A0C,    asm]
[0x869C,    c, func_80017E9C]  VRAM 0x80017E9C, size 0x8 (Phase 5AK)
[0x86A4,    asm]
[0x9850,    c, func_80019050]  VRAM 0x80019050, size 0x8 (Phase 5AL)
[0x9858,    c, func_80019058]  VRAM 0x80019058, size 0x8 (Phase 5AM)
[0x9860,    asm]
[0x98AC,    c, func_800190AC]  VRAM 0x800190AC, size 0x8 (Phase 5AN)
[0x98B4,    c, func_800190B4]  VRAM 0x800190B4, size 0x8 (Phase 5AN)
[0x98BC,    asm]
[0x2E02C,   c, func_8003D82C]  VRAM 0x8003D82C, size 0x8 (Phase 5AJ)
[0x2E034,   asm]
[0x2E7C8,   c, func_8003DFC8]  VRAM 0x8003DFC8, size 0x8 (Phase 5AE)
[0x2E7D0,   asm]
[0x307BC,   c, func_8003FFBC]  VRAM 0x8003FFBC, size 0x10 (Phase 5AT)
[0x307CC,   asm]
[0x330C4,   c, func_800428C4]  VRAM 0x800428C4, size 0x10 (Phase 5AP)
[0x330D4,   asm]
[0x33328,   c, func_80042B28]  VRAM 0x80042B28, size 0x10 (Phase 5AW)
[0x33338,   asm]
[0x3E29C,   c, func_8004DA9C]  VRAM 0x8004DA9C, size 0x8 (Phase 5AO)
[0x3E2A4,   asm]
[0x41518,   c, func_80050D18]  VRAM 0x80050D18, size 0x8 (Phase 5AF)
[0x41520,   asm]
[0x42034,   c, func_80051834]  VRAM 0x80051834, size 0x18 (Phase 5AV)
[0x4204C,   asm]
[0x42648,   c, func_80051E48]  VRAM 0x80051E48, size 0x10 (Phase 5AQ)
[0x42658,   asm]
[0x42D14,   c, func_80052514]  VRAM 0x80052514, size 0x10 (Phase 5AR)
[0x42D24,   c, func_80052524]  VRAM 0x80052524, size 0x10 (Phase 5AS)
[0x42D34,   asm]
[0x42FC0,   c, func_800527C0]  VRAM 0x800527C0, size 0x8 (Phase 5AG)
[0x42FC8,   asm]
[0x4C4A8,   c, func_8005BCA8]  VRAM 0x8005BCA8, size 0x8 (Phase 5AH)
[0x4C4B0,   asm]
[0x645E8,   c, func_80073DE8]  VRAM 0x80073DE8, size 0x10 (Phase 5AU)
[0x645F8,   asm]
[0x65228,   c, func_80074A28]  VRAM 0x80074A28, size 0x10 (Phase 5AX)
[0x65238,   asm]
[0x6E6B0,   c, func_8007DEB0]  VRAM 0x8007DEB0, size 0x10 (Phase 5AY)
[0x6E6C0,   asm]
[0x6FF78,   c, func_8007F778]  VRAM 0x8007F778, size 0x10 (Phase 5AZ)
[0x6FF88,   asm]
[0x704AC,   c, func_8007FCAC]  VRAM 0x8007FCAC, size 0x10 (Phase 5BA)
[0x704BC,   asm]
[0x71140,   c, func_80080940]  VRAM 0x80080940, size 0x10 (Phase 5BB)
[0x71150,   asm]
[0x72AAC,   c, func_800822AC]  VRAM 0x800822AC, size 0x10 (Phase 5BC)
[0x72ABC,   asm]
[0x7D27C,   c, func_8008CA7C]  VRAM 0x8008CA7C, size 0x8 (Phase 5AI)
[0x7D284,   asm]
[0x7FE94,   c, func_8008F694]  VRAM 0x8008F694, size 0x14 (Phase 5K)
[0x7FEA8,   c, func_8008F6A8]  VRAM 0x8008F6A8, size 0x8 (Phase 5AC)
[0x7FEB0,   asm]
[0x80068,   c, func_8008F868]  VRAM 0x8008F868, size 0x18 (Phase 5L)
[0x80080,   c, func_8008F880]  VRAM 0x8008F880, size 0x18 (Phase 5M)
[0x80098,   asm]
[0x804B4,   c, func_8008FCB4]  VRAM 0x8008FCB4, size 0x8 (Phase 5N)
[0x804BC,   asm]
[0x80CA0,   c, func_800904A0]  VRAM 0x800904A0, size 0xC (Phase 5O)
[0x80CAC,   c, func_800904AC]  VRAM 0x800904AC, size 0x8 (Phase 5P)
[0x80CB4,   c, func_800904B4]  VRAM 0x800904B4, size 0x8 (Phase 5Q)
[0x80CBC,   c, func_800904BC]  VRAM 0x800904BC, size 0x8 (Phase 5R)
[0x80CC4,   asm]
[0x80EB4,   c, func_800906B4]  VRAM 0x800906B4, size 0x30 (Phase 5S)
[0x80EE4,   asm]
[0x8120C,   c, func_80090A0C]  VRAM 0x80090A0C, size 0x14 (Phase 5J)
[0x81220,   asm]
[0x81438,   c, func_80090C38]
[0x8144C,   c, func_80090C4C]
[0x81460,   c, func_80090C60]
[0x81474,   c, func_80090C74]
[0x81488,   asm]
[0x81754,   c, func_80090F54]
[0x81768,   asm]
[0x818A0,   rodata]
[0xB2AF8,   asm]
[0xB3340,   c, func_800C2B40]  VRAM 0x800C2B40, size 0x10 (Phase 5G)
[0xB3350,   asm]
[0xB8A68,   c, func_800C8268]  VRAM 0x800C8268, size 0x8 (Phase 5T)
[0xB8A70,   asm]
[0xB9A60,   c, func_800C9260]  VRAM 0x800C9260, size 0x8 (Phase 5U)
[0xB9A68,   asm]
[0xBA6A0,   c, func_800C9EA0]  VRAM 0x800C9EA0, size 0x8 (Phase 5V)
[0xBA6A8,   asm]
[0xBB4D4,   c, func_800CACD4]  VRAM 0x800CACD4, size 0x8 (Phase 5W)
[0xBB4DC,   asm]
[0xBDADC,   c, func_800CD2DC]  VRAM 0x800CD2DC, size 0x8 (Phase 5X)
[0xBDAE4,   c, func_800CD2E4]  VRAM 0x800CD2E4, size 0x8 (Phase 5Y)
[0xBDAEC,   asm]
[0xBDD9C,   c, func_800CD59C]  VRAM 0x800CD59C, size 0x8 (Phase 5Z)
[0xBDDA4,   asm]
[0xBE504,   c, func_800CDD04]  VRAM 0x800CDD04, size 0x8 (Phase 5AA)
[0xBE50C,   asm]
[0xBEBAC,   c, func_800CE3AC]  VRAM 0x800CE3AC, size 0x8 (Phase 5AB)
[0xBEBB4,   asm]
```

**Prior on `main`:** Phase 5F (PR #14), 5E–5B, 4J–4G, Phase 3 parked boundaries.

Phase 1 complete locally; only the official redump cross-check remains open (non-blocking).

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
  git-ignored, never committed). Fifty-two matching C leaves now exist; no
  PC-port work exists. `scripts/setup_env.sh` is implemented (pinned `.venv/`
  install); `verify_us.sh` checks split artifacts and reports the current
  rebuild/C-leaf state. **`scripts/build_us.sh`:** assemble/compile/link/pack/
  compare oracle, exiting 0 only on an exact SHA-1 match. **MIPS LE path:**
  Distrobox `pe-mipsel` (Phase 4G; not on host PATH).

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
- Text/data/bss boundaries inside the image — **prefix + mid-image
  boundaries closed and parked** (see solid state in Current phase above).
  Mid-image and prefix nested audits complete (2026-07-08): no
  extremely-high-confidence nested split found in either region. Phase 3
  boundary work is parked; only pursue further splits on true misclassifications.
- Real symbol/function names — only splat auto-labels (`func_*`, `D_*`).
- The **original game** compiler: `compiler: GCC` in the config is splat
  boilerplate, not a verified identification (Phase 5+ fingerprinting).
- A **modern** MIPS LE assembler/linker path is provisioned (Phase 4G Distrobox
  `pe-mipsel`, binutils 2.44). Phase 4H+4I: asm-only rebuild is an **exact
  SHA-1 match** via `scripts/build_us.sh` (exit 0 only on match). Phase 4J:
  modern GCC 14.2 in `pe-mipsel` emits exact words for the 90Cxx/90F54 leaves at -O1+.
  **Phase 5B–5BC done:** fifty-two production C leaves. Completed
  batches: mid-`2A0C` empty-stub batch `func_80050D18` / `func_800527C0` /
  `func_8005BCA8` / `func_8008CA7C` plus `func_8003DFC8`; **return-1 septuplet
  batch `func_8003D82C` / `func_80017E9C` / `func_80019050` / `func_80019058` /
  `func_800190AC` / `func_800190B4` / `func_8004DA9C`** (identical `jr; li v0,1` —
  Phase 5AJ–5AO) plus **post-septuplet leaves `func_800428C4`**
  (`lui/lw/jr/addiu -1`), **`func_80051E48`** (pure address return
  `lui/addiu/jr/nop`, `-fno-delayed-branch`) **and the 16-bit getter twins
  `func_80052514` / `func_80052524` / `func_80073DE8`** (`lui/lhu/jr/nop`,
  plain -O1), **`func_8003FFBC`** (`lui/lw/jr/nop`, plain -O1), **and
  `func_80051834`** (unsigned indexed bit test, `lui/lw/nop/srlv/jr/andi`,
  plain -O1), **`func_80042B28`** (32-bit global getter,
  `lui/lw/jr/nop`, plain -O1), **`func_80074A28`** (32-bit global getter,
  `lui/lw/jr/nop`, plain -O1), **`func_8007DEB0`** (32-bit global getter,
  `lui/lw/jr/nop`, plain -O1), **`func_8007F778`** (32-bit global getter,
  `lui/lw/jr/nop`, plain -O1), **`func_8007FCAC`** (32-bit global getter,
  `lui/lw/jr/nop`, plain -O1), **`func_80080940`** (32-bit global getter,
  `lui/lw/jr/nop`, plain -O1), **and `func_800822AC`** (32-bit global getter,
  `lui/lw/jr/nop`, plain -O1). Blocked `func_8003DFD0` return-0 (5I-class
  `move` vs `addu`). Existing leaves include the mid-region trio
  `func_8008F694` / `func_8008F868` / `func_8008F880`, the 904xx cluster,
  `func_800906B4`, early 90Cxx leaves, and the completed tail empty-stub batch.

## Next concrete step

**Milestone:** fifty-two matching C leaves on branch
`phase5ae-2a0c-hole-aware` (Phase 5BC: `func_800822AC` — 32-bit global
getter, plain -O1). Fifty-two leaves exact SHA-1.
Return-1 septuplet (7/7) remains complete. Parked: `func_8003DFD0`
return-0 (5I move vs addu), setter `func_8003FFAC` ($at vs $v0),
sb-stub / 5I-class / accessor families.

**Next:** commit this isolated exact target, then continue the established
carve → scratch probe → production exact-match flow one leaf at a time.
Read-only triage ranks `func_800870E0` as the next same-shape getter; it must
still pass its own scratch and production validation before acceptance.
Oracle:

```text
build_us.sh  → exit 0 only on exact SHA-1 match
verify_us.sh → reports rebuild status when candidate present
SHA-1        → 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

**Phase 5BC result — `func_800822AC` (2026-07-12):** VRAM `0x800822AC` /
file `0x72AAC` / size `0x10`. 32-bit global getter —
`lui $v0,%hi(D_8009B70C); lw $v0,%lo(D_8009B70C)($v0); jr $ra; nop`,
i.e. `int func(void){ return D_8009B70C; }`. Plain -O1 matches. Mid-`71150.s`
carve: prefix `0x195C`, C `0x10`, resume `72ABC.s` `0xA7C0` (sums to prior
`0xC12C`). The extracted 16-byte scratch-linked code slice and production
probe match raw ROM `0a80023c 0cb7428c 0800e003 00000000`; the scratch
binary itself has four leading linker-alignment bytes. Re-split + production
**EXACT MATCH** at the correct address; fifty-two leaves. `verify_us.sh`
exits 0 and reports Phase 5BC.

**Phase 5BB result — `func_80080940` (2026-07-12):** VRAM `0x80080940` /
file `0x71140` / size `0x10`. 32-bit global getter —
`lui $v0,%hi(D_8009B554); lw $v0,%lo(D_8009B554)($v0); jr $ra; nop`,
i.e. `int func(void){ return D_8009B554; }`. Plain -O1 matches. Mid-`704BC.s`
carve: prefix `0xC84`, C `0x10`, resume `71150.s` `0xC12C` (sums to prior
`0xCDC0`). The complete 16-byte scratch-linked binary and production probe
match raw ROM `0a80023c 54b5428c 0800e003 00000000`. Re-split + production
**EXACT MATCH** at the correct address; fifty-one leaves. `verify_us.sh`
exits 0 and reports Phase 5BB.

**Phase 5BA result — `func_8007FCAC` (2026-07-12):** VRAM `0x8007FCAC` /
file `0x704AC` / size `0x10`. 32-bit global getter —
`lui $v0,%hi(D_8009B590); lw $v0,%lo(D_8009B590)($v0); jr $ra; nop`,
i.e. `int func(void){ return D_8009B590; }`. Plain -O1 matches. Mid-`6FF88.s`
carve: prefix `0x524`, C `0x10`, resume `704BC.s` `0xCDC0` (sums to prior
`0xD2F4`). The extracted 16-byte scratch-linked code slice and production
probe both match raw ROM `0a80023c 90b5428c 0800e003 00000000`; the scratch
binary itself has four leading linker-alignment bytes. Re-split + production
**EXACT MATCH** at the correct address; fifty leaves. `verify_us.sh` exits 0
and reports Phase 5BA.

**Phase 5AZ result — `func_8007F778` (2026-07-12):** VRAM `0x8007F778` /
file `0x6FF78` / size `0x10`. 32-bit global getter —
`lui $v0,%hi(D_800A3608); lw $v0,%lo(D_800A3608)($v0); jr $ra; nop`,
i.e. `int func(void){ return D_800A3608; }`. Plain -O1 matches. Mid-`6E6C0.s`
carve: prefix `0x18B8`, C `0x10`, resume `6FF88.s` `0xD2F4` (sums to prior
`0xEBBC`). Scratch-linked bytes and the production probe both match raw ROM
`0a80023c 0836428c 0800e003 00000000`. Re-split + production **EXACT
MATCH**; forty-nine leaves. `verify_us.sh` exits 0 and reports Phase 5AZ.

**Phase 5AY result — `func_8007DEB0` (2026-07-12):** VRAM `0x8007DEB0` /
file `0x6E6B0` / size `0x10`. 32-bit global getter —
`lui $v0,%hi(D_8009B4AC); lw $v0,%lo(D_8009B4AC)($v0); jr $ra; nop`,
i.e. `int func(void){ return D_8009B4AC; }`. Plain -O1 matches. Mid-`65238.s`
carve: prefix `0x9478`, C `0x10`, resume `6E6C0.s` `0xEBBC` (sums to prior
`0x18044`). Scratch-linked bytes and the production probe both match raw ROM
`0a80023c acb4428c 0800e003 00000000`. Re-split + production **EXACT
MATCH**; forty-eight leaves. `verify_us.sh` exits 0 and reports Phase 5AY.

**Phase 5AX result — `func_80074A28` (2026-07-11):** VRAM `0x80074A28` /
file `0x65228` / size `0x10`. 32-bit global getter —
`lui $v0,%hi(D_800956EC); lw $v0,%lo(D_800956EC)($v0); jr $ra; nop`,
i.e. `int func(void){ return D_800956EC; }`. Plain -O1 matches. Mid-`645F8.s`
carve: prefix `0xC30`, C `0x10`, resume `65238.s` `0x18044` (sums to prior
`0x18C84`). Clean re-derivation from `7902dd2` after a prior attempt's
safety claim was found unverifiable (prior work was preserved for comparison,
then dropped after accepted commit `27a6ba2`). `verify_us.sh` ladder: in-place 5AX arm matching the 5AW
precedent in `7902dd2` (no preserved 5AW `elif`). Re-split + production
**EXACT MATCH**; forty-seven leaves. `verify_us.sh` exits 0 and reports
Phase 5AX.

**Phase 5AW result — `func_80042B28` (2026-07-11):** VRAM `0x80042B28` /
file `0x33328` / size `0x10`. 32-bit global getter —
`lui $v0,%hi(D_800A1838); lw $v0,%lo(D_800A1838)($v0); jr $ra; nop`,
i.e. `int func(void){ return D_800A1838; }`. Plain -O1 matches. The target
is currently carved from `330D4.s` (the original wider `2E7D0.s` family):
prefix `0x254`, C `0x10`, resume `33338.s` `0xAF64` (sums to prior
`0xB1C8`). Re-split + production **EXACT MATCH**; forty-six leaves.
`verify_us.sh` exits 0 and reports Phase 5AW.

**Phase 5AV result — `func_80051834` (2026-07-11):** VRAM `0x80051834` /
file `0x42034` / size `0x18`. Unsigned indexed bit test —
`lui $v0,%hi(D_800C0E24); lw $v0,%lo(D_800C0E24)($v0); nop; srlv $v0,$v0,$a0; jr $ra; andi $v0,$v0,1`,
i.e. `int func(int a0){ return (D_800C0E24 >> a0) & 1; }` with
`D_800C0E24` declared `unsigned int` (a signed declaration emits the wrong
`srav`). Plain -O1 matches. Mid-`41520.s` carve: prefix `0xB14`, C `0x18`,
resume `4204C.s` `0x5FC` (sums to prior `0x1128`). Re-split + production
**EXACT MATCH**; forty-five leaves. `verify_us.sh` exits 0 and reports Phase
5AV.

**Phase 5AU result — `func_80073DE8` (2026-07-11):** VRAM `0x80073DE8` /
file `0x645E8` / size `0x10`. 16-bit global getter —
`lui $v0,%hi(D_800945E6); lhu $v0,%lo(D_800945E6)($v0); jr $ra; nop`,
i.e. `unsigned short func(void){ return D_800945E6; }`. Plain -O1 matches.
Mid-`4C4B0.s` carve: prefix `0x18138`, C `0x10`, resume `645F8.s`
`0x18C84` (sums to prior `0x30DCC`). Re-split + production **EXACT MATCH**;
forty-four leaves. `verify_us.sh` exits 0 and reports Phase 5AU.

**Phase 5AT result — `func_8003FFBC` (2026-07-11):** VRAM `0x8003FFBC` /
file `0x307BC` / size `0x10`. 32-bit global getter —
`lui $v0,%hi(D_800A1704); lw $v0,%lo(D_800A1704)($v0); jr $ra; nop`,
i.e. `int func(void){ return D_800A1704; }`. Plain -O1 matches. Mid-`2E7D0.s`
carve: prefix `0x1FEC`, C `0x10`, resume `307CC.s` `0x28F8` (sums to prior
`0x48F4`). Setter twin `func_8003FFAC` left in asm. Re-split + production
**EXACT MATCH**; forty-three leaves.

**Phase 5AS result — `func_80052524` (2026-07-10):** VRAM `0x80052524` /
file `0x42D24` / size `0x10`. Twin of `func_80052514` —
`lui $v0,%hi(D_800C0E32); lhu $v0,%lo(D_800C0E32)($v0); jr $ra; nop`,
i.e. `unsigned short func(void){ return D_800C0E32; }`. Plain -O1 matches
(same MIPS-I load-delay reason as 5AR). Zero-prefix carve of former
`42D24.s`: C `0x10` + resume `42D34.s` `0x28C` (sums to prior `0x29C`).
Re-split + production **EXACT MATCH**; forty-two leaves.

**Phase 5AR result — `func_80052514` (2026-07-10):** VRAM `0x80052514` /
file `0x42D14` / size `0x10`. First **16-bit global getter** —
`lui $v0,%hi(D_800C0E28); lhu $v0,%lo(D_800C0E28)($v0); jr $ra; nop`,
i.e. `unsigned short func(void){ return D_800C0E28; }`. jr delay slot is
nop but **plain -O1 matches**: on MIPS-I the `lhu` cannot fill the return
delay slot (load-delay hazard), so no `-fno-delayed-branch` needed.
`D_800C0E28` auto-resolves absolute (0x800C0E28) via probe-link fallback.
Mid-`42658.s` carve: prefix `0x6BC`, C `0x10`, resume then further carved
by 5AS. Re-split + production **EXACT MATCH**; forty-one leaves.

**Phase 5AQ result — `func_80051E48` (2026-07-10):** VRAM `0x80051E48` /
file `0x42648` / size `0x10`. First **pure address-return** leaf —
`lui $v0,%hi(D_800A1B30); addiu $v0,$v0,%lo(D_800A1B30); jr $ra; nop`,
i.e. `return &D_800A1B30;`. ROM leaves the jr delay slot **unfilled**;
GCC 14.2 -O1 otherwise fills it (`lui; jr; addiu`), so this one C unit
compiles with `-fno-delayed-branch` (per-file, `build_us.sh`). `D_800A1B30`
auto-resolves as an absolute (0x800A1B30) via the probe-link fallback.
Mid-`41520.s` carve: prefix `0x1128`, C `0x10`, resume `42658.s` `0x968`
(sums to prior `0x1AA0`). Re-split + production **EXACT MATCH**; forty leaves.

**Phase 5AO result — `func_8004DA9C` (2026-07-10):** VRAM `0x8004DA9C` /
file `0x3E29C` / size `0x8`. Return-1 stub, seventh/final of the seven twins.
Mid-`2E7D0.s` carve: prefix `0xFACC`, C `0x8`, resume `3E2A4.s` `0x3274`
(sums to prior `0x12D48`). Shape scratch-proven in 5AJ. Re-split +
production **EXACT MATCH**; thirty-eight leaves. Return-1 twin batch done.

**Phase 5AN result — `func_800190AC` + `func_800190B4` (2026-07-10):**
VRAM `0x800190AC` / `0x800190B4`, files `0x98AC` / `0x98B4`, size `0x8`
each. Return-1 stubs, fifth and sixth of the seven twins. Mid-`9860.s`
carve: prefix `0x4C`, two adjacent C leaves, resume `98BC.s` `0x24770`
(sums to prior `0x247CC`). Shape scratch-proven in 5AJ. Re-split +
production **EXACT MATCH**; thirty-seven leaves. Last twin:
`func_8004DA9C` in `2E7D0.s`.

**Phase 5AM result — `func_80019058` (2026-07-10):** VRAM `0x80019058` /
file `0x9858` / size `0x8`. Return-1 stub, fourth of the seven twins.
Zero-prefix carve of former `9858.s`: C `0x8`, resume `9860.s` `0x247CC`
(sums to prior `0x247D4`). Shape scratch-proven in 5AJ (identical bytes).
Re-split + production **EXACT MATCH**; thirty-five leaves. Remaining twins
in `9860.s`: `func_800190AC`, `func_800190B4`.

**Phase 5AL result — `func_80019050` (2026-07-09):** VRAM `0x80019050` /
file `0x9850` / size `0x8`. Return-1 stub, third of the seven twins.
Mid-`86A4.s` carve: prefix `0x11AC`, C `0x8`, resume `9858.s` `0x247D4`
(sums to prior `0x25988`). Shape scratch-proven in 5AJ (identical bytes).
Re-split + production **EXACT MATCH**; thirty-four leaves. Next twin
`func_80019058` sits at the head of `9858.s` (zero-prefix carve).

**Phase 5AK result — `func_80017E9C` (2026-07-09):** VRAM `0x80017E9C` /
file `0x869C` / size `0x8`. Return-1 stub, second of the seven twins;
rodata dispatch-table target (`.word func_80017E9C` at file `0x819D0`).
Mid-`2A0C.s` carve: prefix `0x5C90`, C `0x8`, resume `86A4.s` `0x25988`
(sums to prior `0x2B620`). Shape scratch-proven in 5AJ (identical bytes).
Re-split + production **EXACT MATCH**; thirty-three leaves.

**Phase 5AJ result — `func_8003D82C` (2026-07-09):** VRAM `0x8003D82C` /
file `0x2E02C` / size `0x8`. Return-1 stub (`jr $ra; addiu $v0,$zero,1` —
`li` in delay slot), first of the seven byte-identical return-1 twins from
the read-only triage. Mid-`2A0C.s` carve: prefix `0x2B620`, C `0x8`, resume
`2E034.s` `0x794` (sums to prior `0x2BDBC`). Scratch probe
(`int func(void){return 1;}`, GCC 14.2 Phase 4J flags) emitted the exact
words `0800E003 01000224`. Re-split + production **EXACT MATCH**; thirty-two
leaves. Remaining six twins are one-carve-each with the same proven C shape.

**Phase 5AI result — `func_8008CA7C` (2026-07-09):** VRAM `0x8008CA7C` /
file `0x7D27C` / size `0x8`. Final mid-`2A0C` empty `jr`/`nop` stub carve:
prefix `4C4B0.s` `0x30DCC`, C `0x8`, resume `7D284.s` `0x2C10` (sums to
prior `0x339E4`). Re-split + production **EXACT MATCH**; thirty-one leaves.

**Phase 5AH result — `func_8005BCA8` (2026-07-09):** VRAM `0x8005BCA8` /
file `0x4C4A8` / size `0x8`. Empty `jr`/`nop` stub carve: prefix `42FC8.s`
`0x94E0`, C `0x8`, resume `4C4B0.s` `0x339E4` (sums to prior `0x3CECC`).
Re-split + production **EXACT MATCH**; thirty leaves.

**Phase 5AG result — `func_800527C0` (2026-07-09):** VRAM `0x800527C0` /
file `0x42FC0` / size `0x8`. Empty `jr`/`nop` stub carve: prefix `41520.s`
`0x1AA0`, C `0x8`, resume `42FC8.s` `0x3CECC` (sums to prior `0x3E974`).
Re-split + production **EXACT MATCH**; twenty-nine leaves.

**Phase 5AF result — `func_80050D18` (2026-07-09):** VRAM `0x80050D18` / file
`0x41518` / size `0x8`. Empty `jr`/`nop` stub, second mid-`2A0C` carve (from
`2E7D0.s`): prefix `0x12D48`, C `0x8`, resume `41520.s` `0x3E974` (sums to
prior `0x516C4`). Scratch + production **EXACT MATCH**.

**Phase 5AE result — `func_8003DFC8` (2026-07-09):** VRAM `0x8003DFC8` / file
`0x2E7C8` / size `0x8`. Empty `jr`/`nop` stub, first mid-`2A0C.s` carve:
prefix `0x2BDBC`, C `0x8`, resume `2E7D0.s` `0x516C4` (sums to prior
`0x7D488`). Prior failure was wrong trim target `0x2E5BC` (off by `0x2800`),
not ROM holes — instruction stream is contiguous. Scratch + production
**EXACT MATCH**.

**Phase 5AC result — `func_8008F6A8` (2026-07-09):** VRAM `0x8008F6A8` / file
`0x7FEA8` / size `0x8`. Empty `jr`/`nop` stub at the head of former
`7FEA8.s` (after 5K `func_8008F694`): zero-byte prefix, C `0x8`, resume
`7FEB0.s` `0x1B8` (sums to prior `0x1C0`). Scratch + production **EXACT MATCH**.
(Initial mid-`2A0C` attempt for `func_8003DFC8` deferred; later fixed in 5AE.)

**Phase 5Y result — `func_800CD2E4` (2026-07-09):** VRAM `0x800CD2E4` / file
`0xBDAE4` / size `0x8`. Empty `jr`/`nop` stub (twin of CD2DC), sixth mid-tail
carve: zero-byte prefix at segment start, C `0x8`, resume `BDAEC.s` `0x130D14`
(sums to prior `0x130D1C`). Scratch + production **EXACT MATCH**.

**Phase 5X result — `func_800CD2DC` (2026-07-09):** VRAM `0x800CD2DC` / file
`0xBDADC` / size `0x8`. Empty `jr`/`nop` stub, fifth mid-tail carve (from
`BB4DC.s`): prefix `0x2600`, C `0x8`, resume `BDAE4.s` `0x130D1C` (sums to
prior `0x133324`). Scratch + production **EXACT MATCH**.

**Phase 5W result — `func_800CACD4` (2026-07-09):** VRAM `0x800CACD4` / file
`0xBB4D4` / size `0x8`. Empty `jr`/`nop` stub, fourth mid-tail carve (from
`BA6A8.s`): prefix `0xE2C`, C `0x8`, resume `BB4DC.s` `0x133324` (sums to
prior `0x134158`). Scratch + production **EXACT MATCH**.

**Phase 5V result — `func_800C9EA0` (2026-07-09):** VRAM `0x800C9EA0` / file
`0xBA6A0` / size `0x8`. Empty `jr`/`nop` stub, third mid-tail carve (from
`B9A68.s`): prefix `0xC38`, C `0x8`, resume `BA6A8.s` `0x134158` (sums to
prior `0x134D98`). Scratch + production **EXACT MATCH**.

**Phase 5U result — `func_800C9260` (2026-07-09):** VRAM `0x800C9260` / file
`0xB9A60` / size `0x8`. Empty `jr`/`nop` stub, second mid-tail carve (from
`B8A70.s`): prefix `0xFF0`, C `0x8`, resume `B9A68.s` `0x134D98` (sums to
prior `0x135D90`). Scratch + production **EXACT MATCH**.

**Phase 5T result — `func_800C8268` (2026-07-09):** VRAM `0x800C8268` / file
`0xB8A68` / size `0x8`. Empty `jr`/`nop` stub, first mid-`B3350` carve:
prefix `0x5718`, C `0x8`, resume `B8A70.s` `0x135D90` (sums to prior
`0x13B4B0`). Scratch + production **EXACT MATCH**. Full B3350 tail triage
(344 functions) recorded in `docs/ai_context/B3350_TRIAGE.md`: 17 star
candidates (9 empty stubs incl. this one, 8 sb-stubs). **sb-stub family
parked:** ROM `li/jr/sb-in-delay-slot` (0xC); GCC 14.2 emits `li/sb/jr/li`
(0x10, duplicates `li` into the delay slot) across a 30-probe shape × flag
matrix — 5I-class schedule mismatch, revisit with era compiler / maspsx.

**Phase 5S result — `func_800906B4` (2026-07-09):** VRAM `0x800906B4` / file
`0x80EB4` / size `0x30`. Stream advance + OR `0x200` at `+0x38`, OR `0x4400`
at `+0xF4`, store byte at `+0x116`. Mid-`80CC4.s` carve: prefix `0x1F0`, C
`0x30`, resume `80EE4.s` `0x328` (sums to prior `0x548`). Scratch + production
**EXACT MATCH**. First mid-`80CC4` leaf after parking `904C4` and the eight
`90574`–`9068C` twins.

**Phase 5R result — `func_800904BC` (2026-07-09):** VRAM `0x800904BC` / file
`0x80CBC` / size `0x8`. Empty `jr`/`nop` stub. Scratch + production **EXACT
MATCH**. Completes the three consecutive empty stubs after 5O.

**Phase 5Q result — `func_800904B4` (2026-07-09):** VRAM `0x800904B4` / file
`0x80CB4` / size `0x8`. Empty `jr`/`nop` stub. Scratch + production **EXACT
MATCH**.

**Phase 5P result — `func_800904AC` (2026-07-09):** VRAM `0x800904AC` / file
`0x80CAC` / size `0x8`. Empty `jr`/`nop` stub. Scratch + production **EXACT
MATCH**.

**Phase 5O result — `func_800904A0` (2026-07-09):** VRAM `0x800904A0` / file
`0x80CA0` / size `0xC`. Store `1` at halfword `arg0+0x84` (`addiu`/`jr`/`sh`).
Mid-`804BC.s` cut: prefix `0x7E4`, C `0xC`, resume later carved by 5P.
Scratch + production **EXACT MATCH**.

**Phase 5N result — `func_8008FCB4` (2026-07-08):** VRAM `0x8008FCB4` / file
`0x804B4` / size `0x8`. Zero halfword at `arg0+0x82` (`jr`/`sh $zero`).
Mid-`80098.s` cut: prefix `0x41C`, C `0x8`, resume `804BC.s` (later carved by
5O). Scratch + production **EXACT MATCH**. Low semantic value stub; chosen
after nearby stream-reader leaves failed reg-alloc scratch.

**Phase 5M result — `func_8008F880` (2026-07-08):** VRAM `0x8008F880` / file
`0x80080` / size `0x18`. Decrement twin of 5L: `*(u16 *)(arg0+0x7C) =
(*field - 1) & 0xF`. Mid-`80080.s` cut: C `0x18` + resume `80098.s` `0x1174`
(sums to prior `0x118C`). Scratch + production **EXACT MATCH**. Completes
tight mid-region trio with 5K/5L.

**Phase 5L result — `func_8008F868` (2026-07-08):** VRAM `0x8008F868` / file
`0x80068` / size `0x18`. `*(u16 *)(arg0+0x7C) = (*field + 1) & 0xF` with
`lhu`/`addiu`/`andi`/`jr`/`sh` delay slot. Mid-`7FEA8.s` three-way cut:
asm prefix `0x1C0`, C `0x18`, resume `80080.s` `0x118C` (sums to prior
`0x1364` span through `0x8120C`). Scratch + production **EXACT MATCH**.
Next twin candidate: `func_8008F880` (decrement version, already scratch-exact).

**Phase 5K result — `func_8008F694` (2026-07-08):** VRAM `0x8008F694` / file
`0x7FE94` / size `0x14`. `*(unsigned int *)arg0 += 2` with `lw`/`addiu`/`jr`/
`sw` delay slot (same class as 90Cxx leaves). Mid-`2A0C.s` three-way cut:
asm prefix `0x7D488`, C `0x14`, resume `7FEA8.s` `0x1364` (sums to prior
`0x7E800` span through `0x8120C`). Scratch + production **EXACT MATCH**.

**Phase 5J result — `func_80090A0C` (2026-07-08):** VRAM `0x80090A0C` / file
`0x8120C` / size `0x14`. First unconverted 90Cxx sibling (bit-clear `0x8` at
`*(arg0+0x38)`). Mid-`2A0C.s` cut: asm prefix `0x7E800`, C `0x14`, resume asm
`81220.s` `0x218`. Scratch + production **EXACT MATCH**. File-span math note:
`0x8120C - 0x2A0C = 0x7E800` (not `0x7E700`).

### Phase 5H blocker — `func_800C2B10` (2026-07-08)

Target: VRAM `0x800C2B10` / file `0xB3310` / size `0x18` in `asm/disc1/B2AF8.s`.
Leaf (no `jal`), uses raw extern `D_800E2248`. Original words:

```text
sll   $a0, $a0, 2
lui   $v0, %hi(D_800E2248)
lw    $v0, %lo(D_800E2248)($v0)
addiu $a0, $a0, 0x8
jr    $ra
addu  $v0, $v0, $a0
```

ROM hex: `802004000e80023c4822428c080084240800e00321104400`

GCC 14.2 (`pe-mipsel`) with Phase 4J flags never emits that schedule. Closest
0x18-byte bodies (same ops, wrong order):

| Flags | Emitted order |
| --- | --- |
| `-O1` / `-O1 -fno-schedule-insns*` | `sll`, `addiu 8`, `lui/lw`, `jr`, `addu` |
| `-O2` / `-Os` / `-O1 -fschedule-insns*` | `lui/lw`, `sll`, `addiu 8`, `jr`, `addu` |

Tried multiple C shapes (`return D_800E2248 + (arg0<<2) + 8`, staged temps,
`&base[off+8]`, `volatile`, etc.) and schedule/delay-slot flag variants. No
exact word match. **Stop.** Do not integrate. Do not auto-switch to
`func_800C2B28` (same pattern, same risk).

**Decision (2026-07-08):** park the `D_800E2248` accessor siblings
(`func_800C2B10`, `func_800C2B28`) until era-matching compiler / maspsx is
tested.

### Phase 5I blocker — `func_800C7DC4` (2026-07-08)

Target: VRAM `0x800C7DC4` / file `0xB85C4` / size `0x10` in `asm/disc1/B3350.s`.
Leaf (no `jal`), no globals — store `4` to `*arg0`, return `0`. Outside the
parked accessor cluster. Original words:

```text
addiu $v0, $zero, 4
sb    $v0, 0($a0)
jr    $ra
addu  $v0, $zero, $zero
```

ROM hex: `04000224000082a00800e00321100000`

Scratch probe under GCC 14.2 Phase 4J flags (`-O1`) appeared exact on first
three instructions, but production integration after splat cut produced **1-byte
NON-MATCH** at file `0xB85D0`:

| Source | Delay-slot word @ `0xB85D0` | Encoding |
| --- | --- | --- |
| ROM | `0x21100000` | `addu $v0, $zero, $zero` |
| GCC 14.2 | `0x00001025` | `move $v0, $zero` (`or` pseudo-op) |

Tried `-O0`–`-Os`, inline asm, `noreorder` asm blocks — GCC still prefers `move`
in the delay slot or adds prologue/epilogue. **Stop.** No config/C committed.
Duplicate bodies at `func_800C8F08`, `func_800C9C00` (same 0x10-byte pattern)
are also parked.

1. Pick another GCC-friendly leaf; avoid delay-slot pseudo-op sensitivity when
   possible (or accept era toolchain / maspsx for these).
2. Host PATH still has no mipsel tools; C/as/ld stay in Distrobox `pe-mipsel`.
3. When redump.org is reachable, record the official cross-check in `docs/disc_info.md`.
4. Do **not** invent semantic names for pointer/field targets yet.
5. PC port remains out of scope until systems coverage is meaningful.

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
`[0x800, rodata]` + `[0x2A0C, asm]` + `[0x818A0, rodata]` +
`[0xB2AF8, asm]`. Re-split 2026-07-08: splat 0.41.0 OK; output
`asm/disc1/data/800.rodata.s` (prefix), `asm/disc1/2A0C.s` (~149k lines,
text through `func_80091080`), `asm/disc1/data/818A0.rodata.s` (~75k lines,
mid-image data island ending at `D_800C22F0` zero padding),
`asm/disc1/B2AF8.s` (~329k lines, tail code from `func_800C22F8`).
`818A0.rodata.s` no longer folds `0x800C22F8` prologue as `.word` entries;
`B2AF8.s` emits `addiu $sp,$sp,-0x18` at VRAM `0x800C22F8` as text.
pc0 `func_80072534` unchanged/sane in `2A0C.s` (BSS zero-fill, stack init,
`jal func_800726B4`, `jal func_8001220C`, `break 0,1`). Splat still warns
prefix rodata nested splits (`0xEE4`…`0x1E44`) and suggests `0x93CCC`,
`0xB2928`, `0xB2AA4` inside `818A0` — deferred.

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
| `0x800C22F8` | `0xB2AF8` | zeros then `0x27BDFFE8` (`addiu $sp,$sp,-0x18`) | text (code resume) | high | First strong prologue after padding; sustained MIPS follows | **Applied** (`[0xB2AF8, asm]`) |
| `0x80072534` | `0x62D34` | crt0 startup | text | high (anchor) | pc0; do not use as split start | Anchor only |

### Phase 3 mid-image nested audit (2026-07-08)

Read-only audit of file `0x818A0`–`0xB2AF7` / VRAM `0x800910A0`–`0x800C22F7` inside
`asm/disc1/data/818A0.rodata.s` plus raw `SLUS_006.62` bytes. Branch
`phase3-disc1-boundary-audit` at `7886f52`. Generated output present locally,
git-ignored; no config change applied.

**Island layout (confirmed):**

| File range | VRAM range | Size | Contents |
| --- | --- | --- | --- |
| `0x818A0`–`0x8D5F3` | `0x800910A0`–`0x8009CDF3` | ~466 KB | Mixed rodata: func-pointer tables, glyph tiles, Psy-Q strings/tags, scalar tables, 128-word func-pointer block at `0x8D370` |
| `0x8D5F4`–`0xB28C7` | `0x8009CDF4`–`0x800C20C7` | ~149 KB | ROM zero padding (BSS image); **not** a ROM section edge — crt0 zero-fills `D_8009CDF8`→`D_800C20C8` at runtime |
| `0xB28C8`–`0xB2AEF` | `0x800C20C8`–`0x800C22EF` | ~2.3 KB | Tail rodata: Psy-Q thread/error strings, two jump-table clusters, scalar descriptor words |
| `0xB2AF0`–`0xB2AF7` | `0x800C22F0`–`0x800C22F7` | 8 B | Zero padding before code resume |

**Anchors reconfirmed (unchanged):**

- `0xB2AF8` / `0x800C22F8`: asm resumes in `B2AF8.s` (`addiu $sp,$sp,-0x18` at
  `func_800C22F8`); not folded into `818A0.rodata.s`.
- pc0 `func_80072534` at file `0x62D34` in `2A0C.s`: BSS zero-fill, stack init,
  `$gp` setup, `jal func_800726B4`, `jal func_8001220C`, `break 0,1`.

**Candidate nested-boundary table:**

| File offset | VRAM | Observed bytes/words | Class | Confidence | Evidence | Action |
| --- | --- | --- | --- | --- | --- | --- |
| `0x818A0` | `0x800910A0` | `0x80017294`, `0x800172BC`, … dense `0x8001xxxx` | rodata / func-pointer table | high | Island start; `D_800910A0` ends `0x81E48`; `lw` xrefs from `2A0C.s` text | Keep as island start (applied) |
| `0x81C64` | `0x80091464` | `0x2C0F1F17`, `0x4C2C2C2C` (`,,,,L,,,,` glyph-like) | rodata / glyph or bitmap | medium-high | First non-pointer words after `D_800910A0` table tail; abrupt ptr→bitmap transition | Defer; nested rodata only, no misclassification fix |
| `0x81FE0` | `0x800917E0` | ASCII `>?@Am0295i` (Psy-Q heap tag family) | rodata / heap tags | high (content) | Matches `m0290i`-style tags referenced from crt0 stack setup | Include in island; not a section edge |
| `0x838B4` | `0x800930B4` | `0123456789abcdefghiklmnoprstuvwy` | rodata / charset | high (content) | Pure ASCII in raw bytes | Include in island; not a section edge |
| `0x84D8C` | `0x8009458C` | `Library Programs (c) 1993-1997 Sony…` | rodata / SDK string | high (content) | Known Psy-Q copyright string | Include in island; not a section edge |
| `0x86E6C` | `0x8009666C` | `Error: Can't push matrix,stack(max 20) is full!` | rodata / debug strings | high (content) | ASCII error strings cluster in raw bytes | Include in island; not a section edge |
| `0x8D370` | `0x8009CB70` | 128 words `0x8008Fxxx`/`0x80090xxx` func pointers | rodata / pointer table | high (content) | Exactly 128 entries through `0x8D56C`; ends with four `func_80091080` words | Defer split; mid-island table, still rodata |
| `0x8D570` | `0x8009CD70` | `m0290i` heap tags; crt0 `lw` stack bounds | data (initialized) | medium | Used at runtime for stack/heap; **not** a ROM section edge | Do not split |
| `0x8D5F4` | `0x8009CDF4` | `0x00000400` then all zeros for ~149 KB | padding / BSS image | high NOT ROM edge | Last initialized word before zero span; `0x8009CDF8` is crt0 BSS zero-fill **start** | **Do not split** |
| `0x93CCC` | `0x800A34CC` | All raw bytes `0x00000000` | padding (false jtbl) | high NOT real | Splat labels `jtbl_800A34CC` → `.L00000000_main` inside zero BSS image; no real pointers | **Reject** splat hint |
| `0xB28C8` | `0x800C20C8` | `error : service thread not found\n` | rodata strings | high NOT ROM edge | ROM holds strings but address is crt0 BSS zero-fill **end**; spans RAM not one ROM section | **Do not split** |
| `0xB2928` | `0x800C2128` | `0x800C3270`, `0x800C3288`, … (`jtbl_800C2128`) | rodata / jump table | medium-high | Real aligned jtbl after `Wrong Color Mode\n`; targets in tail asm — but still rodata | Defer; splat alignment hint only |
| `0xB2AA4` | `0x800C22A4` | `0x800D41FC`, `0x800D4290`, … (`jtbl_800C22A4`) | rodata / jump table | medium-high | Second jtbl cluster before final scalar words; targets in `B2AF8.s` tail code | Defer; splat alignment hint only |
| `0xB2AF0` | `0x800C22F0` | `0x00000000` × 2 | padding | high | 8-byte zero gap immediately before code prologue | Already bounded by `[0xB2AF8, asm]` |
| `0xB2AF8` | `0x800C22F8` | `0x27BDFFE8` (`addiu $sp,$sp,-0x18`) | text (code resume) | high | Sustained MIPS follows; emitted in `B2AF8.s` | **Applied** — do not touch |

**Audit conclusion:** No candidate rises to **extremely high confidence** for a
config change. Splat's `0x93CCC` hint is a false positive in the BSS zero image.
`0xB2928` and `0xB2AA4` are real jump tables but splitting them only subdivides
rodata (no asm/rodata misclassification fix). `0x8D5F4` and `0xB28C8` coincide
with runtime BSS addresses and must not become ROM boundaries. **No config edit;
docs-only record.**

### Phase 3 prefix rodata nested audit (2026-07-08)

Read-only audit of file `0x800`–`0x2A0B` / VRAM `0x80010000`–`0x8001220B` inside
`asm/disc1/data/800.rodata.s` (~2987 lines) plus raw `SLUS_006.62` bytes. Branch
`phase3-disc1-boundary-audit` at `6ebbedb`. Generated output present locally,
git-ignored; no config change applied.

**Prefix layout (confirmed):**

| File range | VRAM range | Contents |
| --- | --- | --- |
| `0x800`–`0xEE3` | `0x80010000`–`0x800106E3` | Dense jump-table cluster (`jtbl_80010000` … `jtbl_80010690`); ~421/441 words are VRAM pointers |
| `0xEE4`–`0x2A0B` | `0x800106E4`–`0x8001220B` | Interleaved jtbl blocks, scalar tables, glyph-like data, Psy-Q/CDL strings, tail jtbls through `jtbl_8001213C` |

**Anchors reconfirmed (unchanged):**

- `0x2A0C` / `0x8001220C`: asm starts in `2A0C.s` (`addiu $sp,$sp,-0x28` at
  `func_8001220C`); last rodata words at `0x2A00`–`0x2A08` are tail jtbl entries.
- `0xB2AF8` / `0x800C22F8`: asm still resumes in `B2AF8.s` at `func_800C22F8`.
- pc0 `func_80072534` at file `0x62D34` in `2A0C.s`: unchanged/sane.

**Splat candidate boundary table:**

| File | VRAM | Observed bytes/words | Class | Confidence | Evidence | Action |
| --- | --- | --- | --- | --- | --- | --- |
| `0xEE4` | `0x800106E4` | `0x8001F860`, `0x8001F898`, … (`jtbl_800106E4`) | rodata / jtbl | medium-high | Scalar table `D_800106D4` ends with zero at `0xEE0`; new jtbl begins; 4-byte aligned, no `.align 3` | Defer; organizational rodata split only |
| `0x10F0` | `0x800108F0` | `0x8002AAD8`, `0x8002AB40`, … (`jtbl_800108F0`) | rodata / jtbl | medium-high | `D_800108E4` scalar ends `0x10EC`; `.align 3` before jtbl; 8-byte aligned | Defer; organizational rodata split only |
| `0x164C` | `0x80010E4C` | `0x80031598`, `0x800315B0`, … (`jtbl_80010E4C`) | rodata / jtbl | medium-high | `D_80010E38` scalar ends with zero at `0x1648`; jtbl follows; `Warning:`/`Fatal Error:` strings at `0x1694` | Defer; organizational rodata split only |
| `0x1958` | `0x80011158` | `0x8005117C`, `0x80051234`, … (`jtbl_80011158`) | rodata / jtbl | medium | `jtbl_80011144` ends `0x1954`; `.align 3`; another jtbl at `0x1970` follows immediately after `0x196C` zero terminator — mid jtbl stream | Defer |
| `0x1A5C` | `0x8001125C` | `0x80053DF8` × 9, … (`jtbl_8001125C`) | rodata / jtbl | medium-high | `jtbl_80011218` ends `0x1A58`; new jtbl with repeated targets; 4-byte aligned | Defer; organizational rodata split only |
| `0x1B88` | `0x80011388` | `0x80069D10`, `0x80069D4C`, … (`jtbl_80011388`) | rodata / jtbl | **high** (content) | `CD: Read error (%d / %d)\n` ends `0x1B84`; `.align 3`; cleanest string→jtbl transition in prefix | Defer for now; best candidate if pursuing organizational split |
| `0x1E44` | `0x80011644` | `0x80071D60`, `0x8007220C` × N (`jtbl_80011644`) | rodata / jtbl | high (content) | Hex charset `0123456789abcdef` null-terminated at `0x1E40`; jtbl follows; `VSync: timeout\n` at `0x1EFC` | Defer; organizational rodata split only |

**Audit conclusion:** All seven splat hints are **jumptable alignment suggestions
inside already-correct prefix rodata**. None fix asm/rodata misclassification
(the outer `[0x2A0C, asm]` boundary remains the only high-confidence
rodata→text edge). Several candidates (`0xEE4`, `0x1958`, `0x1A5C`) sit
mid-stream between back-to-back jtbl blocks; splitting them would subdivide
rodata without improving classification. **No config edit; docs-only record.**

**Recommended next action:** Stop prefix nested splitting unless splat jtbl
alignment becomes a blocker. If pursued later, try `0x1B88` first (strongest
string→jtbl content transition, `.align 3`, 8-byte aligned) as a single
organizational rodata split — one boundary per commit, re-split and re-check
pc0/`0xB2AF8` each time.

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

Scratch-link byte claims below refer to the extracted function-code slice;
production builds are the authority for final address placement and full-file
identity.

- 2026-07-12: **Phase 5BC fifty-second C leaf integrated — 32-bit global getter.** From accepted Phase 5BB commit `e9f46c1`, converted only `func_800822AC` (file `0x72AAC`, size `0x10`, VRAM `0x800822AC`, `lui v0,%hi(D_8009B70C); lw v0,%lo(D_8009B70C)(v0); jr ra; nop` → `int func(void){ return D_8009B70C; }`). Plain -O1 scratch-linked code slice exactly matches raw ROM; the scratch binary has four leading linker-alignment bytes. Target-only mid-`71150.s` carve `[0x71150, asm]` (`0x195C`) + `[0x72AAC, c]` + `[0x72ABC, asm]` (`0xA7C0`), summing to the prior `0xC12C`. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5BC fifty-two leaves; direct `cmp` exact. No other leaf was added.
- 2026-07-12: **Phase 5BB fifty-first C leaf integrated — 32-bit global getter.** From accepted Phase 5BA commit `79cdc7e`, converted only `func_80080940` (file `0x71140`, size `0x10`, VRAM `0x80080940`, `lui v0,%hi(D_8009B554); lw v0,%lo(D_8009B554)(v0); jr ra; nop` → `int func(void){ return D_8009B554; }`). Plain -O1 scratch-linked bytes exactly match raw ROM. Target-only mid-`704BC.s` carve `[0x704BC, asm]` (`0xC84`) + `[0x71140, c]` + `[0x71150, asm]` (`0xC12C`), summing to the prior `0xCDC0`. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5BB fifty-one leaves; direct `cmp` exact. No other leaf was added.
- 2026-07-12: **Phase 5BA fiftieth C leaf integrated — 32-bit global getter.** From accepted Phase 5AZ commit `86e4a48`, converted only `func_8007FCAC` (file `0x704AC`, size `0x10`, VRAM `0x8007FCAC`, `lui v0,%hi(D_8009B590); lw v0,%lo(D_8009B590)(v0); jr ra; nop` → `int func(void){ return D_8009B590; }`). Plain -O1 scratch-linked bytes exactly match raw ROM. Target-only mid-`6FF88.s` carve `[0x6FF88, asm]` (`0x524`) + `[0x704AC, c]` + `[0x704BC, asm]` (`0xCDC0`), summing to the prior `0xD2F4`. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5BA fifty leaves; direct `cmp` exact. No other leaf was added.
- 2026-07-12: **Phase 5AZ forty-ninth C leaf integrated — 32-bit global getter.** From accepted Phase 5AY commit `ffbff11`, converted only `func_8007F778` (file `0x6FF78`, size `0x10`, VRAM `0x8007F778`, `lui v0,%hi(D_800A3608); lw v0,%lo(D_800A3608)(v0); jr ra; nop` → `int func(void){ return D_800A3608; }`). Plain -O1 scratch-linked bytes exactly match raw ROM. Target-only mid-`6E6C0.s` carve `[0x6E6C0, asm]` (`0x18B8`) + `[0x6FF78, c]` + `[0x6FF88, asm]` (`0xD2F4`), summing to the prior `0xEBBC`. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AZ forty-nine leaves; direct `cmp` exact. No other leaf was added.
- 2026-07-12: **Phase 5AY forty-eighth C leaf integrated — 32-bit global getter.** From accepted Phase 5AX commit `27a6ba2`, converted only `func_8007DEB0` (file `0x6E6B0`, size `0x10`, VRAM `0x8007DEB0`, `lui v0,%hi(D_8009B4AC); lw v0,%lo(D_8009B4AC)(v0); jr ra; nop` → `int func(void){ return D_8009B4AC; }`). Plain -O1 scratch-linked bytes exactly match raw ROM. Target-only mid-`65238.s` carve `[0x65238, asm]` (`0x9478`) + `[0x6E6B0, c]` + `[0x6E6C0, asm]` (`0xEBBC`), summing to the prior `0x18044`. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AY forty-eight leaves; direct `cmp` exact. The completed 5AX comparison stash was dropped before this work; no stash remains. No other leaf was added.
- 2026-07-11: **Phase 5AX forty-seventh C leaf integrated — 32-bit global getter (clean re-derivation).** From confirmed-clean Phase 5AW commit `7902dd2`, converted only `func_80074A28` (file `0x65228`, size `0x10`, VRAM `0x80074A28`, `lui v0,%hi(D_800956EC); lw v0,%lo(D_800956EC)(v0); jr ra; nop` → `int func(void){ return D_800956EC; }`). Plain -O1 matches. Target-only mid-`645F8.s` carve `[0x645F8, asm]` (`0xC30`) + `[0x65228, c]` + `[0x65238, asm]` (`0x18044`). Prior unverified 5AX attempt stashed (not deleted) after a false empty-diff safety claim vs wrong baseline `c3a8424`; fresh tree kept; `verify_us.sh` ladder follows `7902dd2` in-place-edit precedent (no extra 5AW `elif`). Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AX forty-seven leaves. No other leaf was added. Uncommitted pending review.
- 2026-07-11: **Phase 5AW forty-sixth C leaf integrated — 32-bit global getter.** From accepted Phase 5AV commit `7323079`, converted only `func_80042B28` (file `0x33328`, size `0x10`, VRAM `0x80042B28`, `lui v0,%hi(D_800A1838); lw v0,%lo(D_800A1838)(v0); jr ra; nop` → `int func(void){ return D_800A1838; }`). Plain -O1 matches. The current generated container is `330D4.s` within the original `2E7D0.s` family. Target-only carve `[0x330D4, asm]` (`0x254`) + `[0x33328, c]` + `[0x33338, asm]` (`0xAF64`). Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AW forty-six leaves. No other leaf was added.
- 2026-07-11: **Phase 5AV forty-fifth C leaf integrated — unsigned indexed bit test.** From accepted Phase 5AU commit `c3a8424`, converted only `func_80051834` (file `0x42034`, size `0x18`, VRAM `0x80051834`, `lui/lw/nop/srlv/jr/andi 1` → `int func(int a0){ return (D_800C0E24 >> a0) & 1; }` with `D_800C0E24` declared `unsigned int`). Plain -O1 matches; signed C was rejected because it emits `srav`. Target-only mid-`41520.s` carve `[0x41520, asm]` (`0xB14`) + `[0x42034, c]` + `[0x4204C, asm]` (`0x5FC`). Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AV forty-five leaves. No other leaf was added.
- 2026-07-11: **Phase 5AU forty-fourth C leaf integrated — 16-bit global getter.** Reset the accidental broad `c62e642` checkpoint to clean parent `9bb3099`, then converted only `func_80073DE8` (file `0x645E8`, size `0x10`, VRAM `0x80073DE8`, `lui v0,%hi(D_800945E6); lhu v0,%lo(D_800945E6)(v0); jr ra; nop` → `unsigned short func(void){ return D_800945E6; }`). Plain -O1 matches. Target-only mid-`4C4B0.s` carve `[0x4C4B0, asm]` (`0x18138`) + `[0x645E8, c]` + `[0x645F8, asm]` (`0x18C84`). Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AU forty-four leaves. No later leaf was retained.
- 2026-07-11: **Phase 5AT forty-third C leaf integrated — 32-bit global getter.** Branch `phase5ae-2a0c-hole-aware` on top of `bed30d7` (Phase 5AS). Converted `func_8003FFBC` (file `0x307BC`, size `0x10`, VRAM `0x8003FFBC`, `lui v0,%hi(D_800A1704); lw v0,%lo(D_800A1704)(v0); jr ra; nop` → `int func(void){ return D_800A1704; }`). Plain -O1 matches. Mid-`2E7D0.s` carve `[0x2E7D0, asm]` (`0x1FEC`) + `[0x307BC, c]` + `[0x307CC, asm]` (`0x28F8`). Setter twin `func_8003FFAC` parked (`$at` vs `$v0`). Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AT forty-three leaves.
- 2026-07-10: **Phase 5AS forty-second C leaf integrated — twin 16-bit global getter.** Branch `phase5ae-2a0c-hole-aware` on top of `a7efe09` (Phase 5AR). Converted `func_80052524` (file `0x42D24`, size `0x10`, VRAM `0x80052524`, `lui v0,%hi(D_800C0E32); lhu v0,%lo(D_800C0E32)(v0); jr ra; nop` → `unsigned short func(void){ return D_800C0E32; }`). Identical shape to 5AR; plain -O1 matches. Zero-prefix carve of former `42D24.s`: `[0x42D24, c]` + `[0x42D34, asm]` (`0x28C`), `src/func_80052524.c`, build/verify updates. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AS forty-two leaves.
- 2026-07-10: **Phase 5AR forty-first C leaf integrated — first 16-bit global getter.** Branch `phase5ae-2a0c-hole-aware` on top of `8284184` (Phase 5AQ). Converted `func_80052514` (file `0x42D14`, size `0x10`, VRAM `0x80052514`, `lui v0,%hi(D_800C0E28); lhu v0,%lo(D_800C0E28)(v0); jr ra; nop` → `unsigned short func(void){ return D_800C0E28; }`). jr delay slot is nop but **plain -O1 already matches**: on MIPS-I a load cannot fill the return delay slot (load-delay hazard), so unlike 5AQ no `-fno-delayed-branch` is needed. `D_800C0E28` auto-resolves absolute `0x800C0E28` via probe-link fallback. Mid-`42658.s` carve `[0x42658, asm]` (`0x6BC`) + `[0x42D14, c]` + `[0x42D24, asm]` (`0x29C`), `src/func_80052514.c`, build/verify updates. Twin `func_80052524` (`D_800C0E32`, same shape) left in asm for now. Scratch probe (plain + -fno-delayed-branch both) emitted the exact 4 words. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AR forty-one leaves.
- 2026-07-10: **Phase 5AQ fortieth C leaf integrated — first pure address-return leaf.** Branch `phase5ae-2a0c-hole-aware` on top of `6dd7343` (Phase 5AP). Converted `func_80051E48` (file `0x42648`, size `0x10`, VRAM `0x80051E48`, `lui v0,%hi(D_800A1B30); addiu v0,v0,%lo; jr ra; nop` → `return &D_800A1B30;`). ROM's jr delay slot is **unfilled**; GCC 14.2 -O1 otherwise fills it (`lui; jr; addiu`), so this single unit compiles with **`-fno-delayed-branch`** (per-file; the un-filled schedule was characterised in the Phase 4J probe). `D_800A1B30` auto-resolves as absolute `0x800A1B30` via the probe-link undef fallback. Mid-`41520.s` carve `[0x41520, asm]` (`0x1128`) + `[0x42648, c]` + `[0x42658, asm]` (`0x968`), `src/func_80051E48.c`, build/verify updates. Scratch probe with the flag emitted the exact 4 words. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AQ forty leaves.
- 2026-07-10: **Phase 5AP thirty-ninth C leaf integrated — first post-septuplet leaf.** Branch `phase5ae-2a0c-hole-aware` at commit `379083c`. Converted `func_800428C4` (global decrement getter, file `0x330C4`, size `0x10`, VRAM `0x800428C4`, `lui v0,%hi; lw v0,%lo; jr ra; addiu v0,-1`). Config carve `[0x2E7D0, asm]` (`0x48F4`) + `[0x330C4, c]` + `[0x330D4, asm]` (`0xB1C8`), `src/func_800428C4.c`, build/verify updates. During same session attempted `func_8003DFD0` return-0 (file `0x2E7D0`, `0800E003 21100000`): scratch probe shows GCC 14.2 emits `00001025` (`move $v0,$zero`) vs ROM `21100000` (`addu $v0,$zero,$zero`) in jr delay slot — 5I-class pseudo-op mismatch, same as `func_800C7DC4` blocker. Parked return-0 family. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AP thirty-nine leaves.
- 2026-07-10: **Phase 5AO thirty-eighth C leaf integrated — return-1 septuplet batch complete.** Branch `phase5ae-2a0c-hole-aware` at commit `e6002a9`. Converted `func_8004DA9C` (return-1 stub, file `0x3E29C`, size `0x8`, seventh/final of the seven byte-identical return-1 twins from 2026-07-09 triage). Config carve `[0x3E29C, c]` + `[0x3E2A4, asm]` (prefix `0xFACC` from `2E7D0.s`, resume `0x3274`), `src/func_8004DA9C.c`, build/verify updates. Shape scratch-proven in 5AJ (`0800E003 01000224`). Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AO thirty-eight leaves. **Return-1 batch (7/7): `func_8003D82C`, `func_80017E9C`, `func_80019050`, `func_80019058`, `func_800190AC`, `func_800190B4`, `func_8004DA9C` — all `return 1` / `jr; addiu v0, zero, 1`.**
- 2026-07-10: **Phase 5AN thirty-sixth/thirty-seventh C leaves integrated.** Same branch at commit `02176f8` (phase5am+5an). Converted `func_800190AC` (file `0x98AC`, size `0x8`) + `func_800190B4` (file `0x98B4`, size `0x8`), fifth and sixth return-1 twins. Config carve `[0x98AC, c]` + `[0x98B4, c]` + `[0x98BC, asm]` (prefix `0x4C` from `9860.s`, resume `0x24770`). `src/func_800190AC.c` + `src/func_800190B4.c`, build/verify updates. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting thirty-seven leaves.
- 2026-07-10: **Phase 5AM thirty-fifth C leaf integrated.** Same branch at commit `02176f8`. Converted `func_80019058` (return-1 stub, file `0x9858`, zero-prefix carve, fourth twin). Config carve `[0x9858, c]` + `[0x9860, asm]` (resume `0x247CC`). `src/func_80019058.c`, build/verify updates. Re-split OK; **EXACT MATCH** thirty-five leaves.
- 2026-07-09: **Phase 5AL thirty-fourth C leaf integrated.** Same branch at commit `2384fc5`. Converted `func_80019050` (return-1 stub, file `0x9850`, third twin). Config carve `[0x9850, c]` + `[0x9858, asm]` (prefix `0x11AC` from `86A4.s`, resume `0x247D4`). `src/func_80019050.c`, build/verify updates. Re-split OK; **EXACT MATCH** thirty-four leaves.
- 2026-07-09: **Phase 5AK thirty-third C leaf integrated.** Same branch at commit `657b997`. Converted `func_80017E9C` (return-1 stub, file `0x869C`, second twin, rodata dispatch-table target). Config carve `[0x869C, c]` + `[0x86A4, asm]` (prefix `0x5C90` from `2A0C.s`, resume `0x25988`). `src/func_80017E9C.c`, build/verify updates. Re-split OK; **EXACT MATCH** thirty-three leaves.
- 2026-07-09: **Phase 5AJ thirty-second C leaf integrated.** Same branch at commit `8234ada`. Converted `func_8003D82C` (return-1 stub, file `0x2E02C`, first of seven twins). Config carve `[0x2E02C, c]` + `[0x2E034, asm]` (prefix `0x2B620` from `2A0C.s`, resume `0x794`). `src/func_8003D82C.c`, build/verify updates. Shape `int f(void){return 1;}` scratch-proven GCC 14.2 (`0800E003 01000224`). Re-split OK; **EXACT MATCH** thirty-two leaves.
- 2026-07-09: **Phase 5AI thirty-first C leaf integrated.** Same branch at
  commit `176ed39761b1de36d23e2ee17a3c64a51151c1dd`. Converted
  `func_8008CA7C` (empty stub, file `0x7D27C`, final mid-`2A0C` batch leaf).
  Config carve `[0x7D27C, c]` + `[0x7D284, asm]`, `src/func_8008CA7C.c`,
  build/verify updates. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1
  MATCH**; `verify_us.sh` exit 0 reporting Phase 5AI thirty-one leaves.
- 2026-07-09: **Phase 5AH thirtieth C leaf integrated.** Same branch at
  commit `b224fa2c6428994218ecaa8b6eb6a49696d13f84`. Converted
  `func_8005BCA8` (empty stub, file `0x4C4A8`). Config carve
  `[0x4C4A8, c]` + `[0x4C4B0, asm]`, `src/func_8005BCA8.c`, build/verify
  updates. Re-split OK; build/verify report **EXACT SHA-1 MATCH** and thirty
  leaves.
- 2026-07-09: **Phase 5AG twenty-ninth C leaf integrated.** Same branch at
  commit `7a27527d70d088ae554d4e8fedbd6c235803eed9`. Converted
  `func_800527C0` (empty stub, file `0x42FC0`). Config carve
  `[0x42FC0, c]` + `[0x42FC8, asm]`, `src/func_800527C0.c`, build/verify
  updates. Re-split OK; build/verify report **EXACT SHA-1 MATCH** and
  twenty-nine leaves.
- 2026-07-09: **Phase 5AF twenty-eighth C leaf integrated.** Same branch
  `phase5ae-2a0c-hole-aware`. Converted `func_80050D18` (empty stub, file
  `0x41518`, second mid-`2A0C`). Config carve `[0x41518, c]` + `[0x41520, asm]`,
  `src/func_80050D18.c`, build/verify updates. Re-split OK; `build_us.sh`
  exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AF
  twenty-eight leaves.
- 2026-07-09: **Phase 5AE twenty-seventh C leaf integrated.** Branch
  `phase5ae-2a0c-hole-aware` from `main` (26 leaves). Converted
  `func_8003DFC8` (empty stub, file `0x2E7C8`, first mid-`2A0C`). Diagnosed
  prior "ROM hole" as span typo (`0x2E5BC` vs correct `0x2BDBC`); contiguous
  instruction stream. Config carve `[0x2E7C8, c]` + `[0x2E7D0, asm]`,
  `src/func_8003DFC8.c`, build/verify updates. Re-split OK; `build_us.sh`
  exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting Phase 5AE
  twenty-seven leaves.
- 2026-07-09: **Phase 5AC twenty-sixth C leaf integrated.** Branch
  `phase5ac-next-simple-leaf` from `main` (25 leaves). Scanned earlier asm
  segments; selected `func_8008F6A8` (empty stub, file `0x7FEA8`, head of
  `7FEA8.s` — boundary carve after 5K). Mid-`2A0C.s` candidate
  `func_8003DFC8` deferred (later fixed in 5AE). Config carve
  `[0x7FEA8, c]` + `[0x7FEB0, asm]`, `src/func_8008F6A8.c`, build/verify
  updates. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**;
  `verify_us.sh` exit 0 reporting Phase 5AC twenty-six leaves.
- 2026-07-09: **Phase 5Y twenty-second C leaf integrated.** Branch
  `phase5x-func-800CD2E4`. Converted `func_800CD2E4` (empty stub twin of
  CD2DC, file `0xBDAE4`, size `0x8`): config carve `[0xBDAE4, c]` +
  `[0xBDAEC, asm]`, `src/func_800CD2E4.c`, build/verify updates. Re-split OK;
  `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0 reporting
  Phase 5Y twenty-two leaves.
- 2026-07-09: **Phase 5X twenty-first C leaf integrated.** Branch
  `phase5x-func-800CD2DC` from `main` (after PR #30 / 5W). Converted
  `func_800CD2DC` (empty stub, file `0xBDADC`, size `0x8`, from the 5T
  triage star list; ROM bytes + scratch probe re-verified): config carve
  `[0xBDADC, c]` + `[0xBDAE4, asm]`, `src/func_800CD2DC.c`, build/verify
  script updates. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**;
  `verify_us.sh` exit 0 reporting Phase 5X twenty-one leaves.
- 2026-07-09: **Phase 5W twentieth C leaf integrated.** Branch
  `phase5w-func-800CACD4` from `main` (after PR #29 / 5V). Converted
  `func_800CACD4` (empty stub, file `0xBB4D4`, size `0x8`, from the 5T
  triage star list; ROM bytes + scratch probe re-verified): config carve
  `[0xBB4D4, c]` + `[0xBB4DC, asm]`, `src/func_800CACD4.c`, build/verify
  script updates. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**;
  `verify_us.sh` exit 0 reporting Phase 5W twenty leaves.
- 2026-07-09: **Phase 5V nineteenth C leaf integrated.** Branch
  `phase5v-func-800C9EA0` from `main` (after PR #28 / 5U). Converted
  `func_800C9EA0` (empty stub, file `0xBA6A0`, size `0x8`, from the 5T
  triage star list; ROM bytes + scratch probe re-verified): config carve
  `[0xBA6A0, c]` + `[0xBA6A8, asm]`, `src/func_800C9EA0.c`, build/verify
  script updates. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**;
  `verify_us.sh` exit 0 reporting Phase 5V nineteen leaves.
- 2026-07-09: **Phase 5U eighteenth C leaf integrated.** Branch
  `phase5u-func-800C9260` from `main` (after PR #27 / 5T). Converted
  `func_800C9260` (empty stub, file `0xB9A60`, size `0x8`, from the 5T
  triage star list; ROM bytes + scratch probe re-verified): config carve
  `[0xB9A60, c]` + `[0xB9A68, asm]`, `src/func_800C9260.c`, build/verify
  script updates. Re-split OK; `build_us.sh` exit 0 **EXACT SHA-1 MATCH**;
  `verify_us.sh` exit 0 reporting Phase 5U eighteen leaves.
- 2026-07-09: **Phase 5T seventeenth C leaf integrated.** Branch
  `phase5t-func-800C8268` from `main` (after PR #26 / 5S). Full triage of
  the `B3350.s` tail (344 functions; parser + 344/344 adversarial
  re-derivation, zero discrepancies; all star spans ROM-byte-verified)
  recorded in `docs/ai_context/B3350_TRIAGE.md`. Parked the 8-strong
  sb-stub star family (GCC 14.2 delay-slot schedule mismatch, 30-probe
  matrix). Converted `func_800C8268` (empty stub, file `0xB8A68`, size
  `0x8`): config carve `[0xB8A68, c]` + `[0xB8A70, asm]`,
  `src/func_800C8268.c`, build/verify script updates. Re-split OK;
  `build_us.sh` exit 0 **EXACT SHA-1 MATCH**; `verify_us.sh` exit 0
  reporting Phase 5T seventeen leaves.
- 2026-07-08: **Phase 3 prefix rodata nested audit:** read-only pass over file
  `0x800`–`0x2A0B`; evaluated all seven splat hints (`0xEE4`…`0x1E44`). All are
  organizational jtbl-alignment splits inside correct rodata; no
  extremely-high-confidence boundary. `0x1B88` noted as strongest future
  candidate. No config change; `0x2A0C`/`0xB2AF8`/pc0 unchanged.
- 2026-07-08: **Phase 3 mid-image nested audit:** read-only pass over file
  `0x818A0`–`0xB2AF7`; documented island layout (mixed rodata, 149 KB BSS zero
  image, tail strings/jtbls). Rejected `0x93CCC` (false jtbl in zeros); deferred
  `0xB2928`/`0xB2AA4` (organizational rodata). No config change; `0xB2AF8` asm
  resume and pc0 unchanged.
- 2026-07-08: **Phase 3 resume-asm boundary:** applied `[0xB2AF8, asm]` to
  close mid-image data island opened at `0x818A0`. Re-split OK; `818A0.rodata.s`
  shrinks to ~75k lines (ends at zero padding `D_800C22F0`); new `B2AF8.s`
  ~329k lines with `func_800C22F8` prologue as text; pc0 sane; output
  git-ignored.
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
- 2026-07-08: **Phase 3 boundary audit parked.** No config change from
  the prefix or mid-image nested audits. Splat's seven prefix hints
  (`0xEE4`…`0x1E44`) and mid-image hints are organizational only — no
  misclassifications to fix. Critical boundaries solid; work parked until
  a true blocker appears (real code-as-data, strings-as-instructions, etc.).
  Branch ready for merge. Resume from this file.
- 2026-07-08: **Phase 4 initial function inventory started.** Created
  `docs/ai_context/DISC1_FUNCTION_INVENTORY.md` on `phase4-disc1-function-inventory`
  (after Phase 3 merge simulation). ~2359 auto-labeled functions mapped across
  `2A0C.s` + `B2AF8.s`. Anchors (1220C, 72534 pc0, C22F8) documented with
  high confidence. Conservative survey only; docs-only commit. No C/renames/boundary edits.
- 2026-07-08: **Phase 4B Disc 1 call/anchor map recorded.** Created
  `docs/ai_context/DISC1_CALL_ANCHOR_MAP.md`. High-confidence direct `jal`
  relationships focused on anchors + samples + freq callees (as clusters only).
  ~7k+ jals scanned conservatively. Docs-only. No inference beyond disassembly.
- 2026-07-08: **Phase 4C first decomp target triage.** Created
  `docs/ai_context/DISC1_FIRST_DECOMP_TARGETS.md`. 5-7 conservative small-leaf
  candidates (best cluster around func_80090C38/90F54). Strict filters applied;
  recommended first + backups listed with full rationale. Docs-only triage.
- 2026-07-08: **Phase 5 C conversion attempt (blocker).** Switched to
  `phase5-disc1-first-c-leaf` after Phase 4 merge simulation. Identified
  recommended target func_80090C38. Re-verified in asm/disc1/2A0C.s:
  label present, 0x14 bytes / 5 instructions, leaf (no jal), no jump table,
  no indirect call (no jalr), no dangerous hardware/system side effects
  (simple field bit-set on $a0), not an anchor.
  Pre-checks passed (config, split --check, clean tree).
  However, no C conversion performed because infrastructure is missing:
  - verify_us.sh is placeholder (exits with "Phase 0 placeholder", no harness)
  - src/main/ only contains .gitkeep (no C source structure)
  - No C segments in splat config
  - No top-level Makefile or build rules for compiling C into PSX EXE
  - No object file comparison or rebuild path
  Per rules, documented blocker instead of faking conversion.
  Commit: docs-only "Record Phase 5 C conversion blocker".
  Update ACTIVE_HANDOFF.md with result. One function only.
- 2026-07-08: **Phase 4D C/matching harness design audit.** Created
  `docs/ai_context/DISC1_C_HARNESS_PLAN.md`. Full repo audit of scripts,
  configs, build state, and blocker. Distinguished Phase 4E (minimal
  verification harness) from Phase 4F (actual build/matching). Docs-only.
  Commit: "Record Disc 1 C harness plan". No implementation. Ready to resume.
- 2026-07-08: **Phase 4E minimal verification harness.** Replaced
  `scripts/verify_us.sh` placeholder with a real split-artifact checker:
  (1) repo root, (2) disc1.yaml Phase 3 subsegment markers, (3) EXE +
  SHA-1 `452fb033…`, (4) `scripts/split_us.sh --check`, (5) splat pin
  0.41.0, (6) expected files `header.s` / `2A0C.s` / `B2AF8.s` /
  `data/800.rodata.s` / `data/818A0.rodata.s` / `linkers/disc1.ld`,
  (7) gitignore coverage for split outputs. Always prints that
  rebuild/matching is NOT IMPLEMENTED (Phase 4F). Exit 0 only when
  artifacts are sane; exit 1 on real problems. No assemble/link/C/src/
  config/Makefile changes. Verified: `bash -n` clean; run on this
  worktree (no extract/venv/split) exits 1 with 9 expected FAILs and
  coherent report. Commit: "Implement Phase 4E minimal verification harness"
  (`730821d`).
- 2026-07-08: **Phase 4E provisioned-workbench validation.** On main
  checkout with extract/split/venv present, temporary use of the 4E
  `verify_us.sh` produced exit 0, all 7 gates OK, NOT IMPLEMENTED banner
  still printed, `git status --short` showed no asm/linker noise (only the
  temporary script swap, restored afterward). Stripped worktree still
  fails loudly when local data is missing (correct).
- 2026-07-08: **Phase 4F asm-only rebuild harness — blocked (docs only).**
  Inspected generated `linkers/disc1.ld` (expects
  `build/asm/disc1/{header,2A0C,B2AF8,data/*.rodata}.s.o`), split asm
  shape (`.include "macro.inc"`, `glabel`/`nonmatching`, MIPS LE), and
  host tools. Findings:
  - No `mipsel-linux-gnu-{as,ld,objcopy}` (or equivalent) on PATH.
  - Host `/usr/bin/as` (binutils 2.46 x86-64) cannot assemble MIPS text
    (`addiu`/`sw`/`jal` unknown; `.set noat` syntax errors; `.ent` unknown).
  - Host `as` on `header.s` "succeeds" but emits **x86-64** ELF with
    truncated 0x800xxxxx values — wrong arch; not a rebuild.
  - splat 0.41.0 CLI is split-only (`split`/`create_config`/`capy`); no
    build subcommand.
  - No `build_us.sh` / Makefile added; `verify_us.sh` still honestly
    reports rebuild/matching not implemented.
  Structural reference only: sibling Xenogears decomp uses
  `target = mips-linux-gnu`, modern gas + maspsx, and era gcc binaries —
  not copied, not installed here.
  Commit: "Record asm-only rebuild harness blocker".
- 2026-07-08: **Phase 4G MIPS LE toolchain provisioning.** Branch
  `phase4g-mipsel-toolchain-provisioning` rebased onto `main` after PR #4
  (4E/4F). Distrobox `pe-mipsel` (`debian:trixie`) +
  `binutils-mipsel-linux-gnu` **2.44-3cross1+nmu1+b1**. Tools:
  `mipsel-linux-gnu-{as,ld,objcopy,objdump,readelf}` GNU Binutils 2.44.
  Temp-assembled all five split units with `as -EL -mips1 -mabi=32 -I include`
  (ELF32 MIPS R3000 LE). Host PATH unchanged. No `build_us.sh`, no link,
  no C, no matching claim. Note: target image is a **PS-X EXE**, not Windows PE.
  Commit: "Record MIPS toolchain provisioning path".
- 2026-07-08: **Phase 4H asm-only rebuild attempt.** Branch
  `phase4h-asm-only-rebuild` from `main` @ 4d2e903. Added
  `scripts/build_us.sh`: Distrobox `pe-mipsel` assemble (5/5), link with
  experimental ROM-order ld script + `undefined_syms_auto.txt` / residual
  absolute `D_*` / `.L00000000_main=0`, pack header+main to
  `build/disc1.candidate.exe`, SHA-1 compare. **NON-MATCH** (~28% bytes;
  header exact match; jtbl/text offsets wrong due to gas align + reloc layout).
  `verify_us.sh` reports split OK + honest rebuild NON-MATCH banner.
  Flags: `as -EL -mips1 -mabi=32 -I include`; `ld -EL -m elf32ltsmip
  -nostdlib --no-check-sections`. No C, no `func_80090C38`, no matching claim.
  Commit: "Add asm-only Disc 1 rebuild harness".
- 2026-07-08: **Phase 4I asm rebuild parity audit + pad fix.** Branch
  `phase4i-asm-rebuild-parity-audit` from phase4h. Proved section-size drift
  is **only trailing zero pad from ELF sh_addralign** (not mid-section
  inserts): 800.rodata +4 (align 8), 2A0C +12 (align 16), B2AF8 +8 (align 16).
  818A0 and header sizes already exact. Extra bytes all zero after last
  symbol. Added `tools/trim_elf_section_pad.py`; `build_us.sh` trims
  after assemble. Re-run: **exact SHA-1 match**, exit 0. No C. No label chase.
  Commit message for this work: record parity fix.
- 2026-07-08: **Phase 4J MIPS GCC + codegen probe.** Branch
  `phase4j-mipsel-gcc-provisioning` from `main`. Container-only:
  `apt install gcc-mipsel-linux-gnu` → `mipsel-linux-gnu-gcc (Debian 14.2.0-13)
  14.2.0`. Note: `-mips1` requires **`-mfp32`**. Scratch `/tmp` leaf for
  `func_80090C38` with flags `-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic
  -mno-abicalls -ffreestanding -fno-builtin` at **-O1/-O2/-O3/-Os** emits:
  `lw v0,0x38(a0); nop; ori v0,v0,0x10; jr ra; sw v0,0x38(a0)` — **exact
  0x14-byte match** to original ROM words; `.text` object size 0x20 (align-16
  pad). O0 has full frame prologue (not useful). `-fno-delayed-branch` puts
  `sw` before `jr` + nop (wrong for match). `-mips2` drops load-delay nop
  (wrong). ELF32 mipsel R3000 o32. No `src/` C, no build/splat C integration.
  Asm-only `build_us.sh` still exact. Commit: "Record MIPS GCC provisioning
  and C codegen probe".
- 2026-07-08: **Phase 5B first C leaf integrated.** Branch
  `phase5b-integrate-first-c-leaf` from `main` (after PR #8 / 4J). Converted
  **only** `func_80090C38`:
  - `src/func_80090C38.c` — temporary types, bit-set `*(u32*)(arg0+0x38) |= 0x10`
  - `configs/USA/disc1.yaml` — local cut `[0x81438, c, func_80090C38]` +
    `[0x8144C, asm]`
  - `scripts/build_us.sh` — compile with Phase 4J flags; assemble shortened
    `2A0C.s` + new `8144C.s`; trim C `.text` **0x20→0x14** (align-16 pad);
    ROM-order link places C between 2A0C and 8144C; pack + SHA-1 compare
  - `scripts/verify_us.sh` — expected subsegments/artifacts updated
  Validation: `split_us.sh --check` OK; `build_us.sh` exit 0 **EXACT MATCH**
  (leaf probe `3800828c00000000100042340800e003380082ac`). No second function.
  No generated output committed. Commit: "Convert func_80090C38 to C".
  Merged to `main` as PR #9 (`d624812`).
- 2026-07-08: **Phase 5C second C leaf integrated.** Branch
  `phase5c-next-c-leaf` from `main` after PR #9. Converted **only**
  `func_80090C4C` (bit-clear twin of 90C38):
  - `src/func_80090C4C.c` — `*(u32*)(arg0+0x38) &= ~0x10`
  - config: `[0x8144C, c, func_80090C4C]` + `[0x81460, asm]`
  - build: second C object; both trimmed 0x20→0x14; ROM-order places both
  Validation: probe codegen exact before integrate; `build_us.sh` exit 0
  **EXACT MATCH** (both leaf probes match). No third function. Commit:
  "Convert next Disc 1 leaf function to C". Merged as PR #10 (`6e6d444`).
- 2026-07-08: **Phase 5D third C leaf integrated.** Branch
  `phase5d-next-c-leaf` from `main` after PR #10. Converted **only**
  `func_80090F54` (bit-set 0x100000 at `*(arg0+0x38)` via lui/or):
  - `src/func_80090F54.c`
  - config: `[0x81754, c, func_80090F54]` + `[0x81768, asm]`; 81460 shortened
  - build: third C object; all three trimmed 0x20→0x14; ROM-order places mid-segment leaf
  Validation: probe codegen exact; `build_us.sh` exit 0 **EXACT MATCH**
  (three leaf probes). No fourth function. Commit:
  "Convert next Disc 1 leaf function to C". Merged as PR #12 (`9b28a2c`).
- 2026-07-08: **Phase 5E fourth C leaf integrated.** Branch
  `phase5e-next-c-leaf` from `main` after PR #12. Converted **only**
  `func_80090C60` (bit-set 0x20 at `*(arg0+0x38)` via ori):
  - `src/func_80090C60.c`
  - config: `[0x81460, c, func_80090C60]` + `[0x81474, asm]`
  - build: fourth C object; all four trimmed 0x20→0x14; ROM-order
  Validation: probe codegen exact; `build_us.sh` exit 0 **EXACT MATCH**
  (four leaf probes). No fifth function. Commit:
  "Convert next Disc 1 leaf function to C". Merged as PR #13 (`d5e6242`).
- 2026-07-08: **Phase 5F fifth C leaf integrated.** Branch
  `phase5f-next-c-leaf` from `main` after PR #13. Converted **only**
  `func_80090C74` (bit-clear 0x20 at `*(arg0+0x38)`):
  - `src/func_80090C74.c`
  - config: `[0x81474, c, func_80090C74]` + `[0x81488, asm]`
  - build: fifth C object; all five trimmed 0x20→0x14; ROM-order
  Validation: probe codegen exact; `build_us.sh` exit 0 **EXACT MATCH**
  (five leaf probes). No sixth function. Commit:
  "Convert next Disc 1 leaf function to C". Merged as PR #14 (`61c4efa`).
- 2026-07-08: **Phase 5G sixth C leaf integrated.** Branch
  `phase5g-next-c-leaf` from `main` after PR #14. Converted **only**
  `func_800C2B40` (store `arg0` to `*(D_800E2248+0x70)`, size 0x10):
  - first leaf outside the 90Cxx cluster (tail B2AF8)
  - config: `[0xB3340, c, func_800C2B40]` + `[0xB3350, asm]`
  Validation: probe + production **EXACT MATCH**. Commit:
  "Convert next Disc 1 leaf function to C". Merged as PR #15 (`602087f`).
- 2026-07-08: **Phase 5H `func_800C2B10` blocked.** Branch
  `phase5h-next-tail-leaf` from `main` after PR #15. Post-merge
  `split_us.sh --check` / `verify_us.sh` / `build_us.sh` all OK (exact SHA-1).
  Probed `func_800C2B10` (0x18, `D_800E2248 + (arg0<<2) + 8`) under GCC 14.2
  Phase 4J flags: ops match but instruction schedule never matches ROM
  (`sll→lui/lw→addiu` vs GCC `sll→addiu→lui/lw` or `lui/lw→sll→addiu`).
  No C/config change. Did **not** start `func_800C2B28`. Docs-only blocker.
  Explicit decision: park `D_800E2248` accessor siblings; next leaf must be
  outside that pattern (GCC-friendly) until era toolchain/maspsx.
- 2026-07-09: **Phase 5S sixteenth C leaf integrated.** Branch
  `phase5s-func-800906B4` from `phase5r-func-800904BC`. Converted
  **only** `func_800906B4` (stream+flags, size 0x30):
  - `src/func_800906B4.c`
  - config: `[0x80EB4, c, func_800906B4]` + `[0x80EE4, asm]`
  - build: spans `0x1F0` + `0x30` + `0x328` (was `0x548` 80CC4 span)
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x80EB4`).
  Commit: "Convert func_800906B4 to C (exact leaf match)".
- 2026-07-09: **Phase 5R fifteenth C leaf integrated.** Branch
  `phase5r-func-800904BC` from `phase5q-func-800904B4`. Converted
  **only** `func_800904BC` (empty `jr`/`nop` stub, size 0x8):
  - `src/func_800904BC.c`
  - config: `[0x80CBC, c, func_800904BC]` + `[0x80CC4, asm]`
  - build: spans `0x8` + `0x548` (was `0x550` 80CBC span)
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x80CBC`).
  Commit: "Convert func_800904BC to C (exact leaf match)".
- 2026-07-09: **Phase 5Q fourteenth C leaf integrated.** Branch
  `phase5q-func-800904B4` from `phase5p-func-800904AC`. Converted
  **only** `func_800904B4` (empty `jr`/`nop` stub, size 0x8):
  - `src/func_800904B4.c`
  - config: `[0x80CB4, c, func_800904B4]` + `[0x80CBC, asm]`
  - build: spans `0x8` + `0x550` (was `0x558` 80CB4 span)
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x80CB4`).
  Commit: "Convert func_800904B4 to C (exact leaf match)".
- 2026-07-09: **Phase 5P thirteenth C leaf integrated.** Branch
  `phase5p-func-800904AC` from `phase5o-func-800904A0`. Converted
  **only** `func_800904AC` (empty `jr`/`nop` stub, size 0x8):
  - `src/func_800904AC.c`
  - config: `[0x80CAC, c, func_800904AC]` + `[0x80CB4, asm]`
  - build: spans `0x8` + `0x558` (was `0x560` 80CAC span)
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x80CAC`).
  Commit: "Convert func_800904AC to C (exact leaf match)".
- 2026-07-09: **Phase 5O twelfth C leaf integrated.** Branch
  `phase5o-func-800904A0` from `phase5n-func-8008FCB4`. Converted
  **only** `func_800904A0` (`*(u16 *)(arg0+0x84) = 1`, size 0xC):
  - `src/func_800904A0.c`
  - config: `[0x80CA0, c, func_800904A0]` + `[0x80CAC, asm]`
  - build: spans `0x7E4` + `0xC` + `0x560` (was `0xD50` 804BC span)
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x80CA0`).
  Commit: "Convert func_800904A0 to C (exact leaf match)".
- 2026-07-08: **Phase 5N eleventh C leaf integrated.** Branch
  `phase5n-func-8008FCB4` from `phase5m-func-8008F880`. Converted
  **only** `func_8008FCB4` (`*(u16 *)(arg0+0x82) = 0`, size 0x8):
  - `src/func_8008FCB4.c`
  - config: `[0x804B4, c, func_8008FCB4]` + `[0x804BC, asm]`
  - build: spans `0x41C` + `0x8` + `0xD50` (was `0x1174` 80098 span)
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x804B4`).
  Commit: "Convert func_8008FCB4 to C (exact leaf match)".
- 2026-07-08: **Phase 5M tenth C leaf integrated.** Branch
  `phase5m-func-8008F880` from `phase5l-func-8008F868`. Converted
  **only** `func_8008F880` (decrement twin: `*(u16 *)(arg0+0x7C) = (*f - 1) & 0xF`):
  - `src/func_8008F880.c`
  - config: `[0x80080, c, func_8008F880]` + `[0x80098, asm]`
  - build: spans `0x18` + `0x1174` (was `0x118C` 80080 span)
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x80080`).
  Commit: "Convert func_8008F880 to C (exact leaf match)".
- 2026-07-08: **Phase 5L ninth C leaf integrated.** Branch
  `phase5l-func-8008F868` from `phase5k-func-8008F694`. Converted
  **only** `func_8008F868` (`*(u16 *)(arg0+0x7C) = (*field + 1) & 0xF`):
  - `src/func_8008F868.c`
  - config: `[0x80068, c, func_8008F868]` + `[0x80080, asm]`; shortened `7FEA8.s`
  - build: spans `0x1C0` + `0x18` + `0x118C` (was `0x1364` 7FEA8 span)
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x80068`).
  Commit: "Convert func_8008F868 to C (exact leaf match)".
- 2026-07-08: **Phase 5K eighth C leaf integrated.** Branch
  `phase5k-func-8008F694` from `phase5j-func-80090A0C`. Converted
  **only** `func_8008F694` (`*(unsigned int *)arg0 += 2`, mid-2A0C prefix):
  - `src/func_8008F694.c`
  - config: `[0x7FE94, c, func_8008F694]` + `[0x7FEA8, asm]`; shortened `2A0C.s`
  - build: spans `0x7D488` + `0x14` + `0x1364` + existing 90A0C/81220 chain
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x7FE94`).
  Commit: "Convert func_8008F694 to C (exact leaf match)".
- 2026-07-08: **Phase 5J seventh C leaf integrated.** Branch
  `phase5j-func-80090A0C` from `phase5i-next-gcc-friendly-leaf`. Converted
  **only** `func_80090A0C` (bit-clear `0x8` at `*(arg0+0x38)`, earliest 90Cxx
  sibling still in asm on six-leaf `main`):
  - `src/func_80090A0C.c`
  - config: `[0x8120C, c, func_80090A0C]` + `[0x81220, asm]`; shortened `2A0C.s`
  - build: third ROM-order asm unit `81220.s`; spans `0x7E800` + `0x14` + `0x218`
  Validation: scratch + `build_us.sh` exit 0 **EXACT MATCH** (probe `0x8120C`).
  Commit: "Convert func_80090A0C to C".
- 2026-07-08: **Phase 5I `func_800C7DC4` blocked.** Branch
  `phase5i-next-gcc-friendly-leaf` from `main` after PR #16. Post-merge gates
  OK (six leaves, exact SHA-1). Selected `func_800C7DC4` (0x10, no globals,
  outside accessor cluster). Scratch probe matched; production rebuild
  **1-byte NON-MATCH** at file `0xB85D0` (`addu` vs `move` in `jr` delay slot).
  Reverted config/C/build edits; re-split restored six-leaf state. Docs-only
  blocker. Duplicates `func_800C8F08` / `func_800C9C00` parked with C7DC4.

## GitHub Wiki Summary (copy-paste ready)

### Parasite Eve executable decompilation status

The USA Disc 1 executable currently rebuilds byte-for-byte from a mixed
assembly/C layout. `scripts/build_us.sh` produces the exact target SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, and `scripts/verify_us.sh`
passes the split/config checks and reports Phase 5BC with 52 matching C
leaves.

- **Current branch:** `phase5ae-2a0c-hole-aware`
- **Current checkpoint:** accepted Phase 5BB commit `e9f46c1` + isolated target-only Phase 5BC (`git stash list` empty)
- **Matching C leaves:** 52
- **Latest leaf:** `func_800822AC` (`lui/lw/jr/nop`) — 32-bit global getter (`return D_8009B70C;`), plain -O1
- **Prior leaf:** `func_80080940` (`lui/lw/jr/nop`) — 32-bit global getter (`return D_8009B554;`), plain -O1
- **Prior leaf:** `func_8007FCAC` (`lui/lw/jr/nop`) — 32-bit global getter (`return D_8009B590;`), plain -O1
- **Prior leaf:** `func_8007F778` (`lui/lw/jr/nop`) — 32-bit global getter (`return D_800A3608;`), plain -O1
- **Prior leaf:** `func_8007DEB0` (`lui/lw/jr/nop`) — 32-bit global getter (`return D_8009B4AC;`), plain -O1
- **Prior leaf:** `func_80074A28` (`lui/lw/jr/nop`) — 32-bit global getter (`return D_800956EC;`), plain -O1
- **Prior leaf:** `func_80042B28` (`lui/lw/jr/nop`) — 32-bit global getter (`return D_800A1838;`), plain -O1
- **Prior leaf:** `func_80051834` (`lui/lw/nop/srlv/jr/andi 1`) — unsigned indexed bit test (`return (D_800C0E24 >> a0) & 1;`), plain -O1
- **Prior leaf:** `func_80073DE8` (`lui v0,%hi(D_800945E6); lhu v0,%lo(v0); jr ra; nop`) — 16-bit global getter (`unsigned short return D_800945E6;`), plain -O1
- **Prior leaf:** `func_8003FFBC` (`lui v0,%hi(D_800A1704); lw v0,%lo(v0); jr ra; nop`) — 32-bit global getter (`int return D_800A1704;`), plain -O1
- **Prior leaf:** `func_80052524` (`lui v0,%hi(D_800C0E32); lhu v0,%lo(v0); jr ra; nop`) — twin 16-bit global getter (`unsigned short return D_800C0E32;`), plain -O1
- **Prior leaf:** `func_80052514` (`lui v0,%hi(D_800C0E28); lhu v0,%lo(v0); jr ra; nop`) — first 16-bit global getter (`unsigned short return D_800C0E28;`), plain -O1 (MIPS-I load-delay leaves the slot unfilled)
- **Prior leaf:** `func_80051E48` (`lui v0,%hi(D_800A1B30); addiu v0,v0,%lo; jr ra; nop`) — first pure address-return leaf (`return &D_800A1B30;`), compiled `-fno-delayed-branch` for the un-filled jr delay slot
- **Completed batch:** return-1 septuplet — seven byte-identical
  `jr; li v0,1` stubs: `func_8003D82C`, `func_80017E9C`, `func_80019050`,
  `func_80019058`, `func_800190AC`, `func_800190B4`, `func_8004DA9C`
  (shape `return 1`, GCC 14.2 exact)
- **Parked:** return-0 `func_8003DFD0` (5I-class `move` vs `addu` in delay slot, same as `800C7DC4` blocker)
- **Prior completed batch:** mid-`2A0C` empty stubs `func_80050D18`,
  `func_800527C0`, `func_8005BCA8`, `func_8008CA7C` (`jr; nop`)
- **Toolchain:** MIPS little-endian binutils 2.44 and GCC 14.2 in the
  `pe-mipsel` Distrobox; this is a matching modern toolchain, not a claim
  about the original game compiler
- **Repository boundary:** extracted executables, disc images, generated
  assembly, and game assets remain local and git-ignored

Next work is commit of the isolated Phase 5BC checkpoint, followed by a
separately validated same-shape getter candidate (`func_800870E0`).
