# `func_8006DF50` — record lookup + copy-helper forwarder (CD/boot cluster)

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. **`LINK_EXACT`**, 0 word mismatches at the retail VMA.

## Function hood and retail span

- File span `[0x5E750,0x5E7A8)` = `0x58` bytes = 22 words.
- VRAM span `[0x8006DF50,0x8006DFA8)`.
- Carved out of the former `0x5E39C` asm span; follows the newly matched
  `func_8006DED4` and precedes `func_8006DFA8` (asm, handwritten).

## Semantics (from retail bytes, `asm/disc1/5E39C.s`)

`int func_8006DF50(int a0, int a1, int a2, int a3, int a4)`:

- `r = func_8006E514(a0, a1)`.
- If `r == 0` return `-1`.
- Else return `func_80086608(r, a2, a3, a4)`.

This is the fifth argument-forwarding shim of the CD cluster; the source form
is the natural one and needs no traps.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/era_leaf_match.sh src/func_8006DF50.c 0x8006DF50 0x58 -O2 -G0
```

Result: `SIZE_MISMATCH C=0x60 ROM=0x58` (GNU as pad), `MISMATCHES=3`, first at
`0x8006DF64`. All three differing words carry a relocation
(`func_8006E514` ×1, `func_80086608` ×1, plus the internal `j`).

Link-level proof:

```text
linked .text 96 bytes, target 0x58, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- Source `src/func_8006DF50.c`; YAML carve `- [0x5E750, c, func_8006DF50]`
  + resume `- [0x5E7A8, asm]`.
- Default `era_o2_g0`; no profile entry needed.
