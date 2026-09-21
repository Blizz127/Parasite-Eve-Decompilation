/* Vtable dispatch: call the callback at object+8.
 * VRAM 0x80073CC4 / file 0x644C4 / size 0x30. */
extern unsigned char *D_8009566C;

void func_80073CC4(void) {
    ((void (*)(void)) * (unsigned int *)(D_8009566C + 8))();
}
