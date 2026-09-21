# Hand-written thunk scouting — negative result (do NOT reclassify without proof)

Status: reconnaissance only. **This does not change the matching-C count, the
non-C-matchable manifest, or the exact build.** It records why several small
unmatched `asm` spans resisted C matching, and why they are *not* added to
`docs/evidence/non-c-matchable/MANIFEST.json` on this evidence alone.

## Spans probed and why they failed

| span | size | retail signature | best C result |
|---|---:|---|---:|
| `func_800370BC` | 4w | `li $at,0x8000` / **`add`** `$v0,$a0,$at` / `sra $v0,$v0,16` | 2 mismatches |
| `func_800370CC` | 4w | same, `sra ... 8` | 2 mismatches |
| `func_8001A374` | 7w | `lw v0,0(a0)` / **`nop`** / `lw v0,0(v0)` | 4 mismatches |
| `func_8003708C` | 7w | `mult` / **`mflo`** / **`mfhi`** order | 5 mismatches |
| `func_80083578` | 10w | `lw v1,SYM` hoisted; branch delay carries the pointer **reload** | 1 mismatch |

## The two instruction classes that block them

1. **A bare `add` / `sub`.** C arithmetic always reaches the assembler as
   `addu`/`subu` (and `move` is `addu $d,$s,$zero`); `add`/`sub` trap on
   overflow and cc1 never emits them for integer arithmetic. Whole-image
   census: **42 `add` vs 5500 `addu`** — the bare `add`/`sub` forms are an
   sdcc/hand-written marker.

2. **`nop` in a load delay slot.** cc1 never emits a bare `nop` here; the
   assembler's reorder-mode `.$L` fixup normally resolves a load's delay slot
   into a real instruction, and where it cannot, the consumer chain is usually
   reformulated so the slot is fillable. `func_8001A374` has `lw; nop; lw` and
   cc1's schedule hoists/merges the second load into the slot instead.

Combined census over the 1386 unmatched route asm functions: **855** contain
one of these two markers (ranked by that heuristic). A third, softer pattern —
a branch delay slot that *reloads a global* rather than reusing a live register
(`func_80083578`) — accounts for further one-word residuals and is not always
hand-written (C register allocation can produce it), so it is not a marker on
its own.

## Why this is not a manifest update

`tools/analysis/nonmatchable_spans.py` deliberately requires **positive
per-span instruction evidence** for the recognised classes (`jr $t2` BIOS
trampolines, raw `syscall`, COP2 runs, lone-`nop` fillers) — see its docstring.
"cc1 does not emit this instruction" is a *heuristic*, not the enumerated
proof those classes carry, and adding a new class on it would let a genuine
matchable leaf be written off by pattern. The honest disposition for these
spans stays `asm` until either (a) a C formulation is found, or (b) each span's
non-matchability is proven individually and a new class is added with its own
evidence.

## Consequence for the route

Remaining route asm (534 functions / 65853 words) contains a large hand-written
population. Sizing it precisely needs a per-instruction classifier, not this
sample; any such tool should be added to
`tools/analysis/nonmatchable_spans.py` with its evidence, not invented here.
