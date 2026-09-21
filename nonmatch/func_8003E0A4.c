/* VRAM 0x8003E0A4 / file 0x2E8A4 / size 0x2C. */
int func_8003E0A4(int a0, int a1, int a2) {
    int mode;
    if (*(unsigned char *)(*(int *)a0 + 2) == 2)
        mode = 3;
    else
        mode = 1;
    *(int *)(a0 + 0x24) = a1;
    *(unsigned short *)(a0 + 0x28) = (unsigned short)mode;
    *(unsigned short *)(a0 + 0x2A) = (unsigned short)a2;
    return mode;
}
