extern unsigned char *D_8009566C;
void func_80073DB8(void) {
    unsigned int (*f)() = *(unsigned int (**)())(D_8009566C + 0x18);
    f();
}
