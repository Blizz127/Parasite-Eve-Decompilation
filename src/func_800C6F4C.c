/* VRAM 0x800C6F4C / file 0xB774C / size 0x54 (21 words).
 *
 * Reverse of func_800C6EF8: copies `count` 32-bit words out of the
 * 0x800E2370 palette scratch into the mesh's colour block.  era -O2 -G0.
 *
 * Same cc1 frame artifact and counter pin as func_800C6EF8. */
extern int D_800E2370[];

void func_800C6F4C(unsigned char *arg0) {
    int tmp[2];
    unsigned char *var_a3;
    unsigned char *var_a2;
    register int var_a1 asm("$5");
    unsigned short var_v0;
    int temp_v0;
    int count;
    (void)tmp;
    var_a3 = (unsigned char *)D_800E2370;
    var_a1 = 0;
    var_v0 = *(unsigned short *)(arg0 + 8);
    count = *(unsigned short *)(arg0 + 10);
    var_a2 = arg0 + var_v0;
    if (count > 0) {
        do {
            var_a1 += 1;
            temp_v0 = *(int *)var_a3;
            var_a3 += 4;
            *(int *)var_a2 = temp_v0;
            var_v0 = var_a1 < *(unsigned short *)(arg0 + 10);
            var_a2 += 4;
        } while (var_v0 != 0);
    }
}
