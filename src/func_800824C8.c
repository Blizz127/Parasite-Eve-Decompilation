extern int D_800B8AB4;

int func_800824C8(int value) {
    register int *ptr asm("$3") = &D_800B8AB4;
    int old = *ptr;

    *ptr = value;
    return old;
}
