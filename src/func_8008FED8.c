/* Clear one flag bit, set another status bit, and zero a halfword.
 * VRAM 0x8008FED8 / file 0x806D8 / size 0x24. */
void func_8008FED8(int a0) {
    register int v0 asm("$2");
    register int v1 asm("$3");

    v0 = *(int *)(a0 + 0x38);
    v1 = *(int *)(a0 + 0xF4);
    *(short *)(a0 + 0xE8) = 0;
    v0 &= -2;
    v1 |= 0x10;
    *(int *)(a0 + 0x38) = v0;
    *(int *)(a0 + 0xF4) = v1;
}
