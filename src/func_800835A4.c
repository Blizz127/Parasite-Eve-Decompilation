/* Phase 5CK: eighty-sixth matching C leaf.
 * VRAM 0x800835A4 / file 0x73DA4 / size 0xC.
 * Store a1 to *(a0+0x28), a2 to a0[0x34]; plain -O1 matches.
 */
void func_800835A4(unsigned char *arg0, unsigned int arg1, unsigned char arg2) {
    *(unsigned int *)(arg0 + 0x28) = arg1;
    arg0[0x34] = arg2;
}
