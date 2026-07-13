/* Phase 5CB: seventy-seventh matching C leaf.
 * VRAM 0x80077B64 / file 0x68364 / size 0x14.
 * GPU packet header setter a0[3]=0x4, a0[7]=0x20; plain -O1 matches.
 */
void func_80077B64(unsigned char *arg0) {
    arg0[3] = 4;
    arg0[7] = 32;
}
