/* VRAM 0x8008783C / file 0x7803C / size 0x28. */
void func_8008783C(int voice, int value) {
    unsigned short *p = (unsigned short *)(0x1F801C08 + (voice << 4));
    unsigned short v = *p;
    *p = (unsigned short)((v & 0xFF0F) | (value << 4));
}
