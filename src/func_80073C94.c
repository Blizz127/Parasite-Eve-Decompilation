/* Vtable dispatch: call the callback at object+0xC.
 * VRAM 0x80073C94 / file 0x64494 / size 0x30. */
extern unsigned char *D_8009566C;

void func_80073C94(void) {
    ((void (*)(void)) * (unsigned int *)(D_8009566C + 0xC))();
}
