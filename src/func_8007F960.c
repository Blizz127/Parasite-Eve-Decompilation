extern void (*D_800B8AB8)(int);

void func_8007F960(int arg0) {
    if (D_800B8AB8 != 0) {
        D_800B8AB8(arg0 & 0xFF);
    }
}
