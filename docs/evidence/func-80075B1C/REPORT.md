# `func_80075B1C` — display-slot sign-bit query

Outcome: **MATCHED** on era `-O2 -G0` (`BYTE_EXACT` object; `LINK_EXACT`,
0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x6631C,0x6634C)` = 12 words. VRAM `[0x80075B1C,0x80075B4C)`.
- Third leaf of the `0x80075xxx` display-slot cluster (`654C8.s`), all of which
  read the `D_8009574E` state block and the `D_80095744` handler table.

## Semantics (retail bytes)

```text
80075b1c  lui  v0,%hi(D_80095744)
80075b20  lw   v0,%lo(D_80095744)(v0)
80075b24  addiu sp,sp,-0x18
80075b28  sw   ra,0x10(sp)
80075b2c  lw   v0,0x38(v0)
80075b30  nop
80075b34  jalr v0                  ; no arguments
80075b38  nop
80075b3c  lw   ra,0x10(sp)
80075b40  srl  v0,v0,31            ; sign bit as 0/1
80075b44  jr   ra
80075b48  addiu sp,sp,0x18
```

C (`src/func_80075B1C.c`):

```c
extern struct D44d { char pad[0x38]; unsigned int (*f)(); } *D_80095744;

unsigned int func_80075B1C(void) {
    return D_80095744->f() >> 31;
}
```

## Lever: parameterless prototype + the 0x38 slot

The two load-bearing details:

1. The slot is at offset **`0x38`** (the `0x3C` used by neighbours
   `func_80074DC0` and `func_80075358` is a different handler).
2. `f` must be declared **argument-less** (`unsigned int (*f)()`). Declaring
   `unsigned int (*f)(int)` and calling `f(0)` makes cc1 materialize `$a0` in
   the `jalr` delay slot (`move a0,zero`, 4 mismatched words); the
   parameterless form emits the retail `nop`.
3. Return type `unsigned int` yields the retail logical `srl` (an `int`
   return would give an arithmetic `sra`).

## Exact residual accounting

Object-level best is `MISMATCHES=2` — the two relocation fields of
`lui`/`lw` against `D_80095744`; both resolve under the defsym link.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_80075B1C.c 0x80075B1C 0x30 -O2 -G0
```

## Link-level proof

```text
python3 tools/analysis/era_link_check.py src/func_80075B1C.c 0x80075B1C 0x30 -O2 -G0
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

The single undefined symbol `D_80095744` resolves to the retail address retail
actually touches, so the object-level relocation fields are the only
difference.

## Registration

- Source `src/func_80075B1C.c`.
- YAML carve `[0x6631C, c, func_80075B1C]` out of the `0x655C0` asm region.
- Profile: default `era_o2_g0` (`-O2 -G0`).
