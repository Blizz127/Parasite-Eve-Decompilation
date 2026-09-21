# `func_8007506C` — record reset + two-field display push

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x6586C,0x658CC)` = 24 words. VRAM `[0x8007506C,0x800750CC)`.

## Semantics

```c
extern unsigned int *D_80095744;
extern char D_800118D4;
extern void func_80074E28(char *a0, int a1);

void func_8007506C(int a0, int a1) {
    unsigned int *v0;

    func_80074E28(&D_800118D4, a0);
    v0 = D_80095744;
    (*(void (**)(int, int, int, int))(v0 + 2))(v0[8], a0, 8, a1);
}
```

Retail resets the record from the `D_800118D4` template, then calls the
`+0x8` handler with the `+0x20` argument word, `a0`, literal `8` and `a1`.

## Levers

- `D_80095744` is a **pointer global** (`extern unsigned int *`): retail emits
  `lui`/`lw` and keeps the one loaded base in `$v0`, reading both `+0x8`
  (handler) and `+0x20` (argument) from it. Writing the table as a struct and
  re-reading `D_80095744->f` / `->v` makes cc1 reload the global twice and
  diverges; a single `unsigned int *v0 = D_80095744;` local reproduces retail.
- The call has **no** `D_8009574E` gate (the first `beqz`-style check other
  leaves in this cluster have); this pair of leaves calls `func_80074E28`
  directly.
- `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` (patch 3) for the epilogue slot.

Object-level `MISMATCHES=7` (reloc fields + epilogue pair); `LINK_EXACT`.

## Single-leaf object and link proof

```text
tools/analysis/era_leaf_match.sh src/func_8007506C.c 0x8007506C 0x60 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_8007506C.c 0x8007506C 0x60 -O2 -G0
LINK_EXACT
```

## Registration

- Source `src/func_8007506C.c`; YAML carve `[0x6586C, c, func_8007506C]`.
- Profile: `era_o2_g0_fill_epilogue`.
