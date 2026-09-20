/* VRAM 0x80075C6C / file 0x6646C / size 0x28.
 * GPU primitive packet prep: byte +3 = 2, packed word +4, zero +8. */
void func_80075C6C(unsigned char *a0, int a1) {
    a0[3] = 2;
    *(int *)(a0 + 4) = a1 ? 0xE6000001 : 0xE6000000;
    *(int *)(a0 + 8) = 0;
}
