# `func_80086FF8` / `func_80087024` — D_800CCD80 command then `func_8008CBA8`

Outcome: **MATCHED** as a twin pair on the first natural C phrasing under
era `-O2 -G0` (YAML default). Matching-C leaves 562 and 563.

## Function hood

| symbol | file | VRAM | size | next |
|---|---|---|---|---|
| `func_80086FF8` | `[0x777F8,0x77824)` | `[0x80086FF8,0x80087024)` | 11 words | `func_80087024` |
| `func_80087024` | `[0x77824,0x77850)` | `[0x80087024,0x80087050)` | 11 words | `func_80087050` (`bne a0,zero`) |

Both end in canonical `jr ra; nop`. Preceding `0x777F4` is the delay-slot
nop of the previous function. Direct `jal` encodings:

- `func_80086FF8` (`0C021BFE`): 8 sites — `8006A274 8006A5CC 8006D2A0 8006D4A4 8006D84C 8006DA18 8006E890 8006F0A0`
- `func_80087024` (`0C021C09`): 7 sites — `800164CC 8003F354 8003F6D4 8006A27C 8006A5D4 8006B274 8006D65C`

## Retail body (86FF8; 87024 is identical except `li v0,0xF1`)

```text
80086ff8  27bdffe8  addiu sp,sp,-24
80086ffc  240200f0  li    v0,0xF0
80087000  afbf0010  sw    ra,16(sp)
80087004  3c01800c  lui   at,0x800C
80087008  ac22cd80  sw    v0,-0x3280(at)  # D_800CCD80
8008700c  0c0232ea  jal   func_8008CBA8
80087010  00000000  nop
80087014  8fbf0010  lw    ra,16(sp)
80087018  27bd0018  addiu sp,sp,24
8008701c  03e00008  jr    ra
80087020  00000000  nop
```

Semantics: `D_800CCD80 = 0xF0` or `0xF1`, then `func_8008CBA8()`. No
return value consumed.

## Single-leaf objects

```text
AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  OBJCOPY=mipsel-linux-gnu-objcopy \
  tools/analysis/era_leaf_match.sh src/func_80086FF8.c 0x80086FF8 0x2C -O2 -G0
# same for src/func_80087024.c 0x80087024 0x2C
```

Instruction words match retail except link-time placeholders:
`lui/sw` HI16/LO16 for `D_800CCD80` (`3c010000/ac220000` vs
`3c01800c/ac22cd80`) and `jal` `0c000000` vs `0c0232ea`. GNU as pad is
one trailing `nop`.

YAML mid-`765E8` carve: prefix `0x1210`, two C leaves `0x2C` each, resume
`[0x77850, asm]` to existing `func_800870E0` at `0x778E0`.
