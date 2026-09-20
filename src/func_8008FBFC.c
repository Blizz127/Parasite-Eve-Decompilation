/* VRAM 0x8008FBFC / file 0x803FC / size 0x2C.
 * Post-increment a byte cursor and add its sign-extended byte to +0xDE. */
void func_8008FBFC(unsigned char **a0) {
    unsigned char *p = *a0;
    *a0 = p + 1;
    *(unsigned short *)((char *)a0 + 0xDE) =
        (unsigned short)(*(unsigned short *)((char *)a0 + 0xDE) + (signed char)*p);
}
