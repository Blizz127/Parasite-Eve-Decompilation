/* VRAM 0x80083578 / file 0x73D78 / size 0x28.
 * Busy-poll until bit 1 of the field at +0x4 of pointer global D_8009B788
 * is set (retail reloads the global pointer once, then spins on the field). */
extern unsigned char *D_8009B788;

void func_80083578(void) {
    while ((*(volatile unsigned short *)(D_8009B788 + 4) & 2) == 0)
        ;
}
