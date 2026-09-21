# `func_8001735C` — field-VM handler: allocate record + fill three fields

Outcome: **MATCHED** on era `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=
D_8009D2F0` (`LINK_EXACT`, 0 word mismatches).

## Function hood and span

- File span `[0x7B5C,0x7BF4)` = 38 words. VRAM `[0x8001735C,0x800173F4)`.
- Field-VM dispatch target `D_800910A0[0x35C]`.
- Carved from the former `0x7B1C` asm span.

## Semantics (retail bytes)

```text
8001735c  addiu sp,sp,-0x20
80017360  sw    s0,0x18(sp); s0 = a0
8001736c  lw    v0,0(s0);  lw v0,0(v0);  sb v0,0x10(sp)   ; buf[0] = **a0
80017380  lw    v0,4(s0);  lui/lw a1,D_8009D2F0
8001738c  lw    v0,0(v0);  li a2,1
80017394  jal   func_80035038
80017398  sb    v0,0x11(sp)                              ; buf[1] = *a0[1] (delay)
8001739c  lw    v1,8(s0);  lw v1,0(v1);  sw v1,0x28(v0)
800173b0  lw    v1,0xC(s0); lw v1,0(v1); sw v1,0x2C(v0)
800173c4  lw    v1,0x10(s0); lw v1,0(v1); a0 = v0
800173d4  jal   func_8001AA78
800173d8  sw    v1,0x30(a0)                              ; (delay)
800173dc  li    v0,1
800173e0  lw ra,s0 ; restore ; jr ra ; addiu sp,sp,0x20
```

C (`src/func_8001735C.c`):

```c
int func_8001735C(unsigned int **a0) {
    char buf[2];
    unsigned char *p;

    buf[0] = **a0;
    buf[1] = *a0[1];
    p = func_80035038(buf, D_8009D2F0, 1);
    *(int *)(p + 0x28) = *a0[2];
    *(int *)(p + 0x2C) = *a0[3];
    *(int *)(p + 0x30) = *a0[4];
    func_8001AA78(p);
    return 1;
}
```

## Notes

- `buf` is a 2-byte stack record passed by address; the second byte store lands
  in the `func_80035038` delay slot, matching retail.
- `D_8009D2F0` is passed as the raw state pointer (absolute under
  `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0`).

## Link-level proof

```text
MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0 \
  python3 tools/analysis/era_link_check.py src/func_8001735C.c 0x8001735C 0x98 -O2 -G8
LINK_EXACT
```

## Registration

- Source `src/func_8001735C.c`; YAML `- [0x7B5C, c, func_8001735C]`.
- Profile `era_o2_g8_force_d8009d2f0_absolute`.
