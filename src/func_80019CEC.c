/* VRAM 0x80019CEC / file 0xA4EC / size 0x38. */
extern int *D_8009D2F0;

int func_80019CEC(void) {
    *(int *)((char *)D_8009D2F0 + 0x28) = *(short *)((char *)D_8009D2F0 + 0x21C) << 16;
    *(int *)((char *)D_8009D2F0 + 0x2C) = *(short *)((char *)D_8009D2F0 + 0x21E) << 16;
    *(int *)((char *)D_8009D2F0 + 0x30) = *(short *)((char *)D_8009D2F0 + 0x220) << 16;
    return 1;
}
