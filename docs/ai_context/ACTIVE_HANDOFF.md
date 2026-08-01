# ACTIVE HANDOFF

Single source of truth for current working state. Read this first; update after
every meaningful change. Prefer shortening over accruing.

## Current state

| Fact | Value | Derive |
| --- | --- | --- |
| Branch / tip | `main` @ tip (227, synced) | `git branch --show-current` / `git log --oneline -1` |
| Phase | **5FI-62a34 / 227 exact leaves** (62CE4 parked loop-layout, sixth skew instance; main v5 + atom at stash; cc1 archaeology gates boot-to-black) | `scripts/verify_us.sh` summary + exact rebuild |
| Matching C leaves | **227** | `grep -c ',\s*c,' configs/USA/disc1.yaml` |
| Yaml asm segments | **152** | `grep -c ',\s*asm\]' configs/USA/disc1.yaml` |
| Era leaf compiles | **70** | `grep -c '^era_compile \|^\w*=1 era_compile ' scripts/build_us.sh` |
| Target SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` | `scripts/build_us.sh` compare |
| Progress | https://blizz127.github.io/parasite-eve-progress/ | `scripts/publish_progress.sh` |

**Yaml `asm` segments are not remaining functions.** One segment can hold
dozens of glabels; do not subtract it from anything as a function count.

Oracle: bare `scripts/build_us.sh` exits 0 on exact SHA-1; `scripts/verify_us.sh`
reports Phase 5FE-table-2f970 / 224. Disc images / `asm/` / `build/` / `tools/era/`
are git-ignored inputs — never commit them.

**Toolchain**

- Default leaves: GCC 14.2 in Distrobox `pe-mipsel` (Phase 4J flags; selective
  `-G 8` / `-fno-delayed-branch` / `-fno-tree-ter`).
- Era leaves (opt-in): `scripts/setup_era.sh` → `era_compile` =
  cpp → cc1 → maspsx → GNU as, typically `-O2 -G0` (some leaves `-O1 -G0`).
- Era maspsx: `ERA_ASPSX_VER=2.21` + `--dont-expand-li`. **Why:**
  `expand_load_immediate` turns positive small `li` into `ori`; ROM wants
  `addiu`. Defer `li` expansion to GNU as. Same config also preserves
  large-literal `lui;ori` (cc1 emits PSY-Q `li` high + `ori` low natively).
  Do **not** bump aspsx-version casually — that also flips `nop_at_expansion`
  / `addiu_at`.
- **Vendored maspsx LOCAL PATCH:** `tools/era/maspsx/maspsx/__init__.py` is
  repo-tracked (`.gitignore` negations; `setup_era.sh` re-clones upstream
  AROUND it, restores the tracked file from git if absent). Patch 1 =
  **sw-store delay-slot fill**, opt-in per `era_compile` line via env
  `MASPSX_FILL_STORE_DELAY_SLOT=1`: an absolute `sw $r,SYM` macro immediately
  before a bare `j $31` is emitted as `lui $at,%hi` / `j $31` /
  `sw $r,%lo($at)`. sw only — sb/sh macro stores and multi-store epilogues
  are ROM-proven to stay pre-jr with a nop slot (e.g. func_8003FFAC vs
  func_8007FBC0: identical C shape, different ROM scheduling — the original
  units were assembled with different ASPSX scheduling).
- Patch 2 (`f0b9155`) / Patch 3 (`439c244`): **three-word indexed symbolic
  store AND load expansion**, opt-in per leaf via `MASPSX_THREE_WORD_SYMBOL_STORE=1`.
  Standalone `op $r,SYMBOL($index)` uses the retail/ASPSX-2.30-shaped
  `lui $at,%hi` / `addu $at,$at,$index` / `op $r,%lo($at)` sequence,
  for stores (2) and standalone indexed symbolic loads (3: lb/lbu/lh/lhu/lw/lwl/lwr;
  `lwc2` stays outside, durable negative test). Compound semicolon lines retain the
  2.21 four-word expansion. Flag-off rebuild is the exact leaf-count retail SHA.
  Full 224-leaf regression (flag OFF and flag ON over the three existing 3W store
  leaves) both exact; 153 vendored tests; re-clone restore byte-identical.
- Maspsx stdin: closed with `</dev/null` in `era_compile` (non-TTY hang under
  agent sockets). Bare `scripts/build_us.sh` is fine.

## How to count (do not hand-maintain)

```bash
grep -c ',\s*c,' configs/USA/disc1.yaml            # C leaves
grep -c ',\s*asm\]' configs/USA/disc1.yaml          # yaml asm segments (NOT fn count)
grep -c '^era_compile \|^\w*=1 era_compile ' scripts/build_us.sh  # era leaf compiles
git log --oneline -1
```

**Do not** count `asm/disc1/*.s` from disk. That tree is git-ignored and
contains orphans, stale duplicates, and nop-pads. **Yaml is the source of truth.**
Known stale orphans (counter ignore-list): `2E7D0.s` (superseded by live
`2E7D8.s`) and `807C.s` (stale duplicate of live `2A0C.s`; unreferenced in
yaml). Keep both out of function scans.

**asm/ sync invariant:** `$at` family totals from
`tools/analysis/at_absolute_store_counter.py` hard-fail (no SUMMARY) when asm/
is missing units or still holds glabels for yaml C leaves. Re-split with
`scripts/split_us.sh` before planning off a family count. Leaf count stays
yaml-only and still works when asm/ is stale.

## Proven era fingerprints (evidence, not claims)

| Fingerprint | Status |
| --- | --- |
| `move` → `addu` in delay slot | Proven 5EA / 5EB / 5EC / 5ED |
| `$v0` / `$v1` allocation | Proven 5EC / 5ED (sb+ret0 reuse) |
| `li` const materialization (`addiu` not `ori`) | Proven 5EC via `--dont-expand-li` |
| `$at` absolute `sw` macro expansion | Proven by scratch probe; integrated exact in 5EE |
| Branch delay-slot constant hoist (`beqz` slot) | **PROVEN** (5EG-first-branch): era cc1 `-O1 -G 8` reproduces the retail schedule on `func_8004F448` word-for-word |
| Test-and-clear-return if/else (`bnez` + j-over) | **PROVEN, VOLUME** (5ER): era `-O2 -G0` matches the adjacent byte/word twins `func_80038D1C` / `func_80038D48` — shared address in `$v1`, `addu $v0,$zero,$zero` in the `bnez` slot, `addiu $v0,$zero,0xFF` in the unconditional-jump slot, then `sb`/`sw` clear. Direct-global C rebuilt the address and used a 12-word `beq` form; one natural explicit-pointer phrasing retry matched all 11 words without pinning |
| `$a0`-in/`$v0`-out + redundant double store | **PROVEN** (5EH): era `-O2 -G8` preserves both stores + `addu` return-0 on `func_800438C0`; GCC 14.2 `-O1` merges stores and emits `move` — **era required for value-returning leaves**; era+gp `-G8` first proven here |
| Non-leaf stack frame + `jal` | **PROVEN** (5EI; repeated as volume in 5EK): era matches the `func_800197D0` / `func_800197F0` void-callee twins — `addiu $sp,-0x18` / `sw $ra,0x10($sp)` / `jal`+nop / `lw $ra` / `addiu $v0,1` / `jr $ra` with the `addiu $sp,+0x18` teardown **in the `jr` delay slot**, word-exact; 197F0 uses `-O2 -G0` and adds no primitive |
| Outgoing `$a0` + `jal` after double dereference | **PROVEN** (5EJ-outgoing-arg): era `-O2 -G0` on `func_80019484(int **)` emits `lw $v0,0($a0)` / load-delay nop / `lw $a0,0($v0)` / `jal func_800438C0` + nop, then the proven return-1 frame teardown shape; all 11 words exact |
| Return-forwarded `$v0` + teardown-before-`jr` epilogue | **PROVEN** (5EL-return-forwarding): era `-O2 -G0` on `func_8007F7A8` emits the frame + `jal func_8007FCAC` + nop, forwards `$v0` untouched, then `lw $ra`; `addiu $sp,+0x18`; `jr $ra`; nop. Era reproduces this per-function schedule as well as 197D0/F0's opposite teardown-in-slot schedule |
| Straight-line boot pointer-layout scheduling | **PROVEN, COMPILER-CONSTRAINED C** (5EM-boot-6a8d4): era `-O2 -G0` matches all 68 words / 19 absolute pointer stores in retail order. Both the initial plain-local source and one retail-order retry allocate cursors to `$a0/$a1`, constants to `$v0/$v1`, and sink `D_800B0E28` past `D_800B0E2C/E30`. The exact fallback therefore uses the established explicit-register convention (`$v0/$v1` cursors, `$a0/$a1` constants); it is target-specific matching C, not portable natural C |
| Counting-loop back-edge scheduling | **PROVEN; VOLUME-ELIGIBLE** (5EN/5EP `func_8006A674` probe): era `-O2 -G0` puts pointer advances in all five retail back-branch delay slots — `bnez` up-counters (`$a0+4`, `$v1+2`, `$a1+8`) and `bgez` down-counters (`$a3-4`, `$v0-4`) — and preserves the final store in the `jr` delay slot. The leaf remains parked for unrelated constant-hoist scheduling; the loop primitive passed. |
| Natural counting loop in volume | **PROVEN, VOLUME** (5ES `func_8004BF08`): era `-O2 -G0` matches a natural pointer-walk loop over parallel signed `int[8]` arrays in all 14 words, with no pins or maspsx opt-in. Explicit initialization in retail order (`i`, first pointer, second pointer) plus `do/while` phrasing gives `$a1/$a0/$v1` allocation; the first pointer advances before the bound test and the second pointer advances in the backward `bnez` delay slot. The declaration-initialized `for` form was semantically correct but allocated the three live values differently. |
| Pure-register bit-serial loop in volume | **PROVEN, VOLUME** (5ET `func_8005186C`): era `-O2 -G0` matches all 15 words on the first natural-C try — no loads/stores, calls, or `$gp`; explicit-init `do/while`; the unconditional `result <<= 1` fills the forward `bnez` skip-branch delay slot, the `bgez` back-edge keeps a nop slot, and the return lands as `addu $v0,$a1,$zero` in the `jr` delay slot |
| Indexed global-array store/load expansion | **PROVEN, TOOL-SOLVED** (`f0b9155` stores; `439c244` loads): per-leaf `MASPSX_THREE_WORD_SYMBOL_STORE=1` reproduces `lui` / indexed `addu` / op `%lo` and removed the extra L3 word in `func_8006A674` (153→152 words). `439c244` extends the gate to standalone indexed symbolic LOADS (all seven widths; `lwc2` stays outside — durable negative test; compound lines retain the 4-word expansion). Default off is byte-identical. |
| `lui;ori` large-literal synthesis | **PROVEN** (capability probe): both bit15-clear and bit15-set; cc1 emits PSY-Q `li` high + `ori` low; ROM-exact under 2.21 + `--dont-expand-li` |
| Rotated/peeled loop idiom | **PROVEN SHAPE** (5EV `func_80052BCC`, leaf parked on unrelated allocation): write the first iteration explicitly, then `while (cond) { body }` → era `-O2 -G0` emits the rotated shape: `beq`-exit head, bottom-tested `bne` back-edge, pointer advance in both delay slots |
| Signed `char` vs 0xFF-range constant | **PROVEN SHAPE** (5EV `func_80052BCC`, same parked leaf): signed `char c` compared against `0xFF` emits the conversion `andi` on the compare path even after `lbu`; `unsigned char` does not. Typing controls the mask |
| Return-accumulator vs direct-return phrasing | **PROVEN (5FH `func_80037548`)**: a search loop with a default return value must hold the result in an ACCUMULATOR (`signed char result = 0; ... result = v; break; return result;`). Direct `return v;` on the match path makes cc1 emit a SEPARATE `addu $v0,$zero,$zero` default path before `jr` (28 words vs ROM's 27) — the accumulator keeps one `$a2` merge with the `sll/sra` sign-extension pair hoisted to the merged exit |
| `-fschedule-insns2` load-delay `li` hoist | **PROVEN, FIRST LEAF** (5EW `func_80052BCC`, era `-O1 -G0 -fschedule-insns2`): the post-allocation scheduler hoists an independent `li` above `sb`/`andi` into the `lbu` delay — the exact spot retail's ccpsx scheduled it. At plain `-O1` the same `li` emits after the `andi` (14/15). Paired phrasing: two `0xFF` consts of different modes (u8 head const dies at the guard → loop re-materializes into the freed `$v1`; `int` loop byte → mask-free raw `bne`); comparing the loop byte against a *variable* or both consts sharing a mode cross-jumps/CSE-shares head and loop |
| sched2 scope (negative result) | **NARROWED (5EY `func_8003E610`)**: `-fschedule-insns2` is NOT a universal retail fingerprint — it governs **store-adjacent `li`/`addiu` placement and load-delay hoists** only (52BCC head-`li`, 6A674's 21 order swaps). Straight-line `jal`-arg scheduling (`$a0` hoisted + `$a1` in slot for two-arg calls; `$a0` slot-filled single-arg; nop slot no-arg) is already correct at plain `-O2`. Do NOT flip sched2 into the era default |
| dbr_sched `$v0`-steal screening rule | **CHARACTERIZED (5FB `func_800698D4`, PARKED)**: a `beqz`/`beq` whose delay-slot steal candidate is a `$v0`-setter gets the fill when the branch target hits a `jal` immediately (kills `$v0`), but retail DECLINES the steal when the target is the return-computation block (`$v0` live to `jr $ra`) — our cc1 steals anyway. Screening rule: nop slot + `$v0`-constant load on fall-through + branch to a RETURN block → expect divergence; same pattern to a `jal`-adjacent block → matches. reorg.c liveness skew (ccpsx vs 2.7.2-psx), not source-expressible |
| Nested-if defeats range-test collapse | **PROVEN IDIOM (5FB `func_800698D4`)**: `v != 0 && v != -1` folds to `addiu $v0,$v0,1; sltiu $v0,$v0,2; bnez` under era `-O2` (range test, not retail's shape). Two nested `if`s keep the separate `beqz`/`beq` compares. -O1 keeps compares but flattens other structure |
| Frame-size arithmetic for struct locals | **PROVEN (5FB `func_800698D4`)**: size opaque locals from the frame, not the type's rounded size — DsSearchFile's CdlFILE local is `0x18` (pos 4 + size 4 + name 16): `0x10` args + `0x18` local + `$s0` + `$ra` = frame `0x30`. A `0x20` local emits frame `0x38` and fails at word 0 |
| Five-arg call (o32 stack arg) | **PROVEN, FIRST LEAF** (5FC `func_8006E834`, PARKED on unrelated residual): the 5th argument emits `sw $v0,0x10($sp)` in the `jal`'s delay slot — plain C `f(a,b,c,d,e)` with an immediate 5th arg, era `-O2 -G0`, worked first try. `sb $v0,0x29($sp)` (struct byte field) also lands in a `jal` slot |
| Frame decomposition before writing | **PROVEN METHOD (5FB/5FC)**: decompose the frame BEFORE choosing local sizes — `args + locals + saves + pad = frame` must be exact (5FB: CdlFILE `0x18` not `0x20`; 5FC: args `0x18` + env `0x18` + local30 `0x8` + regs `0xC` + pad `0x4` = `0x48`, byte field lands at `env[0x11]` = `0x29($sp)`). Wrong local size fails at word 0 |
| Aggregate element type as addressing-mode lever | **PROVEN (5FD `func_8002F9CC`)**: for an indexed store into a symbol array, declaring the real aggregate element (`SlotRecord D_800A5D58[]`, `arr[i].field = 0`) makes cc1 emit the standalone indexed symbolic store (`sw $0,SYM($3)`) at plain `-O2` — flat `arr[i*55] = 0` instead hoists `la $5,SYM` out of the loop (invariant under `-O2`/`-O1`/`-O1 -fschedule-insns2`; an addressing choice, not scheduling). With the symbol store present, `MASPSX_THREE_WORD_SYMBOL_STORE=1` passes it to GNU as for retail's 3-word `lui $at / addu / sw %lo($at)` form. Also: a lone symbol materialization is NOT an `-O1` signal — the `-O1` lever is for *repeated* constant/address materialization |
| `-O1` per-use constant materialization — SELECTION RULE | **PREDICTIVE (three leaves)**: if ROM materializes the same constant/address more than once, try `-O1` FIRST. `-O2`'s shared hoist runs through the hardwired `optimize>1` path (not flag-reachable); `-O1` re-materializes per use. 6A674 (discovered: per-use `-1`), 6A5BC (applied: `$s0=1` twice), 3E680 (predicted from five per-store `lui`s with a shared `0x8009` high half retail didn't CSE) |
| Return-use readiness of asm callees | **VALIDATED (5EZ `func_8006A5BC`)**: a caller may USE a still-asm callee's return and stay matchable when the use is a **raw full-width compare** (`beq $v0,$s0`, no mask/sign-extend) or a **bare store** (`sh $v0`). Both are codegen-determined regardless of the callee's true return type, so `int f(void)` externs suffice. Extends the 5EY rule (immediates-only args, returns ignored) |
| Fn-ptr arg to still-asm callee | **PROVEN, FIRST LEAF** (5FA `func_8003E680`): `f(func_8003E91C)` emits `lui $a0,%hi(sym)` / `addiu $a0,$a0,%lo(sym)` with R_MIPS_HI16/LO16 relocs against a same-segment TEXT symbol; the linker resolves it exactly like a data symbol. Declare `extern void g(void);` and pass the bare name |
| Unsigned loop-bound compare | **PROVEN (5FA `func_8003E680`)**: ROM `sltiu` (unsigned) vs cc1's `slt` for `int i < const` — declare the counter `unsigned int`. One-word type-driven fix, no flag involvement |

All four fingerprints from the original 5EA era claim are now proven in bytes.
The “~290 era-blocked functions” figure remains an **ESTIMATE**, not a countdown.

## Known-open families

- **sb+ret0:** **done** in 5ED (family closed).
- **`$at` absolute-store population:** counter committed
  (`tools/analysis/at_absolute_store_counter.py`). The historical integration
  inventory was **18 pre-jr** / 14 delay-slot / 5 sb-sh; the current yaml-live
  population is **0 pre-jr** / **0 delay-slot** / 5 sb-sh. Weak-int policy **NO**.
  - **Pinned by 5EG-readers:**
    - `D_8009D240` = `unsigned short *`, `D_8009D260` = `unsigned char *`
      via `func_8008AB1C` (era `-O1 -G0`).
    - `D_800A1870` = `void (*)(void)` via `func_80042B6C` (era `-O2 -G0`).
  - **Integrated:** `func_80085728`; 5EI readers-typed trio; 5EJ `D_8009D28C`
    int-state (4); 5EK `D_8009D270` unsigned flags (2); **5EF all 14
    delay-slot `sw` members**. The pilot `func_8007FBC0` plus the remaining 13
    typed leaves are integrated exact. Current leaf count **217**.
  - **Delay-slot shape: FAMILY CLOSED (5EF).** Vendored maspsx LOCAL PATCH
    (`MASPSX_FILL_STORE_DELAY_SLOT=1`) fills the `j $31` slot with the trailing
    absolute `sw`. Pilot gate exact + objdump-probed (`3C01800A 03E00008
    AC2436A0`). The remaining 13 members now have per-global typing evidence,
    and all 14 members pass the full exact-match gate; see
    `docs/ai_context/PHASE5EF_TYPING.md`.
  - **sb-sh-five: RECLASSIFIED — never tool-blocked.** ROM words show sb/sh
    macro stores stay **pre-jr with a nop slot** (func_80033A2C sb,
    func_800C6ED8/C6EE8 sh, func_800C6EC0 dual-sh; func_8001A374 has a
    cc1-filled `li` slot). Current maspsx already emits that shape; the patch
    deliberately does not touch sb/sh. Remaining work is typing + integration,
    toolchain-independent.
  - **Still open (typing):** remaining opaque-word (`D_800A1868` other writers).
- **`lui;ori`:** **CAPABILITY-VERIFIED** — not a blocker. Constant-heavy
  computational functions (mult/div/mask, e.g. ÷100 via `0x51EB851F`) are
  approachable as a **separate future phase**; synthesis itself is solved.
- **gp arena loop `func_80055724`:** **PARKED-SCHEDULING** (branch
  `phase5eu-gp-loop-55724`; closest candidate stashed as `park phase5eu
  func_80055724 while-form 13-15`). Empty 8-byte frame **solved** (cc1 2.7.2
  `vars=8` home slots, natural). Blocker: three-way scheduling tension —
  while-form keeps frame+regs but hoists the cursor load above the `blez`
  guard (13/15); if+for keeps frame+regs but duplicates the guard and steals
  the prologue into its slot; if+do/while gets word order but `vars=0` and
  flipped regs. era `-O1 -G8` output is **byte-identical** to `-O2` for both
  leading phrasings — no per-function `-O` support from this leaf. Residual is
  scheduling, not proven allocation. Detail: `docs/ai_context/parked_blockers.json`.
- **disc mount `func_800698D4`:** **PARKED-SCHEDULING** (branch
  `phase5fb-boot-698d4`; closest candidate stashed as `park phase5fb
  func_800698D4 nested-ifs 140-141 (search3 beqz-slot residual)`). Disc
  identification/mount — clears the mount flag, verifies drive ready, searches
  for `\FMV1\PEDISC01.IDF;1` / `\PE.IMG;1` / `\FMV2\PEDISC02.IDF;1` via
  `DsSearchFile`, records via `func_80080C48` → `D_800B0DD8` + `D_800B0DCD`
  flag bits. 140/141 words; everything exact except ONE delay-slot steal:
  search #3's `beqz` (`0x5A24C`) — retail nop, ours steals `addiu $v0,$zero,-1`.
  Mechanism is the dbr_sched `$v0`-liveness screening rule (fingerprint table);
  not source-expressible. Banked idioms: nested-if defeats range-test collapse;
  CdlFILE local is `0x18` not `0x20` (frame arithmetic). Detail:
  `docs/ai_context/parked_blockers.json` (`boot-698d4-dbr-sched`).
- **post-mount loader `func_8006E834`:** **PARKED-ALLOCATION** (branch
  `phase5fc-boot-6e834`; candidate stashed as `park phase5fc func_8006E834
  89-91 (call-result register-home residual)`). Post-mount image loader +
  display env: reads a `D_80093164` lhu offset/size pair from the mounted
  image base `D_800B0DD8` (written by parked 698D4 — the two are producer/
  consumer), polls `func_800811E4`, then `VSync(0)`/`SetDispMask(0)`/
  `func_800749D8(&env,0,0,320,240)` (PROBABLE SetDefDrawEnv)/`PutDispEnv`.
  89/91 content words at era `-O2 -G0`. PROVEN firsts: five-arg call (5th
  arg `sw $v0,0x10($sp)` in the `jal` slot); frame decomposition method.
  Retail FOLDS the `r==0||r==-1` range test in this unit (698D4's did not —
  per-TU compile-settings datapoint). Residual: poll result homed in `$v0`
  by retail (two restores) vs `$v1` by ours — call-result register-home skew,
  not source-expressible. `$v0`-liveness rule NOT exercised (non-event).
  Detail: `docs/ai_context/parked_blockers.json` (`boot-6e834-register-home`).
- **flag-clear loop `func_800374E8`:** **PARKED-ALLOCATION, register COLORING**
  (branch `phase5ff-374e8`; candidate stashed as `park phase5ff func_800374E8
  (register-coloring skew; structure correct)`). Flag-clear loop over 4 x 56-byte
  records at `D_800BCEA8` — **RECORD TYPE ESTABLISHED** (durable deliverable;
  propagates to `func_80037548`): +0x00 `unsigned char` (lbu/sb), +0x0C
  `unsigned int` flags (lw/sw; bit 0x02000000 cleared here), +0x10 `signed short`
  (lh/sh); extent closes EXACTLY at +0xE0 = 4 x 56. **STRUCTURE CORRECT**: 5FD
  aggregate-subscript rule (no `rec` pointer) + the landed load gate (`439c244`)
  produce retail's 3-word indexed-symbolic shape (no `la` hoist, correct DAG and
  scheduling). **RESIDUAL — register coloring only**: era cc1 assigns
  mask->`$v0`/chain->`$v1`/value->`$v0`; ROM is mask->`$v1`/chain->`$v0`/value->`$v1`.
  Five phrasings x two loop forms x ladder rungs are ALL byte-identical —
  invariant under phrasing. Same class as 6E834's call-result home:
  hard-register-assignment skew. This leaf MOTIVATED the maspsx load-gate patch.
  Detail: `docs/ai_context/parked_blockers.json` (`register-coloring-374e8`).
  **TWIN FALSIFIED (5FH)**: `func_80037548` was probed and MATCHES 27/27
  (accumulator shape) — no coloring skew. Refined rule: coloring skew is
  LIVE-VALUE-PRESSURE DEPENDENT (374E8: mask+chain+value all live;
  37548: needle/accumulator/index in $a0/$a2/$a1 leave $v0/$v1 free), not
  per-table. Predict skew only when 3+ scratch values compete.
- **sentinel walk `func_80062CE4`:** **PARKED-SCHEDULING, loop-LAYOUT**
  (do/while form stashed as `park func_80062CE4 (loop-layout scheduling;
  do/while lever proven source-invariant)`). Sentinel validate-and-consume
  over the D_8009D154 list: if D_8009D160 (pending) is still linked, promote
  it to D_8009D15C (confirmed); clear D_8009D160 either way. 12/18; PROVEN
  source-invariant — both while-form and do/while produce BYTE-IDENTICAL
  output; ROM has sentinel-at-top->advance->null-back-edge. cc1
  canonicalizes loop body order before block layout. SIXTH skew instance.
  CARVE CORRECTION: spimdisasm 0x5C label OVERSHOOTS — active span 0x48;
  trailing 5 words are func_80062Fxx prologue. Postmortem: dual gp-four
  filters could not catch loop-layout skew (no pre-compile tell known).
  Detail: `docs/ai_context/parked_blockers.json` (`loop-layout-62ce4`).

- **`main` (`func_8001220C`, 187 words):** **PARKED-SCHEDULING, WITH COMPLETE CANDIDATE**
  (candidate preserved at stash; five drafting iterations on scratch /tmp/mainvN.c).
  The boot keystone: init sequence, 20-call-site mount/read/dispatch loop, volume gate,
  A8-code three-way state switch. ~180 words match at opcode/position. DURABLE DELIVERABLES:
  the 9-word scratchpad stack handoff (sp → 0x1F8003FC, jal 8019234C, restore) is
  BYTE-EXACT as fenced inline asm with full caller-saved clobbers — the fenced-exception
  mechanism (register-pinning precedent) is validated for when main integrates. Role map
  pinned: $s0 data ptr (D_800B0CD8), $s1 dispatch, $s2 flagbyte (+0xF5), $s3 state_val
  (0xA9400048), $s4 bitmask (0x100000). All 20 externs typed (69B08 int, 1909B4→6E9A0
  raw-flow chain). RESIDUAL — ONE mechanism, proven scheduler-driven by an
  init-placement lever test (draft 4 declared bitmask at top, draft 5 moved init after two
  calls; cc1 kept the li at the same position and the $s-save interleaving identical —
  source cannot express the difference): prologue save-batching + invariant-constant
  placement, cc1 ordering pass vs ccpsx. Fifth scheduling-family instance.
  NOTE: this park joins 698D4 and 6E834 as the THIRD boot-chain function in the skew set —
  the cc1 archaeology now directly gates boot-to-black. Detail:
  `docs/ai_context/parked_blockers.json` (`main-prologue-scheduling`).

- **ccpsx-vs-2.7.2 SKEW SET — four distinct mechanisms:** (1) the
  allocation/scheduling family (`6A674`/`55724`/`52BCC`; two recovered via
  `-O1`), (2) dbr_sched `$v0`-liveness slot-steal (`698D4`), (3) call-result
  register home (`6E834`), (4) register coloring / pseudo-numbering
  (`374E8`). All unreachable from C. (3)+(4) are both register-ASSIGNMENT
  skew, strengthening the case that the single highest-value open lever is
  whether a closer-to-ccpsx cc1 build is obtainable — that would be the
  one-layer-up analog of the maspsx patches and could address the whole set.
  SIX instances now documented (four scheduling, two register-assignment), and THREE are boot-chain functions (`main`,
  `698D4`, `6E834`) — the cc1 question is no longer archaeology for curiosity;
  it directly gates boot-to-black under plan A.
  Do not chase mid-leaf.
- **PARKED-ALLOCATION/SCHEDULING family:** cc1 2.7 register
  allocation/scheduling decisions that natural C cannot steer and `-O` level
  does not change. **FAMILY INVESTIGATED (read-only, accepted): NO SINGLE
  KNOB.** All residuals are present in cc1's **raw** output, pre-maspsx
  (maspsx does only `move`→`addu`, delay-slot nops, the 2.21 indexed-store
  expansion — no reordering/renaming), so a maspsx patch cannot fix any of
  them; the `addiu_at` template does not apply. Pass attribution
  (flag-probed) and current status:
  - `52BCC`: **MATCHED (5EW, leaf 218)** — the `-O1`→`-O2` flip required
    exactly `-fexpensive-optimizations` + `-fschedule-insns2` (regclass +
    post-alloc scheduler; bisection-proven minimal pair). Retried at era
    `-O1 -G0 -fschedule-insns2`: two-const-mode phrasing (u8 head const dies
    at the guard → loop const re-materializes into `$v1`; `int` loop byte →
    mask-free raw `bne`) + sched2 hoisting the head `li` into the `lbu`
    delay = all 15 words exact. First `-fschedule-insns2` leaf.
  - `55724`: pre-reorg RTL emission order (C statement order); NOT
    `dbr_sched` (`-fno-delayed-branch` doesn't move it), `-O`-invariant.
    Retail *sank* the p-load below the guard; 2.7.2-psx has no pass that
    sinks loads past conditional branches. **No lever** — constrained-C or
    acceptance. (Still parked; see entry above.)
  - `6A674`: **MATCHED (5EX, leaf 219)** — the `-O`-sensitive constant
    materialization runs through a hardwired `optimize>1` path (not
    flag-reachable), so `-O1` is the only lever; at `-O1 -G0` the residual
    shrank 45→21 (all pure `li`/`addiu`-before-store order swaps), and
    `-fschedule-insns2` closed them to **0/152** with the 5EP pins intact.
  - **`-fschedule-insns2` is a GENERAL RETAIL FINGERPRINT** (two independent
    leaves, 22 positions): retail's ccpsx ran post-allocation scheduling;
    our default doesn't. Try sched2 early on future scheduling-position
    residuals. Hypothesis to test later (carefully; current leaves match
    without it): sched2 may belong in the era default flag set.
  Evidence: scratch compiles `/tmp/fam_inv` + `/tmp/o1` (session-recorded).
- Complex `$gp` / GTE / BIOS / mult-div / large non-leaves: still open; not
  inventoried here. Path forward is matching real logic, not harvesting
  trivial setters.

## Boot Rung 1 — COMPLETE, climbing `main`'s call chain

```text
main -> func_8006A5BC ✓ exact C (5EZ, leaf 221)   # boot init, VSync waits
     -> func_8006A64C ✓ exact C -> { func_8006A8D4 ✓ exact C,
                                     func_8006A674 ✓ exact C (5EX, leaf 219) }
     -> func_8003E610 ✓ exact C (5EY, leaf 220)   # display/graphics bring-up
     -> func_8003E680 ✓ exact C (5FA, leaf 222)   # subsystem-init dispatcher
```

- `func_8003E680` is **MATCHED (5FA)**: era `-O1 -G0`, all 53 words exact.
  Zero five state globals (Stage-0 reader types: D1C4/D280 unsigned compares,
  D1A0 flags, D250 opaque, CDDC `int` index), 2000-pass poll loop with `i++`
  in the `jal` delay slot (ROM `sltiu` → `unsigned int` counter — the only
  phrasing fix needed), callback registration, ~11 subsystem inits. **First
  fn-ptr-to-asm-callee arg**: `func_80073D24(func_8003E91C)` →
  `lui $a0,%hi` / `addiu $a0,$a0,%lo` with R_MIPS_HI16/LO16 against the
  same-segment text symbol; links exactly. `-O1` predicted by the selection
  rule (five per-store `lui`s, no CSE). Segment-head carve of 2EE80:
  C `0xD4`, resume `2EF54.s` `0x1858`. Next candidates: `func_800698D4`
  (159L, disc mount w/ SDK `DsSearchFile`), `func_8003F3C4` (245L).

- `func_8006A5BC` is **MATCHED (5EZ)**: era `-O1 -G0`, all 36 words exact on
  the first attempt. Four setup calls, two structurally identical
  `while (f() != 1) VSync(0);` loops (VSync = `func_80073A44`, SDK), then
  `func_8007F7A8()`'s return stored to `D_800B0DD4` (`unsigned short`, typed
  by its `lhu` reader). `-O1` reproduces retail's per-use `$s0=1`
  materialization — in `func_80086FF8`'s delay slot AND re-materialized
  between the loops (the 6A674 `-O1` lever, third leaf). Return-use safety:
  both loop conditions compare `$v0` raw (full 32-bit `beq`, no
  mask/sign-extend), so asm callees declared `int(void)` are codegen-safe.
  Identical loop bodies did NOT cross-jump. Mid-55430 carve: prefix
  `0x598C`, C `0x90`, then the three existing boot C leaves — **four
  contiguous C carves, no asm between**. Next candidates up the chain:
  `func_8003E680` (56L, state zeroing + 2000-pass poll + callback
  registration), `func_800698D4` (159L, disc mount w/ SDK `DsSearchFile`).

- `func_8003E610` is **MATCHED (5EY)**: era `-O2 -G0`, all 28 words byte-exact
  on the **first** attempt — no sched2, no pins. Straight-line dispatcher of
  ten calls with immediate args (`0x140`/`0xE0` = 320x224 display res), no
  branches/loops/`$gp`/globals; plain `-O2` reproduces ccpsx's mixed
  arg-load/delay-slot placement exactly. All ten callees are extern-declared
  with call-site-determined signatures (immediate args, no returns used —
  callee bodies don't affect codegen; one already C: `func_80080CC8`).
  Mid-2E7D8 carve: prefix `0x638`, C `0x70`, resume `2EE80.s` `0x192C`.
  Readiness ranking of `main`'s remaining callees (size + callee C/SDK
  coverage) put it first; next candidates in order: `func_8006A5BC` (42L,
  two wait loops + one `sh` global), `func_8003E680` (56L, state zeroing +
  2000-pass poll loop + callback registration).

- `func_8006A674` is **MATCHED (5EX)**: era `-O1 -G0 -fschedule-insns2` +
  `MASPSX_THREE_WORD_SYMBOL_STORE=1`, all 152 words byte-exact. `-O1` gives
  retail's per-use `-1` materialization (the `-O2` shared hoist is hardwired
  `optimize>1`, not flag-reachable); sched2 places every `li`/`addiu` before
  its adjacent store (21 order swaps). The six semantic pins from the 5EP
  bounded candidate are load-bearing (dropping all six → 46 mismatches).
  Mid-55430 carve fills the 6A64C/6A8D4 gap exactly (0x260); the three boot
  C carves are contiguous.
- `func_8006A64C` matches all 10 words on era `-O2 -G0`: two sequential
  `void(void)` calls, teardown before `jr`, and a nop delay slot. Both
  `R_MIPS_26` relocations resolve at link time; matching a caller requires a
  known callee signature, not that every callee already be C.

## Standing policy

1. **PROBE BEFORE GRIND.** The two biggest unblocks (maspsx stdin hang;
   `expand_load_immediate` forcing `ori`) were short diagnostics, not
   integrations. When a family is blocked, diagnose before more members.
2. **Homogeneous families may be batched.** Risk lives in the first member.
3. **`asm/` is not a source of truth for counts.** Use `configs/USA/disc1.yaml`.
4. **Commit messages are not evidence.** A claim is proven when a gate is green
   and the leaf is objdump-probed (not SHA alone on carves).
5. **No weak-int cheat:** do **not** invent width a narrower store contradicts
   (e.g. `sh`/`sb` → `int`). Distinct from **opaque-word** typing (consistent
   32-bit `sw`/`lw` everywhere) — that is a separate lead ruling, currently
   open under `TYPING-POLICY` in `parked_blockers.json`.
6. **Width-only setters are triaged in `parked_blockers.json`.**
   `READY-FROM-READER` (src reader already *types* it), `BLOCKED-ON-READER`
   (undecompiled reader not yet proven to be a mere use-site),
   `TYPING-POLICY` (opaque 32-bit word; use-site found, no narrowing possible),
   or `DECISION-BLOCKED` (write-only; no reader). A use-site is not a type-site
   (`func_800405A4` lesson). Re-check after every reader phase.
   `5EF-delay-slot` **CLOSED** (14/14 integrated); `sb-sh-five` reclassified
   typing-only.
7. **Register pinning is an evidence-backed fallback, not a shortcut.** Use it
   only after natural C and a retail-order phrasing retry prove that the
   residual is register **allocation**, not statement order. Pins must have
   semantic names and a source comment recording the allocation proof
   (`func_8006A8D4` exact; `func_8006A674` bounded parked example).

## Resolved blockers

- **Phase 5I** delay-slot (`move`/`or` vs `addu`): **SOLVED in 5EC** by era.
- **Maspsx non-TTY hang:** **SOLVED** (`</dev/null` in `era_compile`).
- **`lui;ori` large-literal synthesis:** **CAPABILITY-VERIFIED** (scratch probe;
  both sign cases; no flag change).
- **5EF delay-slot (sw in `j $31` slot):** **CLOSED in 5EF** by the
  vendored maspsx LOCAL PATCH (`MASPSX_FILL_STORE_DELAY_SLOT=1`). Key evidence:
  `func_8003FFAC` vs `func_8007FBC0` — identical C, different ROM scheduling
  (pre-jr+nop vs in-slot) ⇒ original units assembled under different ASPSX
  scheduling; behavior is opt-in per leaf. All 14 members are integrated exact;
  sb/sh never fill (ROM-proven).

## History (append-only, truncated)

| Phase | **224 exact leaves** (tools: maspsx load gate `439c244` on main; parked: 698D4/6E834/374E8) | `scripts/verify_us.sh` summary + exact rebuild |
| --- | --- | --- |
| 4I–4J | 0→1 path | Exact asm rebuild; GCC 14.2 first leaf |
| 5B–5CW | →98 | Empty stubs, getters, store/setter batch |
| 5CX–5DB | →103 | Countdown memset/memcpy (`$2`/`$3` pins) |
| 5DC–5DJ | →156 | `$gp` small-data (`_gp`+`-G 8`); `-fno-tree-ter` |
| 5EA | 157 | Era dual-toolchain; return-0 `addu` |
| 5EB | 161 | Return-0 twins via mid-segment holes |
| 5EC | 163 | sb+ret0; `--dont-expand-li`; 5I dead |
| 5ED | 170 | sb+ret0 batch harvest (family closed) |
| 5EE | 171 | `$at` absolute-`sw` integrated pilot; delay-slot shapes blocked |
| 5EG-readers | 173 | Type-pinning readers `func_8008AB1C` / `func_80042B6C`; `D_800A1870` decl fix |
| 5EG-setter | 174 | `func_80085728` dual-store; first reader-recoverable pre-jr setter |
| 5EH-opaque-word | 182 | u32 opaque-word ruling; 8 A182x setters (`42BD8`…`42C64`) |
| 5EI-ready-from-reader | 185 | READY-FROM-READER setters `42910`/`42B38`/`42B50` |
| 5EJ-d8009d28c-state | 189 | `D_8009D28C` int-state setters `17FDC`/`17FF0`/`192B8`/`192C8` |
| 5EK-d8009d270-bitwise | 191 | `D_8009D270` unsigned flags setters `87198`/`87414` |
| lui-ori probe | 191 | Large-literal `lui;ori` CAPABILITY-VERIFIED (docs only) |
| 5EF-pilot | 192 | Vendored maspsx LOCAL PATCH (sw delay-slot fill); `func_8007FBC0` integrated |
| 5EF | 205 | Remaining 13 delay-slot `sw` members typed and integrated; family closed 14/14 |
| 5EG-first-branch | 206 | First branchy leaf `func_8004F448`; era cc1 `-O1 -G 8` hoists const into `beqz` delay slot word-exact (branch scheduling capability proven) |
| 5EH-arg-return | 207 | First value-returning leaf `func_800438C0` on era path: `-O2 -G8` preserves double store, `addu` return-0, era+gp proven; GCC 14.2 store-merge + `move` documented as $CC-path limits |
| 5EI-first-nonleaf | 208 | First non-leaf `func_800197D0` on era `-O2 -G8`: frame (`addiu $sp,∓0x18`, `sw/lw $ra,0x10($sp)`) + `jal func_800375B4`; teardown `addiu $sp,+0x18` lands **in the `jr` delay slot** word-exact |
| 5EJ-outgoing-arg | 209 | `func_80019484(int **)` on era `-O2 -G0`: double-dereference load schedule sets outgoing `$a0` before `jal func_800438C0`; load-delay nop, jal nop, frame, and teardown-in-`jr`-slot all word-exact |
| 5EK-volume-197f0 | 210 | First post-probe volume leaf: `func_800197F0` on era `-O2 -G0` transfers the proven 197D0 frame + void `jal` + return-1 + teardown-in-`jr`-slot shape word-exact; no new primitive |
| 5EL-return-forwarding | 211 | `func_8007F7A8` on era `-O2 -G0` forwards `func_8007FCAC`'s `$v0` untouched and reproduces retail's opposite epilogue schedule: teardown before `jr`, nop in the delay slot; all eight words exact |
| 5EM-boot-6a8d4 | 212 | First Rung-1 boot leaf: `func_8006A8D4` on era `-O2 -G0` lays out boot memory regions with 19 ordered absolute pointer stores; register-pinned byte cursors reproduce all 68 retail words exactly after two plain-local phrasings fail the retail register allocation/store schedule. Compiler-constrained, target-specific C is documented in source |
| maspsx indexed-store | 212 | Toolchain patch `f0b9155`: default-off `MASPSX_THREE_WORD_SYMBOL_STORE=1` opt-in adds the three-word symbol+register store form; exact 212-leaf regression, 148 tests, and live re-clone durability passed |
| 5EN/5EP-loop-probe | 212 | `func_8006A674` proves five `bnez`/`bgez` loop back-edge delay slots plus store-in-`jr`-slot; L2 and late allocation deltas cleared, but the leaf is parked with a 45-word `$v1` constant-hoist residual and no 213 claim |
| 5EQ-boot-6a64c | 213 | Boot wrapper `func_8006A64C` on era `-O2 -G0`: calls matched-C `func_8006A8D4` then live-asm `func_8006A674`, both proven `void(void)`; both `R_MIPS_26` relocations resolve and teardown-before-`jr` + nop-slot matches all 10 words |
| 5ER-d1c-d48 | 215 | Adjacent byte/word test-and-clear-return twins `func_80038D1C` / `func_80038D48` on era `-O2 -G0`; explicit pointer reuse gives retail `bnez` + j-over delay-slot returns and `sb`/`sw` clears, all 11 words each exact after one natural phrasing retry |
| 5ES-loop-4bf08 | 216 | First loop-as-volume leaf: natural explicit-init pointer walk in `func_8004BF08` clears two parallel `int[8]` arrays; era `-O2 -G0` reproduces all 14 words, including the split pointer advances and backward-`bnez` delay slot, with no pinning or tool flag |
| 5ET-loop-5186c | 217 | Loop-as-volume repeats: pure-register 16-pass bit-serial loop `func_8005186C` on era `-O2 -G0`, all 15 words on the first natural-C try; unconditional `result <<= 1` fills the forward `bnez` skip slot, nop `bgez` back-edge; mid-4204C carve (prefix 0x20, C 0x3C, resume 420A8.s 0x5A0) |
| 5EU/5EV parks | 217 | `func_80055724` (p-load hoist; `-O1`≡`-O2`) and `func_80052BCC` (rotated-loop `$v0`/`$v1` role swap, 13/15) parked as the **PARKED-ALLOCATION/SCHEDULING family** (with `6A674`): cc1 global allocation/scheduling choices natural C can't steer. Banked idioms: rotated loop = explicit first iteration + `while`; signed `char` vs `0xFF` emits the `andi`. Docs only, no carve |
| family diagnosis | 217 | Read-only investigation: **NO SINGLE KNOB**. All three residuals are in cc1 raw output (maspsx can't fix any). `55724` = pre-reorg emission order, no lever; `52BCC` = regclass+sched2 pair (`-fexpensive-optimizations`+`-fschedule-insns2`), `-O1` shows retail loop roles — retry at `-O1`; `6A674` = hardwired `optimize>1`, only lever `-O1` (untested). Toolchain-patch hypothesis closed; per-leaf `-O1` is the route |
| 5EW-52bcc-o1 | 218 | `func_80052BCC` MATCHED: era `-O1 -G0 -fschedule-insns2` (first sched2 leaf) + two-const-mode phrasing (u8 head const dies at guard → loop reload into `$v1`; `int` loop byte → raw `bne`); sched2 hoists head `li` into the `lbu` delay like ccpsx. All 15 words exact; mid-42FC8 carve (prefix 0x404, C 0x3C, resume 43408.s 0x2A8). Also fixed a latent pipefail/SIGPIPE flake in toolchain detection (`grep -q` → `grep … >/dev/null`) |
| 5EX-6a674-o1 | 219 | `func_8006A674` MATCHED after three parked attempts: era `-O1 -G0 -fschedule-insns2` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`, all 152 words + relocs exact with the 5EP semantic pins (load-bearing; dropping → 46 mismatches). `-O1` = per-use `-1` materialization; sched2 = `li`/`addiu`-before-store placement (21 fixes) — **sched2 confirmed as a general retail fingerprint**. Boot Rung 1 complete (`main → 6A64C ✓ → {6A8D4 ✓, 6A674 ✓}`); mid-55430 gap filled exactly (0x260), three contiguous C carves |
| 5EY-boot-3e610 | 220 | Boot display/graphics bring-up `func_8003E610` on era `-O2 -G0` — all 28 words exact on the **first** attempt, no sched2/pins: straight-line dispatcher, ten calls with immediate args (`0x140`/`0xE0` = 320x224), callees extern-declared with call-site-determined signatures. Readiness ranking of `main`'s callees (callee C/SDK coverage, not raw size) picked it; next up the chain: `func_8006A5BC`, `func_8003E680`. Mid-2E7D8 carve (prefix 0x638, C 0x70, resume 2EE80.s 0x192C) |
| 5EZ-boot-6a5bc | 221 | Boot init `func_8006A5BC` on era `-O1 -G0`, all 36 words exact first attempt: four setup calls, two identical `while (f() != 1) VSync(0);` loops (no cross-jump), `7F7A8()` return → `D_800B0DD4` (`unsigned short` via `lhu` reader). `-O1` per-use `$s0=1` materialization (delay-slot + between-loops re-materialization) — third `-O1`-lever leaf; return-use confirmed codegen-safe (raw `$v0` `beq`, no mask). Mid-55430 carve extends the boot block backward: **four contiguous C carves** (prefix 0x598C, C 0x90, then 6A64C/6A674/6A8D4) |
| 5FA-boot-3e680 | 222 | Boot subsystem-init dispatcher `func_8003E680` on era `-O1 -G0`, all 53 words exact: zero 5 globals (Stage-0 reader types), 2000-pass poll loop (`i++` in `jal` slot; `unsigned int` counter for ROM `sltiu` — the only phrasing fix), callback registration + ~11 inits. **First fn-ptr-to-asm-callee arg** (`&func_8003E91C` via R_MIPS_HI16/LO16 against a text symbol). `-O1` predicted by the per-use selection rule (five per-store `lui`s, no CSE). Fingerprint table banks: `-O1` selection rule, return-use readiness, sched2 scope narrowing, fn-ptr arg, unsigned loop compare. Segment-head carve of 2EE80 (C 0xD4, resume 2EF54.s 0x1858) |
| 5FB park | 222 | `func_800698D4` (disc mount, 141 words) PARKED-SCHEDULING at 140/141: nested-if phrasing defeats gcc's range-test collapse (`v!=0 && v!=-1` → `addiu`+`sltiu`+`bnez`), everything exact except one dbr_sched delay-slot steal at search #3's `beqz` — retail declines a `$v0`-setter steal when the branch target is the return block (`$v0` live to `jr`); ours steals. Screening rule + CdlFILE `0x18` frame note banked; candidate stashed; no carve, no leaf claim |
| 5FC park | 222 | `func_8006E834` (post-mount loader + display env, 91 words) PARKED-ALLOCATION at 89/91: five-arg call PROVEN (5th arg `sw $v0,0x10($sp)` in `jal` slot, first try); frame decomposition method banked; retail folds the range test in this unit (per-TU datapoint vs 698D4). Residual: call-result register home (`$v0`+restores vs `$v1`), not source-expressible. Third ccpsx-vs-2.7.2 skew mechanism recorded; candidate stashed; no carve, no claim |
| 5FD-table-2f9cc | 223 | Table clear `func_8002F9CC` (17 words) on era `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`: zero the in-use flag of all 7×220-byte records at `D_800A5D58` (record typed from the `func_8002F7D8` reader; extent `0x604` = 7×220). Key finding: aggregate element type is an addressing-mode lever — `arr[i].field = 0` keeps the symbolic indexed store; flat `arr[i*55] = 0` hoists `la` (flag-invariant). `unsigned char` counter (`andi 0xFF` masks), `sltiu` bound, stride 220B/55W (not 196B/49W). Mid-11718 carve (prefix 0xEAB4, C 0x44, resume 20210.s 0x4010) |
| 5FE-table-2f970 | 224 | Table twin `func_8002F970` (23 words) on era `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`: pointer-match search-and-clear over the 2F9CC table (`SlotRecord` typing inherited unchanged); `*p == D_800A5D58[i].body` → clear `inUse`, then `*p = 0` with the `sw` in the `jr` delay slot (5EN pattern). `$a3` body-base hoist = the aggregate lever producing (not preventing) a hoist; back-branch slot FILLED vs 2F9CC's nop — slot fill is per-shape, not per-table. One phrasing fix: operand order in the compare (`body == *p`) for `bne $v0,$v1`. Object-level `%lo` difference on the hoisted base (`D_800A5D58+4` vs `D_800A5D5C`) resolves to identical bytes at link. Contiguous carve with 2F9CC (prefix 0xEA58, C 0x5C, C 0x44, resume 20210.s) |
| 5FF-maspsx-loads | 224 | Toolchain patch `439c244`: `MASPSX_THREE_WORD_SYMBOL_STORE` extended from stores to standalone indexed symbolic LOADS (lb/lbu/lh/lhu/lw/lwl/lwr) under addiu_at — pass-through emits the ASPSX 2.30 three-word lui/addu/op-%lo form; compound lines retain legacy; `lwc2` stays outside (durable negative). Store path untouched; one gate, existing name. Full gate: flag-OFF 224 exact SHA; flag-ON 224 exact SHA (6A674/2F9CC/2F970 unchanged under the extended meaning); 153 vendored tests (was 148, +5 load); re-clone restores all three tracked files byte-identical. `func_800374E8` (which motivated the patch) PARKED — register-coloring residual (structure correct; see Known-open families + parked_blockers.json). 224 unchanged, no carve.
| 5FG-363f4 | 225 | Search-and-clear `func_800363F4` (21 words / 0x54 @ 0x26BF4): 16-entry D_800A7624 scan, clear key on match, break. era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 — FIRST leaf exercising the load gate; the probe EXPOSED the 439c244 bug (GNU as uses the DESTINATION reg as temp for lw; ROM uses $at), fixed at 5dac87e. 21/21 words; mid-2422C carve (prefix 0x29C8, C 0x54, resume 26C48.s 0xD5C); full 225 build EXACT SHA.
| 5FH-twin-37548 | 226 | Record-field lookup twin `func_80037548` (27 words / 0x6C @ 0x27D48): scan 4 x 56-byte D_800BCEA8 records for short needle at +0x10, return signed byte0 (+0x00) on match else 0. era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 (lh/lbu indexed pair through the 5dac87e $at gate). Twin-hypothesis FALSIFIED: matches 27/27, no coloring skew — live-value-pressure rule refined (see register-coloring-374e8 parked entry). Accumulator phrasing banked (fingerprint table). Mid-27C6C carve: prefix 0xDC, C 0x6C, resume 27DB4 (existing sibling); full 226 build EXACT SHA; packed-span byte-exact.

Detail and leaf-by-leaf narrative: git history + wiki
([Current Status](https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki/Current-Status)).
PC port remains out of scope. Redump.org cross-check still open (non-blocking).
