extern unsigned char *D_8009566C;
void func_80073D88(void) {
    unsigned int (*f)() = *(unsigned int (**)())(D_8009566C + 0x10);
    f();
}
