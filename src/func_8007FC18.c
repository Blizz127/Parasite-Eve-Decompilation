/* Phase 5BF: fifty-fifth matching C leaf (mid-70418 carve).
 * VRAM 0x8007FC18 / file 0x70418 / size 0x10.
 * Original: lui $v0,%hi(D_8009B581); lbu $v0,%lo(D_8009B581)($v0);
 *           jr $ra; nop
 * Unsigned-byte global getter; plain Phase 4J -O1 emits the exact four words.
 */
extern unsigned char D_8009B581;

unsigned char func_8007FC18(void) {
    return D_8009B581;
}
