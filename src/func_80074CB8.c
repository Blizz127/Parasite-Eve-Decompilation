/* Phase 5BO: sixty-fourth matching C leaf (mid-65238 carve).
 * VRAM 0x80074CB8 / file 0x654B8 / size 0x10.
 * Original: lui $v0,%hi(D_8009574E); lbu $v0,%lo(D_8009574E)($v0);
 *           jr $ra; nop
 */
extern unsigned char D_8009574E;

unsigned char func_80074CB8(void) {
    return D_8009574E;
}
