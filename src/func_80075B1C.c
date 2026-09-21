/* VRAM 0x80075B1C / file 0x6631C / size 0x30.
 * Call the +0x38 method of *D_80095744 and return its sign bit. */
extern unsigned char *D_80095744;
int func_80075B1C(void) {
    int (*fn)(void) = *(int (**)(void))(D_80095744 + 0x38);
    unsigned int r = (unsigned int)fn();
    return (int)(r >> 31);
}
