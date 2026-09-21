/* VRAM 0x80042770 / file 0x32F70 / size 0x28.
 * Byte-table flag read, stride 1048 = ((i*33)*4 - i)*8. */
extern unsigned char D_800A0ED4[];

int func_80042770(int index) {
    return D_800A0ED4[index * 1048] & 1;
}
