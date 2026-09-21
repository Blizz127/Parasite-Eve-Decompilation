/* VRAM 0x800659C8 / file 0x561C8 / size 0x30.
 * 16-byte-stride record field store through pointer global D_800B1624. */
extern unsigned char *D_800B1624;

int func_800659C8(int index, unsigned int value) {
    *(unsigned short *)(D_800B1624 + *(int *)(D_800B1624 + 0x10)
                        + (index << 4) + 8) = (unsigned short)(value >> 8);
    return 0;
}
