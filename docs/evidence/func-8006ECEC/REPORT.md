# `func_8006ECEC` — boot CD-stream orchestrator

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. Linked at its retail VMA with its referenced symbols
defined, the object `.text` is **byte-identical** to retail (`LINK_EXACT`,
0 word mismatches).

## Function hood and retail span

- File span `[0x5F4EC,0x5F844)` = `0x358` bytes = 214 words.
- VRAM span `[0x8006ECEC,0x8006F044)`.
- Carved out of the former `0x5F4EC` asm span; immediately follows matched
  `func_8006EC84` (`0x8006EC84`) and immediately precedes `func_8006F044`
  (also matched now).
- The poll labels `8006ED88` / `EE5C` / `EEE8` / `EFB8` / `F0F0` / `F1A0`
  are internal basic blocks of this one function (the six `jal 0x800811E4`
  sites), not separate functions — matching this leaf collapses the
  cluster.

## Semantics (from retail bytes)

Four table-driven read stages over `D_80093168` offset/size pairs:

| stage | table | destination | B/C arena field |
|---|---|---|---|
| A | `D_80093168[0..1]` | `flags[0x57]` = `D_800B0E34` | — |
| B | `D_80093168[1..2]` | `flags[0x5B]` = `D_800B0E44` | when `D_800A77FC & 0x2000` |
| C | `D_80093168[2..3]` | `flags[0x5B]` = `D_800B0E44` | otherwise |
| D | `D_80093168[3..4]` | `D_80011614` (absolute) | base reloads `D_800B0DD8` |

Each stage: `func_8006E6D4(base + tbl[0], 0, dest, tbl[1] - tbl[0])`,
retrying while the result is `-1`; then poll `func_800811E4(buf)` — `-1`
restarts the stage, `0` advances, any other value retries the poll; when
the poll is `-1` or `0` it clears `D_800B0CD8 &= 0xFEFFBFFF`.

Between stages the loaded arena pointer is walked as an int offset table
calling `func_800718D0` (`base + *(int *)(base + (short)i*4)`): 3 entries
after stage A, `0x106` entries after B/C. Stage D ends with
`func_80074DC0(0)` and the `func_80072714` / `func_800726C4` /
`func_80072724` trio; returns 0.

```text
8006ecec  addiu sp,sp,-56 ; save s0..s3/ra
8006ecf4  s3 = *D_800B0DD8
8006ed0c  func_80073A44(0) ; 8006ed14 func_80074D28(0)
8006ed1c  func_8006CDA4(1, 0xCC, 0, *D_800B0E6C, 0x21, 1)
8006ed4c  s2 = &D_800B0CD8
   ... stage A read/poll; a 3-entry func_800718D0 walk
8006ee08  if (D_800A77FC & 0x2000) stage B else stage C
8006ef38  s1 = flags[0x5B]; 0x106-entry func_800718D0 walk
8006ef68  func_80074DC0(0); stage D (reloads D_800B0DD8)
8006f008  func_80072714/726C4/72724 ; return 0
```

## Levers

1. **One `$s2 = &D_800B0CD8` pointer for the +0x15C/+0x16C field loads**
   (retail `lw $a2,0x15C($s2)`), while the flag clear stays on the scalar
   global `D_800B0CD8` so cc1 emits its absolute
   `lui/lw` + `lui $at/sw` form. Writing the pointer inside the read loop
   makes cc1 re-materialize `la $a2,D_800B0E34` absolutely and mismatches.
2. **`(int)flags[0x57]` / `(int)flags[0x5B]` used as the `func_800718D0`
   walk base** reproduces the shared `lw $s1,0x15C/0x16C($s2)`.
3. **Stage D must reference `D_800B0DD8` directly** (retail reloads it);
   using the cached `base` local makes cc1 keep `$s3` and mismatches.
4. **Poll uses the proven `func_8006E834` `$v1`-backup / `$v0`-restore
   split** (pins + zero-code barriers), reproducing `move $v1,$v0` /
   `addiu $v0,$v1,1` and the delayed `move $v0,$v1`.
5. Arena-selection polarity `if ((unsigned)idx >= 0xB) <E8> else <E4>` and
   the `== 0`-guard form (as in the sibling reports).

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/era_leaf_match.sh src/func_8006ECEC.c 0x8006ECEC 0x358 -O2 -G0
```

Result: `SIZE_MISMATCH C=0x360 ROM=0x358` (GNU as alignment pad, trimmed by
`tools/trim_elf_section_pad.py`), `MISMATCHES=57`, first at `0x8006ECF4`.
Every one of the 57 differing words carries a relocation; after the
object's relocations are resolved none remain.

Relocated symbols: `D_800B0DD8`, `D_800B0E6C`, `D_800B0CD8`, `D_80093168`,
`D_800A77FC`, `D_80011614`, `func_80073A44`, `func_80074D28`,
`func_8006CDA4`, `func_8006E6D4`, `func_800811E4`, `func_800718D0`,
`func_80074DC0`, `func_80072714`, `func_800726C4`, `func_80072724`.

Link-level proof (assemble, link at `0x8006ECEC` with each referenced
symbol `--defsym`'d to its retail address, compare the linked `.text`
word-for-word with the ROM):

```text
linked .text 864 bytes, target 0x358, word mismatches=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006ECEC.c`; YAML carve
  `- [0x5F4EC, c, func_8006ECEC]` + resume `- [0x5F844, asm]` (that asm
  span was subsequently carved for `func_8006F044`).
- Build profile: default `era_o2_g0` (no assignment entry needed).
- `python3 tools/build/disc1_plan.py --check` → `862 spans (572 c, 288
  asm, 2 rodata)`, geometry `0x1EE000`.
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
