/* VRAM 0x8001784C / file 0x804C / size 0x30.
 * Two +0x18/+0x1C field copies out of the gp-relative D_8009D300 record
 * pointer; era -O2 -G8 keeps the loads gp-relative (0x590($gp)). */
extern unsigned int *D_8009D300;

int func_8001784C(int **arg0) {
    *(int *)arg0[0] = *(int *)((char *)D_8009D300 + 0x18);
    *(int *)arg0[1] = *(int *)((char *)D_8009D300 + 0x1C);
    return 1;
}
