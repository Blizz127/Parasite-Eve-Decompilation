/* VRAM 0x800702DC / file 0x60ADC / size 0x118.
 * First-half battle-slot scrub loop (indices 0..0xA): for each slot whose
 * active flag func_8006FC18(...) is clear, zero the 0x72 marker and wipe the
 * record header; a set flag aborts the walk and its value is returned.
 * era -O2 -G0. */
extern int D_800942E8;
extern int D_800942E4;
extern unsigned int D_800B0CD8;
extern unsigned char D_800E0EF0[];

int func_8006FC18();

int func_800702DC(void) {
    int temp_v0;
    int result;
    int i;
    unsigned char *base;
    int off_a;
    int off_b;
    unsigned char *p;
    unsigned int j;
    unsigned char *q;

    i = 0;
    base = D_800E0EF0;
    off_a = 0;
    off_b = -0xB84;
loop:
    temp_v0 = func_8006FC18(i, 0, 1);
    if (temp_v0 != 0) {
        result = temp_v0;
        goto out;
    }
    if ((unsigned int)i < 0x16) {
        if ((unsigned int)i >= 0xB) {
            p = (unsigned char *)(D_800942E8 + off_b);
        } else {
            p = (unsigned char *)(D_800942E4 + off_a);
        }
        if (p[1] == 0x72) {
            j = 0x6C;
            q = base + 0x1B0;
            do {
                *(int *)q = 0;
                j++;
                q += 4;
            } while (j < 0x73);
            D_800B0CD8 &= 0xFFFEFFFF;
        }
        p[0] = 0;
        p[1] = 0xFF;
        p[2] = 0xFF;
        p[3] = 0xFF;
        *(int *)(p + 4) = 0;
        *(int *)(p + 8) = 0;
    }
    off_a += 0xA0C;
    i++;
    off_b += 0x10C;
    if (i < 0xB) goto loop;
    result = temp_v0;
out:
    return result;
}
