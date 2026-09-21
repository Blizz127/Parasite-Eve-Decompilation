# `func_800176B8` — field-VM handler: OR an operand word into the state flags

Outcome: **MATCHED** on era `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=
D_8009D2F0` (`LINK_EXACT`, 0 word mismatches).

## Function hood and span

- File span `[0x7EB8,0x7EE0)` = 10 words. VRAM `[0x800176B8,0x800176E0)`.
- Field-VM dispatch target `D_800910A0[0x6B8]`.
- Carved from the former `0x7C10` asm span (which is the file covering this
  region); follows matched-C `func_80017588` block, precedes
  `func_800176E0` at `0x7EE0`.

## Semantics (retail bytes)

```text
800176b8  3c05800a  lui  a1,0x800a
800176bc  8ca5d2f0  lw   a1,%lo(D_8009D2F0)(a1)
800176c0  8c830000  lw   v1,0(a0)
800176c4  8ca20098  lw   v0,0x98(a1)       ; state flags
800176c8  8c630000  lw   v1,0(v1)          ; **a0
800176cc  (nop)
800176d0  00431025  or   v0,v0,v1
800176d4  aca20098  sw   v0,0x98(a1)
800176d8  03e00008  jr   ra
800176dc  24020001  li   v0,1
```

C (`src/func_800176B8.c`):

```c
int func_800176B8(unsigned int **a0) {
    D_8009D2F0[0x26] |= **a0;
    return 1;
}
```

## Lever

Same absolute-base split as `func_80017294`: under `-G8` the state object would
be gp-relocated, so `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0` keeps the
`lui`/`lw` base (see `docs/evidence/func-80017294/REPORT.md`).

## Link-level proof

```text
MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0 \
  python3 tools/analysis/era_link_check.py src/func_800176B8.c 0x800176B8 0x28 -O2 -G8
LINK_EXACT
```

## Registration

- Source `src/func_800176B8.c`; YAML `- [0x7EB8, c, func_800176B8]`.
- Profile `era_o2_g8_force_d8009d2f0_absolute`.
