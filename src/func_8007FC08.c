/* Phase 5BE: fifty-fourth matching C leaf (mid-6FF88 carve).
 * VRAM 0x8007FC08 / file 0x70408 / size 0x10.
 * Original: lui $v0,%hi(D_8009B580); lbu $v0,%lo(D_8009B580)($v0);
 *           jr $ra; nop
 * Unsigned-byte global getter; plain Phase 4J -O1 emits the exact four words.
 */
extern unsigned char D_8009B580;

unsigned char func_8007FC08(void) {
    return D_8009B580;
}
