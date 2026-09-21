/* VRAM 0x80057ED8 / file 0x486D8 / size 0x3C.
 * Signed-index clamp: for 0 <= i < D_8009D078 return the sign-extended
 * 16-bit D_800A1FD4[i], else 0.  The count is the gp-relative scalar at
 * 0x308($gp) (D_8009D078); the halfword table is an incomplete array so it
 * stays absolute (lui/addu/lh %lo).  era -O2 -G8 + ERA_ASPSX_VER=2.30
 * (retail's three-word lui/addu/op-%lo indexed-load expansion). */
extern int D_8009D078;
extern short D_800A1FD4[];

int func_80057ED8(int arg0) {
    if (arg0 >= 0 && arg0 < D_8009D078) {
        return D_800A1FD4[arg0];
    }
    return 0;
}
