/* Phase 5BP: sixty-fifth matching C leaf (mid-33338 carve).
 * VRAM 0x80042BC8 / file 0x333C8 / size 0x10.
 * Original: lui $v0,%hi(D_800A1870); lw $v0,%lo(D_800A1870)($v0);
 *           jr $ra; sltu $v0,$zero,$v0
 * Boolean nonzero test of a 32-bit global; plain -O1 matches.
 */
extern int D_800A1870;

int func_80042BC8(void) {
    return D_800A1870 != 0;
}
