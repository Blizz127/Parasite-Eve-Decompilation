# Exact packed rebuild IN PLACE on the live tree

Status (2026-09-11): **`scripts/split_us.sh && scripts/build_us.sh &&
scripts/verify_us.sh` produce the retail packed SHA-1 against the live working
tree** (not a frozen snapshot), with the three previously-sibling-owned leaf
corrections applied in `src/`.

```
orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
RESULT: EXACT MATCH

VERIFY_US=PASS
matching-C count: 578 (from YAML)
```

Build-environment facts (rootless `tools/mipsel-host/` toolchain, toolchain
resolution order, package pins) live in `docs/ai_context/TOOLCHAIN_REBUILD.md`.
This report owns only the three leaf fixes and the in-place proof.

## The three leaf fixes

All three were exposed the first time the full link ran (object-level matches
that did not survive relocation/scheduling resolution). Each fix is an
address/lifetime correction only — no behavioral change.

### 1. `func_80086FF8` / `func_80087024` — wrong global address

Retail stores to `0x800BCD80`; the source named the symbol `D_800CCD80`, so the
linker's name-derived absolute `D_<addr>` fallback gave `0x800CCD80` — exactly
`0x10000` too high.

Retail words (`func_80086FF8`, file 0x777F8):

```text
80087004  3c01800c  lui   at,0x800C
80087008  ac22cd80  sw    v0,-0x3280(at)   ; 0x800C0000 - 0x3280 = 0x800BCD80
```

Fix (both files):

| | before | after |
|---|---|---|
| declaration | `extern int D_800CCD80;` | `extern int D_800BCD80;` |
| body | `D_800CCD80 = 0xF0;` / `= 0xF1;` | `D_800BCD80 = 0xF0;` / `= 0xF1;` |

The address is now derived from the correct name; object-level output is
unchanged (HI16/LO16 relocations vs retail, same as the original match), and
the linked value is `0x800BCD80`.

No code reference to `D_800CCD80` remains. Stale *comment-only* references
survive in sibling-owned or historical docs and were deliberately not edited
(this lane does not own them): `configs/USA/disc1.yaml` line 1399 (sibling-owned
carve comment), `docs/evidence/func-80086FF8/REPORT.md` (historical match
record), and `docs/ai_context/TOOLCHAIN_REBUILD.md` (describes the bug). The
comment header in both source files was updated with the rename.

### 2. `func_8006F2C4` — hoisted `&D_800B0CD8` lifetime

`flagsPtr` was initialized at its declaration (before the `D_800E10A0` clear
loop), so cc1 hoisted the address materialization into a caller-saved register
ahead of the loop. Retail materializes the `0xFFFEFFFF` mask and then
`&D_800B0CD8` **after** the loop terminates.

Retail words:

```text
8006f338  2404006c  li    a0,0x6c
8006f33c  3c03800e  lui   v1,0x800E
8006f340  246310a0  addiu v1,v1,0x10A0     ; &D_800E10A0
8006f344  ac600000  sw    zero,0(v1)       ; loop body
...
8006f350  1440fffc  bnez  v0,0x8006F344    ; loop back-branch
8006f354  24630004  addiu v1,v1,4          ; delay slot (loop)
8006f358  3c04fffe  lui   a0,0xFFFE        ; mask materialized AFTER loop
8006f35c  3c03800b  lui   v1,0x800B        ; &D_800B0CD8 AFTER loop
8006f360  24630cd8  addiu v1,v1,0xCD8
8006f364  8c620000  lw    v0,0(v1)
8006f368  3484ffff  ori   a0,a0,0xFFFF
8006f36c  00441024  and   v0,v0,a0
```

Fix:

```c
        unsigned int i;
        unsigned int *flagsPtr;

        for (i = 0x6C; i < 0x73; i++)
            D_800E10A0[i - 0x6C] = 0;
        flagsPtr = &D_800B0CD8;
        *flagsPtr &= 0xFFFEFFFF;
```

Object-level output is byte-identical to the pre-fix object (the pre-fix
version also emitted `addiu`/`lui %hi`/`lui %hi` after the loop at the
instruction level; the difference only appears once the relocation emits the
`addiu` for `lui at` and the scheduling/resolution settles). The linked
`.text` now matches retail word-for-word.

## Per-leaf object checks (post-fix)

```
AS=tools/mipsel-host/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/bin/mipsel-linux-gnu-objcopy \
  tools/analysis/era_leaf_match.sh <src> <vram> <size> -O2 -G0
```

| leaf | result |
|---|---|
| `src/func_80086FF8.c` | `MISMATCHES=3` — all HI16/LO16 + `jal` link-time fields; size 0x30 vs 0x2C |
| `src/func_80087024.c` | `MISMATCHES=3` — same, only the `li` immediate differs (0xF0 vs 0xF1) |
| `src/func_8006F2C4.c` | `MISMATCHES=10` — the documented relocation set (26-bit jumps + HI16/LO16); size 0xE0 vs 0xD8 |

Every mismatch is a link-time relocation field, i.e. object-level matching is
preserved and the in-place link resolves them exactly.

## In-place exact run

Commands, from the repo root, in order:

```
scripts/split_us.sh      # exit 0; 287 asm + 577 c + 2 rodata
scripts/build_us.sh      # exit 0; RESULT: EXACT MATCH
scripts/verify_us.sh     # exit 0; VERIFY_US=PASS
```

The first in-place build ran on the **577-leaf** plan (current at build start);
the sibling matching worker added a 578th leaf while it ran, so the first
`verify_us.sh` was executed against the 578-leaf plan. `exact_checks` only
requires that the candidate hash equals the plan's `expected_sha1` and that
each packed C span equals retail — the candidate from the 577-leaf build still
satisfied that at 578 leaves. To remove any ambiguity, the entire
**split → build → verify** cycle was repeated with the YAML hash pinned across
all three steps:

| step | result | YAML SHA-256 |
|---|---|---|
| `split_us.sh` | exit 0, 867 spans (578 c, 287 asm, 2 rodata) | `eecb10939ff5b7183c1c9ebcb1e8a8c751966af6bddf906fc98699634c50d87f` |
| `build_us.sh` | exit 0, `RESULT: EXACT MATCH` | `eecb10939ff5b7183c1c9ebcb1e8a8c751966af6bddf906fc98699634c50d87f` |
| `verify_us.sh` | exit 0, `VERIFY_US=PASS` | `eecb10939ff5b7183c1c9ebcb1e8a8c751966af6bddf906fc98699634c50d87f` |

State at the moment of the self-consistent exact run:

- YAML SHA-256 `eecb10939ff5b7183c1c9ebcb1e8a8c751966af6bddf906fc98699634c50d87f`
- plan SHA-256 `496c44e653238c99eb9c9057483d3b92aa2990774639d9d245c0c6f5d427fea7`
- 867 spans = 578 c + 287 asm + 2 rodata, geometry `0x1EE000`
- `orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `RESULT: EXACT MATCH`

### Race attempts

1. Cycle 1 (`split` on 577, `build` on 577): `build` reported `RESULT: EXACT
   MATCH`; the YAML then advanced to 578 (`eecb1093…`, plan `496c44e65323`)
   during the build window, so `verify` ran against the newer plan and passed
   (the 577-leaf candidate is byte-identical to retail, hence trivially still
   exact at 578). No mismatch was induced by a file I do not own, so no sibling
   file was touched.
2. Cycle 2 (`split` → `build` → `verify`): the YAML hash was stable at
   `eecb1093…` for the whole 317 s cycle; plan `496c44e65323`; `RESULT: EXACT
   MATCH`; `VERIFY_US=PASS`. This is the clean, fully self-consistent in-place
   result.

## Gates

| gate | result |
|---|---|
| `python3 tools/build/test_disc1_plan.py` | 8 tests OK |
| `python3 tools/build/disc1_plan.py --check` | `867 spans (578 c, 287 asm, 2 rodata), geometry=0x1EE000, plan=496c44e65323` |
| `scripts/verify_us.sh` (full) | `VERIFY_US=PASS`, 7/7 gates, 578 packed C spans equal retail |
| `scripts/verify_us.sh --public` | `PUBLIC_VERIFY=PASS`, plan SHA-256 `496c44e65323…` |
| `scripts/setup_mipsel_host.sh` re-run | `OK mipsel-host toolchain already installed` (no-op, exit 0) |
| `cmake -S pc_port -B pc_port/build && ./pc_port/build/pe-native-tests` | 1347 run, 1346 passed, 1 failed (`DAY2_movie_autonomous`) |

Native-suite note: `pc_port/` is owned by a sibling worker and was modified
concurrently (`pc_port/tests/test_movie_autonomous.h` is new/untracked;
`movie_overlay_port.c`, `cd_stream_port.c`, `pe_cdreg.c`, `test_movie_player.h`,
`test_native.c` are all dirty). `cmake` resolves only via
`/tmp/pe-tools/env.sh` (`PE_TOOLS=/tmp/pe-tools/root`); without it `cmake` is
not on `PATH`. The sole failure is
`DAY2_movie_autonomous... FAIL: autonomous disc stream stopped` — in the
sibling-owned movie/cd-stream port, **not** touched by this lane and unrelated
to the three `src/` fixes or the mipsel `build_us.sh` path. Not investigated or
edited here (constraint: do not touch `pc_port/`).

## What is proven / what is not

Proven: the packed USA disc 1 executable `SLUS_006.62` round-trips byte-exact
through `split_us.sh` → `build_us.sh` in place, from the committed YAML + `src/`
tree, on the rootless `tools/mipsel-host/` toolchain (GCC 14.2.0 / binutils
2.44). Every one of the 578 registered C spans and 287 asm spans equals retail;
the three previously link-only mismatches are resolved.

Sibling drift after the proof: the matching worker continued adding leaves, so
by the end of this lane the YAML was at 579 c / 288 asm (SHA-256
`91c75f26…`, plan `bae734bb73c1`) — i.e. the exact-run state above (578 c,
`eecb1093…`) is already one leaf behind the live tree. The in-place proof is
recorded against that specific frozen state; the candidate it produced
(`build/disc1.candidate.exe`) still carries the retail SHA-1.

Not proven / not complete: this is a **recompilation-exactness** result for the
covered spans, not a 1:1 decompile of the full boot→end-of-day-2 span. 289
file spans remain asm (`287 asm + 2 rodata`), i.e. not yet lifted to matching C,
and the `pc_port/` native port is incomplete wherever its evidence says so.
The persistent goal (100% 1:1 retail-accurate decompile + port from boot through
end of day 2) is therefore **not** complete — the rebuild being exact is a
necessary, not sufficient, milestone.
