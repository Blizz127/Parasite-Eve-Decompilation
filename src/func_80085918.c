extern unsigned int *D_8009B7CC;
extern unsigned int D_8009B7D4[];
int func_80085918(unsigned int a0) {
    unsigned int i = a0 & 0xFFFF;
    D_8009B7CC[1] &= ~D_8009B7D4[i];
    return 1;
}
