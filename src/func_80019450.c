/* VRAM 0x80019450 / file 0x9C50 / size 0x34.
 * (2*sign16(+0x224)) * **a0 >> 16 stored at short (+0x1B4)+0x14. */
extern unsigned char *D_8009D2F0;
int func_80019450(int **a0) {
    unsigned char *p = D_8009D2F0;
    int idx = **a0;
    int v = ((int)(short)*(short *)(p + 0x224) << 1) * idx;
    *(short *)(*(int *)(p + 0x1B4) + 0x14) = (short)(v >> 16);
    return 1;
}
