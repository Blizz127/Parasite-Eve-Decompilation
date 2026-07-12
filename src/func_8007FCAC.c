/* Phase 5BA: fiftieth matching C leaf (mid-6FF88 carve).
 * VRAM 0x8007FCAC / file 0x704AC / size 0x10.
 * Original: lui $v0,%hi(D_8009B590); lw $v0,%lo(D_8009B590)($v0);
 *           jr $ra; nop
 * 32-bit global getter; plain Phase 4J -O1 emits the exact four words.
 */
extern int D_8009B590;

int func_8007FCAC(void) {
    return D_8009B590;
}
