/* VRAM 0x8007506C / file 0x6586C / size 0x60.
 * Register D_800118D4 with func_80074E28, then dispatch through the +0x8
 * method of *D_80095744 with the +0x20 handle field as the first argument
 * and (arg0, 8, arg1).  era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT. */
extern unsigned char *D_80095744;
extern unsigned char D_800118D4[];
extern void func_80074E28();

int func_8007506C(int arg0, int arg1) {
    func_80074E28(D_800118D4, arg0);
    return (*(int (**)(int, int, int, int))(D_80095744 + 8))(*(int *)(D_80095744 + 0x20), arg0, 8, arg1);
}
