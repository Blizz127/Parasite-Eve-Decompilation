# `func_80075B4C` — display-list node init (rect region)

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x6634C,0x66384)` = 14 words. VRAM `[0x80075B4C,0x80075B84)`.

## Semantics (retail bytes)

```text
80075b4c  addiu sp,sp,-0x18
80075b50  sw    s0,0x10(sp)
80075b54  move  s0,a0
80075b58  li    v0,2
80075b5c  move  a0,a1
80075b60  sw    ra,0x14(sp)
80075b64  jal   func_800762BC
80075b68  sb    v0,3(s0)          ; delay slot
80075b6c  sw    v0,4(s0)
80075b70  sw    zero,8(s0)
80075b74  lw    ra,0x14(sp)
80075b78  lw    s0,0x10(sp)
80075b7c  jr    ra
80075b80  addiu sp,sp,0x18        ; filled epilogue slot
```

C (`src/func_80075B4C.c`):

```c
void func_80075B4C(unsigned char *a0, int a1) {
    a0[3] = 2;
    *(int *)(a0 + 4) = func_800762BC(a1);
    *(int *)(a0 + 8) = 0;
}
```

## Lever: patch 3 for the epilogue slot

The body shape is already exact; the only structural residual is the epilogue
pair. Retail puts `addiu sp,sp,0x18` in the `jr ra` delay slot (the classic
ASPSX reorder fill), while cc1 leaves the `addiu` before the jump.
`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` performs exactly that move. Object-level
`MISMATCHES=3` (two relocation fields + the swapped epilogue), link-level
`LINK_EXACT` with patch 3.

## Single-leaf object

```text
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  tools/analysis/era_leaf_match.sh src/func_80075B4C.c 0x80075B4C 0x38 -O2 -G0
```

## Link-level proof

```text
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_80075B4C.c 0x80075B4C 0x38 -O2 -G0
LINK_EXACT
```

`func_800762BC` resolves to its retail address; the remaining object diffs are
HI16/LO16 fields.

## Registration

- Source `src/func_80075B4C.c`; YAML carve `[0x6634C, c, func_80075B4C]`.
- Profile: `era_o2_g0_fill_epilogue`.
