/*
 * func_80038910 — seed a fixed gp-relative record from six arguments.
 * VRAM 0x80038910 / file 0x29110 / size 0x30 (12 words).
 *
 * Args 5 and 6 arrive on the stack (0x10/0x14($sp)); the first four are in
 * $a0..$a3. Destinations are gp-relative: halfwords at 0x148/0x14C/0x150 and
 * bytes at 0x144/0x154/0x158/0x15C. era -O2 -G8.
 */
extern unsigned short D_8009CEB8;
extern unsigned short D_8009CEBC;
extern unsigned short D_8009CEC0;
extern unsigned char D_8009CEB4;
extern unsigned char D_8009CEC4;
extern unsigned char D_8009CEC8;
extern unsigned char D_8009CECC;

void func_80038910(unsigned int a0, unsigned int a1, unsigned int a2,
                   unsigned int a3, unsigned char a4, unsigned char a5) {
    D_8009CEB8 = a0;
    D_8009CEB4 = 1;
    D_8009CEBC = a1;
    D_8009CEC0 = a2;
    D_8009CEC4 = a3;
    D_8009CEC8 = a4;
    D_8009CECC = a5;
}
