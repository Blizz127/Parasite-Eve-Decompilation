extern void func_80084B78(void);
extern void func_80084C4C(void);
extern void func_80084F8C(void);

extern void (*D_8009B73C)(void);
extern void (*D_8009B740)(void);
extern void (*D_8009B744)(void);

void func_80084B44(void) {
    D_8009B73C = func_80084B78;
    D_8009B740 = func_80084F8C;
    D_8009B744 = func_80084C4C;
}
