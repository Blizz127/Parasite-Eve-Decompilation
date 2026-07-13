/* Phase 5BJ: fifty-ninth matching C leaf (70428 prefix carve).
 * VRAM 0x8007FC28 / file 0x70428 / size 0xC.
 * Original: lui $v0,%hi(D_8009B582); jr $ra;
 *           addiu $v0,$v0,%lo(D_8009B582)
 * Address-of global; plain -O1 schedules addiu in the jr delay slot.
 */
extern unsigned char D_8009B582;

unsigned char *func_8007FC28(void) {
    return &D_8009B582;
}
