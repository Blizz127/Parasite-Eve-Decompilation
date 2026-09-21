# func_800866A4 — 0x4C bytes, LINK_EXACT

Retail VRAM `0x800866A4` (file offset `0x76EA4`, span size `0x4C`), in
`asm/disc1/76768.s`. Era toolchain `-O2 -G0` (default profile
`era_o2_g0`, no assignment needed).

## Retail semantics

Two-argument MMIO command issue:

```c
void func_800866A4(unsigned int a0, unsigned int a1) {
    a0 &= 0xFFFF;
    a1 &= 0xFFFFFF;
    D_800BCD80 = 0x21;
    D_800BCD84 = a0;
    D_800BCD88 = a1;
    func_8008CBA8(a0, a1);
}
```

`D_800BCD80`/`84`/`88` are opaque command words (write-only here; earlier
siblings `func_80086FF8`/`func_80087024` write `0xF0`/`0xF1` through the same
`jal func_8008CBA8` shape, so this is the `0x21` command of the same family).
`func_8008CBA8` receives the **masked** values (`a0 & 0xFFFF` in `$a0`,
`a1 & 0xFFFFFF` in `$a1`) — the masks are computed before the stores and the
masked registers are the call arguments.

## Commands and results

```
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu"
export PATH="$PWD/tools/mipsel-host/bin:$PATH"
tools/analysis/era_leaf_match.sh src/func_800866A4.c 0x800866A4 0x4C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_800866A4.c 0x800866A4 0x4C -O2 -G0
```

Object-level: `ROM .text 76 bytes  C .text 80 bytes` — the 4-byte surplus is
the assembler's trailing alignment pad; the 7 object "mismatches" are the
three `lui $at,%hi(D_800BCD8x)` and one `jal func_8008CBA8` relocation slots
(`3c010000`/`0c000000` in the unlinked object). Link-level at the retail VMA:

```
linked .text 80 bytes, target 0x4c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- YAML carve in `configs/USA/disc1.yaml`: split the `0x76768` asm run into
  prefix `0x76768 asm` (0x73C), `0x76EA4 c func_800866A4` (0x4C),
  resume `0x76EF0 asm`.
- No `disc1_build_profiles.json` assignment (default profile is load-bearing).
- `tools/analysis/profile_necessity.py` run covers it as default-exact.
