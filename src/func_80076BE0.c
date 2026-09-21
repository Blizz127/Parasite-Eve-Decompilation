extern unsigned int *D_80095854;
extern unsigned int *D_80095850;
unsigned int func_80076BE0(unsigned int a0) {
    *D_80095854 = a0 | 0x10000000;
    return *D_80095850 & 0xFFFFFF;
}
