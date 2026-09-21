# `func_8001A680` — body handler-slot activator (boot→Day 2 fan-in 27)

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. **`LINK_EXACT`**, 0 word mismatches at the retail VMA.

## Function hood and retail span

- File span `[0xAE80,0xAF84)` = `0x104` bytes = 65 words.
- VRAM span `[0x8001A680,0x8001A784)`.
- Carved out of the former `0xAB74` asm span; precedes `func_8001A784`
  (still asm) and follows the `0xAB74` block.
- Highest on-path fan-in of the coverage-map gap list (27 distinct callers in
  Tier A ∪ Tier B, `docs/ai_context/BOOT_TO_DAY2_COVERAGE.md` §4.2).

## Semantics (from retail bytes, `asm/disc1/AB74.s`)

`void func_8001A680(Body *body, unsigned int id)`:

- Handler lookup: `D_800B0E98[body->f0C].entries[(u16)id]` — a table of
  `0xC0`-byte rows indexed by the body's class byte at `+0x0C`, then a
  word-entry indexed by the low 16 bits of `id`.
- Stores: `body->f0E = id` (`sb $s2,0xE`), `body->f14 = 0`,
  `body->f18 = 0`, `body->f1B0 = handler`.
- `body->f98 &= ~0x200`.
- `body->f0F = handlerByte[2] - 1` (reloads `body->f1B0`; `addu $v0,$v0,-1`).
- If `body->f98 & 0x100000`, walk `D_8009D20C[0]` via `+4` and for each entry
  whose `+0x18C == body` and `f98 & 0x200000`, recurse with `id & 0xFFFF`.

## Levers

1. **`tbl = D_800B0E98;` local, then `tbl[body->classId].entries[(u16)id]`.**
   Writing the aggregate directly as `D_800B0E98[classId].entries[...]` makes
   cc1 hoist the `la D_800B0E98` base ahead of the class/index arithmetic and
   moves the `andi (u16)id`/`sll 2` up (8 mismatched words). The `tbl` local
   produces retail's exact order: `lbu classId` / `la base` / `sll/addu/sll`
   (class) / `andi` / `sll 2` (index) / `addu base` / `addu index` / `lw`.
2. **Handler byte must be re-read through `body->handler`**, not through the
   local `h`: `body->handlerCount = ((unsigned char *)body->handler)[2] - 1;`
   reproduces retail's `lw $a0,0x1B0($s1)` reload.
3. `D_8009D20C` is the same absolute `void *[]` head as in matched
   `func_80012774` / `func_800292EC`; the recursion mirrors the retail
   `jal` with `andi $a1,$s2,0xffff` in the delay slot.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/era_leaf_match.sh src/func_8001A680.c 0x8001A680 0x104 -O2 -G0
```

Result: `SIZE_MISMATCH C=0x110 ROM=0x104` (GNU as alignment pad),
`MISMATCHES=5`, first at `0x8001A6A4`. All five differing words carry a
relocation (`D_800B0E98` ×2, `D_8009D20C` ×2, `func_8001A680`).

Relocated symbols: `D_800B0E98`, `D_8009D20C`, `func_8001A680`.

Link-level proof (`tools/analysis/era_link_check.py`):

```text
linked .text 272 bytes, target 0x104, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- Source `src/func_8001A680.c`; YAML carve `- [0xAE80, c, func_8001A680]`
  + resume `- [0xAF84, asm]` (the former `0xAB74` asm span).
- Default `era_o2_g0`; no profile entry needed.
