/* Vtable dispatch: call the callback at object+0x10.
 * VRAM 0x80073D88 / file 0x64588 / size 0x30. */
extern unsigned char *D_8009566C;

void func_80073D88(void) {
    ((void (*)(void)) * (unsigned int *)(D_8009566C + 0x10))();
}
