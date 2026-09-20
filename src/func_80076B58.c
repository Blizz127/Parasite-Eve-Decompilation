extern int *D_80095854;
extern int *D_80095850;
int func_80076B58(int *a0, int a1) {
    int a2 = a1 - 1;
    *D_80095854 = 0x04000000;
    if (a1 != 0) {
        do {
            int v = *a0;
            a0++;
            *D_80095850 = v;
            a2--;
        } while (a2 != -1);
    }
    return 0;
}
