extern short D_8009489C[];
extern short D_8009589C[];

int func_80077D30(int a0) {
    if (a0 < 0x801) {
        if (a0 < 0x401)
            return D_8009589C[a0];
        return D_8009589C[0x800 - a0];
    }
    if (a0 < 0xC01)
        return -D_8009489C[a0];
    return -D_8009589C[0x1000 - a0];
}
