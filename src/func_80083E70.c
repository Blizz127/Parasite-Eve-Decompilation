/* Phase 5CM: eighty-eighth matching C leaf.
 * VRAM 0x80083E70 / file 0x74670 / size 0x14.
 * Init a0[0x36]=0x45, *(a0+0x2C)=0, a0[0x35]=0; plain -O1 matches.
 */
void func_80083E70(unsigned char *arg0) {
    arg0[0x36] = 0x45;
    *(unsigned int *)(arg0 + 0x2C) = 0;
    arg0[0x35] = 0;
}
