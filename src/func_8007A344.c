/* Phase 5BM: sixty-second matching C leaf (mid-65238 carve).
 * VRAM 0x8007A344 / file 0x6AB44 / size 0x10.
 * Original: lui $v0,%hi(D_8009AFD5); lbu $v0,%lo(D_8009AFD5)($v0);
 *           jr $ra; nop
 */
extern unsigned char D_8009AFD5;

unsigned char func_8007A344(void) {
    return D_8009AFD5;
}
