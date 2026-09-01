extern int D_800B8AB0;

int func_800824B4(int value) {
    register int *ptr asm("$3") = &D_800B8AB0;
    int old = *ptr;

    *ptr = value;
    return old;
}
