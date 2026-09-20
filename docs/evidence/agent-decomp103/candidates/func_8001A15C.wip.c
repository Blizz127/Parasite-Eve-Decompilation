/* VRAM 0x8001A15C / file 0xA95C / size 0x4C. */
extern int func_80079FB4(int, int);
int func_8001A15C(int arg0) {
    int v = func_80079FB4(*(int *)*(int **)arg0, *(int *)*(int **)(arg0 + 4));
    *(int *)*(int **)(arg0 + 8) = v;
    return 1;
}
