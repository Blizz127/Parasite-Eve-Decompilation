/* Phase 5BU: seventieth matching C leaf (mid-654C8 carve).
 * VRAM 0x8007633C / file 0x66B3C / size 0x18.
 * Original: lui/lw/nop/lw 0/jr/nop on D_80095854.
 * Pointer double-deref getter; plain -O1 matches.
 */
extern int *D_80095854;

int func_8007633C(void) {
    return *D_80095854;
}
