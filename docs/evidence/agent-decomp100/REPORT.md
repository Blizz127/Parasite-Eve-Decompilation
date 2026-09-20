# agent/decomp100 — slice-1 carve (largest non-matching functions)

Date: 2026-09-20
Branch: `agent/decomp100`, worktree `/tmp/pe-agent-decomp100`, base `ec097017`
Scope: worklist ranks 1–36 (parent "slice 1" — the 36 largest remaining
non-matching functions, 103,000 bytes).

## Outcome (honest)

**0 newly matched leaves. 1 deep park with a near-miss; 35 functions
assessed as structurally out of reach for the bounded run.**

The baseline gate passed exactly (below). The chosen target,
`func_80024250` (rank 30, 507 words — the only top-36 function that is a
jump-table switch with a complete hand-written `pc_port` semantic
translation), was reconstructed far enough to match the first 0x2F4 bytes
byte-for-byte (head + cases 0..6 + the switch dispatch), then diverged on a
compiler-scheduling choice in case 7 that could not be moved by any flag or
source-shape lever tried. It was parked rather than forced.

Stop reason: bounded-scope feasibility. This is **not** a claim of four
genuine two-attempt parks — the remaining 35 were triaged structurally (see
the table) because every one of them is 2–9× the largest C leaf this project
has ever matched (976 bytes / 244 words, `func_80012850`), and the
recurring blockers are quantified below.

## Baseline gate (reproduced before any change)

```
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp100 && bash scripts/build_us.sh'
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp100 && bash scripts/verify_us.sh'
```

```
  RESULT: EXACT MATCH
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (709 registered C leaves)
  PASS 1032 spans = 709 c + 321 asm + 2 rodata
  PASS 709 YAML C spans -> source -> object -> verify span
  PASS all 709 packed C spans equal retail
VERIFY_US=PASS
VERIFY_EXIT=0
```

No leaf was committed, so the tree at HEAD is still the baseline; the
final fresh build/verify on the final commit is recorded in
`docs/ai_context/ACTIVE_HANDOFF.md`.

## Tooling note: a try-lever gap

`tools/analysis/try_leaf.py` cannot exercise
`MASPSX_FORCE_ABSOLUTE_SYMBOLS` (it is applied in
`tools/build/disc1_build.py::compile_era`, not by maspsx). Any leaf that
needs a symbol forced absolute (the `D_8009D2E8`-style scalar) will show a
spurious diff under `try_leaf` while still matching in the full build. The
`func_80024250` work below reimplements the try loop locally with the force
step so the triage matches the build. Adding a `--force-absolute` flag to
`try_leaf.py` would close this gap.

## Slice-1 structural table

`GTE` = count of `/* handwritten instruction */` COP2 words in the function;
`port` = an in-tree `pc_port/…_port.c` semantic translation exists.
`file off` = offset of the function's first word in `asm/disc1/<unit>.s`.

| rank | function | bytes | words | jr | jal | br | GTE | port | file off |
|---:|---|---:|---:|---:|---:|---:|---|---|---|
| 1 | `func_8001D340` | 8596 | 2149 | 1 | 27 | 87 | 0 | yes | 0x0DB40 |
| 2 | `func_8002BC90` | 5472 | 1368 | 1 | 12 | 28 | 0 | - | 0x1C490 |
| 3 | `func_8002DC58` | 5208 | 1302 | 1 | 10 | 25 | 0 | yes | 0x1E458 |
| 4 | `func_800299CC` | 4300 | 1075 | 1 | 43 | 67 | 0 | yes | 0x1A1CC |
| 5 | `func_80037870` | 4256 | 1064 | 2 | 40 | 61 | 0 | yes | 0x28070 |
| 6 | `func_80041108` | 3864 | 966 | 2 | 47 | 93 | 0 | yes | 0x31908 |
| 7 | `func_8005A318` | 3300 | 825 | 1 | 20 | 107 | 0 | - | 0x4AB18 |
| 8 | `func_80030894` | 3152 | 788 | 1 | 42 | 11 | 0 | yes | 0x21094 |
| 9 | `func_800494AC` | 3100 | 775 | 1 | 156 | 54 | 0 | - | 0x39CAC |
| 10 | `func_8001B5FC` | 2920 | 730 | 1 | 10 | 65 | 0 | - | 0x0BDFC |
| 11 | `func_80015DAC` | 2916 | 729 | 2 | 56 | 26 | 0 | yes | 0x065AC |
| 12 | `func_80024A3C` | 2688 | 672 | 2 | 33 | 30 | 0 | - | 0x1523C |
| 13 | `func_8002D1F0` | 2664 | 666 | 1 | 4 | 17 | 0 | - | 0x1D9F0 |
| 14 | `func_8006062C` | 2584 | 646 | 1 | 15 | 30 | 0 | - | 0x50E2C |
| 15 | `func_80022394` | 2536 | 634 | 1 | 20 | 53 | 0 | yes | 0x12B94 |
| 16 | `func_800D3114` | 2536 | 634 | 1 | 10 | 27 | **57** | - | 0xC3914 |
| 17 | `func_80063E0C` | 2500 | 625 | 1 | 18 | 92 | 0 | yes | 0x5460C |
| 18 | `func_80036448` | 2432 | 608 | 1 | 6 | 79 | 0 | - | 0x26C48 |
| 19 | `func_800CAE0C` | 2372 | 593 | 1 | 16 | 0 | **108** | - | 0xBB60C |
| 20 | `func_8007C564` | 2332 | 583 | 1 | 10 | 43 | 0 | - | 0x6CD64 |
| 21 | `func_800C71E4` | 2300 | 575 | 1 | 3 | 20 | 0 | yes | 0xB79E4 |
| 22 | `func_8007041C` | 2292 | 573 | 1 | 3 | 36 | **39** | yes | 0x60C1C |
| 23 | `func_80071A84` | 2180 | 545 | 2 | 3 | 63 | 0 | yes | 0x62284 |
| 24 | `func_800562A4` | 2176 | 544 | 1 | 7 | 86 | 0 | - | 0x46AA4 |
| 25 | `func_8006B4F8` | 2160 | 540 | 1 | 18 | 54 | 0 | yes | 0x5BCF8 |
| 26 | `func_80027D14` | 2144 | 536 | 1 | 15 | 65 | 0 | yes | 0x18514 |
| 27 | `func_80032B0C` | 2128 | 532 | 1 | 2 | 11 | 0 | - | 0x2330C |
| 28 | `func_8001F9C4` | 2072 | 518 | 1 | 52 | 52 | 0 | yes | 0x101C4 |
| 29 | `func_8008E8D0` | 2048 | 512 | 1 | 7 | 49 | 0 | - | 0x7F0D0 |
| 30 | `func_80024250` | 2028 | 507 | 2 | 5 | 25 | 0 | yes | 0x14A50 |
| 31 | `func_800C3324` | 2016 | 504 | 1 | 3 | 8 | **35** | - | 0xB3B24 |
| 32 | `func_8003D050` | 2012 | 503 | 1 | 3 | 37 | 0 | yes | 0x2D850 |
| 33 | `func_8001AE40` | 1980 | 495 | 1 | 14 | 52 | **4** | yes | 0x0B640 |
| 34 | `func_800C3B04` | 1952 | 488 | 1 | 7 | 16 | **8** | yes | 0xB4304 |
| 35 | `func_80020288` | 1896 | 474 | 2 | 12 | 36 | 0 | - | 0x10A88 |
| 36 | `func_800D0728` | 1888 | 472 | 1 | 14 | 36 | **19** | - | 0xC0F28 |

## Park 1 — `func_80024250` (rank 30) — deep near-miss

VRAM `0x80024250`, file `0x14A50`, size `0x7EC` (507 words, 20-entry jump
table `jtbl_800107D4`). WIP source preserved at
`docs/evidence/agent-decomp100/func_80024250.wip.c`.

### What matched

With `-O2 -G8`, maspsx 2.30, `MASPSX_THREE_WORD_SYMBOL_STORE=1`,
`MASPSX_DISPATCH_FOLD=jtbl_800107D4`, and `MASPSX_FORCE_ABSOLUTE_SYMBOLS`
on `D_8009D1A0,D_8009D278,D_8009D254,D_8009D1AC,D_8009D2E8,D_80010760`,
the candidate reproduces the function **word-for-word from 0x14A50 through
0x14D44** (head: the 40-byte `lwl/lwr` table copy from `D_80010760`, the
`func_8006DDCC` call, the record relink, the `sltiu`/`jr` dispatch, and
cases 0–6 including the `0x55555556` divide-by-3 in case 5). That is the
first 0x2F4/0x7EC bytes.

Levers proven along the way (all useful for other leaves):
- `D_8009D1A0/D_8009D278/D_8009D254/D_8009D1AC/D_8009D2E8` are **absolute**
  (`lui`/`lw`); declaring them as arrays / stripping their `.extern` with
  `MASPSX_FORCE_ABSOLUTE_SYMBOLS` is required. Under plain `-G8` scalars
  GCC emits gp-relative and the head diverges immediately.
- `D_8009D1A8` and `D_8009D228` **are** gp-relative in retail
  (`sw $s2,0x438($gp)`, `sh $v0,0x4B8($gp)`; `_gp = 0x8009CD70`).
- `cost` must be pinned to `$16` (`register int cost asm("$16")`) to get
  retail's `s0 = cost / s1 = index` split; without the pin GCC swaps s0/s1
  and the whole function is shifted.
- The dispatch fold needs maspsx **2.30** (`addiu_at`): 2.21 expands the
  indexed `lw` to the 4-word `lui/addiu/addu/lw` form instead of retail's
  3-word `lui/addu/lw %lo`.

### Exact divergence (first differing word)

```
0x14D48 (rel 0x02F8):  retail 34A5FFFF  ori   $a1,$a1,0xFFFF   (finish m10)
                       cand   2404EFFF  li    $a0,-0x1001      (m0 too early)
```

Retail materialises the four 2-word masks in one run —
`a3=~0x8000`, `a1=~0x10000`, `t0=~0x20000`, `t1=~0x40000` — then
`li a0,~0x1000`, then loads `D_8009D1AC` and `*(int*)actor`. The
candidate's scheduler interleaves the single-instruction `li a0` between
`lui a1` and `ori a1`, and sinks `m20`/`m40` past the body load:
`m8, m10, m0, body, m20, m40`. The accumulator also lands in `$4` rather
than retail's `$3`.

That one-word schedule difference shifts/serialises the remaining ~146
differing words (all of case 7 onward). The function is 507 words; 146
words still differ.

### Levers tried (all failed to move 0x02F8)

- all 32 combinations of body-pin × acc-pin × `bit`-variable vs inline ×
  `m0` inline vs pinned × mask assign order (forward/reverse);
- `-O2`, `-O3`, `-O1`;
- `-fschedule-insns`, `-fschedule-insns2`, both;
- `-fno-schedule-insns2` (breaks the whole function), `-fno-delayed-branch`
  (breaks it), `-fno-strength-reduce`, `-fno-expensive-optimizations`
  (moves the constant order to retail but then loads `body` before
  `D_8009D1AC`);
- source re-orderings of the mask assignments, `acc = D1AC & ~0x1000`
  before/after `body = *(int*)actor`, and direct `*(int*)actor` access
  instead of the cached body pointer;
- `asm volatile("" ::: "memory")` scheduling barriers at every position
  around the mask block / `acc` init / body load (individually and in
  pairs), plus an explicit pinned `m0 = ~0x1000` variable. The best barrier
  placement gets the four big masks contiguous (first diff moves to 0x310)
  but then leaves either an extra `nop` in the `D_8009D1AC` load-delay slot
  or interleaves the `body` load between the `lui`/`lw` of `D_8009D1AC` —
  retail keeps `m0`, `D_8009D1AC`, `body` strictly ordered. Retaining a
  barrier costs a `nop`; removing it lets sched2 re-interleave.

Verdict: the remaining difference is a pure sched2/global-alloc choice in
GCC 2.7.2 that the source shape does not control. Parking rather than
forcing an unsound variant.

## Structural blockers (ranks 1–36)

**(a) GTE / COP2 functions — 7 of 36, currently unmatchable.**
Ranks 16, 19, 22, 31, 33, 34, 36 contain `ctc2`/`mtc2`/`mvmva`/`mfc2`
("handwritten instruction") words (4–108 each). The era `cc1` has **no**
GTE codegen — `strings cc1 | grep -i gte` is empty — and none of the 709
matched leaves contains a COP2 word. There is no C path to these bytes
until the harness supports inline asm or a GTE builtin, and per the project
rules a raw-asm leaf is not decompilation. `func_800CAE0C` (rank 19, 593
words, 0 branches) is the extreme case: 108 COP2 words.

**(b) Giant unrolled symbol+register field-store blocks.**
Ranks 1–5, 7–15, 18, 20, 24–27 are dominated by straight-line runs of
stores to a global struct array indexed by a runtime global (e.g.
`func_8002D1F0`: 666 words of
`sb const,%lo(D_800B00EC)($at)` with `$at = %hi + D_8009CDDC*36`;
`func_8002BC90`/`func_8002DC58`: 1300+ words of the same shape). The
compiler re-loads the index and re-derives the multiply for every store
because the store may alias the index global. Reconstructing the C means
reproducing hundreds of statements with retail's exact constant CSE and
register rotation; the project's largest-ever matched leaf is 244 words and
this class starts at 472. Not approachable statement-by-statement in one
bounded run.

**(c) Jump-table state machines with long case bodies.**
Ranks 6, 17, 21, 23, 28, 30, 32, 35 are `switch`/state dispatchers. The
dispatch machinery itself is already solved (`MASPSX_DISPATCH_FOLD` +
three-word gate + maspsx 2.30), but the case bodies are 20–70 instructions
each (e.g. the `func_80020288` table `jtbl_8001070C` has 16 cases whose
smallest body is 5 words and largest 70). `func_80024250` (rank 30) was the
best of this class and still hit the scheduler wall above.

## Recommendation to the parent

1. **Re-slice.** Slice 1 as given is 2–9× the project's demonstrated C-leaf
   ceiling; slice it into medium (150–500 word) functions so parallel
   agents can land leaves, and keep the >1500-word functions for a
   dedicated, longer-running effort.
2. **Unblock GTE first.** 7 of the 36 largest functions (≈15.9 KB) are
   unreachable without a GTE codegen story. Decide the policy
   (inline-asm leaf vs a documented `NONMATCHING` disposition) before
   ranking them.
3. **Close the `try_leaf` gap.** Add `--force-absolute SYM[,SYM…]` so
   scalar-absolute symbols can be triaged outside a full build; the
   `func_80024250` head only matched once that step was emulated locally.
4. `func_80024250` is the best candidate for a future focused attempt: the
   first 0x2F4 bytes already match and the only open problem is case-7
   scheduler shape. A person who can bisect the exact case-7 source
   formulation (or a GCC knob that pins the constant order) may land it
   quickly.
