/* VRAM 0x800C6EF8 / file 0xB76F8 / size 0x54 (21 words).
 *
 * Copies `count` 32-bit words from the mesh's colour block into the
 * 0x800E2370 palette scratch.  era -O2 -G0.
 *
 * The dead 8-byte local `tmp` is a cc1 frame artifact: the retail prologue
 * reserves 8 bytes ("addiu $sp,$sp,-8") with no stack access anywhere, which
 * only appears when cc1 counts an address-taken aggregate that the optimiser
 * later removes.  `(void)tmp;` keeps the frame without emitting code.
 * `register ... asm("$5")` pins the loop counter to retail's $a1; without it
 * cc1 colours the counter $a2 and the source pointer $a1 (swapped). */
extern int D_800E2370[];

void func_800C6EF8(unsigned char *arg0) {
    int tmp[2];
    unsigned char *var_a3;
    unsigned char *var_a2;
    register int var_a1 asm("$5");
    unsigned short var_v0;
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
            *(int *)var_a3 = *(int *)var_a2;
            var_a2 += 4;
            var_v0 = var_a1 < *(unsigned short *)(arg0 + 10);
            var_a3 += 4;
        } while (var_v0 != 0);
    }
}
