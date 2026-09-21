/* VRAM 0x80087798 / file 0x77F98 / size 0x24.
 * Write two 15-bit SPU voice registers at 0x1F801C00 + index*16. */
void func_80087798(int index, int a, int b) {
    volatile unsigned short *p =
        (volatile unsigned short *)(0x1F801C00 + index * 16);
    p[0] = (unsigned short)(a & 0x7FFF);
    p[1] = (unsigned short)(b & 0x7FFF);
}
