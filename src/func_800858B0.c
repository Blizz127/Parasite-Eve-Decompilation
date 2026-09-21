extern unsigned char *D_8009B7D0;
int func_800858B0(int a0) {
    register int u asm("$3");
    register unsigned char *base asm("$2");
    u = a0 & 0xFFFF;
    if (u >= 3) return 0;
    base = D_8009B7D0;
    return *(unsigned short *)(base + u * 16);
}
