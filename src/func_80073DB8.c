/* Vtable dispatch: call the callback at object+0x18.
 * VRAM 0x80073DB8 / file 0x645B8 / size 0x30. */
extern unsigned char *D_8009566C;

void func_80073DB8(void) {
    ((void (*)(void)) * (unsigned int *)(D_8009566C + 0x18))();
}
