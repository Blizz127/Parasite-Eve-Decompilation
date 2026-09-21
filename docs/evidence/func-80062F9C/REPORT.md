# func_80062F9C — MATCHED (20 words, LINK_EXACT)

VRAM `0x80062F9C`, file `0x5379C`, span `0x50`. Carve: splits the former
`[0x5373C, asm]` span into prefix `0x260`, C `0x50`, resume `0x1AC`
(`func_80062FEC` follows at file `0x537EC`).

```c
extern unsigned char D_800A22E0[];
extern unsigned int *D_8009D158;
extern unsigned int D_8009D15C;
extern unsigned int D_8009D154;

void func_80062F9C(void) {
    unsigned char *p = D_800A22E0;
    unsigned char *end = D_800A22E0 + 0xD80;

    while (p < end) {
        *(unsigned int *)p = (unsigned int)(p + 0x90);
        p += 0x90;
    }
    *(unsigned int *)0x800A2FD0 = 0;
    D_8009D158 = (unsigned int *)D_800A22E0;
    D_8009D15C = 0;
    D_8009D154 = 0;
}
```

**Semantics.** Threads the 24-slot pool at `D_800A22E0` (stride `0x90`,
extent `0x90*0x18 = 0xD80`) into a singly-linked free list: each slot's
`+0x00` word points at the next slot, and the final slot `D_800A2FD0`
(`= D_800A22E0 + 23*0x90`) is nulled by an explicit overwrite after the loop.
Three gp globals are seeded: `D_8009D158` (`+0x3E8`) to the pool base, and
`D_8009D15C` (`+0x3EC`, tail/count) and `D_8009D154` (`+0x3E4`, head) to
zero. This matches the independently derived geometry in
`pc_port/tools/pool_oracle.py` (`POOL_BASE 0x800A22E0`, `POOL_STRIDE 0x90`,
`POOL_SLOTS 24`, `GA_HEAD/GA_TAIL/GA_AUX`).

**Levers.**
1. **Pointer arithmetic must be over `unsigned char *` so the `+0x90` stride
   is in bytes.** An `unsigned int *` loop scales the literal by 4
   (`addiu $v0,$v1,0x90` becomes `$v1+0x90*4`) and folds the entry guard,
   giving 18 mismatches.
2. **`unsigned int` compare keeps `sltu`** (both bounds are byte pointers).
3. **Retail rematerializes the pool base** in `$v0` before the tail stores
   (rather than reusing `$v1`); the byte-pointer form above reproduces this.
4. **maspsx patch 3** (`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`) fills the `jr`
   delay slot (`nop` in place of the moved `addiu`) — the leaf is framed by
   generic fill with no net stack change.

**Build profile.** New `era_o2_g8_fill_epilogue_delay_slot`:
era `-O2 -G8` (the three `D_8009D15x` stores are gp-relative) plus
`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`. First use of the combined profile.

**Gate.**

```
tools/analysis/check_leaf.sh func_80062F9C 0x80062F9C 0x50 -O2 -G8
  → linked .text 80 bytes, target 0x50, word mismatches=0, nonzero_pad=0
  → LINK_EXACT
  → disc1_preflight: PASS (deep, 786 c / 347 asm / 2 rodata)
```
