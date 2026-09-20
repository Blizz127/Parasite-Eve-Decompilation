/* VRAM 0x8008FCE4 / file 0x804E4 / size 0x2C.
 * Post-increment a byte cursor and add its sign-extended byte to +0xE0. */
void func_8008FCE4(unsigned char **a0) {
    unsigned char *p = *a0;
    *a0 = p + 1;
    *(unsigned short *)((char *)a0 + 0xE0) =
        (unsigned short)(*(unsigned short *)((char *)a0 + 0xE0) + (signed char)*p);
}
