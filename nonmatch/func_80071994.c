/* VRAM 0x80071994 / file 0x62194 / size 0x30. */
int func_80071994(int a0) {
    int v = a0 + 8;
    if (*(int *)(a0 + 4) & 8)
        v += *(int *)(a0 + 8);
    return v + 0xC;
}
