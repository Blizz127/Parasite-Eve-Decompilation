extern unsigned char D_8009574E;
extern int (*D_80095748)(char *, void *, int);
extern char D_800118F8;
extern char D_8009580C;
extern char D_800957F8;
unsigned char *func_800751E4(unsigned char *a0, int a1) {
    register unsigned char *s0 asm("$16") = a0;
    register int s1 asm("$17") = a1;
    int n;
    if (D_8009574E >= 2) D_80095748(&D_800118F8, s0, s1);
    n = s1 - 1;
    while (n != 0) {
        unsigned char *next = s0 + 4;
        unsigned int lo = 0xFFFFFFu, hi = 0xFF000000u;
        n--;
        s0[3] = 0;
        *(int *)s0 = (*(int *)s0 & hi) | ((unsigned int)next & lo);
        s0 = next;
    }
    *(int *)&D_8009580C = ((unsigned int)&D_800957F8 & 0xFFFFFFu) | 0x4000000u;
    *(int *)s0 = (unsigned int)&D_8009580C & 0xFFFFFFu;
    return s0;
}
