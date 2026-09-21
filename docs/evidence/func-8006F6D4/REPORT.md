# `func_8006F6D4` — record query + offset-8 handler invoke

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as
matching-C leaf. Linked at its retail VMA with its three referenced symbols
defined, the object `.text` is **byte-identical** to retail (`LINK_EXACT`).

## Function hood and retail span

- File span `[0x5FED4,0x60020)` = `0x14C` bytes = 83 words.
- VRAM span `[0x8006F6D4,0x8006F820)`.
- Carved from the `0x5FB9C` asm span; immediately precedes matched
  `func_8006F820`.
- Same `D_800942E4`/`D_800942E8` arena pair and `D_800942E0` handler table
  as `func_8006F224`/`func_8006F2C4`/`func_8006F820`/`func_8006F8EC`.

## Semantics (from retail bytes)

```text
8006f6d4  27bdffe0  addiu sp,sp,-0x20
8006f6d8  8fa90030  lw    t1,0x30(sp)      ; arg4
8006f6dc  8faa0034  lw    t2,0x34(sp)      ; arg5
8006f6e0  2c820016  sltiu v0,a0,0x16
8006f6e4  14400003  bnez  v0,0x8006F6F4
8006f6e8  afbf0018  sw    ra,0x18(sp)      ; delay
8006f6ec  0801be04  j     0x8006F810
8006f6f0  2402fff6  li    v0,-0xA           ; delay
   ... arena select (D_800942E8 stride 0x10C / D_800942E4 stride 0xA0C)
8006f744  00822021  addu  a0,v1,v0        ; a0 = record
8006f748  90880001  lbu   t0,1(a0)        ; handler byte
8006f750  2d0200c0  sltiu v0,t0,0xC0
8006f754  14400003  bnez  v0,0x8006F764
8006f758  2d020055  sltiu v0,t0,0x55     ; delay
8006f75c  0801be04  j     0x8006F810 / delay li v0,-0xB
.L8006F764:
8006f764  14400002  bnez  v0,0x8006F770
8006f76c  24080055  li    t0,0x55           ; clamp
.L8006F770:
   ... lw v0,D_800942E0[t0]
8006f788  14400003  bnez  v0,0x8006F798    ; handler non-null
8006f790  0801be04  j     0x8006F810 / delay li v0,-0xC
.L8006F798:
8006f798  8c420008  lw    v0,8(v0)          ; handler[8]
8006f7a0  14400003  bnez  v0,0x8006F7B0
8006f7a4  24020001  li    v0,1              ; delay
8006f7a8  0801be04  j     0x8006F810 / delay li v0,-1
.L8006F7B0:
8006f7b0  14a2000c  bne   a1,v0,0x8006F7E4  ; a1 == 1 ?
8006f7b8  14c0000a  bnez  a2,0x8006F7E4    ; a2 == 0 ?
8006f7c0  90820002  lbu   v0,2(a0) / sw v0,0(a3)
8006f7cc  90820003  lbu   v0,3(a0) / sw v0,0(t1)
8006f7d8  8c820004  lw    v0,4(a0) / sw v0,0(t2)
.L8006F7E4:
   ... reload D_800942E0[t0][8]; sw t1,0x10(sp); sw t2,0x14(sp); jalr v0
.L8006F810:
8006f810  8fbf0018  lw    ra,0x18(sp)
8006f814  27bd0020  addiu sp,sp,0x20
8006f818  03e00008  jr    ra
8006f81c  00000000  nop
```

C shape (`src/func_8006F6D4.c`):

```c
int func_8006F6D4(int idx, int a1, int a2, int *a3, int *a4, int *a5) {
    unsigned char *p;
    int h;
    if ((unsigned int)idx >= 0x16) return -0xA;
    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;
    h = p[1];
    if ((unsigned int)h >= 0xC0) return -0xB;
    if ((unsigned int)h >= 0x55) h = 0x55;
    if (D_800942E0[h] == 0) return -0xC;
    if (*(F6D4Fn *)((unsigned char *)D_800942E0[h] + 8) == 0) return -1;
    if (a1 == 1 && a2 == 0) {
        *a3 = p[2]; *a4 = p[3]; *a5 = *(int *)(p + 4);
    }
    return (*(F6D4Fn *)((unsigned char *)D_800942E0[h] + 8))(p, a1, a2, a3, a4, a5);
}
```

## Levers

1. **Guard written `== 0` with the body as fallthrough.** The `!= 0`
   compound form made cc1 emit `beqz` and put the `-1` return in the branch
   target; retail has `bnez v0,0x8006F7B0` with `return -1` as the
   fallthrough, so the `== 0`-guard form is the one that lays out correctly.
2. **No local function-pointer variable.** The offset-8 expression is written
   twice (guard + call) so cc1 re-loads the `D_800942E0[h]` pointer chain
   after the `*a3/*a4/*a5` stores, exactly as retail does (the stores can
   alias the table).
3. **`h` is `int`** (no `andi` re-mask before the `sltiu`/clamp chain).
4. Pointer-global arenas and `D_800942E0` (`lui`/`lw` bases), as in the
   sibling leaves.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
  tools/analysis/era_leaf_match.sh src/func_8006F6D4.c 0x8006F6D4 0x14C -O2 -G0
```

Result: `MISMATCHES=13`, size `0x150` (4-byte link pad) vs ROM `0x14C`. Every
mismatch is a link-time relocation field:

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x8006F6EC` | `0801be04` | `0800004f` | R_MIPS_26 local `.L8006F810` |
| `0x8006F714` | `3c028009` | `3c020000` | R_MIPS_HI16 `D_800942E8` |
| `0x8006F718` | `8c4242e8` | `8c420000` | R_MIPS_LO16 `D_800942E8` |
| `0x8006F71C` | `0801bdd1` | `0800001c` | R_MIPS_26 local `.L8006F744` |
| `0x8006F738` | `3c038009` | `3c030000` | R_MIPS_HI16 `D_800942E4` |
| `0x8006F73C` | `8c6342e4` | `8c630000` | R_MIPS_LO16 `D_800942E4` |
| `0x8006F75C` | `0801be04` | `0800004f` | R_MIPS_26 local |
| `0x8006F770` | `3c038009` | `3c030000` | R_MIPS_HI16 `D_800942E0` |
| `0x8006F774` | `8c6342e0` | `8c630000` | R_MIPS_LO16 `D_800942E0` |
| `0x8006F790` | `0801be04` | `0800004f` | R_MIPS_26 local |
| `0x8006F7A8` | `0801be04` | `0800004f` | R_MIPS_26 local |
| `0x8006F7E4` | `3c038009` | `3c030000` | R_MIPS_HI16 `D_800942E0` |
| `0x8006F7E8` | `8c6342e0` | `8c630000` | R_MIPS_LO16 `D_800942E0` |

Link-level proof (assemble, link at `0x8006F6D4` with
`--defsym D_800942E4=0x800942E4 --defsym D_800942E8=0x800942E8
--defsym D_800942E0=0x800942E0`, compare the linked `.text` word-for-word
with the ROM):

```text
linked .text 336 bytes, target 0x14c, word mismatches=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006F6D4.c`; YAML carve
  `- [0x5FED4, c, func_8006F6D4]` inside the `0x5FB9C` asm span. Default
  `era_o2_g0` profile.
- `python3 tools/build/disc1_plan.py --check` → 861 spans
  (571 c, 288 asm, 2 rodata), geometry `0x1EE000`.
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
