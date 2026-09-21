extern unsigned int D_8009AFDC[];
extern unsigned char D_800119CC[];
unsigned int func_8007A400(unsigned int a0) {
    unsigned int i = a0 & 0xFF;
    if (i >= 0x1C) return (unsigned int)D_800119CC;
    return D_8009AFDC[i];
}
