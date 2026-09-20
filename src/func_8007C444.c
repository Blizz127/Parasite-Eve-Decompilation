extern int *D_800C0DC8;
void func_8007C444(int a0, unsigned int a1) {
    unsigned int i = 0;
    if (a1 == 0)
        return;
    do {
        int off = i + a0;
        i++;
        *(int *)((char *)D_800C0DC8 + (off << 5)) = 0;
    } while (i < a1);
}
