/* VRAM 0x8007C444 / file 0x6CC44 / size 0x34. */
extern char *D_800C0DC8;
void func_8007C444(int arg0, unsigned int arg1) {
    unsigned int i;
    for (i = 0; i < arg1; i++) {
        *(int *)(D_800C0DC8 + ((arg0 + i) * 32)) = 0;
    }
}
