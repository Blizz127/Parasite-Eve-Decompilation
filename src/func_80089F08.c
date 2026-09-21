/*
 * func_80089F08 — copy one strided table halfword into *a1.
 *
 * VRAM 0x80089F08 / file 0x7A108 / size 0x1C (7 words).
 * ROM: asm/disc1/7A510.s.
 *
 * v = D_8009B3FC + a0 * 0x10; *a1 = *(u16 *)(v + 0xC). D_8009B3FC is a
 * pointer global and the index base is computed into $a0 so the indexed load
 * keeps the 0xC displacement on the base register.
 */
extern unsigned short *D_8009B3FC;

void func_80089F08(unsigned int a0, unsigned short *a1) {
    a0 = (a0 << 4) + (unsigned int)D_8009B3FC;
    *a1 = *(unsigned short *)(a0 + 0xC);
}
