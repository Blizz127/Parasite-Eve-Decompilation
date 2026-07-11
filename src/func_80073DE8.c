/* Phase 5AU: forty-fourth matching C leaf (mid-4C4B0 carve).
 * VRAM 0x80073DE8 / file 0x645E8 / size 0x10.
 * Original: lui $v0,%hi(D_800945E6); lhu $v0,%lo(D_800945E6)($v0); jr $ra; nop
 * 16-bit unsigned global getter (returns D_800945E6).
 * Same load-return schedule as 5AR/5AS: plain Phase 4J -O1 matches because
 * the MIPS-I load delay keeps lhu out of the jr delay slot.
 */
extern unsigned short D_800945E6;

unsigned short func_80073DE8(void) {
    return D_800945E6;
}
