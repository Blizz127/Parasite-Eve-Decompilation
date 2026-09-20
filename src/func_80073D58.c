/* Vtable dispatch: call the callback at object+0x14.
 * VRAM 0x80073D58 / file 0x64558 / size 0x30. */
extern unsigned char *D_8009566C;

void func_80073D58(void) {
    ((void (*)(void)) * (unsigned int *)(D_8009566C + 0x14))();
}
