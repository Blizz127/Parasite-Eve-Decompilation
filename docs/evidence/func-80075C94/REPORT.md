# `func_80075C94` — display-list node init (command + reset triplet)

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x66494,0x664E8)` = 21 words. VRAM `[0x80075C94,0x80075CE8)`.

## Semantics

```c
extern int func_80076150(int a0, int a1, int a2);
extern int func_800762BC(int a0);

void func_80075C94(unsigned char *a0, int a1, int a2, int a3, int a4) {
    a0[3] = 2;
    *(int *)(a0 + 4) = func_80076150(a1, a2, a3 & 0xFFFF);
    *(int *)(a0 + 8) = func_800762BC(a4);
}
```

Retail:

```text
80075c94  addiu sp,sp,-0x20
80075c98  sw    s0,0x10(sp)      ; s0 = a0
80075c9c  move  s0,a0
80075ca0  move  a0,a1
80075ca4  li    v0,2
80075ca8  move  a1,a2
80075cac  sw    s1,0x14(sp)      ; s1 = lw 0x30(sp) (fifth arg)
80075cb0  lw    s1,0x30(sp)
80075cb4  andi  a2,a3,0xffff
80075cb8  sw    ra,0x18(sp)
80075cbc  jal   func_80076150
80075cc0  sb    v0,3(s0)         ; delay slot
80075cc4  sw    v0,4(s0)
80075cc8  jal   func_800762BC
80075ccc  move  a0,s1
80075cd0  sw    v0,8(s0)
...       filled epilogue slot
```

## Levers

- The fifth parameter `a4` is a normal C argument: cc1 spills/loads it from
  the incoming stack slot `0x30(sp)`, exactly as retail.
- `a3 & 0xFFFF` (an `int` `a3`) produces retail's `andi a2,a3,0xffff`; a `short`
  parameter would produce a sign-extension pair instead.
- `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` (patch 3) for the closing
  `addiu sp,sp,0x20` slot.

Object-level `MISMATCHES=4` (three reloc fields + the epilogue pair);
`LINK_EXACT` under the defsym link.

## Single-leaf object and link proof

```text
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  tools/analysis/era_leaf_match.sh src/func_80075C94.c 0x80075C94 0x54 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_80075C94.c 0x80075C94 0x54 -O2 -G0
LINK_EXACT
```

## Registration

- Source `src/func_80075C94.c`; YAML carve `[0x66494, c, func_80075C94]`.
- Profile: `era_o2_g0_fill_epilogue`.
