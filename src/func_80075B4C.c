/* VRAM 0x80075B4C / file 0x6634C / size 0x38.
 * Set a 2-byte marker and call through; era -O2 -G0 with the epilogue stack
 * restore filled into the `jr $ra` delay slot (ASPSX order; maspsx
 * MASPSX_FILL_EPILOGUE_DELAY_SLOT=1). */
extern int func_800762BC(int);

int func_80075B4C(char *arg0, int arg1) {
    int v;

    arg0[3] = 2;
    v = func_800762BC(arg1);
    *(int *)(arg0 + 4) = v;
    *(int *)(arg0 + 8) = 0;
    return v;
}
