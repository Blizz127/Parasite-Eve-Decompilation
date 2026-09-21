# `func_80075AE8` — 0x14-byte record reset wrapper

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x662E8,0x6631C)` = 13 words. VRAM `[0x80075AE8,0x80075B1C)`.

## Semantics

```c
extern int func_80071A34(void *a0, void *a1, int a2);
extern char D_800957B8;

void *func_80075AE8(void *a0) {
    func_80071A34(a0, &D_800957B8, 0x14);
    return a0;
}
```

Retail copies the 0x14-byte `D_800957B8` template into `a0` and returns `a0`.

## Levers

- `&D_800957B8` as a **data symbol** (not a folded integer constant) so cc1
  emits `lui`/`addiu` HI16/LO16 relocs against `D_800957B8`, matching retail.
- `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` (patch 3) for the `addiu sp,sp,0x18`
  epilogue slot.

Object-level `MISMATCHES=5`; `LINK_EXACT` under the defsym link.

## Single-leaf object and link proof

```text
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  tools/analysis/era_leaf_match.sh src/func_80075AE8.c 0x80075AE8 0x34 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_80075AE8.c 0x80075AE8 0x34 -O2 -G0
LINK_EXACT
```

## Registration

- Source `src/func_80075AE8.c`; YAML carve `[0x662E8, c, func_80075AE8]`.
- Profile: `era_o2_g0_fill_epilogue`.
