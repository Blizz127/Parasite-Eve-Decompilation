/* func_80075C44 - VRAM 0x80075C44, file 0x66444, size 0x28. */
void func_80075C44(unsigned char *a0, int a1, unsigned int a2) {
    unsigned int v1 = 0xE6000000u;
    *(volatile unsigned char *)(a0 + 3) = 2;
    if (a1 != 0)
        v1 = 0xE6000002u;
    *(volatile unsigned int *)(a0 + 4) = v1 | (a2 != 0);
    *(volatile unsigned int *)(a0 + 8) = 0;
}
