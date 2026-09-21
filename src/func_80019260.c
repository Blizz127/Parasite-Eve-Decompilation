/* VRAM 0x80019260 / file 0x9A60 / size 0x38. */
extern int *D_8009D2F0;
extern void func_8003E0D0(void *arg);

int func_80019260(void) {
    func_8003E0D0((char *)D_8009D2F0 + 0x1B4);
    *(int *)((char *)D_8009D2F0 + 0x18C) = 0;
    return 1;
}
