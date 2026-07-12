/* Phase 5BC: fifty-second matching C leaf (mid-71150 carve).
 * VRAM 0x800822AC / file 0x72AAC / size 0x10.
 * Original: lui $v0,%hi(D_8009B70C); lw $v0,%lo(D_8009B70C)($v0);
 *           jr $ra; nop
 * 32-bit global getter; plain Phase 4J -O1 emits the exact four words.
 */
extern int D_8009B70C;

int func_800822AC(void) {
    return D_8009B70C;
}
