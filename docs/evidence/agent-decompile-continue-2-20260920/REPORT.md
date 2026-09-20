# agent/decompile-continue-2 — 6 matching C leaves (649 -> 655)

Toolchain: era gcc-2.7.2-psx + maspsx, distrobox `pe-mipsel`.
Retail EXE: `build/extracted/disc1/SLUS_006.62`,
SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Build/verify commands (run inside the distrobox so `as`/`ld` are direct and the
build does not spawn one container per assembly call):

```text
distrobox enter pe-mipsel -- bash -lc 'cd <worktree> && bash scripts/build_us.sh'
distrobox enter pe-mipsel -- bash -lc 'cd <worktree> && bash scripts/verify_us.sh'
```

Result: `RESULT: EXACT MATCH`, `Matching claim: YES (655 registered C leaves)`;
`verify_us.sh` -> `VERIFY_US=PASS`, `all 655 packed C spans equal retail`.

## Matched leaves

| leaf | VRAM | file | size | intent / lever |
|---|---|---|---:|---|
| `func_800762A0` | 0x800762A0 | 0x66AA0 | 0x1C | GPU primitive packer `(a0 & 0x7FF) \| 0xE5000000 \| ((a1 & 0x7FF) << 11)`; explicit `hi`/`lo` temporaries force retail's a1-first order |
| `func_80076BE0` | 0x80076BE0 | 0x673E0 | 0x30 | tag the word through `D_80095854` with 0x10000000, return `*D_80095850 & 0x00FFFFFF` |
| `func_8008780C` | 0x8008780C | 0x7800C | 0x30 | SPU `0x1F801C08+(voice<<4)`: bit15 from **logical** `a2>>2` (unsigned param => `srl`, not `sra`), bits 8-14 from `a1`, low byte preserved |
| `func_8008783C` | 0x8008783C | 0x7803C | 0x28 | same register: replace the 0x00F0 field |
| `func_80087864` | 0x80087864 | 0x78064 | 0x28 | same register: replace the low 4 bits |
| `func_80038CE4` | 0x80038CE4 | 0x294E4 | 0x28 | double index into the byte table at `*D_80091A28` |

Two levers found this run:
1. **Non-volatile MMIO pointers.** With `volatile`, cc1 cannot move the store
   into the `jr $ra` delay slot and emits an extra word; the retail source used
   a plain pointer cast, so dropping `volatile` is what matches
   (`func_8008783C`/`func_80087864`).
2. **Unsigned shift operand.** `a2 >> 2` must be logical (`srl`); declaring the
   parameter `unsigned int` (or casting) turns `sra` into `srl`
   (`func_8008780C`, one word from matching before the fix).

Every leaf was triaged with
`python3 tools/analysis/try_leaf.py src/<name>.c <file> <size>` (`WORDS MATCH`)
and then confirmed by a full `build_us.sh` + `verify_us.sh`.

## Parked drafts (non-matching, `nonmatch/`)

Triaged under `-O2 -G0` (and per-leaf gates where noted); none is registered.

- `func_80076B20` (0x67320, 0x24): store through `D_80095854`, then
  `D_800A3348[value >> 24]`. The `sb` lands before `jr`; the three-word store
  gate gets within one word but does not fill the delay slot.
- `func_80087798` (0x77F98, 0x24): SPU voice pair. Candidate materializes
  `0x1F801C00` through `$at`; retail uses `lui`/`ori`/`addu`.
- `func_80018E84` (0x9684, 0x30): two double-indirect halfword stores; era
  reorders the loads/stores.
- `func_80071964` / `func_80071994` (0x62164 / 0x62194, 0x30): base `a0+8` plus
  optional `p[2]`; era folds the branch into a straight `addu` while retail
  keeps the two-path `addiu`/`j` shape.
- `func_80056C14` (0x47414, 0x2C): bounds-checked 32-byte-stride table read;
  branch polarity/layout differs.
- `func_8006599C` / `func_800659C8` (0x5619C / 0x561C8): `D_800B1624` record
  field read/store; retail reloads the pointer twice, era CSEs it.
- `func_8008F4E8` / `func_8008FBFC` (0x7FCE8 / 0x803FC, 0x2C): byte-cursor
  consume helpers; era reorders the load/store sequence.
- `func_8003E0A4` (0x2E8A4, 0x2C): mode/store helper; `$a3` vs retail `$v0`
  allocation.
- `func_80084F8C` (0x7578C, 0x2C): two-level flag test; era emits `sltu`.
- `func_800199CC` (0xA1CC, 0x2C): RMW of `+0x250`, store of `+0x24E`; store
  order differs.

## Tooling

`tools/analysis/asm_leaf_candidates.py` now also excludes GTE/cop2 stubs
(`lwc2`/`swc2`/`mtc2`/`ctc2`/`mfc2`/`cfc2`) in addition to nop-only labels and
BIOS `jr $t2` stubs, leaving 36 genuinely C-expressible call-free functions
<= 0x30 in the tree.
