/* VRAM 0x80090B5C / file 0x8135C / size 0x44. */
extern void func_800902AC(int);
void func_80090B5C(int arg0) {
    unsigned char **pp = (unsigned char **)arg0;
    int v = *(*pp)++;
    v = v ? v + 1 : 0x101;
    *(short *)(arg0 + 0xBC) = v;
    func_800902AC(arg0);
}
