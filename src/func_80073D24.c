extern unsigned char *D_8009566C;
void func_80073D24(unsigned int a0) {
    unsigned int (*f)(unsigned int, unsigned int) = *(unsigned int (**)(unsigned int, unsigned int))(D_8009566C + 0x14);
    f(4, a0);
}
