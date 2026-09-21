/* VRAM 0x8004BC80 / file 0x3C480 / size 0x34.
 * Allocate command 0x16 and install func_8004BCB4 at +0x30. */
extern int func_80062D2C(int a0, int a1, int a2, int a3);
extern void func_8004BCB4(void);
int func_8004BC80(void) {
    int obj = func_80062D2C(0x16, 0, 0, 0);
    *(void **)(obj + 0x30) = (void *)func_8004BCB4;
    return obj;
}
