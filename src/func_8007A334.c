/* Phase 5BL: sixty-first matching C leaf (mid-65238 carve).
 * VRAM 0x8007A334 / file 0x6AB34 / size 0x10.
 * Original: lui $v0,%hi(D_8009AFD4); lbu $v0,%lo(D_8009AFD4)($v0);
 *           jr $ra; nop
 */
extern unsigned char D_8009AFD4;

unsigned char func_8007A334(void) {
    return D_8009AFD4;
}
