extern unsigned char D_8009574E;
extern int (*D_80095748)(char *, void *, void *);
extern unsigned int *D_80095744;
extern char D_80011954;
extern void func_80075EE0(void *, void *);
extern int func_80071A34(void *, void *, int);
void func_800754E4(unsigned int a0, void *a1) {
    unsigned char *s3 = &D_8009574E;
    unsigned int s2 = a0;
    void *s1 = a1;
    char *s0;
    unsigned int *v1;
    if (*s3 >= 2) D_80095748(&D_80011954, (void *)s2, s1);
    s0 = (char *)s1 + 0x1C;
    func_80075EE0(s0, s1);
    ((int *)s1)[7] = (((int *)s1)[7] & 0xFF000000) | (s2 & 0xFFFFFF);
    v1 = D_80095744;
    (*(void (**)(int, int, int, int))(v1 + 2))(v1[6], (int)s0, 0x40, 0);
    func_80071A34(s3 + 0xE, s1, 0x5C);
}
