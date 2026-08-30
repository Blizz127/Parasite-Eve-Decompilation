extern unsigned int D_800BD02C;
extern unsigned int D_800A76D0;

void func_80084FC4(unsigned int limit) {
    unsigned short count = *(volatile unsigned short *)0x1F801120;

    D_800BD02C = limit;
    D_800A76D0 = count;
}
