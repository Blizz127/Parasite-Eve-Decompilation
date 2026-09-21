# func_80075B84 — MATCHED (`LINK_EXACT`)

VRAM `0x80075B84`, size `0x80` (32 words), file `0x66384` in
`asm/disc1/66384.s`. era flags `-O2 -G0` + **maspsx patch 3**
(`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`, profile
`era_o2_g0_fill_epilogue_delay_slot`).

## Semantics

Display-list node init from a four-short rectangle record:

```c
a0[3] = 2;
*(int *)(a0 + 4) = func_80076170(a1->x, a1->y);
*(int *)(a0 + 8) = func_80076208(a1->x + a1->w - 1, a1->y + a1->h - 1);
```

`func_80076170` packs a clamped x/y pair into GP0 `0xE3` and `func_80076208`
the `0xE4` equivalent for `x+w-1`/`y+h-1`.

## Source

```c
struct XY {
    short x;
    short y;
    short w;
    short h;
};

extern int func_80076170(short a0, short a1);
extern int func_80076208(short a0, short a1);

void func_80075B84(unsigned char *a0, struct XY *a1) {
    a0[3] = 2;
    *(int *)(a0 + 4) = func_80076170(a1->x, a1->y);
    *(int *)(a0 + 8) = func_80076208(a1->x + a1->w - 1, a1->y + a1->h - 1);
}
```

`src/func_80075B84.c`.

## Semantics note — write order

The `a0[3] = 2` byte store comes **first** in retail (scheduled into the
window before the first call), so the C statement order above is
load-bearing; moving it after the calls changes the schedule.

## Commands

```
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  tools/analysis/era_leaf_match.sh src/func_80075B84.c 0x80075B84 0x80 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_80075B84.c 0x80075B84 0x80 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1
linked word mismatches=0, nonzero_pad=0
LINK_EXACT
```

Patch 3 is load-bearing here: with the profile off the epilogue
`addiu sp,sp,N` is not filled into the `jr $ra` delay slot
(`word mismatches=2`).

## Provenance

`configs/USA/disc1.yaml`: `- [0x66384, c, func_80075B84]`. Registered in
`configs/USA/disc1_build_profiles.json` under
`era_o2_g0_fill_epilogue_delay_slot`.
