# `func_8006F39C` — command-record allocator + dispatch

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. **`LINK_EXACT`**, 0 word mismatches at the retail VMA.

## Function hood and retail span

- File span `[0x5FB9C,0x5FED4)` = `0x338` bytes = 206 words.
- VRAM span `[0x8006F39C,0x8006F6D4)`.
- Carved out of the former `0x5FB9C` asm span; sits between matched
  `func_8006F2C4` and `func_8006F6D4`.

## Semantics (from retail bytes)

`int func_8006F39C(int id, int arg)`:

1. `if ((unsigned)id >= 0xC0) return -7;`
2. **Lazy overlay prelude** — only when `0x6C <= id <= 0x72` and
   `D_800B0CD8` bit `0x10000` is clear: read the `D_80093162` pair into
   `D_80011618` (`func_8006E6A8`, retry on `-1`), poll `func_8006E7E8`
   (`-1` restarts, `0` advances, else retry), the `func_80072714`/`726C4`/
   `72724` trio, then install the seven overlay handler descriptors
   `D_801F1BD8`, `D_801F1C58`, `D_801F1D00`, `D_801F1D8C`, `D_801F1E18`,
   `D_801F1EA4`, `D_801F1EF0` into `D_800E10A0[0..6]` and set
   `D_800B0CD8 |= 0x10000`.
3. `func_8006914C(0)`.
4. Clamp the handler id: `hbyte = id; if ((unsigned)id >= 0x55) {
   slot = id - 0x55; id = 0x55; }`.
5. `if (D_800942E0[id] == 0) return -8;` and
   `if (*(int *)((char *)D_800942E0[id] + 4) == 0) return -1;`
6. **Inlined free-slot search** (the `func_8006F224` body): `return -3`
   when full; otherwise arena selection
   `if ((unsigned)s1 >= 0xB) D_800942E8 + (s1-0xB)*0x10C else
   D_800942E4 + s1*0xA0C`.
7. Format the record: `p[0]=1`, `p[1]=hbyte` (the original, unclamped id),
   `p[2]=p[3]=0`, `*(int*)(p+4)=0`, `*(int*)(p+8)=arg`.
8. `if (id == 0x55) func_800CE49C(p, slot);`
9. Dispatch: `(*(void (**)(void *))((char *)D_800942E0[id] + 4))(p)`,
   return the slot index.

This matches the semantic note in `docs/evidence/pe-btl115-d25c-4-8/REPORT.md`
(overlay clear bit `0x10000`, `D_800E1044[0x17..]` = `D_800E10A0[0..6]`,
`D4698` jalrs `*(rec+0x30)`).

## Levers

1. **`D_800942E0` is `extern void **` (pointer-to-handler-pointer-table),
   NOT `void *[]`.** Retail loads the pointer value first
   (`lui $v1,%hi(D_800942E0)` / `lw $v1,%lo($v1)` / `sll $v0,$s2,2` /
   `addu` / `lw`). Declaring it as an array (`void *D_800942E0[]`) makes
   cc1 take its *address* and emit `addiu at,at,%lo(D_800942E0)` instead —
   three mismatched words at each of the two lookups (0x8006F514 and
   0x8006F684). This is the durable dict from the sibling reports.
2. **Overlay handler addresses are referenced as data symbols** (`extern
   unsigned char D_801F1BD8[]` → `(unsigned int)D_801F1BD8`), not as
   `0x801F1BD8` integer constants: retail emits `lui $v0,%hi(...)` /
   `addiu $v0,$v0,%lo(...)` with `R_MIPS_HI16/LO16` against the symbol.
   These symbols are defined by the surrounding split asm, so the link
   resolves them exactly.
3. **`&D_800B0CD8` via a pointer local for the final OR.** A bare
   `D_800B0CD8 |= 0x10000;` makes cc1 emit absolute `lui/lw` + `lui
   $at/sw`, but retail uses one `a0 = &D_800B0CD8` for both the `lw` and
   the `sw`. Retail materializes that `a0` *between* the 6th and 7th
   `D_800E10A0` stores; placing `flagsPtr = &D_800B0CD8;` on the source
   line between those two stores reproduces the schedule.
4. **`hbyte` must be captured before the clamp** and stored to `p[1]`;
   `id` itself is clamped to `0x55` (so `p[1]` and the range checks see
   the original id via the saved byte).
5. **`int slot = 0;` initialized at declaration.** Assigning `0` on the
   `func_8006914C` path birth-orderizes `$s0` differently and mismatches;
   the initializer reproduces retail's `addu $s0,$zero,$zero`.
6. **`int D_800B0DD8` (not a pointer)** for the `func_8006E6A8`
   base+offset argument (`addiu $a0,$s1,0x...`), removing the
   pointer-from-integer warning.
7. `func_8006E7E8` poll is written in the plain retail order
   (`-1` first, then `0`), unlike the `$v1`-backup family — the stage-0
   poll block layout in this leaf does not need the register pins.
8. Arena-selection polarity `>= 0xB → E8` (with `- 0xB` subtract) and the
   `== 0` guard form for the `D_800942E0` tests, as in the siblings.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/era_leaf_match.sh src/func_8006F39C.c 0x8006F39C 0x338 -O2 -G0
```

Result: `SIZE_MISMATCH C=0x340 ROM=0x338` (GNU as alignment pad),
`MISMATCHES=67`, first at `0x8006F3CC`. All 67 differing words carry
relocations; after the object's relocations are resolved none remain
(`uncovered=0`).

Relocated symbols: `D_800B0DD8`, `D_800B0CD8`, `D_80011618`,
`D_80093162`, `D_800E10A0`, `D_800942E4`, `D_800942E8`, `D_800942E0`,
`D_801F1BD8`, `D_801F1C58`, `D_801F1D00`, `D_801F1D8C`, `D_801F1E18`,
`D_801F1EA4`, `D_801F1EF0`, `func_8006E6A8`, `func_8006E7E8`,
`func_8006914C`, `func_80072714`, `func_800726C4`, `func_80072724`,
`func_800CE49C`.

Link-level proof:

```text
linked .text 832 bytes, target 0x338, word mismatches=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006F39C.c`; YAML carve `- [0x5FB9C, c,
  func_8006F39C]` replacing the former `- [0x5FB9C, asm]` (the asm resume
  was already `func_8006F6D4` at `0x5FED4`).
- Build profile: default `era_o2_g0` (no assignment entry needed).
- `python3 tools/build/disc1_plan.py --check` → `862 spans (574 c, 286
  asm, 2 rodata)`, geometry `0x1EE000`.
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
- `scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`, count 574.
