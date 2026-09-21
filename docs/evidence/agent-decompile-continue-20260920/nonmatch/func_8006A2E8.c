/* VRAM 0x8006A2E8 / file 0x5AAE8 / size 0x30.
 * Bounded (value < 0x10) triple store of the low 16/8 bits; returns 0. */
extern unsigned short D_800BCE9E;
extern unsigned short D_800BCE8A;
extern unsigned char D_800B0DB1;

int func_8006A2E8(int unused, unsigned int value) {
    if (value < 0x10) {
        D_800BCE9E = value;
        D_800BCE8A = value;
        D_800B0DB1 = value;
    }
    return 0;
}
