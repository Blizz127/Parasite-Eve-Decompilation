/* Phase 5BH: fifty-seventh matching C leaf (mid-70428 carve).
 * VRAM 0x8007FC44 / file 0x70444 / size 0x10.
 * Original: lui $v0,%hi(D_8009B587); lbu $v0,%lo(D_8009B587)($v0);
 *           jr $ra; nop
 */
extern unsigned char D_8009B587;

unsigned char func_8007FC44(void) {
    return D_8009B587;
}
