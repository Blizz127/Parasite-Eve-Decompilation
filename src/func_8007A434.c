extern unsigned int D_8009B05C[];
extern unsigned char D_800119CC[];
unsigned int func_8007A434(unsigned int a0) {
    unsigned int i = a0 & 0xFF;
    if (i >= 7) return (unsigned int)D_800119CC;
    return D_8009B05C[i];
}
