/* VRAM 0x80071964 / file 0x62164 / size 0x30. */
int func_80071964(int a0) {
    int v = a0 + 8;
    if (*(int *)(a0 + 4) & 8)
        v += *(int *)(a0 + 8);
    return v + 4;
}
