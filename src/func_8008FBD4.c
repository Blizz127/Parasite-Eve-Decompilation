/* VRAM 0x8008FBD4 / file 0x803D4 / size 0x28.
 * Advance a cursor and store the fetched byte sign-extended into +0xDE. */
void func_8008FBD4(unsigned char *obj) {
    unsigned char *cursor = *(unsigned char **)obj;
    *(unsigned char **)obj = cursor + 1;
    *(short *)(obj + 0xDE) = (signed char)*cursor;
}
