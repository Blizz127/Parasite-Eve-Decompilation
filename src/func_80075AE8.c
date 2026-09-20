/* VRAM 0x80075AE8 / file 0x662E8 / size 0x34.
 * Forward to func_80071A34 with the D_800957B8 table and a 0x14 size, then
 * return arg0; era -O2 -G0 with the epilogue stack restore filled into the
 * `jr $ra` delay slot.  Twin of func_800755BC (different table/size). */
extern char D_800957B8[];
extern void func_80071A34(int, char *, int);

int func_80075AE8(int arg0) {
    func_80071A34(arg0, D_800957B8, 0x14);
    return arg0;
}
