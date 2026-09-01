extern unsigned short *D_80095674;

int func_80073E10(unsigned short value) {
    int old = *D_80095674;
    *D_80095674 = value;
    return old;
}
