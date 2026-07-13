/* Phase 5BK: sixtieth matching C leaf (mid-65238 carve).
 * VRAM 0x8007A324 / file 0x6AB24 / size 0x10.
 * Original: lui $v0,%hi(D_8009AFC4); lbu $v0,%lo(D_8009AFC4)($v0);
 *           jr $ra; nop
 */
extern unsigned char D_8009AFC4;

unsigned char func_8007A324(void) {
    return D_8009AFC4;
}
