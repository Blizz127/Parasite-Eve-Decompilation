/* VRAM 0x80087798 / file 0x77F98 / size 0x24.
 * SPU voice register pair at 0x1F801C00 + (voice << 4): store two 15-bit
 * masked fields into +0 and +2. */
void func_80087798(int voice, int a1, int a2) {
    unsigned short *p = (unsigned short *)(0x1F801C00 + (voice << 4));
    p[0] = (unsigned short)(a1 & 0x7FFF);
    p[1] = (unsigned short)(a2 & 0x7FFF);
}
