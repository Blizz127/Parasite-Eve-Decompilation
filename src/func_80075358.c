/* VRAM 0x80075358 / file 0x65B58 / size 0x5C.
 * Two indirect calls through the *D_80095744 method table: first the +0x3C
 * method with argument 0, then the +0x14 method with (arg0 + 4) and the
 * byte arg0[3] saved before the first call.  era -O2 -G0 with the stack
 * restore in the `jr $ra` slot (MASPSX_FILL_EPILOGUE_DELAY_SLOT). */
extern unsigned char *D_80095744;

int func_80075358(unsigned char *arg0) {
    unsigned char s1 = arg0[3];
    (*(int (**)(int))(D_80095744 + 0x3C))(0);
    return (*(int (**)(void *, int))(D_80095744 + 0x14))(arg0 + 4, s1);
}
