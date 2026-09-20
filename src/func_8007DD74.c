/* VRAM 0x8007DD74 / file 0x6E574 / size 0x34.
 * Two-call forwarding wrapper (a0, then a0/0x3F/0); era -O2 -G0 with the
 * epilogue stack restore filled into the `jr $ra` delay slot. */
extern void func_8007DDC4(int);
extern void func_8007DDB4(int, int, int);

void func_8007DD74(int arg0) {
    func_8007DDC4(arg0);
    func_8007DDB4(arg0, 0x3F, 0);
}
