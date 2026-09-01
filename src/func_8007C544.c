/* VRAM 0x8007C544 / file 0x6CD44 / size 0x1C. */
extern unsigned int D_800C0DC0;
extern unsigned int D_800B6918;
extern unsigned int D_800C0DBC;

void func_8007C544(unsigned int first, unsigned int second,
                   unsigned int third) {
    D_800C0DC0 = first;
    D_800B6918 = second;
    D_800C0DBC = third;
}
