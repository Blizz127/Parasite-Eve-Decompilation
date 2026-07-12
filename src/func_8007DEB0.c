/* Phase 5AY: forty-eighth matching C leaf (mid-65238 carve).
 * VRAM 0x8007DEB0 / file 0x6E6B0 / size 0x10.
 * Original: lui $v0,%hi(D_8009B4AC); lw $v0,%lo(D_8009B4AC)($v0);
 *           jr $ra; nop
 * 32-bit global getter; plain Phase 4J -O1 emits the exact four words.
 */
extern int D_8009B4AC;

int func_8007DEB0(void) {
    return D_8009B4AC;
}
