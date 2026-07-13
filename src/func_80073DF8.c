/* Phase 5BX: seventy-third matching C leaf (zero-prefix carve of 645F8.s).
 * VRAM 0x80073DF8 / file 0x645F8 / size 0x18.
 * Original: lui/lw/nop/lhu 0/jr/nop on D_80095674.
 * Halfword pointer deref; plain -O1 matches.
 */
extern unsigned short *D_80095674;

unsigned short func_80073DF8(void) {
    return *D_80095674;
}
