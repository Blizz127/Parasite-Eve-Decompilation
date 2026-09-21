/*
 * func_8006F820 — VRAM 0x8006F820, size 0xCC, file 0x60020-0x600EC.
 *
 * Two-arena id-record accessor. a0 selects the arena: 0..0xA use
 * D_800942E4 with stride 0xA0C, 0xB..0x15 use D_800942E8 with stride
 * 0x10C; larger ids return -0xD. The clamped handler byte (arena+1, capped
 * at 0x55) indexes D_800942E0; a null entry returns -0xF. Otherwise the
 * caller either reads the byte (a1 == 0) or writes it via a2. era -O2 -G0.
 */
extern unsigned char *D_800942E4;
extern unsigned char *D_800942E8;
extern unsigned int *D_800942E0;

int func_8006F820(unsigned int a0, unsigned int a1, unsigned int a2) {
    unsigned char *p;
    unsigned int v1;

    if (a0 >= 0x16)
        return -0xD;
    if (a0 >= 0xB)
        p = D_800942E8 + (a0 - 0xB) * 0x10C;
    else
        p = D_800942E4 + a0 * 0xA0C;

    v1 = p[1];
    if (v1 >= 0x55)
        v1 = 0x55;
    if (D_800942E0[v1] == 0)
        return -0xF;

    if (a1 == 0) {
        if (a2 < 6)
            p[0] = a2;
    } else {
        *(unsigned int *)a2 = p[0];
    }
    return p[0];
}
