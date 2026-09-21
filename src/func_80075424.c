extern unsigned char D_8009574E;
extern int (*D_80095748)(char *, void *);
extern unsigned int *D_80095744;
extern char D_8001193C;
extern void func_80075EE0(void *, void *);
extern int func_80071A34(void *, void *, int);
void *func_80075424(void *a0) {
    unsigned char *s2 = &D_8009574E;
    void *s1 = a0;
    char *s0;
    unsigned int *v1;
    if (*s2 >= 2) D_80095748(&D_8001193C, s1);
    s0 = (char *)s1 + 0x1C;
    func_80075EE0(s0, s1);
    ((int *)s1)[7] |= 0xFFFFFF;
    v1 = D_80095744;
    (*(void (**)(int, int, int, int))(v1 + 2))(v1[6], (int)s0, 0x40, 0);
    func_80071A34(s2 + 0xE, s1, 0x5C);
    return s1;
}
