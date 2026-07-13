/* Phase 5DA: one-hundred-second matching C leaf.
 * VRAM 0x80077A28 / file 0x68228 / size 0x24.
 * byte memset (fill): countdown do-while filling bytes; register asm forces
 * retail $v0/$v1 allocation under plain -O1.
 */
void func_80077A28(unsigned char *arg0, unsigned char arg1, int arg2) {
    register int v0 asm("$2");
    register int v1 asm("$3");

    if (arg2 == 0) {
        return;
    }
    v0 = arg2 - 1;
    v1 = -1;
    do {
        *arg0 = arg1;
        v0 = v0 - 1;
        arg0 = arg0 + 1;
    } while (v0 != v1);
}
