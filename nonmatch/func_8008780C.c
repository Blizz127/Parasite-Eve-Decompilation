/* VRAM 0x8008780C / file 0x7800C / size 0x30.
 * SPU voice register at 0x1F801C08 + (voice << 4): bit15 from a2>>2, bits
 * 8-14 from a1, bits 0-7 preserved from the existing low byte. */
void func_8008780C(int voice, int a1, int a2) {
    unsigned char *p = (unsigned char *)(0x1F801C08 + (voice << 4));
    unsigned char low = *p;
    *(unsigned short *)p =
        (unsigned short)(low | ((a2 >> 2) << 15) | (a1 << 8));
}
