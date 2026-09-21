extern unsigned char D_8009574E;
extern int (*D_80095748)(char *, void *, int);
extern unsigned int *D_80095744;
extern char D_80011910;
extern char D_8009580C;
extern char D_800957F8;
unsigned char *func_800752AC(unsigned char *a0, int a1) {
    unsigned char *s0 = a0;
    int s1 = a1;
    unsigned int *v0;
    if (D_8009574E >= 2) D_80095748(&D_80011910, s0, s1);
    v0 = D_80095744;
    (*(void (**)(void *, int))(v0 + 11))(s0, s1);
    *(int *)&D_8009580C = ((unsigned int)&D_800957F8 & 0xFFFFFFu) | 0x4000000u;
    *(int *)s0 = (unsigned int)&D_8009580C & 0xFFFFFFu;
    return s0;
}
