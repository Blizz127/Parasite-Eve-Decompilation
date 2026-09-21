# func_80073CF4 — 0x30 bytes, LINK_EXACT

Retail VRAM `0x80073CF4` (file offset `0x644F4`, span size `0x30`), in
`asm/disc1/621E4.s`. Era toolchain `-O2 -G0` (default profile `era_o2_g0`).

## Retail semantics

Third member of the `D_8009566C` handler-dispatch family
(`func_80073C94` = `+0xC`, `func_80073CC4` = `+0x8`, this one = `+0x4`):

```c
extern unsigned int *D_8009566C;

void func_80073CF4(void) {
    unsigned int (*f)() = (unsigned int (*)())D_8009566C[1];
    f();
}
```

`D_8009566C` is a **pointer global** (`lui`/`lw` base, one base register), and
the slot is declared **argument-less** — retail clears no argument registers in
the `jalr` delay slot (a prototype with a parameter makes cc1 materialize
`$a0`).

## Commands and results

```
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu"
export PATH="$PWD/tools/mipsel-host/bin:$PATH"
tools/analysis/era_leaf_match.sh src/func_80073CF4.c 0x80073CF4 0x30 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_80073CF4.c 0x80073CF4 0x30 -O2 -G0
```

Object-level: `ROM .text 48 bytes  C .text 48 bytes`, `MISMATCHES=2` — both are
the `lui %hi(D_8009566C)`/`lw %lo(...)` relocation slots
(`3c020000`/`8c420000` unlinked). Link-level at the retail VMA:

```
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- YAML carve in `configs/USA/disc1.yaml`: the `0x644F4 asm` run (which spanned
  the whole 621E4 region) is split into `0x644F4 c func_80073CF4` (0x30) and
  resume `0x64524 asm`.
- No `disc1_build_profiles.json` assignment (default profile load-bearing).
- `tools/analysis/profile_necessity.py` covers it as default-exact.
