/* Phase 5BI: fifty-eighth matching C leaf (mid-70428 carve).
 * VRAM 0x8007FC54 / file 0x70454 / size 0x10.
 * Original: lui $v0,%hi(D_8009B56C); lbu $v0,%lo(D_8009B56C)($v0);
 *           jr $ra; nop
 */
extern unsigned char D_8009B56C;

unsigned char func_8007FC54(void) {
    return D_8009B56C;
}
