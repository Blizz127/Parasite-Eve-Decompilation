extern short D_8009409C[];
extern short D_8009509C[];
extern short D_8009589C[];

int func_80077DC4(int a0) {
    if (a0 < 0)
        a0 = -a0;
    a0 &= 0xFFF;
    if (a0 < 0x801) {
        if (a0 < 0x401)
            return D_8009589C[0x400 - a0];
        return -D_8009509C[a0];
    }
    if (a0 < 0xC01)
        return -D_8009589C[0xC00 - a0];
    return D_8009409C[a0];
}
