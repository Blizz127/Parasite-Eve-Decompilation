extern unsigned char *D_8009566C;
void func_80073C94(void) {
    unsigned int (*f)() = *(unsigned int (**)())(D_8009566C + 0xC);
    f();
}
