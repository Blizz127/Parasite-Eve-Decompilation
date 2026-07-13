/* Phase 5CL: eighty-seventh matching C leaf.
 * VRAM 0x800835B0 / file 0x73DB0 / size 0x10.
 * Store a1/a2/a3 into a0 fields 0x36/0x2C/0x35; plain -O1 matches.
 */
void func_800835B0(unsigned char *arg0, unsigned char arg1, unsigned int arg2, unsigned char arg3) {
    arg0[0x36] = arg1;
    *(unsigned int *)(arg0 + 0x2C) = arg2;
    arg0[0x35] = arg3;
}
