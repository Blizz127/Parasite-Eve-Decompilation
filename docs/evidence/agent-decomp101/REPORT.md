# agent-decomp101 — slice 2 (large functions, ranks 37–109) — REPORT

Worktree `/tmp/pe-agent-decomp101` · branch `agent/decomp101` · base `58ffdd4b`
Scope: `docs/generated/ASM_WORKLIST_SLICES.md` slice 2 (73 funcs / 102,248 bytes).
Time budget: 3 h. Stop rule: 8 leaves / 4 parks in a row / 3 h.

## 1. Baseline gate (reproduced before any edit)

`bash scripts/split_us.sh` then, inside `distrobox enter pe-mipsel`:

```
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (709 registered C leaves)
VERIFY_US=PASS
```

## 2. Final gate (fresh build + verify on the final commit)

```
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (710 registered C leaves)
VERIFY_US=PASS          (all 7 steps PASS, VERIFY_EXIT=0)
```

`scripts/verify_us.sh` step 3 requires the generated
`docs/generated/DISC1_MATCHING_STATUS.md` to be current, so the leaf commit also
carries `python3 tools/build/disc1_plan.py --write-status` output.

## 3. Newly matched leaves (1)

| leaf | VRAM | file | size | era / lever |
|---|---|---|---|---|
| `func_8008D140` | 0x8008D140 | 0x7D940 | 0x4D0 (308 words) | `-O2 -G0`; 32 explicit `if (!flags \|\| (flags & (1u<<i)))` statements unrolled by source, plus an explicit `unsigned int zero = (flags == 0);` local so cc1 materialises the zero test once (`sltiu $a2,$a1,1`) and reuses `bnez $a2` in all 32 blocks |

`func_8008D140` copies `p->vals[i]` (halfword at `arg0+4+2i`) to
`D_8009B3FC[0xE0+i]` for each dirty bit, and copies all 32 when the flag word
is zero. It was the only slice-2 candidate whose retail body is a fully
unrolled straight-line repetition; it hit `WORDS MATCH` on the second attempt
(the first used `flags == 0`, which made cc1 emit a per-statement
`beqz $a1` instead of the shared `$a2`).

## 4. Parked leaves (2-attempt rule)

### 4a. `func_80087AA8` — VRAM 0x80087AA8, file 0x782A8, size 0x4F8 (318 words)
Per-object timer/animation decay with ~13 `if (countdown) { countdown--; … }`
blocks, plus keyframe-list pointer advance (`p[0]==0 && p[1]==0` →
`p += p[2]*2`) and three `mult`/`mflo` fixed-point chains.
A full faithful transcription reached `candidate 1280` vs `retail 1272` bytes.
Divergences:

* **Prologue/block 1.** First transcription (function-scope `v0/v1/a0/a1/a2`
  locals): retail keeps the `f72` countdown in `$v0`, `f44` in `$v1`, `f48` in
  `$a0`; the candidate used `$a1` for the countdown and `$a0`/`$a2` for
  `f44`/`f48` (a consistent +1 register shift). Rewriting every block with
  block-local temporaries fixed this — words 0x00–0x6C then matched.
* **Block 2 (`f60`).** Same instructions and registers (`$v0` sum, `$v1`
  `fF4`, `$a0` `fD6`) but scheduled differently: retail `sh f60; lhu f5E;
  lhu fD6; lw fF4; addu; ori; sh f5E; sw fF4`; candidate hoisted the `fD6`
  and `fF4` loads above `sh f60` and the `addu`.
* **Block 4 (`f74`).** `register unsigned int x asm("$N")` pins (the lever
  used by `src/func_800293F4.c`) fixed blocks 1/3/5 to retail
  (`$v1`/`$a0`/`$a1`), but block 4 still colors the `f74` countdown into
  `$a1` where retail uses `$v0`, and reorders `lh fD8; lh fDA; sh f74; lw
  f38` into `lw f38; sh f74; lh fD8; lh fDA`.
* `-O2 -G0 -fschedule-insns2` and `-O1 -G0` were both tried; neither aligned
  block 2/4 scheduling.

### 4b. `func_800334AC` — VRAM 0x800334AC, file 0x23CAC, size 0x4F4 (317 words)
Nine unrolled 28-byte-record submit blocks (four 2-bit direction fields plus
five single-bit state masks), each writing X/Y halfwords at
`D_8009E968 + idx*364 + sel*28 + 8/+0xA` and calling
`func_80077AC4(D_800B0E38[idx] + 0x1C, rec - 8)`. Requires `-O2 -G8`
(`D_8009CE80` = `gp+0x110`) with `D_8009D278` declared as an unknown-size
array so it stays absolute (`lui/lw %lo`), matching retail.

Divergence (candidate 317 words, 253/317 words differ, all prologue and
block-scheduling):

* retail saves `$s1` and loads the actor base first:
  `sw s1,0x14; lui s1,%hi(D_8009D278); lw s1,%lo; lbu v1,0x110(gp); …;
  addiu s4,s1,0x4C (in the first beqz delay slot)`.
* candidate saves `$s3` first, loads the actor base into `$s3`, and does not
  materialise the `$s4 = base+0x4C` pointer at all.
* in block 1 retail keeps `sel` in `$a1` and the record base in `$a2`, and
  loads `D_8009E968` before `D_8009CDDC`; candidate keeps `sel` in `$v0` and
  loads `D_8009CDDC` first. A block-local `register int sel asm("$5")` pin did
  not change cc1's choice. `-O1 -G8` got closer (236/317) but not exact.

## 5. Survey / triage notes (why the rest was not attempted)

Of the 73 slice-2 functions, 23 contain GTE coprocessor ops (`mvmva`/`ctc2`/
`mtc2`/`mfc2`/`nclip`/`swc2`/`lwc2`). No matched leaf in `src/` contains GTE
code (`grep -rl` over `src/` finds only a comment in `func_80037548.c`), so
those were left alone. The remaining 50 are all ≥1044 bytes; the most linear
by `jal`/branch count (`func_80031760`, `func_80034104`, `func_800334AC`) were
inspected and are dominated by `volatile`-style repeated global reloads,
28-byte/364-byte index chains, or jump-table state machines. `func_8008D140`
was the only fully-unrolled straight-line body found.

## 6. Commits

```
2768c2fc→4c90986c  leaf func_8008D140 (src + YAML carve + regenerated status doc)
```
Range on `agent/decomp101`: `58ffdd4b..<docs commit>` (leaf commit
`4c90986c`, amended to include the regenerated status doc).
