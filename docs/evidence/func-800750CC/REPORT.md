# `func_800750CC` — record reset + two-field display push (1C slot)

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x658CC,0x6592C)` = 24 words. VRAM `[0x800750CC,0x8007512C)`.

## Semantics

```c
extern unsigned int *D_80095744;
extern char D_800118E0;
extern void func_80074E28(char *a0, int a1);

void func_800750CC(int a0, int a1) {
    unsigned int *v0;

    func_80074E28(&D_800118E0, a0);
    v0 = D_80095744;
    (*(void (**)(int, int, int, int))(v0 + 2))(v0[7], a0, 8, a1);
}
```

Twin of matched `func_8007506C`: the same reset-then-push shape, but with the
`D_800118E0` template and the `+0x1C` argument word (`v0[7]`).

## Levers

- `D_80095744` as a **pointer global** with a single `v0` base local (see
  `func-8007506C/REPORT.md`).
- `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` (patch 3) for the epilogue slot.

Object-level `MISMATCHES=7` (reloc fields + epilogue pair); `LINK_EXACT`.

## Single-leaf object and link proof

```text
tools/analysis/era_leaf_match.sh src/func_800750CC.c 0x800750CC 0x60 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_800750CC.c 0x800750CC 0x60 -O2 -G0
LINK_EXACT
```

## Registration

- Source `src/func_800750CC.c`; YAML carve `[0x658CC, c, func_800750CC]`.
- Profile: `era_o2_g0_fill_epilogue`.
