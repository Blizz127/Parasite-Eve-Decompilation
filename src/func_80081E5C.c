extern int D_8009B708;

int func_80081E5C(int value) {
    register int *ptr asm("$3") = &D_8009B708;
    int old = *ptr;

    *ptr = value;
    return old;
}
