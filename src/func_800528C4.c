/* VRAM 0x800528C4 / file 0x430C4 / size 0x2C. 12-byte-stride table store; value = a1*60. */
extern int D_800A76A4[];

void func_800528C4(int index, int value) {
    D_800A76A4[index * 3] = value * 60;
}
