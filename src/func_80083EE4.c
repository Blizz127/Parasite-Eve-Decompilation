/* Phase 5CN: eighty-ninth matching C leaf.
 * VRAM 0x80083EE4 / file 0x746E4 / size 0x14.
 * Init a0[0x36]=0x4B, *(a0+0x2C)=0, a0[0x35]=0; plain -O1 matches.
 */
void func_80083EE4(unsigned char *arg0) {
    arg0[0x36] = 0x4B;
    *(unsigned int *)(arg0 + 0x2C) = 0;
    arg0[0x35] = 0;
}
