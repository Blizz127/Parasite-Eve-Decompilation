void func_8007E594(unsigned char *a0) {
    int i;
    unsigned char *q;

    *(unsigned int *)a0 = 0;
    a0[4] = 0;
    i = 3;
    q = a0 + 3;
    do {
        q[5] = 0;
        i--;
        q--;
    } while (i >= 0);
    *(unsigned int *)(a0 + 0xC) = 0;
    *(unsigned int *)(a0 + 0x10) = 0;
    *(unsigned int *)(a0 + 0x14) = 0;
}
