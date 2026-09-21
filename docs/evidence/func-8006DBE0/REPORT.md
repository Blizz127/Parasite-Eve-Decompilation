# `func_8006DBE0` — tagged byte-pair search (returns the pair index)

Outcome: **MATCHED** on era `-O2 -G0`. Integrated as matching-C leaf 583.
Linked at its retail VMA with `D_800B0CD8` defined, the object `.text` is
**byte-identical** to retail (`LINK_EXACT`, 0 word mismatches).

## Function hood and retail span

- File span `[0x5E3E0,0x5E418)` = `0x38` bytes = 14 words.
- VRAM span `[0x8006DBE0,0x8006DC18)`.
- Preceded by `func_8006DB9C` at `0x5E39C` (this session); followed by asm
  at `0x5E418`.

## Semantics (from retail bytes)

```text
8006dbe0  00001821  move  v1,zero          ; i = 0
8006dbe4  3c05800b  lui   a1,0x800b
8006dbe8  24a50cd8  addiu a1,a1,-13096      ; a1 = &D_800B0CD8
8006dbec  80a200dc  lb    v0,220(a1)        ; t->tag[i][0]
8006dbf0  00000000  nop
8006dbf4  10440006  beq   v0,a0,L
8006dbf8  00601021  move  v0,v1            ; delay: return i
8006dbfc  24630001  addiu v1,v1,1           ; i++
8006dc00  28620002  slti  v0,v1,2
8006dc04  1440fff9  bnez  v0,0x8006DBEC
8006dc08  24a50002  addiu a1,a1,2           ; p += 2
8006dc0c  2402ffff  li    v0,-1
8006dc10  03e00008  jr    ra
8006dc14  00000000  nop
```

C shape (`src/func_8006DBE0.c`):

```c
int func_8006DBE0(int a0) {
    Tag *t = &D_800B0CD8;
    int i;

    for (i = 0; i < 2; i++) {
        if (t->tag[i][0] == a0)
            return i;
    }
    return -1;
}
```

## Lever: same aggregate-member indexing as `func_8006DB9C`

Retail keeps the `0xDC` displacement on the load and the `return i` lands in
the `beq` delay slot (`move $2,$3`). Walking a `signed char *p` collapses the
displacement into the base (3 residual words). Indexing `t->tag[i][0]` with a
local `t` reproduces retail's `$a1` base / `$v1` counter allocation and the
delay-slot `move`.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_8006DBE0.c 0x8006DBE0 0x38 -O2 -G0
```

Result: `MISMATCHES=2`, size `0x38` = ROM — both mismatches are link-time
relocation fields:

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x8006DBE4` | `3c05800b` | `3c050000` | R_MIPS_HI16 `D_800B0CD8` |
| `0x8006DBE8` | `24a50cd8` | `24a50000` | R_MIPS_LO16 `D_800B0CD8` |

Link-level proof (`python3 tools/analysis/era_link_check.py
src/func_8006DBE0.c 0x8006DBE0 0x38 -O2 -G0`), resolving
`D_800B0CD8=0x800B0CD8`:

```text
linked .text 64 bytes, target 0x38, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006DBE0.c`.
- YAML carve: `- [0x5E3E0, c, func_8006DBE0]` (was part of `0x5E39C` asm).
- `python3 tools/build/disc1_plan.py --check` → 874 spans
  (583 c, 289 asm, 2 rodata).
