/* VRAM 0x80087864 / file 0x78064 / size 0x28. */
void func_80087864(int voice, int value) {
    unsigned short *p = (unsigned short *)(0x1F801C08 + (voice << 4));
    unsigned short v = *p;
    *p = (unsigned short)((v & 0xFFF0) | value);
}
