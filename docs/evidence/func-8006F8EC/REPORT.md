# `func_8006F8EC` — dispatch a command record's offset-0xC handler

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as
matching-C leaf. Linked at its retail VMA with its three referenced symbols
defined, the object `.text` is **byte-identical** to retail (`LINK_EXACT`).

## Function hood and retail span

- File span `[0x600EC,0x601F0)` = `0x104` bytes = 65 words.
- VRAM span `[0x8006F8EC,0x8006F9F0)`.
- Carved directly after matched `func_8006F820`; followed by `func_8006F9F0`.
- Uses the `D_800942E4`/`D_800942E8` arenas and the `D_800942E0` handler
  table (same family as `func_8006F224`/`func_8006F2C4`/`func_8006F820`).

## Semantics (from retail bytes)

```text
8006f8ec  27bdffe8  addiu sp,sp,-0x18
8006f8f0  2c820016  sltiu v0,a0,0x16      ; id < 0x16 ?
8006f8f4  14400003  bnez  v0,0x8006F904
8006f8f8  afbf0010  sw    ra,0x10(sp)      ; delay
8006f8fc  0801be78  j     0x8006F9E0
8006f900  2402fff0  li    v0,-0x10         ; delay: return -0x10
.L8006F904:
8006f904  2c82000b  sltiu v0,a0,0xB        ; id < 0xB ?
8006f908  1440000A  bnez  v0,0x8006F934
8006f90c  00041080  sll   v0,a0,2          ; delay
   (id>=0xB) ... / lw v0,D_800942E8         ; stride 0x10C
.L8006F934:
   (id<0xB)  ... / lw v1,D_800942E4         ; stride 0xA0C
8006f954  addu a0,v1,v0                     ; a0 = record
8006f958  lbu v0,0(a0)
8006f960  addiu v0,v0,-1
8006f964  sltiu v0,v0,2                     ; state in {1,2} ?
8006f968  beqz v0,0x8006F9E0
8006f96c  00001021  move  v0,zero           ; delay: return 0
8006f970  lbu a1,1(a0)                      ; handler byte
8006f978  sltiu v0,a1,0xC0
8006f97c  bnez  v0,0x8006F98C
8006f980  2ca20055  sltiu v0,a1,0x55         ; delay
8006f984  0801be78  j     0x8006F9E0
8006f988  2402ffef  li    v0,-0x11           ; delay
.L8006F98C:
8006f98c  14400002  bnez  v0,0x8006F998
8006f990  00000000  nop
8006f994  24050055  li    a1,0x55            ; clamp
.L8006F998:
8006f998  3c038009  lui   v1,0x8009
8006f99c  8c6342e0  lw    v1,D_800942E0
8006f9a0  00051080  sll   v0,a1,2 / addu / lw v0,0(v0)
8006f9b4  14400003  bnez  v0,0x8006F9C0
8006f9bc  0801be78  j     0x8006F9E0 / delay li v0,-0x12   (null handler)
.L8006F9C0:
8006f9c0  8c42000c  lw    v0,0xC(v0)         ; handler[0xC]
8006f9c8  14400003  bnez  v0,0x8006F9D8
8006f9d0  0801be78  j     0x8006F9E0 / delay li v0,-1
.L8006F9D8:
8006f9d8  0040f809  jalr  v0                 ; fn(record)
8006f9dc  00000000  nop
.L8006F9E0:
8006f9e0  8fbf0010  lw    ra,0x10(sp)
8006f9e4  27bd0018  addiu sp,sp,0x18
8006f9e8  03e00008  jr    ra
8006f9ec  00000000  nop
```

C shape (`src/func_8006F8EC.c`):

```c
int func_8006F8EC(int idx) {
    unsigned char *p;
    int h;
    int (*fn)(unsigned char *);
    if ((unsigned int)idx >= 0x16)
        return -0x10;
    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;
    if ((unsigned int)(p[0] - 1) >= 2)
        return 0;
    h = p[1];
    if ((unsigned int)h >= 0xC0)
        return -0x11;
    if ((unsigned int)h >= 0x55)
        h = 0x55;
    if (D_800942E0[h] == 0)
        return -0x12;
    fn = *(int (**)(unsigned char *))((unsigned char *)D_800942E0[h] + 0xC);
    if (fn != 0)
        return fn(p);
    return -1;
}
```

## Levers

1. **`h` must be `int`, not `unsigned char`.** With a char local cc1 emits
   `andi v1,a1,0xff` before the `sltiu`/`slti` chain (10 extra bytes);
   retail compares the `lbu` result directly.
2. **Pointer-global arenas** (`lui`/`lw`) as in the sibling leaves.
3. **`fn != 0` (not `fn == 0`) for the last test** so cc1 places the `jalr`
   call as the branch target and the `return -1` as fallthrough, exactly as
   retail's `bnez v0,0x8006F9D8`.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
  tools/analysis/era_leaf_match.sh src/func_8006F8EC.c 0x8006F8EC 0x104 -O2 -G0
```

Result: `MISMATCHES=11`, size `0x110` (12-byte link pad) vs ROM `0x104`. All
mismatches are link-time relocation fields (`lui`/`lw` of the three symbols
and the local `j` targets):

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x8006F8FC` | `0801be78` | `0800003d` | R_MIPS_26 local `.L8006F9E0` |
| `0x8006F924` | `3c028009` | `3c020000` | R_MIPS_HI16 `D_800942E8` |
| `0x8006F928` | `8c4242e8` | `8c420000` | R_MIPS_LO16 `D_800942E8` |
| `0x8006F92C` | `0801be55` | `0800001a` | R_MIPS_26 local `.L8006F954` |
| `0x8006F948` | `3c038009` | `3c030000` | R_MIPS_HI16 `D_800942E4` |
| `0x8006F94C` | `8c6342e4` | `8c630000` | R_MIPS_LO16 `D_800942E4` |
| `0x8006F984` | `0801be78` | `0800003d` | R_MIPS_26 local |
| `0x8006F998` | `3c038009` | `3c030000` | R_MIPS_HI16 `D_800942E0` |
| `0x8006F99C` | `8c6342e0` | `8c630000` | R_MIPS_LO16 `D_800942E0` |
| `0x8006F9B8` | `0801be78` | `0800003d` | R_MIPS_26 local |
| `0x8006F9D0` | `0801be78` | `0800003d` | R_MIPS_26 local |

Link-level proof (assemble, link at `0x8006F8EC` with
`--defsym D_800942E4=0x800942E4 --defsym D_800942E8=0x800942E8
--defsym D_800942E0=0x800942E0`, compare the linked `.text` word-for-word
with the ROM):

```text
linked .text 272 bytes, target 0x104, word mismatches=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006F8EC.c`; YAML carve
  `- [0x600EC, c, func_8006F8EC]` (asm resumes at `0x601F0`). Default
  `era_o2_g0` profile.
- `python3 tools/build/disc1_plan.py --check` → 860 spans
  (570 c, 288 asm, 2 rodata), geometry `0x1EE000`.
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
