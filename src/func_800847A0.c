/* Phase 5CO: ninetieth matching C leaf.
 * VRAM 0x800847A0 / file 0x74FA0 / size 0x10.
 * Move a0[0x36] to a0[0x37] and clear a0[0x36]; plain -O1 matches.
 */
void func_800847A0(unsigned char *arg0) {
    unsigned char v0 = arg0[0x36];
    arg0[0x36] = 0;
    arg0[0x37] = v0;
}
