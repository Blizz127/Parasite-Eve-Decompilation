/* VRAM 0x80042964 / file 0x33164 / size 0x28.
 * Twin of func_80042770 without the final mask: D_800A0EDE[i*1048]. */
extern unsigned char D_800A0EDE[];

int func_80042964(int index) {
    return D_800A0EDE[index * 1048];
}
