/* VRAM 0x800755BC / file 0x65DBC / size 0x34.
 * Forward to func_80071A34 with the D_8009575C table and a 0x5C size, then
 * return arg0; era -O2 -G0 with the epilogue stack restore filled into the
 * `jr $ra` delay slot (MASPSX_FILL_EPILOGUE_DELAY_SLOT=1). */
extern char D_8009575C[];
extern void func_80071A34(int, char *, int);

int func_800755BC(int arg0) {
    func_80071A34(arg0, D_8009575C, 0x5C);
    return arg0;
}
