extern void func_800375E0(int a0, int a1, short *a2);
int func_80017410(short **a0) {
    short buf[8];
    buf[0] = -1;
    func_800375E0(**a0, 0, buf);
    return 1;
}
