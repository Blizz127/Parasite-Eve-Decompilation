# `func_80075C04` — display-list node init (point pair)

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x66404,0x66444)` = 16 words. VRAM `[0x80075C04,0x80075C44)`.

## Semantics (retail bytes)

```text
80075c04  addiu sp,sp,-0x18
80075c08  sw    s0,0x10(sp)
80075c0c  move  s0,a0
80075c10  li    v0,2
80075c14  sw    ra,0x14(sp)
80075c18  sb    v0,3(s0)
80075c1c  lh    a0,0(a1)
80075c20  lh    a1,2(a1)
80075c24  jal   func_800762A0
80075c28  nop
80075c2c  sw    v0,4(s0)
80075c30  sw    zero,8(s0)
80075c34  lw    ra,0x14(sp)
80075c38  lw    s0,0x10(sp)
80075c3c  jr    ra
80075c40  addiu sp,sp,0x18        ; filled epilogue slot
```

C (`src/func_80075C04.c`):

```c
void func_80075C04(unsigned char *a0, short *a1) {
    a0[3] = 2;
    *(int *)(a0 + 4) = func_800762A0(a1[0], a1[1]);
    *(int *)(a0 + 8) = 0;
}
```

The second parameter is a `short *` so the two `lh` loads read the point
components signed (retail uses `lh`, not `lhu`); the `nop` after `jal` is the
unfilled call delay slot.

## Lever: patch 3 for the epilogue slot

Same as `func_80075B4C`: the body shape is already exact and the only residual
is the epilogue pair, which `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` reorders into
the retail `jr` delay slot. Object-level `MISMATCHES=3` (two relocation fields
+ the swapped epilogue), link-level `LINK_EXACT` with patch 3.

## Single-leaf object

```text
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  tools/analysis/era_leaf_match.sh src/func_80075C04.c 0x80075C04 0x40 -O2 -G0
```

## Link-level proof

```text
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_80075C04.c 0x80075C04 0x40 -O2 -G0
LINK_EXACT
```

## Registration

- Source `src/func_80075C04.c`; YAML carve `[0x66404, c, func_80075C04]`.
- Profile: `era_o2_g0_fill_epilogue`.
