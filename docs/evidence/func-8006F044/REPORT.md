# `func_8006F044` — two-stage stream loader + reset

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. **`LINK_EXACT`**, 0 word mismatches at the retail VMA.

## Function hood and retail span

- File span `[0x5F844,0x5FA24)` = `0x1E0` bytes = 120 words.
- VRAM span `[0x8006F044,0x8006F224)`.
- Carved out of the former `0x5F844` asm span; precedes matched
  `func_8006F224`.

## Semantics (from retail bytes)

1. Reset six status bytes to `-1`: `D_800B0DB2`..`D_800B0DB7` (retail
   `sb` order 0xB5, 0xB4, 0xB7, 0xB6, 0xB3, 0xB2).
2. `D_800B0CD8 &= ~0xF0`; call `func_80086FF8()`.
3. Stage 1: read the `D_8009315E` pair into `D_8001160C`, poll
   `func_800811E4`, then the `func_80072714`/`726C4`/`72724` trio.
4. Stage 2: read the `D_80093166` pair into `D_80011610`, poll, trio,
   return 0.

Both stages use `func_8006E6D4(D_800B0DD8 + tbl[0], 0, dest, tbl[1]-tbl[0])`
with `-1` read retry, and the poll loop restarting the stage on `-1`,
advancing on `0`, else retrying; the `-1`/`0` cases clear
`D_800B0CD8 &= 0xFEFFBFFF`. This is the `func_8006E834` shape with two
stages and a reset prologue.

## Levers

1. **The six `sb`-to-`-1` resets must be written as `signed char` globals.**
   They are `sb $at`/`sb` byte stores in retail (not word stores); typing
   them as `signed char` selects `sb`, and the sink order is the source
   order 0xB2..0xB7 (cc1 emits in reverse birth order, which is why the
   retail store order is 0xB5,0xB4,0xB7,0xB6,0xB3,0xB2 for a top-down
   `0xB2..0xB7` source — preserved verbatim).
2. **`d += 0` prologue form** for the scalar/pointer mix — `D_800B0DB2`
   ..`DB7` are declared `signed char` (byte) while `D_800B0CD8` is
   `unsigned int`.
3. **Reuse of the sibling `func_8006E834` body shape verbatim**: identical
   read macro (`func_8006E6D4`), identical poll with the `$v1`-backup /
   `$v0`-restore register pins, identical `D_800B0CD8 &= 0xFEFFBFFF` on
   `-1`/`0`, and the trailing `func_80072714`/`726C4`/`72724` trio.
4. `func_80086FF8` takes no arguments and returns void.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/era_leaf_match.sh src/func_8006F044.c 0x8006F044 0x1E0 -O2 -G0
```

Result: `ROM .text 480 bytes  C .text 480 bytes` (size exact),
`MISMATCHES=49`, first at `0x8006F04C`. Every one of the 49 differing
words is a relocation field (its bytes depend on the linked address of an
external symbol); none are instruction-selection or operand differences.

Relocated symbols: `D_800B0DB2`..`D_800B0DB7`, `D_800B0CD8`,
`D_800B0DD8`, `D_8001160C`, `D_80011610`, `D_8009315E`, `D_80093166`,
`func_8006E6D4`, `func_800811E4`, `func_80086FF8`, `func_80072714`,
`func_800726C4`, `func_80072724`.

Link-level proof:

```text
linked .text 480 bytes, target 0x1E0, word mismatches=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006F044.c`; YAML carve `- [0x5F844, c,
  func_8006F044]` + resume `- [0x5FA24, asm]`.
- Build profile: default `era_o2_g0` (no assignment entry needed).
- `python3 tools/build/disc1_plan.py --check` → `862 spans (573 c, 288
  asm, 2 rodata)`, geometry `0x1EE000`.
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
