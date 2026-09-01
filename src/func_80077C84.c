void func_80077C84(void *arg0, int arg1, int arg2, int arg3) {
    register unsigned int command asm("$3");
    register unsigned int flags asm("$2");

    *(unsigned char *)((unsigned char *)arg0 + 3) = 1;
    command = 0xE1000000;
    if (arg2 != 0) {
        command |= 0x200;
    }
    flags = arg3 & 0x9FF;
    if (arg1 != 0) {
        flags |= 0x400;
    }
    *(unsigned int *)((unsigned char *)arg0 + 4) = command | flags;
}
