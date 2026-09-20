/* Clear one flag bit, set another status bit, and zero a halfword.
 * VRAM 0x80090178 / file 0x80978 / size 0x24. */
void func_80090178(int a0) {
    register int v0 asm("$2");
    register int v1 asm("$3");

    v0 = *(int *)(a0 + 0x38);
    v1 = *(int *)(a0 + 0xF4);
    *(short *)(a0 + 0xEC) = 0;
    v0 &= -5;
    v1 |= 0x3;
    *(int *)(a0 + 0x38) = v0;
    *(int *)(a0 + 0xF4) = v1;
}
