# func_8005FF28

- VRAM `0x8005FF28`, file `0x50728-0x5086C`, size `0x144` (324 B).
- Profile `era_o2_g8_expand_div` (`-O2 -G8` + `MASPSX_EXPAND_DIV=1`).
- Commit `1603a5fc`, landed 884 -> 890.

Four-digit signed decimal emitter with a forced sign glyph. `value < 0` emits
`0x52` after negation; `value > 0` emits `0x89`; zero skips the prefix and keeps
the full four-digit field. Same decimal loop and cursor idiom as
`func_8005FA3C`.

Match lever specific to this leaf: the two prefix arms must be an
`if (value < 0) { ... } else if (value > 0) { ... }` chain. cc1 then merges
both arms into the single `func_8005EB64` call site with the glyph
materialised in the branch delay slots, reproducing retail's
`bgez s3` / `blez s3` two-test structure. `value != 0` gives a single `beqz`
test (58 diffs) and a precomputed `glyph` ternary re-tests `value < 0` a third
time (60 diffs). See `../func_8005FA3C/REPORT.md` for the other levers.
