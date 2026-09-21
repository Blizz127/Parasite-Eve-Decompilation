# func_80064C54 — MATCHED (`LINK_EXACT`)

VRAM `0x80064C54`, size `0x2C` (11 words), file `0x55454` in
`asm/disc1/55454.s`. era flags `-O2 -G8` (profile `era_o2_g8`).

## Semantics

`func_8005F354(func_8005DC4C(), D_8009D164)`: fetch a value, then pass it
plus the gp-relative global as the two arguments of a second call.

## Source

```c
extern int D_8009D164;
extern int func_8005DC4C(void);
extern void func_8005F354(int a0, int a1);
void func_80064C54(void) {
    func_8005F354(func_8005DC4C(), D_8009D164);
}
```

`src/func_80064C54.c`.

## Semantics note — argument hoisting across the branch

Retail saves `func_8005DC4C`'s result in `$v0`, loads the gp-relative
`D_8009D164` (`lw $a1,0x3F4($gp)`) **after** the first `jal`, then moves
`$v0` into `$a0` in the second `jal` delay slot. Writing the nested call
directly (rather than a named temporary) is what produces the
gp-relative load in the right place under `-G8`; the gp displacement
`0x3F4` matches `D_8009D164` given `_gp = 0x8009CD70`.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80064C54.c 0x80064C54 0x2C -O2 -G8
ROM  .text 44 bytes  C .text 48 bytes  target 44
MISMATCHES=3  (two `jal` relocation placeholders + the gp `lw` %lo)
python3 tools/analysis/era_link_check.py src/func_80064C54.c 0x80064C54 0x2C -O2 -G8
linked .text 48 bytes, target 0x2C, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x55454, c, func_80064C54]` (asm resumes at
`0x55480`). Registered in `configs/USA/disc1_build_profiles.json` under
`era_o2_g8`.
