/* VRAM 0x80017E68 / file 0x8668 / size 0x34. */
extern int *D_8009D2F0;

int func_80017E68(int **arg0) {
    int mask = *(int *)((char *)D_8009D2F0 + 0x98);
    int value = *arg0[0];
    *arg0[1] = ((mask & value) == value);
    return 1;
}
