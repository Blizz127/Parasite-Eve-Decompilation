# func_8007F72C — 0x4C bytes, LINK_EXACT

Retail VRAM `0x8007F72C` (file offset `0x6FF2C`, span size `0x4C`), in
`asm/disc1/6F684.s`. Era toolchain `-O2 -G0` + maspsx patch 3
(`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`), profile
`era_o2_g0_fill_epilogue_delay_slot`.

## Retail semantics

```c
extern int func_8007FBF0(int a0);
extern int func_8007F778(void);

int func_8007F72C(void) {
    int s0 = func_8007FBF0(0);
    if (s0 == 1 && func_8007F778() > 0)
        s0 = 2;
    return s0;
}
```

The result is held in `$s0` across both calls and returned; the `== 1` guard
returns `$s0` unchanged on the false path.

## Commands and results

```
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu"
export PATH="$PWD/tools/mipsel-host/bin:$PATH"
tools/analysis/era_leaf_match.sh src/func_8007F72C.c 0x8007F72C 0x4C -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  python3 tools/analysis/era_link_check.py src/func_8007F72C.c 0x8007F72C 0x4C -O2 -G0
```

Without patch 3 the only divergence is the epilogue: cc1 emits
`addiu $sp,$sp,0x18` **before** `jr $ra`; retail fills the `jr $ra` delay slot
with it. The three remaining object "mismatches" are relocation slots.
Link-level at the retail VMA:

```
linked .text 80 bytes, target 0x4c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- YAML carve in `configs/USA/disc1.yaml`: the `0x6F684 asm` run is split into
  `0x6FF2C c func_8007F72C` (0x4C) and resume `0x6FF78 c func_8007F778`.
- `disc1_build_profiles.json`: added `func_8007F72C` to the
  `era_o2_g0_fill_epilogue_delay_slot` assignment list. Confirmed load-bearing
  by `tools/analysis/profile_necessity.py` (default `era_o2_g0` does **not**
  reproduce the leaf).
