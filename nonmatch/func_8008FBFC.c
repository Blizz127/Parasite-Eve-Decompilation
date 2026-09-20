/* VRAM 0x8008FBFC / file 0x803FC / size 0x2C.
 * Bump the byte cursor at +0, then combine the consumed byte (<<24) with the
 * +0xDE halfword. */
void func_8008FBFC(int a0) {
    unsigned char *cursor = *(unsigned char **)a0;
    *(unsigned char **)a0 = cursor + 1;
    unsigned short h = *(unsigned short *)(a0 + 0xDE);
    unsigned int v = (unsigned int)(*(unsigned char *)cursor) << 24;
    *(unsigned int *)(a0 + 0x38) = v | h;
}
