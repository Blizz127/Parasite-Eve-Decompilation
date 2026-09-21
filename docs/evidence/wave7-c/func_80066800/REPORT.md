# wave7-c — func_80066800 (executed-path C leaf)

**Result: LANDED.** Commit `4b6f0394` on `agent/wave7-c`. Count transition
**904 -> 905** (the shared commit also lands `func_80035C84`, 903 -> 904).

## Identity

| field | value |
| --- | --- |
| function | `func_80066800` |
| file offset | `0x57000` |
| size | `0x18C` (99 words) |
| VRAM | `0x80066800` |
| splat source | `asm/disc1/56438.s` |
| profile | default `era_o2_g0` (`-O2 -G0`), no new profile |

## Semantics

Apply a 52-byte view record to the published GTE state. Record =
`*D_800B1624 + <offset at container+0x1C> + index*52`. Publishes the record's
leading halfword through `*D_800BCFA8` and `func_80079024`, copies the nine
rotation halfwords (`+0x02..+0x12`) and the three translation words
(`+0x14/+0x18/+0x1C`) through `*D_800BCFA4`, records the byte index at
`D_800BCFFD` and sets bit `0x80` in `D_800BCF88`. Returns 0.

## Fresh-build authority

```
EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (905 registered C leaves)
VERIFY_US=PASS
```

`try_leaf.py src/func_80066800.c 0x57000 0x18C` before the carve:
`WORDS MATCH (+4 pad bytes, trimmed by the build)`.

## Levers

- **`*volatile` on a pointer global forces per-store reloads.** Both
  `D_800B1624` and `D_800BCFA4` are declared
  `extern ... *volatile`. Retail reloads `D_800BCFA4` (`lui`/`lw`) before
  *every* one of the twelve stores and loads `D_800B1624` twice at entry; a
  plain pointer lets cc1 hoist the base into a register and drops ~19 words
  (320 bytes vs 396). This is the general lever for any leaf whose retail asm
  is a repeated `lui/lw` + store pair sequence.
- cc1 2.7.2 does **not** unroll a constant 9-trip copy loop; the nine halfword
  copies must be written out literally.
- `D_800BCFA4` is byte-addressed, so the three trailing word stores are
  explicit `*(int *)(D_800BCFA4 + 0x14/0x18/0x1C)`.
- `D_800BCF88 |= 0x80;` with the byte store `D_800BCFFD = index` reordered by
  the scheduler into retail's `lw; sb; ori; sw`.

## Divergence history

`for`-loop version: 75 diffs (not unrolled, hoisted base). Unrolled with plain
pointers: 53 diffs (320 bytes vs 396; base still hoisted). Adding `volatile` to
`D_800BCFA4` closed it.
