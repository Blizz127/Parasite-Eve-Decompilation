/* Phase 5BB: fifty-first matching C leaf (mid-704BC carve).
 * VRAM 0x80080940 / file 0x71140 / size 0x10.
 * Original: lui $v0,%hi(D_8009B554); lw $v0,%lo(D_8009B554)($v0);
 *           jr $ra; nop
 * 32-bit global getter; plain Phase 4J -O1 emits the exact four words.
 */
extern int D_8009B554;

int func_80080940(void) {
    return D_8009B554;
}
