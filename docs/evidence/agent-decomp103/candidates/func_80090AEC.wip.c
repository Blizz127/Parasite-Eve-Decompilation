/* VRAM 0x80090AEC / file 0x812EC / size 0x44. */
extern void func_8009019C(int);
void func_80090AEC(int arg0) {
    unsigned char **pp = (unsigned char **)arg0;
    int v = *(*pp)++;
    v = v ? v + 1 : 0x101;
    *(short *)(arg0 + 0xBA) = v;
    func_8009019C(arg0);
}
