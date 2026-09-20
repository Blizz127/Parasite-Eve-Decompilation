/*
 * func_800CCB6C — twin with an 8/0x1A4 step that also clamps the byte to 0.
 * VRAM 0x800CCB6C / file 0xBD36C / size 0x3C (15 words). era -O2 -G0.
 */
void func_800CCB6C(int a0, unsigned char *a1, unsigned char *a2) {
    *(unsigned char *)(a2 + 3) -= 8;
    *(unsigned short *)(a2 + 4) += 0x1A4;
    if (*(signed char *)(a2 + 3) < 0x14) {
        *(unsigned char *)(a2 + 3) = 0;
        a1[1] = 2;
    }
}
