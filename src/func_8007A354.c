/* Phase 5BN: sixty-third matching C leaf (mid-65238 carve).
 * VRAM 0x8007A354 / file 0x6AB54 / size 0xC.
 * Original: lui $v0,%hi(D_8009AFD0); jr $ra;
 *           addiu $v0,$v0,%lo(D_8009AFD0)
 * Address-of global; plain -O1 schedules addiu in the jr delay slot.
 */
extern unsigned char D_8009AFD0;

unsigned char *func_8007A354(void) {
    return &D_8009AFD0;
}
