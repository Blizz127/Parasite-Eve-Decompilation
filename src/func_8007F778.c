/* Phase 5AZ: forty-ninth matching C leaf (mid-6E6C0 carve).
 * VRAM 0x8007F778 / file 0x6FF78 / size 0x10.
 * Original: lui $v0,%hi(D_800A3608); lw $v0,%lo(D_800A3608)($v0);
 *           jr $ra; nop
 * 32-bit global getter; plain Phase 4J -O1 emits the exact four words.
 */
extern int D_800A3608;

int func_8007F778(void) {
    return D_800A3608;
}
