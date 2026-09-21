# `func_800755BC` — 0x5C-byte record reset wrapper

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x65DBC,0x65DF0)` = 13 words. VRAM `[0x800755BC,0x800755F0)`.

## Semantics

```c
extern int func_80071A34(void *a0, void *a1, int a2);
extern char D_8009575C;

void *func_800755BC(void *a0) {
    func_80071A34(a0, &D_8009575C, 0x5C);
    return a0;
}
```

Retail copies the 0x5C-byte `D_8009575C` template into `a0` and returns `a0`.

## Levers

- `&D_8009575C` as a **data symbol** so cc1 emits HI16/LO16 relocs against
  `D_8009575C` (a literal address folds to one `lui`).
- `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` (patch 3) for the `addiu sp,sp,0x18`
  epilogue slot.

Object-level `MISMATCHES=5`; `LINK_EXACT` under the defsym link.

## Single-leaf object and link proof

```text
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  tools/analysis/era_leaf_match.sh src/func_800755BC.c 0x800755BC 0x34 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_800755BC.c 0x800755BC 0x34 -O2 -G0
LINK_EXACT
```

## Registration

- Source `src/func_800755BC.c`; YAML carve `[0x65DBC, c, func_800755BC]`.
- Profile: `era_o2_g0_fill_epilogue`.
