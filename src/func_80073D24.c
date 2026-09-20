/* VRAM 0x80073D24 / file 0x64524 / size 0x34. */
extern int *D_8009566C;

void func_80073D24(int arg0) {
    void (*callback)(int, int) = (void (*)(int, int))*(void **)((char *)D_8009566C + 0x14);
    callback(4, arg0);
}
