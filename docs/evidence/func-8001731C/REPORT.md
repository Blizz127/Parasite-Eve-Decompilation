# `func_8001731C` — field-VM handler: conditional cursor repoint

Outcome: **MATCHED** on era `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=
D_8009D2F0` (`LINK_EXACT`, 0 word mismatches).

## Function hood and span

- File span `[0x7B1C,0x7B5C)` = 16 words. VRAM `[0x8001731C,0x8001735C)`.
- Field-VM dispatch target `D_800910A0[0x31C]`.
- Carved from the former `0x7B1C` asm span in `asm/disc1/7B1C.s`.

## Semantics (retail bytes)

```text
8001731c  8c820000  lw   v0,0(a0)
80017320  (nop)
80017324  8c420000  lw   v0,0(v0)
80017328  (nop)
8001732c  14400009  bnez v0,.L80017354     ; if (**a0 != 0) skip
80017330  (nop)
80017334  8c820004  lw   v0,0x4(a0)         ; *a0[1]
80017338  3c03800a  lui  v1,0x800a
8001733c  8c63d2f0  lw   v1,%lo(D_8009D2F0)(v1)
80017340  8c420000  lw   v0,0(v0)
80017344  8c63009c  lw   v1,0x9c(v1)
80017348  00021040  sll  v0,v0,1
8001734c  00621821  addu v1,v1,v0
80017350  af830090  sw   v1,0x90(gp)        ; D_8009CE00
.L80017354
80017354  03e00008  jr   ra
80017358  24020001  li   v0,1
```

C (`src/func_8001731C.c`):

```c
int func_8001731C(unsigned int **a0) {
    if (**a0 == 0) {
        unsigned int v = *a0[1];
        D_8009CE00 = D_8009D2F0[0x27] + v * 2;
    }
    return 1;
}
```

## Lever

Inherits the `func_80017294` absolute-base / gp-relative-store split (see
`docs/evidence/func-80017294/REPORT.md`).

## Link-level proof

```text
MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0 \
  python3 tools/analysis/era_link_check.py src/func_8001731C.c 0x8001731C 0x40 -O2 -G8
LINK_EXACT
```

## Registration

- Source `src/func_8001731C.c`; YAML `- [0x7B1C, c, func_8001731C]`.
- Profile `era_o2_g8_force_d8009d2f0_absolute`.
