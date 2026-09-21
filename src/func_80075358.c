extern unsigned int *D_80095744;

void func_80075358(unsigned char *a0) {
    unsigned int *v0;
    unsigned char s1;

    v0 = D_80095744;
    s1 = a0[3];
    (*(void (**)(int))(v0 + 15))(0);
    v0 = D_80095744;
    (*(void (**)(int, int))(v0 + 5))((int)(a0 + 4), s1);
}
