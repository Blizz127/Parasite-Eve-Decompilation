# `func_800753B4` — gated reset + display-slot push

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x65BB4,0x65C24)` = 28 words. VRAM `[0x800753B4,0x80075424)`.

## Semantics

```c
extern unsigned char D_8009574E;
extern int (*D_80095748)(char *, int);
extern unsigned int *D_80095744;
extern char D_80011928;

void func_800753B4(int a0) {
    unsigned int *v0;

    if (D_8009574E >= 2)
        D_80095748(&D_80011928, a0);
    v0 = D_80095744;
    (*(void (**)(int, int, int, int))(v0 + 2))(v0[6], a0, 0, 0);
}
```

Retail gates on the `D_8009574E` state byte, logs `D_80011928` through the
`D_80095748` vector, then calls the `+0x8` handler with the `+0x18` argument
word, `a0` and two zeroed registers.

## Levers

- `D_80095744` as a **pointer global** with one `v0` base local: retail keeps
  the loaded base register and reads both `+0x8` and `+0x18` from it. Declaring
  `D_80095744` as a `char` and taking its address makes cc1 fold to
  `lui/addiu` and load `+0x1C` (the sibling struct's slot) — the base must be
  the loaded pointer value.
- The `D_8009574E >= 2` gate and the `D_80095748` log vector match the
  boot-spine siblings `func_80074D28`/`func_80074DC0`.
- `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` (patch 3) for the epilogue slot.

Object-level `MISMATCHES=10` (reloc fields + epilogue pair); `LINK_EXACT`.

## Single-leaf object and link proof

```text
tools/analysis/era_leaf_match.sh src/func_800753B4.c 0x800753B4 0x70 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_800753B4.c 0x800753B4 0x70 -O2 -G0
LINK_EXACT
```

## Registration

- Source `src/func_800753B4.c`; YAML carve `[0x65BB4, c, func_800753B4]`.
- Profile: `era_o2_g0_fill_epilogue`.
