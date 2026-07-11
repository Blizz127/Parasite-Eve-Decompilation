/* Phase 5AX: forty-seventh matching C leaf (mid-645F8 carve).
 * VRAM 0x80074A28 / file 0x65228 / size 0x10.
 * Original: lui $v0,%hi(D_800956EC); lw $v0,%lo(D_800956EC)($v0);
 *           jr $ra; nop
 * 32-bit global getter; plain Phase 4J -O1 emits the exact four words.
 *
 * Clean re-derivation from 7902dd2 (5AW / 46 leaves) after a prior 5AX
 * attempt's safety claim (empty diff vs c3a8424) was found unverifiable.
 */
extern int D_800956EC;

int func_80074A28(void) {
    return D_800956EC;
}
