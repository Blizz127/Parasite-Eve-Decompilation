/*
 * func_800144FC — some multi-state machine (state at D_800B0CD8+0xF4).
 *
 * VRAM 0x800144FC / file 0x4CFC / size 0x198 (102 words). Frame 0x20,
 * saves $s0/$s1/$ra. Calls func_80042EDC, func_8006D60C, func_80042F20,
 * func_8006914C, func_80029810.
 *
 * s1 = arg; s0 = D_800B0CD8.  Loop: state = s0[0xF4]; if (state >= 0x3C)
 * return 1; dispatch through the shared rodata pool table jtbl_800100A0
 * (state 0 -> set bit 0x800000, state 55..59 -> transition cases, states
 * 1..54 -> return 1).  Two cases return through the shared "zero" block
 * (D_8009D090 -= 0xC; *(D_8009D300 + 0x10) = 1; return 0).
 *
 * D_8009D090 (gp+0x90) and D_8009D300 (gp+0x590, a pointer) are small data
 * (gp base 0x8009CD70); D_800B0CD8 and D_8009D1A0 are absolute, declared as
 * arrays so cc1 emits lui/lw instead of gp-relative.
 *
 * Build: era -O2 -G8 + MASPSX_THREE_WORD_SYMBOL_STORE=1 +
 * MASPSX_DISPATCH_FOLD=jtbl_800100A0.
 * ROM: asm/disc1/38B4.s @ file 0x4CFC, 102 words (0x198 bytes).
 */

typedef struct {
    /* 0x00 */ int f00;
    /* 0x04 */ unsigned char pad04[0x0A];
    /* 0x0E */ unsigned char f0E;
    /* 0x0F */ unsigned char pad0F[0xE5];
    /* 0xF4 */ unsigned char fF4;
} Blob;

extern unsigned char D_800B0CD8[];
extern unsigned char D_8009D1A0[];
extern int D_8009D090;                  /* gp+0x90, small data */
extern int *D_8009D300;                 /* gp+0x590, small data */

extern void func_80042EDC(void);
extern int func_8006D60C(int arg);
extern void func_80042F20(void);
extern int func_8006914C(int arg);
extern void func_80029810(int arg);

int func_800144FC(int *arg) {
    int *s1 = arg;
    Blob *s0 = (Blob *)D_800B0CD8;

    for (;;) {
        unsigned char state = s0->fF4;
        if (state >= 0x3C)
            return 1;

        switch (state) {
        case 0:
            if ((s0->f0E & 3) == 0)
                s0->fF4 = 0x37;
            s0->f00 |= 0x800000;
            goto zero_out;
        case 55:
            if (!(s0->f00 & 0x400000))
                func_80042EDC();
            s0->fF4 = 0x38;
            continue;
        case 56:
            if (func_8006D60C(1) == 1)
                goto zero_out;
            if (!(s0->f00 & 0x400000))
                func_80042F20();
            s0->fF4 = 0x39;
            continue;
        case 57:
            if (func_8006914C(1) == 1)
                goto zero_out;
            s0->fF4 = 0x3A;
            continue;
        case 58:
            *(int *)D_8009D1A0 |= 2;
            func_80029810(*s1);
            s0->fF4 = 0x3B;
            goto zero_out;
        case 59:
            if ((s0->f0E & 3) != 0)
                goto zero_out;
            s0->fF4 = 0;
            s0->f00 &= 0xFF7FFFFF;
            return 1;
        default:
            return 1;
        }
    }

zero_out:
    D_8009D090 -= 0xC;
    *(int *)((char *)D_8009D300 + 0x10) = 1;
    return 0;
}
