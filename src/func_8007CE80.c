/* VRAM 0x8007CE80 / file 0x6D680 / size 0x2C.
 * Word copy of `count` elements; retail rotates to a bottom-tested loop. */
void func_8007CE80(unsigned int *dst, unsigned int *src, unsigned int count) {
    unsigned int i = 0;
    if (count != 0) {
        do {
            *dst = *src;
            src++;
            i++;
            dst++;
        } while (i < count);
    }
}
