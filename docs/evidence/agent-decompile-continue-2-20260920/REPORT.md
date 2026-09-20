# agent/decompile-continue-2 — 4 matching C leaves (649 -> 653)

Toolchain: era gcc-2.7.2-psx + maspsx, distrobox `pe-mipsel`.
Retail EXE: `build/extracted/disc1/SLUS_006.62`,
SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Command (run inside the distrobox to avoid one container spawn per `as` call):

```text
distrobox enter pe-mipsel -- bash -lc 'cd <worktree> && bash scripts/build_us.sh'
```

Result: `RESULT: EXACT MATCH`, `Matching claim: YES (653 registered C leaves)`.

## Matched leaves

| leaf | VRAM | file | size | era flags | intent |
|---|---|---|---:|---|---|
| `func_800762A0` | 0x800762A0 | 0x66AA0 | 0x1C | -O2 -G0 | GPU primitive packer: `(a0 & 0x7FF) \| 0xE5000000 \| ((a1 & 0x7FF) << 11)`; explicit `hi`/`lo` temporaries force retail's a1-first order |
| `func_80076BE0` | 0x80076BE0 | 0x673E0 | 0x30 | -O2 -G0 | tag the word through `D_80095854` with 0x10000000, then return `*D_80095850 & 0x00FFFFFF` |
| `func_8008783C` | 0x8008783C | 0x7803C | 0x28 | -O2 -G0 | per-voice SPU register `0x1F801C08 + (voice<<4)`: replace the 0x00F0 field |
| `func_80087864` | 0x80087864 | 0x78064 | 0x28 | -O2 -G0 | same register: replace the low 4 bits |

`func_8008783C`/`func_80087864` use a **non-volatile** pointer: with
`volatile` the store could not be moved into the `jr $ra` delay slot and the
candidate emitted an extra word. Dropping `volatile` (matching the retail
source's plain pointer cast to an MMIO address) let cc1 fill the delay slot.

Each leaf was first triaged with
`python3 tools/analysis/try_leaf.py src/<name>.c <file> <size>` inside the
distrobox (`WORDS MATCH`), then confirmed by the full `build_us.sh`.

## Parked drafts (non-matching, `nonmatch/`)

Triage only under `-O2 -G0` and the per-leaf gates tried:

- `func_80076B20` (0x67320, 0x24): store `value` through `D_80095854`, then
  `D_800A3348[value >> 24] = value`. First mismatch: candidate puts the `sb`
  before `jr` (`0x20: retail A0243348 cand 03E00008`); the 3-word store gate
  gets within 1 word but the delay-slot store is still not filled. Parked.
- `func_80087798` (0x77F98, 0x24): SPU voice pair. Candidate materializes
  `0x1F801C00` through `$at` (addressing form) instead of retail's `lui $v0` /
  `ori $v0` / `addu`. First mismatch `0x000C`. Parked.
- `func_8008780C` (0x7800C, 0x30): SPU voice register bitfield (bit15 from
  `a2>>2`, bits 8-14 from `a1`, low byte preserved). First mismatch `0x0010`
  (`srl` shift amount/order). Parked.
- `func_80018E84` (0x9684, 0x30): two double-indirect halfword stores; era
  reorders the loads/stores. First mismatch `0x000C`. Parked.
- `func_80071964` / `func_80071994` (0x62164 / 0x62194, 0x30): pointer helpers
  with a base `a0+8` plus optional `p[2]`. Era folds the branch into a straight
  `addu`; retail keeps the two-path `addiu`/`j` shape. First mismatch
  `0x0014`. Parked.

Tooling note: `tools/analysis/asm_leaf_candidates.py` now also excludes
GTE/cop2 stubs (`lwc2`/`swc2`/`mtc2`/`ctc2`/`mfc2`/`cfc2`), which plus the
existing nop/BIOS filters leaves 36 genuinely C-expressible call-free
functions <= 0x30 in the current tree.
