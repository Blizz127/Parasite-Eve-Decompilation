/* VRAM 0x800515C0 / file 0x41DC0 / size 0x38.
 * Latch a halfword into *(*D_8009D254)+0xC when present, and into D_800C0E08. */
extern int *D_8009D254;
extern unsigned short D_800C0E08;
void func_800515C0(unsigned short a0) {
    int *p = D_8009D254;
    if (p) {
        int *q = (int *)*p;
        if (q)
            *(unsigned short *)((char *)q + 0xC) = a0;
    }
    D_800C0E08 = a0;
}
