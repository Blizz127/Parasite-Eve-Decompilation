extern char D_800A5B70[];

int func_80084AE8(void *arg0) {
    register int index asm("$5") = 0;
    register int slot asm("$6") = 0x10;
    register char *entry asm("$3") = D_800A5B70;

    while (index < 2) {
        if (arg0 == entry) {
            return slot;
        }
        slot += 0x10;
        index++;
        entry += 0xF0;
    }
    return 0xFF;
}
