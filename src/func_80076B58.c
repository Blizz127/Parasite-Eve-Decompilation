extern unsigned int *D_80095854;
extern unsigned int *D_80095850;
int func_80076B58(unsigned int *a0, int a1) {
    int c = a1 - 1;
    *D_80095854 = 0x4000000;
    if (a1 != 0) {
        do {
            unsigned int v = *a0;
            a0++;
            *D_80095850 = v;
            c--;
        } while (c != -1);
    }
    return 0;
}
