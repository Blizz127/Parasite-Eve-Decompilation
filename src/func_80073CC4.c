extern unsigned char *D_8009566C;
void func_80073CC4(void) {
    unsigned int (*f)() = *(unsigned int (**)())(D_8009566C + 8);
    f();
}
