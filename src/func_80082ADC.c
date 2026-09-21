/*
 * func_80082ADC — install two callbacks and two zero words around D_800A5AB4.
 * VRAM 0x80082ADC / file 0x732DC / size 0x2C (11 words).
 *
 * The barrier holds the arena base in one register so every store addresses
 * through it, matching retail's single `lui/addiu` base materialisation.
 * era -O2 -G0.
 */
extern int D_800A5AB4[];
extern void func_80082B70(void);
extern void func_80082B08(void);
void func_80082ADC(void) {
    int *p = D_800A5AB4;
    __asm__ volatile("" : "=r"(p) : "0"(p));
    p[0] = (int)func_80082B70;
    p[1] = (int)func_80082B08;
    p[-1] = 0;
    p[2] = 0;
}
