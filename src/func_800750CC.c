/* VRAM 0x800750CC / file 0x658CC / size 0x60.
 * Register D_800118E0 with func_80074E28, then dispatch through the +0x8
 * method of *D_80095744 with the +0x1C handle field as the first argument
 * and (arg0, 8, arg1).  era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT. */
extern unsigned char *D_80095744;
extern unsigned char D_800118E0[];
extern void func_80074E28();

int func_800750CC(int arg0, int arg1) {
    func_80074E28(D_800118E0, arg0);
    return (*(int (**)(int, int, int, int))(D_80095744 + 8))(*(int *)(D_80095744 + 0x1C), arg0, 8, arg1);
}
