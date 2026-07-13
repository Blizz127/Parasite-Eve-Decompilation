/* Phase 5BG: fifty-sixth matching C leaf (mid-70428 carve).
 * VRAM 0x8007FC34 / file 0x70434 / size 0x10.
 * Original: lui $v0,%hi(D_8009B586); lbu $v0,%lo(D_8009B586)($v0);
 *           jr $ra; nop
 */
extern unsigned char D_8009B586;

unsigned char func_8007FC34(void) {
    return D_8009B586;
}
