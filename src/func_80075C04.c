/* VRAM 0x80075C04 / file 0x66404 / size 0x40.
 * Two signed halfword args from *arg1, marker + call; era -O2 -G0 with the
 * epilogue stack restore filled into the `jr $ra` delay slot. */
extern int func_800762A0(short, short);

int func_80075C04(char *arg0, short *arg1) {
    int v;

    arg0[3] = 2;
    v = func_800762A0(arg1[0], arg1[1]);
    *(int *)(arg0 + 4) = v;
    *(int *)(arg0 + 8) = 0;
    return v;
}
