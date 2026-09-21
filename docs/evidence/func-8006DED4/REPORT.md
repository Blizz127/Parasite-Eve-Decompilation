# `func_8006DED4` — spline/packet builder (CD/boot cluster)

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. **`LINK_EXACT`**, 0 word mismatches at the retail VMA.

## Function hood and retail span

- File span `[0x5E6D4,0x5E750)` = `0x7C` bytes = 31 words.
- VRAM span `[0x8006DED4,0x8006DF50)`.
- Carved out of the former `0x5E39C` asm span; sits between
  `func_8006DE80` (asm) and the newly matched `func_8006DF50`.

## Semantics (from retail bytes, `asm/disc1/5E39C.s`)

`int func_8006DED4(int a0, int a1, int a2, int a3, unsigned short a4,
unsigned short a5)`:

- Packs three shorts into a stack buffer at `sp+0x18/0x1A/0x1C` (from `a3` and
  the zero-extended `a4`/`a5`, retail `lhu 0x48/0x4C(sp)`).
- `func_8006DFA8(sp+0x18, sp+0x20, sp+0x24)` fills two words.
- Returns `func_8006DF50(a0, a1, a2, word[sp+0x20], word[sp+0x24])` — the
  second forwarded value is a **word** load (`lw`), not a sign-extended
  halfword.

## Levers

1. **`a4`/`a5` are `unsigned short` parameters**, not `int`. Retail loads
   them as `lhu 0x48(sp)` / `lhu 0x4C(sp)`; `int` parameters give
   `lw`/`sll`/`sra`. Only the 6th/7th arguments (both on the stack) need this.
2. **The `func_8006DF50` last argument is the whole word** (`v1`), not
   `(short)v1`: retail `lw $a3,0x20(sp)` + `sw $v0,0x10(sp)`, with no `lh`
   on the second value.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/era_leaf_match.sh src/func_8006DED4.c 0x8006DED4 0x7C -O2 -G0
```

Result: `SIZE_MISMATCH C=0x80 ROM=0x7C` (GNU as pad), `MISMATCHES=2`, first at
`0x8006DF10`. Both differing words carry a relocation
(`func_8006DFA8`, `func_8006DF50`).

Link-level proof:

```text
linked .text 128 bytes, target 0x7c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- Source `src/func_8006DED4.c`; YAML carve `- [0x5E6D4, c, func_8006DED4]`.
- Default `era_o2_g0`; no profile entry needed.
