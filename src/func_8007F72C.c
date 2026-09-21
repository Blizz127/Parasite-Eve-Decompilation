extern int func_8007FBF0(int a0);
extern int func_8007F778(void);

int func_8007F72C(void) {
    int s0 = func_8007FBF0(0);
    if (s0 == 1 && func_8007F778() > 0)
        s0 = 2;
    return s0;
}
