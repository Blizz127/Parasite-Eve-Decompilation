/* Phase 5CZ: one-hundred-first matching C leaf.
 * VRAM 0x8007474C / file 0x64F4C / size 0x24.
 * word memset twin: countdown do-while clearing words; register asm forces
 * retail $v0/$v1 allocation under plain -O1.
 */
void func_8007474C(unsigned int *arg0, int arg1) {
    register int v0 asm("$2");
    register int v1 asm("$3");

    if (arg1 == 0) {
        return;
    }
    v0 = arg1 - 1;
    v1 = -1;
    do {
        *arg0 = 0;
        v0 = v0 - 1;
        arg0 = arg0 + 1;
    } while (v0 != v1);
}
