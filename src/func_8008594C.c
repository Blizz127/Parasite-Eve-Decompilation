extern unsigned char *D_8009B7D0;
int func_8008594C(int a0) {
    int u = a0 & 0xFFFF;
    if (u >= 3) return 0;
    *(unsigned short *)((unsigned char *)D_8009B7D0 + u * 16) = 0;
    return 1;
}
