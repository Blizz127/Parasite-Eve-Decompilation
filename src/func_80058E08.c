/* VRAM 0x80058E08 / file 0x49608 / size 0x3C.
 * Signed-index clamp: for 0 <= i < D_8009D044 return the sign-extended
 * 16-bit D_800A1E00[i], else 0.  Same shape as func_80057ED8; count is the
 * gp-relative scalar at 0x2D4($gp).  era -O2 -G8 + ERA_ASPSX_VER=2.30. */
extern int D_8009D044;
extern short D_800A1E00[];

int func_80058E08(int arg0) {
    if (arg0 >= 0 && arg0 < D_8009D044) {
        return D_800A1E00[arg0];
    }
    return 0;
}
