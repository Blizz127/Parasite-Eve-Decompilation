/*
 * func_80051CC4 — resource command-state initializer.
 *
 * VRAM 0x80051CC4 / file 0x424C4 / size 0x134 (77 words). Frame 0x20,
 * saves $ra/$s0, calls func_80052F0C/func_80052E30/func_8005332C/
 * func_8005218C.
 *
 * saved = func_80052F0C(); func_80052E30(0); D_8009D018 = 0; clear the
 * seven words 0x800A1B30..0x800A1B4B downward with counter $a0; record =
 * func_8005332C(*(signed char*)D_800C0E22); if record != 0, scan
 * record[0x14] bytes at record+0x15 and dispatch on (byte & 0x1F) through
 * the shared rodata pool table jtbl_800111F8 (cases 8..15; 8/9/10 share a
 * `1 << (cmd-8)` body, 14 is a no-op).
 *
 * The scan loop reloads record[0x14] every iteration (retail does too: the
 * body's global stores may alias `record`), and retail hoists the table
 * base out of the loop (`lui/addiu $t6`), then does
 * `addu $v0,$v0,$t6; lw $v0,0($v0); jr $v0`. The clear counter and scan
 * index share one variable, which is why retail keeps both in $a0.
 *
 * D_8009D018 is small-data in retail (`sw $zero,0x2A8($gp)`, gp base
 * 0x8009CD70); D_800A1B30/D_800A1B48/D_800C0E22 are absolute, so they are
 * declared as arrays / a non-small type.
 *
 * Build: era -O2 -G8 + MASPSX_THREE_WORD_SYMBOL_STORE=1 +
 * MASPSX_DISPATCH_FOLD=jtbl_800111F8.
 * ROM: asm/disc1/420A8.s @ file 0x424C4, 77 words (0x134 bytes).
 */

extern int func_80052F0C(void);
extern void func_80052E30(int kind);
extern void *func_8005332C(int source);
extern void func_8005218C(void);

extern int D_8009D018;                  /* gp+0x2A8, small data */
extern signed char D_800C0E22[];        /* absolute; element is `lb` */
extern unsigned int D_800A1B30[7];      /* 0x800A1B30..0x800A1B4B */
extern unsigned int D_800A1B48[];       /* same block +0x18 */

void func_80051CC4(void) {
    int saved = func_80052F0C();
    unsigned char *record;
    unsigned int *p;
    int i;

    func_80052E30(0);

    i = 6;
    p = D_800A1B48;
    D_8009D018 = 0;
    do {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);

    record = (unsigned char *)func_8005332C(D_800C0E22[0]);
    if (record != 0) {
        i = 0;
        while (i < record[0x14]) {
            unsigned char cmd = *(record + i + 0x15) & 0x1F;
            switch (cmd) {
            case 8:
            case 9:
            case 10:
                D_8009D018 = 1 << (cmd - 8);
                break;
            case 11:
                D_800A1B30[0] = 3;
                break;
            case 12:
                D_800A1B30[1] = 2;
                break;
            case 13:
                D_800A1B30[5] = -2;
                break;
            case 14:
                break;
            case 15:
                D_800A1B30[1] = -2;
                break;
            }
            i++;
        }
    }

    func_8005218C();
    func_80052E30(0);
    func_80052E30(saved);
}
