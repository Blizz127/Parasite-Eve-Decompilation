# `func_80074DC0` — display-slot dispatch with optional debug log

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x655C0,0x65628)` = 26 words. VRAM `[0x80074DC0,0x80074E28)`.
- Sibling of matched `func_80074D28` in the `0x80074xxx` display cluster;
  same `D_8009574E`/`D_80095748` gate, different log template and handler slot.

## Semantics

```c
extern unsigned char D_8009574E;
extern int (*D_80095748)(char *, int);
extern struct D44b { char pad[0x3C]; unsigned int (*f)(int); } *D_80095744;
extern char D_80011884;

unsigned int func_80074DC0(int a0) {
    if (D_8009574E >= 2)
        D_80095748(&D_80011884, a0);
    return D_80095744->f(a0);
}
```

Retail:

```text
80074dc0  lui  v0,%hi(D_8009574E)
80074dc4  lbu  v0,%lo(...)         ; gate byte
80074dc8  addiu sp,sp,-0x18
80074dcc  sw   s0,0x10(sp)
80074dd0  move s0,a0
80074dd4  sltiu v0,v0,2
80074dd8  bnez v0,.L80074DFC
80074ddc  sw   ra,0x14(sp)         ; delay slot
80074de0  lui  a0,%hi(D_80011884)  ; log template
80074de4  addiu a0,a0,%lo(...)
80074de8  lw   v0,%lo(D_80095748)
80074dec  nop
80074df0  jalr v0
80074df4  move a1,s0
.L80074DFC:
80074dfc  lw   v0,%lo(D_80095744)
80074e00  nop
80074e04  lw   v0,0x3C(v0)
80074e08  nop
80074e0c  jalr v0
80074e10  move a0,s0
...       filled epilogue (`addiu sp,sp,0x18` in the jr slot)
```

## Levers

- `&D_80011884` as a **data symbol** (a cast constant folds to a single `lui`).
- The `+0x3C` slot is a handler **returning a value** (`unsigned int (*f)(int)`).
- `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` (patch 3) for the epilogue slot.

Object-level `MISMATCHES=10` (all relocation fields + the epilogue pair);
`LINK_EXACT` under the defsym link.

## Single-leaf object and link proof

```text
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  tools/analysis/era_leaf_match.sh src/func_80074DC0.c 0x80074DC0 0x68 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_80074DC0.c 0x80074DC0 0x68 -O2 -G0
LINK_EXACT
```

## Registration

- Source `src/func_80074DC0.c`; YAML carve `[0x655C0, c, func_80074DC0]`.
- Profile: `era_o2_g0_fill_epilogue`.
