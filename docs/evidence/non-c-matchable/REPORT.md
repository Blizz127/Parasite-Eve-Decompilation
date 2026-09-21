# Non-C-matchable split-asm spans — classification

This report separates the split asm under `asm/disc1/` into provably
non-C-matchable spans and real remaining asm, so the "remaining asm" count stops
overstating the work. Nothing is reclassified without positive per-span
instruction evidence, and nothing here changes the matching-C count or the
exact build: these spans stay in the YAML as `asm`.

Tool: `tools/analysis/nonmatchable_spans.py` (idempotent; prints counts,
`--list`, `--json`). Machine-readable inventory:
`docs/evidence/non-c-matchable/MANIFEST.json`.

## Resulting counts

| bucket | functions | words |
|---|---:|---:|
| matched C (YAML `c` spans) | **695** | 9091 (route 332 / 5842) |
| non-C-matchable, whole image | **184** | **20252** |
| — of which on the boot→Day-2 route | 76 | 5414 |
| — of which off-path (attic/overlays) | 108 | 14838 |
| alignment filler (not functions) | 33 spans | 33 |
| `gte-inline` (informational; still liftable) | 5 spans | 192 |
| **real remaining asm (route)** | **571** | **66437** |

Route totals from `tools/analysis/route_coverage.py` (plan
`8a90f1e110e7f9390d1ccc2d82712c4ab07c0848e6359166068e164b77649c46`): 979
functions on route, 332 matched C / 571 asm; 5842 matched C words / 66437 asm
words.

Class breakdown of the 184: `handwritten-gte-wrapper` 107 / 19958w,
`handwritten-jr-t2` 53 / 163w, `handwritten-cop` 22 / 123w,
`handwritten-syscall` 2 / 8w. The 33 alignment spans include the 4-byte pad at
`0x685C0` that the deep-size repair freed from the old `func_80077D30` span
(session cont. 19).

## Class 1 — `handwritten-jr-t2` (53 functions, 163 words)

Entire body is one or more repeated triples:

```
addiu $t2, $zero, 0xA0      ; 0xA0 = BIOS A0, 0xB0 = BIOS B0
jr    $t2
addiu $t1, $zero, 0x2F      ; delay slot carries the function index
```

Representative spans: `func_800719E4` (two triples, `0xB0`/`0x38` then
`0xA0`/`0x15`), `func_80071A04` (`0xA0`/`0x18`), `func_80071A54` (`0xA0`/`0x2F`),
`func_80073C5C` (`0xB0`/`0x3F`). All live in `asm/disc1/621E4.s`, `64B54.s`,
`6E538.s`, `6E6C0.s`, `75F44.s`.

**Why no C form reproduces it.** The natural C spelling is an indirect call:

```c
typedef void (*fn_t)(int);
void func_80071A54(void) { ((fn_t)0xA0)(0x2F); }
```

era `cc1 -O2 -G0` emits:

```
subu  $sp,$sp,24
sw    $31,16($sp)
li    $1,160
jal   $31,$1          ; jal through $at, callee in $1
li    $4,0x2f         ; argument in $a0
lw    $31,16($sp)
addu  $sp,$sp,24
j     $31
```

Retail has **no frame, no `$ra` save, no `lw`/`addu` epilogue**, uses `jr $t2`
(not `jal $31,$at`), keeps the function index in `$t2` and the argument in
`$t1`. This is an SDK/handwritten BIOS dispatcher; the `addiu $t1` delay-slot
argument and the frameless `jr` are not reachable from any C call shape. The
only way to emit it from C is inline `__asm__`, which the residual policy
forbids.

## Class 2 — `handwritten-syscall` (2 functions, 8 words)

`func_80072714` (`li $a0,1` / `syscall 0` / `jr $ra` / `nop`) and
`func_80072724` (`li $a0,2` / `syscall 0` / `jr $ra` / `nop`).

**Why no C form reproduces it.** C has no `syscall` operator. The only source
spelling is inline `__asm__`. Even ignoring the policy ban, era `cc1` renders
the return via `j $31` rather than the retail `jr $ra` + `nop`, so it cannot be
byte-identical.

## Class 3 — `alignment-filler` (31 spans, 31 words)

A lone `nop` between functions (the disassembler prints the preceding
`endlabel` then the `nop`). These are not functions at all — they are layout
padding required by the next function's alignment. They are counted separately
and are **not** part of the non-C-matchable function total.

## Class 4 — `handwritten-cop` (22 functions, 123 words)

Pure COP2/GTE primitives whose body is only COP ops plus `jr $ra`/`nop` (e.g.
`func_80078EB4`, `func_80078EE4`, `func_80078F4C`, `func_80079228`,
`func_80079024`). They are the classic Psy-Q `gte`/libgte internal helpers:

```
func_80078EB4:  lwc2 $0,0x0($a0) ; lwc2 $1,0x4($a0)          ; jr $ra ; nop
func_80078EE4:  lwc2 $0..$5 from $a0/$a1/$a2                 ; jr $ra ; nop
func_80078F4C:  mtc2 $a0,$16 ; $a1,$17 ; $a2,$18 ; $a3,$19   ; jr $ra ; nop
func_80079228:  mtc2 $a0,$30 ; nop ; nop ; mfc2 $v0,$31      ; jr $ra ; nop
func_80079024:  ctc2 $a0,$26                                 ; jr $ra ; nop
```

All are flagged `/* handwritten instruction */` by the disassembler itself
where a REGIMM/COP transfer is involved.

**Why no C form reproduces it.** C cannot name or type a COP2 register; there is
no GTE intrinsic in the era frontend and no `__asm__`-free spelling. Register
numbers ($16…$31, $26) and the exact COP op per argument are only expressible in
assembly.

## Class 5 — `handwritten-gte-wrapper` (107 functions, 19958 words)

The largest bucket: a span that either contains a **GTE command op**
(`rtps`/`rtpt`/`mvmva`/`nclip`/`avsz3`/`sqr`/`op`/`gpf`/…), or a contiguous run
(>=3) of raw GTE ops, or has no conditional branch and no call while still
containing a COP op. Members include:

- the whole `asm/disc1/68664.s` GTE library (`func_80078344` … `func_80079414`),
- the `asm/disc1/561C8.s` GTE wrappers (`func_80065E48`, `func_800661A4`,
  `func_800661CC`, `func_800665A0`, `func_8006698C`, …),
- the `asm/disc1/2A19C.s` / `2E8FC.s` model-transform families
  (`func_8003A088`, `func_8003A6A8`, `func_8003B144`, `func_8003B97C`,
  `func_8003AC90`, `func_8003C0B4`, `func_8003C2E0`, `func_8003E188`),
- the `asm/disc1/BF0F0.s` / `C5060.s` overlay GTE users that expand an `rtps`
  (four such spans were previously mis-bucketed as `gte-inline` — a single
  `rtps` is a literal Psy-Q `gte_rtps` macro, not an isolated register
  transfer),
- the `asm/disc1/B3390.s` overlay GTE users
  (`func_800C3324`, `func_800C42A4`, `func_800DE0A8`, …).

**Why no C form reproduces it.** A C front end cannot emit a COP2 op:
- the COP2 register transfers are all flagged `/* handwritten instruction */`
  by the disassembler — 1810 of the 2419 transfer ops in this bucket carry the
  flag (the remainder are the implied-register `swc2`/`lwc2` forms the
  disassembler does not annotate);
- the GTE command ops (`mvmva`, `rtps`, `rtpt`, `nclip`, `avsz3/4`, `sqr`, `op`,
  `gpf`/`gpl`, `intpl`, …) are Psy-Q/libgte macro spellings with no C operator;
- **0 of the 695 matched C leaves contains a single COP op**. Every genuinely
  C-derived leaf in this repo is COP-free, which is the empirical proof that
  `cc1` never synthesises one.

The surrounding integer code may still be liftable, so the honest end-state for
these spans is a C body plus an asm-side GTE primitive — or an ACCEPTED-RESIDUAL
disposition. Until then they are counted as non-C-matchable **as a span** rather
than inflating the "real remaining asm" column.

## `gte-inline` (5 spans, 192 words) — informational, NOT counted

A span containing only an isolated COP2 **register-transfer** pair (`mtc2` /
`mfc2` / `swc2` / `lwc2`, max contiguous run < 3, no GTE command op) inside
otherwise clean, branch/call-bearing integer code — i.e. the span is dominated
by ordinary liftable C and the transfer is a localised island:

`func_8003EAC8` (15w — `sw $a0,($sp)` / `mtc2 $a0,$30` / compare / `swc2 $31,($sp)`
/ reload / index a `D_800A76F0` table with `$a1`),
`func_800130B4` (77w),
`func_80077F7C` (32w — `D_800960AC` `$ra` save plus a COP0 `mfc0/mtc0 $12` +
five `ctc2` constant seeds),
`func_80078004` (33w — `mtc2 $a0,$30` / `mfc2 $v0,$31` then pure integer
table interpolation),
`func_80078094` (35w — same shape with two outputs).

These are excluded from the non-C total and reported for visibility. They are
the genuine GTE-adjacent frontier; `func_80077F7C`/`func_80078004`/
`func_80078094` additionally carry `/* handwritten instruction */` on their
integer ops (`sub`/`addi`), so the residual is the flag-plus-COP2 pattern being a
hand-written macro region rather than clean liftable C. `func_8003EAC8` is the
smallest and most clearly liftable.

## Deliberately not classified

- **`func_8001F814`** — already `NONMATCHING_C` (jump table in the `0x800`
  rodata pool, `configs/USA/disc1_nonmatching_sources.json` disposition
  `nonmatching-native-cut`). Distinct taxonomy; left alone.
- **The 24 ACCEPTED-RESIDUAL leaves** — compiler-decision residuals, not
  handwritten code. Distinct taxonomy; left alone.
- Large functions that merely *contain* a BIOS call (`func_8003BCE0`,
  `func_80067E1C`, `func_8007E334`, …) are **not** classified: their bodies are
  ordinary C-matchable code with a thunk call at the end.

## Manifest contract

`docs/evidence/non-c-matchable/MANIFEST.json` keys:

- `spans` — consumer (non-C) contract. Every key here except `alignment-filler`
  is a non-C-matchable **function**; `route_coverage.py` reads these names
  read-only and subtracts them from the asm column.
- `matchable_cop_inline` — retained for backwards compatibility; now carries the
  5 `gte-inline` spans (`class: "gte-inline"`). Consumers must keep treating it
  as informational, not as non-C.
- `counts` / `words` — per-class reporting totals.

The classification rule for a COP-bearing span is now: **any** GTE command op
(`rtps`/`mvmva`/`nclip`/…), **or** a contiguous COP run >= 3, **or** a body with
no conditional branch and no call, is `handwritten-gte-wrapper` (non-C). Only an
isolated COP2 register-transfer pair inside branch/call-bearing integer code is
`gte-inline` (informational).

## Reproduction

```
python3 tools/analysis/nonmatchable_spans.py            # counts
python3 tools/analysis/nonmatchable_spans.py --list     # per-span evidence
python3 tools/analysis/nonmatchable_spans.py --json
python3 tools/analysis/route_coverage.py --no-history    # three-bucket totals
```
