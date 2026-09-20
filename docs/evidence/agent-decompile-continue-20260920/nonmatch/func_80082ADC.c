/* VRAM 0x80082ADC / file 0x732DC / size 0x2C.
 * Install two callbacks and clear the neighbours in D_800A5AB4[-1..2];
 * retail keeps the table base in one register and stores by offset. */
extern void func_80082B70(void);
extern void func_80082B08(void);
extern void (*D_800A5AB4[])(void);

void func_80082ADC(void) {
    void (**p)(void) = D_800A5AB4;
    p[0] = func_80082B70;
    p[1] = func_80082B08;
    p[-1] = 0;
    p[2] = 0;
}
