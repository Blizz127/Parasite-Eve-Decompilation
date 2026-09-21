# `func_80017410` — field-VM handler: setter call with a stack result word

Outcome: **MATCHED** on era `-O2 -G8` (no knob needed); `LINK_EXACT`, 0 word
mismatches.

## Function hood and span

- File span `[0x7C10,0x7C44)` = 13 words. VRAM `[0x80017410,0x80017444)`.
- Field-VM dispatch target `D_800910A0[0x410]`.
- Carved from the former `0x7C10` asm span in `asm/disc1/7C10.s`.

## Semantics (retail bytes)

```text
80017410  27bdffd8  addiu sp,sp,-0x28
80017414  2402ffff  li    v0,-1
80017418  afbf0020  sw    ra,0x20(sp)
8001741c  a7a20010  sh    v0,0x10(sp)       ; buf[0] = -1
80017420  8c820000  lw    v0,0(a0)
80017424  00002821  addu  a1,zero,zero
80017428  84440000  lh    a0,0(v0)          ; (short)*a0[0]
8001742c  0c00dd78  jal   func_800375E0
80017430  27a60010  addiu a2,sp,0x10       ; (delay) buf
80017434  8fbf0020  lw    ra,0x20(sp)
80017438  24020001  li    v0,1
8001743c  03e00008  jr    ra
80017440  27bd0028  addiu sp,sp,0x28       ; (delay)
```

C (`src/func_80017410.c`):

```c
int func_80017410(unsigned short **a0) {
    short buf[8];
    buf[0] = -1;
    func_800375E0((short)*a0[0], 0, buf);
    return 1;
}
```

## Lever: `buf` must be an array

A scalar `short buf` only reserves one stack slot (frame `0x20`, `sh` at
`0x10` but no room for a `sp+0x10` argument window), so cc1 emits
`addiu sp,sp,-0x20` and mismatches retail's `0x28` frame. Declaring
`short buf[8]` produces the retail `0x28` frame with the store at `0x10`
(`era_leaf_match.sh` reports `MISMATCHES=1`; the one diff is the `sp`-adjust
relocation).

## Link-level proof

```text
python3 tools/analysis/era_link_check.py src/func_80017410.c 0x80017410 0x34 -O2 -G8
LINK_EXACT
```

## Registration

- Source `src/func_80017410.c`; YAML `- [0x7C10, c, func_80017410]`.
- Profile `era_o2_g8`.
