/* VRAM 0x800199CC / file 0xA1CC / size 0x2C. */
extern unsigned char *D_8009D2F0;

int func_800199CC(int a0) {
    int value = *(int *)(*(int *)a0);
    unsigned char *r = D_8009D2F0;
    unsigned short h = *(unsigned short *)(r + 0x250);
    *(unsigned short *)(r + 0x250) = (unsigned short)(h | 0x10);
    *(unsigned short *)(r + 0x24E) = (unsigned short)value;
    return 1;
}
