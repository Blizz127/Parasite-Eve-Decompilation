/* Vtable dispatch: call the callback at object+4.
 * VRAM 0x80073CF4 / file 0x644F4 / size 0x30. */
extern unsigned char *D_8009566C;

void func_80073CF4(void) {
    ((void (*)(void)) * (unsigned int *)(D_8009566C + 4))();
}
