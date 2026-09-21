# `func_8006DB9C` — tagged byte-pair search (returns the pair's value byte)

Outcome: **MATCHED** on era `-O2 -G0`. Integrated as matching-C leaf 582.
Linked at its retail VMA with `D_800B0CD8` defined, the object `.text` is
**byte-identical** to retail (`LINK_EXACT`, 0 word mismatches).

## Function hood and retail span

- File span `[0x5E39C,0x5E3E0)` = `0x44` bytes = 17 words.
- VRAM span `[0x8006DB9C,0x8006DBE0)`.
- Preceded by matched-C `func_8006DB48` (ends `0x5E39C`); followed by
  `func_8006DBE0` at `0x5E3E0` (this session) then asm at `0x5E418`.

## Semantics (from retail bytes)

```text
8006db9c  00002821  move  a1,zero          ; i = 0
8006dba0  3c03800b  lui   v1,0x800b
8006dba4  24630cd8  addiu v1,v1,-13096      ; v1 = &D_800B0CD8
8006dba8  806200dc  lb    v0,220(v1)        ; t->tag[i][0]
8006dbac  00000000  nop
8006dbb0  14440004  bne   v0,a0,L
8006dbb4  00000000  nop
8006dbb8  806200dd  lb    v0,221(v1)        ; return t->tag[i][1]
8006dbbc  0801b6f6  j     0x8006DBD8        ; return
8006dbc0  00000000  nop
8006dbc4  24a50001  addiu a1,a1,1          ; i++
8006dbc8  28a20002  slti  v0,a1,2
8006dbcc  1440fff6  bnez  v0,0x8006DBA8
8006dbd0  24630002  addiu v1,v1,2          ; p += 2
8006dbd4  2402ffff  li    v0,-1
8006dbd8  03e00008  jr    ra
8006dbdc  00000000  nop
```

C shape (`src/func_8006DB9C.c`):

```c
int func_8006DB9C(int a0) {
    Tag *t = &D_800B0CD8;
    int i;

    for (i = 0; i < 2; i++) {
        if (t->tag[i][0] == a0)
            return t->tag[i][1];
    }
    return -1;
}
```

## Lever: index the aggregate member, don't walk a `signed char *`

Retail keeps the array element addressing as **symbol base + `0xDC`
displacement** on each `lb` (`lb $2,220($3)` / `lb $2,221($3)`) and
increments the base by 2. Writing the walk as a `signed char *p =
&tag[0][0]` pointer first materializes `&D_800B0CD8 + 0xDC` into the base
(`lui`/`addiu` with `0x0cdc`) and drops both displacements — 5 residual
words. Indexing the real aggregate (`t->tag[i][0]`, with a local `t =
&D_800B0CD8`) keeps the displacement on the load and reproduces retail's
`$v1` base / `$a1` counter allocation exactly. The same lever also keeps
the `j` to the shared return block (`0x8006DBBC` local label).

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_8006DB9C.c 0x8006DB9C 0x44 -O2 -G0
```

Result: `MISMATCHES=3`, size `0x44` = ROM — every mismatch is a link-time
relocation field:

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x8006DBA0` | `3c03800b` | `3c030000` | R_MIPS_HI16 `D_800B0CD8` |
| `0x8006DBA4` | `24630cd8` | `24630000` | R_MIPS_LO16 `D_800B0CD8` |
| `0x8006DBBC` | `0801b6f6` | `0800000f` | R_MIPS_26 `.text` (local return block) |

Link-level proof (`python3 tools/analysis/era_link_check.py
src/func_8006DB9C.c 0x8006DB9C 0x44 -O2 -G0`), resolving
`D_800B0CD8=0x800B0CD8`:

```text
linked .text 80 bytes, target 0x44, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006DB9C.c`.
- YAML carve: `- [0x5E39C, c, func_8006DB9C]` (was `- [0x5E39C, asm]`).
- `python3 tools/build/disc1_plan.py --check` → 874 spans
  (583 c, 289 asm, 2 rodata).
