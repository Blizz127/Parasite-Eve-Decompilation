extern void func_80080F98(void);
extern void func_80081D74(void (*f)(void), int x);
void func_80080F64(int a0) {
    if ((a0 & 0xFF) == 2) func_80081D74(func_80080F98, -1);
}
