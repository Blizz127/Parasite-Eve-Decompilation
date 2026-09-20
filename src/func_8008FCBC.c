/* VRAM 0x8008FCBC / file 0x804BC / size 0x28.
 * Advance a cursor and store the fetched byte sign-extended into +0xE0. */
void func_8008FCBC(unsigned char *obj) {
    unsigned char *cursor = *(unsigned char **)obj;
    *(unsigned char **)obj = cursor + 1;
    *(short *)(obj + 0xE0) = (signed char)*cursor;
}
