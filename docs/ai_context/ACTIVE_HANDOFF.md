# ACTIVE HANDOFF

Single source of truth for the current working state. Any agent or human
picking up this project reads this first and updates it after every
meaningful change.

## Current phase

**Phase 5S — `func_800906B4` integrated (sixteenth matching C leaf)** (branch
`phase5s-func-800906B4`). Phase 5R on `phase5r-func-800904BC`. Stack continues
through 5J (PR #17). Phase 5I/5H parked. Sixteen matching C leaves on this
branch. Parked in `80CC4` prefix: `904C4` (globals/branches) and the eight
`90574`–`9068C` stream-reader twins (reg-alloc).

Oracle: `scripts/build_us.sh` exits 0 with exact SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` (sixteen leaves).

Solid-state config (`configs/USA/disc1.yaml`):

```text
[0x800,     rodata]
[0x2A0C,    asm]
[0x7FE94,   c, func_8008F694]  VRAM 0x8008F694, size 0x14 (Phase 5K)
[0x7FEA8,   asm]
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
  git-ignored, never committed). No matching build, decompiled C, or
  PC-port work exists. `scripts/setup_env.sh` is implemented (pinned
  `.venv/` install); `verify_us.sh` is Phase 4E split-artifact sanity + honest
  Phase 4H rebuild status report. **`scripts/build_us.sh`:** asm-only
  assemble/link/pack/compare (non-matching). **MIPS LE path:** Distrobox
  `pe-mipsel` (Phase 4G; not on host PATH).

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
  **Phase 5B–5S done:** sixteen production C leaves (incl. mid-region trio
  `func_8008F694` / `func_8008F868` / `func_8008F880`, stubs `func_8008FCB4` /
  `func_800904A0` / `904AC` / `904B4` / `904BC`, mid-`80CC4` stream+flags
  `func_800906B4`, early 90Cxx `func_80090A0C`, and tail `func_800C2B40`).

## Next concrete step

**Milestone:** sixteen matching C leaves on branch `phase5s-func-800906B4`
(ready for PR). Resume triage at `80EE4.s` head (`func_800906E4`, size 0x38 —
has global `D_800B290C`). Oracle:

```text
build_us.sh  → exit 0 only on exact SHA-1 match
verify_us.sh → reports rebuild status when candidate present
SHA-1        → 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

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
