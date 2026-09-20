/* VRAM 0x8006599C / file 0x5619C / size 0x2C.
 * 16-byte-stride record field read through the pointer global D_800B1624. */
extern unsigned char *D_800B1624;

short func_8006599C(int index) {
    return *(short *)(D_800B1624 + *(int *)(D_800B1624 + 0x10) + (index << 4) + 6);
}
