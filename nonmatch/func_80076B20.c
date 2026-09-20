/* VRAM 0x80076B20 / file 0x67320 / size 0x24.
 * Store the word through pointer global D_80095854, then index the byte
 * table D_800A3348 by the value's top byte. */
extern unsigned int *D_80095854;
extern unsigned char D_800A3348[];

void func_80076B20(unsigned int value) {
    unsigned int index = value >> 24;
    *D_80095854 = value;
    D_800A3348[index] = (unsigned char)value;
}
