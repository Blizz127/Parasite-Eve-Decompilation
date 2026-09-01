extern int D_800B8AB8;

int func_800824DC(int value) {
    register int *ptr asm("$3") = &D_800B8AB8;
    int old = *ptr;

    *ptr = value;
    return old;
}
