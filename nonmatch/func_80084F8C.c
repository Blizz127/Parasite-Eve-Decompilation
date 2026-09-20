/* VRAM 0x80084F8C / file 0x7578C / size 0x2C. */
int func_80084F8C(int a0) {
    if (*(unsigned short *)(a0 + 0xE6) != 0) {
        if (*(unsigned char *)(a0 + 0x46) == 0xFF)
            return 0;
    }
    return 1;
}
