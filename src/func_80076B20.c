extern unsigned int *D_80095854;
extern unsigned char D_800A3348[];
void func_80076B20(unsigned int a0) {
    *D_80095854 = a0;
    D_800A3348[a0 >> 24] = a0;
}
