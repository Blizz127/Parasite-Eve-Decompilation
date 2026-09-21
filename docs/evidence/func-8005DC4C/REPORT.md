# func_8005DC4C — 0x50 bytes, LINK_EXACT

Retail VRAM `0x8005DC4C` (file offset `0x4E44C`, span size `0x50`), in
`asm/disc1/4E44C.s`. Era toolchain `-O2 -G0` (default profile `era_o2_g0`).

## Retail semantics

Byte/word record walker over a `D_800A8028`-based pooled structure:

```c
extern unsigned char D_800A8028;
extern unsigned int D_800A802C;

unsigned int func_8005DC4C(unsigned int a0) {
    unsigned char *rec = &D_800A8028 + D_800A802C;
    unsigned char *tbl = rec + *(int *)(rec + 4);
    if (a0 >= *(unsigned short *)tbl)
        return 0;
    return (unsigned int)(tbl + *(short *)(tbl + 2 + a0 * 2));
}
```

`rec` is the stream base; `rec+4` is a signed/word offset to a pair table;
`tbl[0]` (halfword) is the count and `tbl[2 + a0*2]` (signed halfword) is a
table-relative offset to the entry. Out-of-range returns 0.

**Load-bearing lever:** the guard must be written as an **early return**
(`if (a0 >= n) return 0;`) — the `?:`/if-else form with a shared exit makes
cc1 keep the tail live across the join and emit `j` + `move $v0,$zero` for the
zero arm, which is 6 words longer (96 vs 80 bytes). The early-return polarity
gives retail's `sltu`/`beqz` + fallthrough tail exactly.

## Commands and results

```
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu"
export PATH="$PWD/tools/mipsel-host/bin:$PATH"
tools/analysis/era_leaf_match.sh src/func_8005DC4C.c 0x8005DC4C 0x50 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8005DC4C.c 0x8005DC4C 0x50 -O2 -G0
```

Object-level: `ROM .text 80 bytes  C .text 80 bytes`, `MISMATCHES=5` — all five
are `lui`/`addiu`/`lw` relocation slots for `D_800A8028`/`D_800A802C`. Link-level
at the retail VMA:

```
linked .text 80 bytes, target 0x50, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- YAML carve in `configs/USA/disc1.yaml`: the `0x4E44C asm` run is split into
  `0x4E44C c func_8005DC4C` (0x50) and resume `0x4E49C asm`.
- No `disc1_build_profiles.json` assignment (default profile load-bearing).
- `tools/analysis/profile_necessity.py` covers it as default-exact.
