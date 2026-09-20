/* literal base, do-while via if(t5){t5--;goto}, 15 bodies, test on pre-value */
void func_80070D10(void) {
    int *t0 = (int *)0x80070E0Cu;
    int t3, t4, t5;
    t3 = 1; t0[0x10] = t3;
    t3 = 2; t0[0xF] = t3;
    t5 = 0xE;
body:
    t3 = t0[0x10];
    t4 = t0[0xF];
    t0--;
    t0[0xF] = t3 + t4;
    if (t5) {
        t5--;
        goto body;
    }
    *(int *)0x80070E04u = 0x40;
    *(int *)0x80070E08u = 0x10;
}
