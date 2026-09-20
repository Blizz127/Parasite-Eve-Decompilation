/* VRAM 0x80087798 / file 0x77F98 / size 0x24. */
void func_80087798(int voice, int a1, int a2) {
    unsigned int base = 0x1F801C00;
    unsigned short *p = (unsigned short *)(base + (voice << 4));
    p[0] = (unsigned short)(a1 & 0x7FFF);
    p[1] = (unsigned short)(a2 & 0x7FFF);
}
