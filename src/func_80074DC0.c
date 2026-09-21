/* VRAM 0x80074DC0 / file 0x655C0 / size 0x68.
 * When the byte D_8009574E is at least 2, call the global hook D_80095748
 * with D_80011884 and arg0; then always call the +0x3C method of
 * *D_80095744 with arg0.  era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT. */
extern unsigned char D_8009574E;
extern unsigned char *D_80095744;
extern unsigned char D_80011884[];
extern int (*D_80095748)();

int func_80074DC0(int arg0) {
    if (D_8009574E >= 2) {
        D_80095748(D_80011884, arg0);
    }
    return (*(int (**)(int))(D_80095744 + 0x3C))(arg0);
}
