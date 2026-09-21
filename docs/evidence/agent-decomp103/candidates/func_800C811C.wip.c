/* VRAM 0x800C811C / file 0xB891C / size 0x40. */
extern unsigned short D_800E2348;
extern unsigned short D_800E234C;
extern char *D_8009D254;
void func_800C811C(int arg0, int arg1, short *arg2) {
    unsigned short v0;
    unsigned short v1;
    char *p;
    v0 = D_800E2348;
    p = D_8009D254;
    arg2[4] = v0;
    arg2[5] = *(short *)(p + 0x2E);
    v1 = D_800E234C;
    arg2[2] = 0x7F;
    arg2[3] = 0x224;
    arg2[6] = v1;
}
