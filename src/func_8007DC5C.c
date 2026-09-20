/* VRAM 0x8007DC5C / file 0x6E45C / size 0x28.
 * RMW: clear bits 24-27 of the word at the pointer global, set 0x20000000. */
extern unsigned int *D_8009B410;

void func_8007DC5C(void) {
    *D_8009B410 = (*D_8009B410 & 0xF0FFFFFFu) | 0x20000000u;
}
